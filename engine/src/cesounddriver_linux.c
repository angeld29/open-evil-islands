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
#include <assert.h>

#include <unistd.h>
#include <alloca.h>

#include <alsa/asoundlib.h>

#include "celib.h"
#include "cealloc.h"
#include "celogging.h"
#include "cesounddriver.h"

typedef struct {
	snd_pcm_t* handle;
} ce_sounddriver_alsa;

static void ce_sounddriver_alsa_error_handler(const char* file, int line, const char* function, int code, const char* format, ...)
{
	// TODO: use logging
	va_list args;
	va_start(args, format);
	fprintf(stderr, "alsa_error_handler\n");
	vfprintf(stderr, format, args);
	fprintf(stderr, "\n");
	va_end(args);
}

static snd_pcm_format_t ce_sounddriver_alsa_choose_format(int bps)
{
	snd_pcm_format_t format = SND_PCM_FORMAT_UNKNOWN;
	switch (bps) {
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

static const char* ce_sounddriver_alsa_choose_device(int channels)
{
	const char* device = "default";
	switch (channels) {
	case 1:
	case 2:
		break;
	default:
		assert(false);
	}
	return device;
}

static int ce_sounddriver_alsa_set_params(ce_sounddriver* sounddriver)
{
	ce_sounddriver_alsa* alsadriver = (ce_sounddriver_alsa*)sounddriver->impl;

	snd_pcm_hw_params_t* hwparams;
	snd_pcm_sw_params_t* swparams;

	snd_pcm_hw_params_alloca(&hwparams);
	snd_pcm_sw_params_alloca(&swparams);

	int code, dir;
	snd_pcm_uframes_t size;

	// choose all parameters
	code = snd_pcm_hw_params_any(alsadriver->handle, hwparams);
	if (code < 0) {
		ce_logging_error("sounddriver: broken configuration for playback: no configurations available");
		return code;
	}

	// set hardware resampling
	code = snd_pcm_hw_params_set_rate_resample(alsadriver->handle, hwparams, 1);
	if (code < 0) {
		ce_logging_error("sounddriver: resampling setup failed for playback");
		return code;
	}

	// set the interleaved read/write format
	code = snd_pcm_hw_params_set_access(alsadriver->handle, hwparams, SND_PCM_ACCESS_RW_INTERLEAVED);
	if (code < 0) {
		ce_logging_error("sounddriver: access type not available for playback");
		return code;
	}

	// set the sample format
	code = snd_pcm_hw_params_set_format(alsadriver->handle, hwparams,
		ce_sounddriver_alsa_choose_format(sounddriver->bps));
	if (code < 0) {
		ce_logging_error("sounddriver: sample format not available for playback");
		return code;
	}

	// set the count of channels
	code = snd_pcm_hw_params_set_channels(alsadriver->handle, hwparams, sounddriver->channels);
	if (code < 0) {
		ce_logging_error("sounddriver: channels count (%d) not available for playbacks", sounddriver->channels);
		return code;
	}

	unsigned int rate = sounddriver->rate;

	// set the stream rate
	code = snd_pcm_hw_params_set_rate_near(alsadriver->handle, hwparams, &rate, &dir);
	if (code < 0) {
		ce_logging_error("sounddriver: rate %d Hz not available for playback", sounddriver->rate);
		return code;
	}

	if ((int)rate != sounddriver->rate) {
		ce_logging_warning("sounddriver: sample rate %d Hz not supported by the hardware, using %u Hz", sounddriver->rate, rate);
	}

	// ring buffer length in us
	unsigned int buffer_time = 500000;

	// set the buffer time
	code = snd_pcm_hw_params_set_buffer_time_near(alsadriver->handle, hwparams, &buffer_time, &dir);
	if (code < 0) {
		ce_logging_error("sounddriver: unable to set buffer time %u for playback", buffer_time);
		return code;
	}

	// period time in us
	unsigned int period_time = 100000;

	// set the period time
	code = snd_pcm_hw_params_set_period_time_near(alsadriver->handle, hwparams, &period_time, &dir);
	if (code < 0) {
		ce_logging_error("sounddriver: unable to set period time %u for playback", period_time);
		return code;
	}

	code = snd_pcm_hw_params_get_buffer_size(hwparams, &size);
	if (code < 0) {
		ce_logging_error("sounddriver: unable to get buffer size for playback");
		return code;
	}

	snd_pcm_sframes_t buffer_size = size;

	code = snd_pcm_hw_params_get_period_size(hwparams, &size, &dir);
	if (code < 0) {
		ce_logging_error("sounddriver: unable to get period size for playback");
		return code;
	}

	snd_pcm_sframes_t period_size = size;

	// write the parameters to device
	code = snd_pcm_hw_params(alsadriver->handle, hwparams);
	if (code < 0) {
		ce_logging_error("sounddriver: unable to set hw params for playback");
		return code;
	}

	// get the current swparams
	code = snd_pcm_sw_params_current(alsadriver->handle, swparams);
	if (code < 0) {
		ce_logging_error("sounddriver: unable to determine current swparams for playback");
		return code;
	}

	// start the transfer when the buffer is almost full:
	// (buffer_size / avail_min) * avail_min
	code = snd_pcm_sw_params_set_start_threshold(alsadriver->handle, swparams,
												(buffer_size / period_size) * period_size);
	if (code < 0) {
		ce_logging_error("sounddriver: unable to set start threshold mode for playback");
		return code;
	}

	// allow the transfer when at least period_size samples can be processed
	code = snd_pcm_sw_params_set_avail_min(alsadriver->handle, swparams, period_size);
	if (code < 0) {
		ce_logging_error("sounddriver: unable to set avail min for playback");
		return code;
	}

	// write the parameters to the playback device
	code = snd_pcm_sw_params(alsadriver->handle, swparams);
	if (code < 0) {
		ce_logging_error("sounddriver: unable to set sw params for playback");
		return code;
	}

	return 0;
}

static bool ce_sounddriver_alsa_ctor(ce_sounddriver* sounddriver, va_list args)
{
	ce_unused(args);
	ce_sounddriver_alsa* alsadriver = (ce_sounddriver_alsa*)sounddriver->impl;

	// TODO: move in soundsystem
	snd_lib_error_set_handler(ce_sounddriver_alsa_error_handler);

	// TODO: move in soundsystem
	ce_logging_write("sounddriver: using alsa (Advanced Linux Sound Architecture)");

	const char* device = ce_sounddriver_alsa_choose_device(sounddriver->channels);
	int code;

	code = snd_pcm_open(&alsadriver->handle, device, SND_PCM_STREAM_PLAYBACK, 0);
	if (code < 0) {
		ce_logging_error("sounddriver: playback open error: %s", snd_strerror(code));
		return false;
	}

	code = ce_sounddriver_alsa_set_params(sounddriver);
	if (code < 0) {
		ce_logging_error("sounddriver: setting of hw/sw params failed: %s", snd_strerror(code));
		return false;
	}

#ifndef NDEBUG
	snd_output_t* output;
	code = snd_output_stdio_attach(&output, stderr, 0);
	if (code >= 0) {
		//snd_pcm_dump(alsadriver->handle, output);
		snd_output_close(output);
	}
#endif

	return true;
}

static void ce_sounddriver_alsa_dtor(ce_sounddriver* sounddriver)
{
	ce_sounddriver_alsa* alsadriver = (ce_sounddriver_alsa*)sounddriver->impl;
	if (NULL != alsadriver->handle) {
		snd_pcm_drain(alsadriver->handle);
		snd_pcm_close(alsadriver->handle);
	}
}

static int ce_sounddriver_alsa_recovery(ce_sounddriver_alsa* alsadriver, int code)
{
	// no data transferred or interrupt signal
	if (-EAGAIN == code || -EINTR == code) {
		return 0;
	}

	// under-run
	if (-EPIPE == code) {
		code = snd_pcm_prepare(alsadriver->handle);
		if (code < 0) {
			ce_logging_error("sounddriver: can't recovery from underrun, prepare failed");
			return code;
		}
	}

	if (-ESTRPIPE == code) {
		while (-EAGAIN == (code = snd_pcm_resume(alsadriver->handle))) {
			sleep(1); // wait until the suspend flag is released
		}

		if (code < 0) {
			code = snd_pcm_prepare(alsadriver->handle);
			if (code < 0) {
				ce_logging_error("sounddriver: can't recovery from suspend, prepare failed");
				return code;
			}
		}

		return 0;
	}

	return code;
}

static void ce_sounddriver_alsa_write(ce_sounddriver* sounddriver, const void* block)
{
	ce_sounddriver_alsa* alsadriver = (ce_sounddriver_alsa*)sounddriver->impl;

	size_t sample_count = sounddriver->sample_count;
	const char* data = block;
	int code;

	while (sample_count > 0) {
		code = snd_pcm_writei(alsadriver->handle, data, sample_count);
		if (code < 0) {
			code = ce_sounddriver_alsa_recovery(alsadriver, code);
			if (code < 0) {
				ce_logging_error("sounddriver: write error: %s", snd_strerror(code));
				break;
			}
		} else {
			data += code * sounddriver->sample_size;
			sample_count -= code;
		}
	}
}

ce_sounddriver* ce_sounddriver_create_platform(int bps, int rate, int channels)
{
	return ce_sounddriver_new((ce_sounddriver_vtable){sizeof(ce_sounddriver_alsa),
		ce_sounddriver_alsa_ctor, ce_sounddriver_alsa_dtor, ce_sounddriver_alsa_write},
		bps, rate, channels);
}
