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
#include <stdbool.h>
#include <assert.h>

#include <alloca.h>
#include <poll.h>

#include <alsa/asoundlib.h>

#include "cealloc.h"
#include "celogging.h"
#include "cesounddriver.h"

static void ce_sounddriver_error_handler(const char* file, int line, const char* function, int code, const char* format, ...)
{
	// TODO: use logging
	va_list args;
	va_start(args, format);
	vfprintf(stderr, format, args);
	va_end(args);
}

struct ce_sounddriver {
	int bps, rate, channels;
	snd_pcm_t* handle;
	snd_pcm_channel_area_t* areas;
};

static snd_pcm_format_t ce_sounddriver_choose_format(int bps)
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

static const char* ce_sounddriver_choose_device(int channels)
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

static int ce_sounddriver_set_params(ce_sounddriver* sounddriver,
										snd_pcm_format_t format,
										snd_pcm_access_t access)
{
	snd_pcm_hw_params_t* hwparams;
	snd_pcm_sw_params_t* swparams;

	snd_pcm_hw_params_alloca(&hwparams);
	snd_pcm_sw_params_alloca(&swparams);

	int code, dir;

	// choose all parameters
	code = snd_pcm_hw_params_any(sounddriver->handle, hwparams);
	if (code < 0) {
		ce_logging_error("sounddriver: broken configuration for playback: no configurations available");
		return code;
	}

	// enable alsa-lib resampling
	int resample = 1;

	// set hardware resampling
	code = snd_pcm_hw_params_set_rate_resample(sounddriver->handle, hwparams, resample);
	if (code < 0) {
		ce_logging_error("sounddriver: resampling setup failed for playback");
		return code;
	}

	// set the interleaved read/write format
	code = snd_pcm_hw_params_set_access(sounddriver->handle, hwparams, access);
	if (code < 0) {
		ce_logging_error("sounddriver: access type not available for playback");
		return code;
	}

	// set the sample format
	code = snd_pcm_hw_params_set_format(sounddriver->handle, hwparams, format);
	if (code < 0) {
		ce_logging_error("sounddriver: sample format not available for playback");
		return code;
	}

	// set the count of channels
	code = snd_pcm_hw_params_set_channels(sounddriver->handle, hwparams, sounddriver->channels);
	if (code < 0) {
		ce_logging_error("sounddriver: channels count (%d) not available for playbacks", sounddriver->channels);
		return code;
	}

	unsigned int rate = sounddriver->rate;

	// set the stream rate
	code = snd_pcm_hw_params_set_rate_near(sounddriver->handle, hwparams, &rate, &dir);
	if (code < 0) {
		ce_logging_error("sounddriver: rate %d Hz not available for playback", sounddriver->rate);
		return code;
	}

	if ((int)rate != sounddriver->rate) {
		ce_logging_warning("sounddriver: rate doesn't match (requested %d Hz, get %u Hz)", sounddriver->rate, rate);
	}

	// ring buffer length in us
	unsigned int buffer_time = 500000;

	// set the buffer time
	code = snd_pcm_hw_params_set_buffer_time_near(sounddriver->handle, hwparams, &buffer_time, &dir);
	if (code < 0) {
		ce_logging_error("sounddriver: unable to set buffer time %u for playback", buffer_time);
		return code;
	}

	snd_pcm_uframes_t size;

	code = snd_pcm_hw_params_get_buffer_size(hwparams, &size);
	if (code < 0) {
		ce_logging_error("sounddriver: unable to get buffer size for playback");
		return code;
	}

	snd_pcm_sframes_t buffer_size = size;

	// period time in us
	unsigned int period_time = 100000;

	// set the period time
	code = snd_pcm_hw_params_set_period_time_near(sounddriver->handle, hwparams, &period_time, &dir);
	if (code < 0) {
		ce_logging_error("sounddriver: unable to set period time %u for playback", period_time);
		return code;
	}

	code = snd_pcm_hw_params_get_period_size(hwparams, &size, &dir);
	if (code < 0) {
		ce_logging_error("sounddriver: unable to get period size for playback");
		return code;
	}

	snd_pcm_sframes_t period_size = size;

	// write the parameters to device
	code = snd_pcm_hw_params(sounddriver->handle, hwparams);
	if (code < 0) {
		ce_logging_error("sounddriver: unable to set hw params for playback");
		return code;
	}

	// get the current swparams
	code = snd_pcm_sw_params_current(sounddriver->handle, swparams);
	if (code < 0) {
		ce_logging_error("sounddriver: unable to determine current swparams for playback");
		return code;
	}

	// start the transfer when the buffer is almost full:
	// (buffer_size / avail_min) * avail_min
	code = snd_pcm_sw_params_set_start_threshold(sounddriver->handle, swparams,
												(buffer_size / period_size) * period_size);
	if (code < 0) {
		ce_logging_error("sounddriver: unable to set start threshold mode for playback");
		return code;
	}

	int period_event = 0; // produce poll event after each period

	// allow the transfer when at least period_size samples can be processed
	// or disable this mechanism when period event is enabled (aka interrupt like style processing)
	code = snd_pcm_sw_params_set_avail_min(sounddriver->handle, swparams,
										period_event ? buffer_size : period_size);
	if (code < 0) {
		ce_logging_error("sounddriver: unable to set avail min for playback");
		return code;
	}

	// enable period events when requested
	if (period_event) {
		code = snd_pcm_sw_params_set_period_event(sounddriver->handle, swparams, 1);
		if (code < 0) {
			ce_logging_error("sounddriver: unable to set period event");
			return code;
		}
	}

	// write the parameters to the playback device
	code = snd_pcm_sw_params(sounddriver->handle, swparams);
	if (code < 0) {
		printf("sounddriver: unable to set sw params for playback");
		return code;
	}

	return 0;
}

ce_sounddriver* ce_sounddriver_new(int bps, int rate, int channels)
{
	// TODO: move in soundsystem
	snd_lib_error_set_handler(ce_sounddriver_error_handler);

	ce_sounddriver* sounddriver = ce_alloc_zero(sizeof(ce_sounddriver));

	sounddriver->bps = bps;
	sounddriver->rate = rate;
	sounddriver->channels = channels;

	snd_pcm_format_t format = ce_sounddriver_choose_format(bps);
	const char* device = ce_sounddriver_choose_device(channels);

	ce_logging_write("sounddriver: using alsa (Advanced Linux Sound Architecture)");
	ce_logging_write("sounddriver: playback device is '%s'", device);
	ce_logging_write("sounddriver: stream parameters are %s, %d Hz, %d channels",
		snd_pcm_format_name(format), rate, channels);

	int code;

	code = snd_pcm_open(&sounddriver->handle, device, SND_PCM_STREAM_PLAYBACK, SND_PCM_NONBLOCK);
	if (code < 0) {
		ce_logging_error("sounddriver: playback open error: %s", snd_strerror(code));
		ce_sounddriver_del(sounddriver);
		return NULL;
	}

	code = ce_sounddriver_set_params(sounddriver, format, SND_PCM_ACCESS_RW_INTERLEAVED);
	if (code < 0) {
		ce_logging_error("sounddriver: setting of hw/sw params failed: %s", snd_strerror(code));
		ce_sounddriver_del(sounddriver);
		return NULL;
	}

	sounddriver->areas = ce_alloc(channels * sizeof(snd_pcm_channel_area_t));

#ifndef NDEBUG
	snd_output_t* output;
	code = snd_output_stdio_attach(&output, stderr, 0);
	if (code >= 0) {
		snd_pcm_dump(sounddriver->handle, output);
		snd_output_close(output);
	}
#endif

	return sounddriver;
}

void ce_sounddriver_del(ce_sounddriver* sounddriver)
{
	if (NULL != sounddriver) {
		ce_free(sounddriver->areas, sounddriver->channels * sizeof(snd_pcm_channel_area_t));
		if (NULL != sounddriver->handle) {
			snd_pcm_drain(sounddriver->handle);
			snd_pcm_close(sounddriver->handle);
		}
		ce_free(sounddriver, sizeof(ce_sounddriver));
	}
}

static int ce_sounddriver_recovery(ce_sounddriver* sounddriver, int code)
{
	if (-EPIPE == code) { // under-run
		code = snd_pcm_prepare(sounddriver->handle);
		if (code < 0) {
			ce_logging_error("sounddriver: can't recovery from underrun, prepare failed: %s", snd_strerror(code));
			return code;
		}
	}

	if (-ESTRPIPE == code) {
		while (-EAGAIN == (code = snd_pcm_resume(sounddriver->handle))) {
			sleep(1); // wait until the suspend flag is released
		}

		if (code < 0) {
			code = snd_pcm_prepare(sounddriver->handle);
			if (code < 0) {
				ce_logging_error("sounddriver: can't recovery from suspend, prepare failed: %s", snd_strerror(code));
				return code;
			}
		}

		return 0;
	}

	return code;
}

static int ce_sounddriver_wait_for_poll(ce_sounddriver* sounddriver,
										struct pollfd* ufds, unsigned int count)
{
	unsigned short revents;

	for (;;) {
		poll(ufds, count, -1);
		snd_pcm_poll_descriptors_revents(sounddriver->handle, ufds, count, &revents);

		if (revents & POLLERR) {
			return -EIO;
		}

		if (revents & POLLOUT)
			return 0;
	}
}