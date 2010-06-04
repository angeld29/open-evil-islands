/*
 *  This file is part of Cursed Earth.
 *
 *  Cursed Earth is an open source, cross-platform port of Evil Islands.
 *  Copyright (C) 2009-2010 Yanis Kurganov.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <string.h>
#include <assert.h>

#include <unistd.h>
#include <alloca.h>

#include <alsa/asoundlib.h>

#include "celib.h"
#include "cealloc.h"
#include "celogging.h"
#include "cesoundsystem.h"

typedef struct {
	snd_pcm_t* handle;
} ce_soundsystem_alsa;

static void ce_soundsystem_alsa_error_handler(const char* file,
	int line, const char* function, int code, const char* format, ...)
{
	ce_logging_debug("soundsystem: alsa: error %d (%s:%d in %s)",
									code, file, line, function);
	va_list args;
	va_start(args, format);
	char buffer[strlen(format) + 32];
	snprintf(buffer, sizeof(buffer), "soundsystem: alsa: %s", format);
	ce_logging_error_va(buffer, args);
	va_end(args);
}

static snd_pcm_format_t ce_soundsystem_alsa_choose_format()
{
	snd_pcm_format_t format = SND_PCM_FORMAT_UNKNOWN;
	switch (CE_SOUNDSYSTEM_BITS_PER_SAMPLE) {
	case 8:
		format = SND_PCM_FORMAT_S8;
		break;
	case 16:
		format = SND_PCM_FORMAT_S16;
		break;
	case 24:
		format = SND_PCM_FORMAT_S24;
		break;
	case 32:
		format = SND_PCM_FORMAT_S32;
		break;
	default:
		assert(false);
	}
	return format;
}

static const char* ce_soundsystem_alsa_choose_device()
{
	const char* device = "default";
	switch (CE_SOUNDSYSTEM_CHANNEL_COUNT) {
	case 1:
	case 2:
		break;
	default:
		assert(false);
	}
	return device;
}

static int ce_soundsystem_alsa_set_params(ce_soundsystem* soundsystem)
{
	ce_soundsystem_alsa* alsasystem = (ce_soundsystem_alsa*)soundsystem->impl;

	snd_pcm_hw_params_t* hwparams;
	snd_pcm_sw_params_t* swparams;

	snd_pcm_hw_params_alloca(&hwparams);
	snd_pcm_sw_params_alloca(&swparams);

	int code, dir;
	snd_pcm_uframes_t size;

	// choose all parameters
	code = snd_pcm_hw_params_any(alsasystem->handle, hwparams);
	if (code < 0) {
		ce_logging_error("soundsystem: broken configuration for playback: no configurations available");
		return code;
	}

	// set hardware resampling
	code = snd_pcm_hw_params_set_rate_resample(alsasystem->handle, hwparams, 1);
	if (code < 0) {
		ce_logging_error("soundsystem: resampling setup failed for playback");
		return code;
	}

	// set the interleaved read/write format
	code = snd_pcm_hw_params_set_access(alsasystem->handle, hwparams, SND_PCM_ACCESS_RW_INTERLEAVED);
	if (code < 0) {
		ce_logging_error("soundsystem: access type not available for playback");
		return code;
	}

	// set the sample format
	code = snd_pcm_hw_params_set_format(alsasystem->handle, hwparams, ce_soundsystem_alsa_choose_format());
	if (code < 0) {
		ce_logging_error("soundsystem: sample format not available for playback");
		return code;
	}

	// set the count of channels
	code = snd_pcm_hw_params_set_channels(alsasystem->handle, hwparams, CE_SOUNDSYSTEM_CHANNEL_COUNT);
	if (code < 0) {
		ce_logging_error("soundsystem: channels count (%u) not available for playbacks", CE_SOUNDSYSTEM_CHANNEL_COUNT);
		return code;
	}

	// set the stream rate
	code = snd_pcm_hw_params_set_rate_near(alsasystem->handle, hwparams, &soundsystem->sample_rate, &dir);
	if (code < 0) {
		ce_logging_error("soundsystem: sample rate %u Hz not available for playback", CE_SOUNDSYSTEM_SAMPLE_RATE);
		return code;
	}

	// ring buffer length in us
	unsigned int buffer_time = 500000;

	// set the buffer time
	code = snd_pcm_hw_params_set_buffer_time_near(alsasystem->handle, hwparams, &buffer_time, &dir);
	if (code < 0) {
		ce_logging_error("soundsystem: unable to set buffer time %u for playback", buffer_time);
		return code;
	}

	// period time in us
	unsigned int period_time = 100000;

	// set the period time
	code = snd_pcm_hw_params_set_period_time_near(alsasystem->handle, hwparams, &period_time, &dir);
	if (code < 0) {
		ce_logging_error("soundsystem: unable to set period time %u for playback", period_time);
		return code;
	}

	code = snd_pcm_hw_params_get_buffer_size(hwparams, &size);
	if (code < 0) {
		ce_logging_error("soundsystem: unable to get buffer size for playback");
		return code;
	}

	snd_pcm_sframes_t buffer_size = size;

	code = snd_pcm_hw_params_get_period_size(hwparams, &size, &dir);
	if (code < 0) {
		ce_logging_error("soundsystem: unable to get period size for playback");
		return code;
	}

	snd_pcm_sframes_t period_size = size;

	// write the parameters to device
	code = snd_pcm_hw_params(alsasystem->handle, hwparams);
	if (code < 0) {
		ce_logging_error("soundsystem: unable to set hw params for playback");
		return code;
	}

	// get the current swparams
	code = snd_pcm_sw_params_current(alsasystem->handle, swparams);
	if (code < 0) {
		ce_logging_error("soundsystem: unable to determine current swparams for playback");
		return code;
	}

	// start the transfer when the buffer is almost full:
	// (buffer_size / avail_min) * avail_min
	code = snd_pcm_sw_params_set_start_threshold(alsasystem->handle, swparams,
												(buffer_size / period_size) * period_size);
	if (code < 0) {
		ce_logging_error("soundsystem: unable to set start threshold mode for playback");
		return code;
	}

	// allow the transfer when at least period_size samples can be processed
	code = snd_pcm_sw_params_set_avail_min(alsasystem->handle, swparams, period_size);
	if (code < 0) {
		ce_logging_error("soundsystem: unable to set avail min for playback");
		return code;
	}

	// write the parameters to the playback device
	code = snd_pcm_sw_params(alsasystem->handle, swparams);
	if (code < 0) {
		ce_logging_error("soundsystem: unable to set sw params for playback");
		return code;
	}

	return 0;
}

static bool ce_soundsystem_alsa_ctor(ce_soundsystem* soundsystem)
{
	ce_soundsystem_alsa* alsasystem = (ce_soundsystem_alsa*)soundsystem->impl;

	ce_logging_write("soundsystem: using alsa (Advanced Linux Sound Architecture)");

	snd_lib_error_set_handler(ce_soundsystem_alsa_error_handler);

	const char* device = ce_soundsystem_alsa_choose_device();
	int code;

	code = snd_pcm_open(&alsasystem->handle, device, SND_PCM_STREAM_PLAYBACK, 0);
	if (code < 0) {
		ce_logging_error("soundsystem: playback open error: %s", snd_strerror(code));
		return false;
	}

	code = ce_soundsystem_alsa_set_params(soundsystem);
	if (code < 0) {
		ce_logging_error("soundsystem: setting of hw/sw params failed: %s", snd_strerror(code));
		return false;
	}

#ifndef NDEBUG
	snd_output_t* output;
	code = snd_output_stdio_attach(&output, stderr, 0);
	if (code >= 0) {
		//snd_pcm_dump(alsasystem->handle, output);
		snd_output_close(output);
	}
#endif

	return true;
}

static void ce_soundsystem_alsa_dtor(ce_soundsystem* soundsystem)
{
	ce_soundsystem_alsa* alsasystem = (ce_soundsystem_alsa*)soundsystem->impl;

	if (NULL != alsasystem->handle) {
		snd_pcm_drain(alsasystem->handle);
		snd_pcm_close(alsasystem->handle);
	}
}

static int ce_soundsystem_alsa_recovery(ce_soundsystem_alsa* alsasystem, int code)
{
	// no data transferred or interrupt signal
	if (-EAGAIN == code || -EINTR == code) {
		return 0;
	}

	// under-run
	if (-EPIPE == code) {
		code = snd_pcm_prepare(alsasystem->handle);
		if (code < 0) {
			ce_logging_error("soundsystem: can't recovery from underrun, prepare failed");
			return code;
		}
	}

	if (-ESTRPIPE == code) {
		while (-EAGAIN == (code = snd_pcm_resume(alsasystem->handle))) {
			sleep(1); // wait until the suspend flag is released
		}

		if (code < 0) {
			code = snd_pcm_prepare(alsasystem->handle);
			if (code < 0) {
				ce_logging_error("soundsystem: can't recovery from suspend, prepare failed");
				return code;
			}
		}

		return 0;
	}

	return code;
}

static bool ce_soundsystem_alsa_write(ce_soundsystem* soundsystem, const void* block)
{
	ce_soundsystem_alsa* alsasystem = (ce_soundsystem_alsa*)soundsystem->impl;
	const char* data = block;
	int code;

	for (size_t sample_count = CE_SOUNDSYSTEM_SAMPLES_IN_BLOCK; sample_count > 0; ) {
		code = snd_pcm_writei(alsasystem->handle, data, sample_count);
		if (code < 0) {
			code = ce_soundsystem_alsa_recovery(alsasystem, code);
			if (code < 0) {
				ce_logging_error("soundsystem: %s", snd_strerror(code));
				return false;
			}
		} else {
			data += code * CE_SOUNDSYSTEM_SAMPLE_SIZE;
			sample_count -= code;
		}
	}

	return true;
}

ce_soundsystem* ce_soundsystem_new_platform(void)
{
	return ce_soundsystem_new((ce_soundsystem_vtable){sizeof(ce_soundsystem_alsa),
		ce_soundsystem_alsa_ctor, ce_soundsystem_alsa_dtor, ce_soundsystem_alsa_write});
}
