/*
 *  This file is part of Cursed Earth.
 *
 *  Cursed Earth is an open source, cross-platform port of Evil Islands.
 *  Copyright (C) 2009-2015 Yanis Kurganov <ykurganov@users.sourceforge.net>
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

#include <cstdio>
#include <cstring>
#include <cassert>

#include <unistd.h>
#include <alloca.h>

#include <alsa/asoundlib.h>

#include "lib.hpp"
#include "alloc.hpp"
#include "logging.hpp"
#include "soundsystem.hpp"

typedef struct {
    snd_pcm_t* handle;
} ce_alsa;

static void ce_alsa_error_handler(const char* file,
    int line, const char* function, int code, const char* format, ...)
{
    ce_logging_debug("alsa: error %d (%s:%d in %s)", code, file, line, function);
    va_list args;
    va_start(args, format);
    char buffer[strlen(format) + 16];
    snprintf(buffer, sizeof(buffer), "alsa: %s", format);
    ce_logging_debug_va(buffer, args);
    va_end(args);
}

static snd_pcm_format_t ce_alsa_choose_format()
{
    snd_pcm_format_t format = SND_PCM_FORMAT_UNKNOWN;
    switch (CE_SOUND_SYSTEM_BITS_PER_SAMPLE) {
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

static const char* ce_alsa_choose_device()
{
    const char* device = "default";
    switch (CE_SOUND_SYSTEM_CHANNEL_COUNT) {
    case 1:
    case 2:
        break;
    default:
        assert(false);
    }
    return device;
}

static int ce_alsa_set_params(void)
{
    ce_alsa* alsa = (ce_alsa*)ce_sound_system->impl;

    snd_pcm_hw_params_t* hwparams;
    snd_pcm_sw_params_t* swparams;

    snd_pcm_hw_params_alloca(&hwparams);
    snd_pcm_sw_params_alloca(&swparams);

    int code, dir;
    snd_pcm_uframes_t size;

    // choose all parameters
    code = snd_pcm_hw_params_any(alsa->handle, hwparams);
    if (code < 0) {
        ce_logging_error("alsa: broken configuration for playback; no configurations available");
        return code;
    }

    // set hardware resampling
    code = snd_pcm_hw_params_set_rate_resample(alsa->handle, hwparams, 1);
    if (code < 0) {
        ce_logging_error("alsa: resampling setup failed for playback");
        return code;
    }

    // set the interleaved read/write format
    code = snd_pcm_hw_params_set_access(alsa->handle, hwparams, SND_PCM_ACCESS_RW_INTERLEAVED);
    if (code < 0) {
        ce_logging_error("alsa: access type not available for playback");
        return code;
    }

    // set the sample format
    code = snd_pcm_hw_params_set_format(alsa->handle, hwparams, ce_alsa_choose_format());
    if (code < 0) {
        ce_logging_error("alsa: sample format not available for playback");
        return code;
    }

    // set the count of channels
    code = snd_pcm_hw_params_set_channels(alsa->handle, hwparams, CE_SOUND_SYSTEM_CHANNEL_COUNT);
    if (code < 0) {
        ce_logging_error("alsa: channels count (%u) not available for playbacks", CE_SOUND_SYSTEM_CHANNEL_COUNT);
        return code;
    }

    // set the stream rate
    code = snd_pcm_hw_params_set_rate_near(alsa->handle, hwparams, &ce_sound_system->samples_per_second, &dir);
    if (code < 0) {
        ce_logging_error("alsa: sample rate %u Hz not available for playback", CE_SOUND_SYSTEM_SAMPLES_PER_SECOND);
        return code;
    }

    // ring buffer length in us
    unsigned int buffer_time = 500000;

    // set the buffer time
    code = snd_pcm_hw_params_set_buffer_time_near(alsa->handle, hwparams, &buffer_time, &dir);
    if (code < 0) {
        ce_logging_error("alsa: unable to set buffer time %u for playback", buffer_time);
        return code;
    }

    // period time in us
    unsigned int period_time = 100000;

    // set the period time
    code = snd_pcm_hw_params_set_period_time_near(alsa->handle, hwparams, &period_time, &dir);
    if (code < 0) {
        ce_logging_error("alsa: unable to set period time %u for playback", period_time);
        return code;
    }

    code = snd_pcm_hw_params_get_buffer_size(hwparams, &size);
    if (code < 0) {
        ce_logging_error("alsa: unable to get buffer size for playback");
        return code;
    }

    snd_pcm_sframes_t buffer_size = size;

    code = snd_pcm_hw_params_get_period_size(hwparams, &size, &dir);
    if (code < 0) {
        ce_logging_error("alsa: unable to get period size for playback");
        return code;
    }

    snd_pcm_sframes_t period_size = size;

    // write the parameters to device
    code = snd_pcm_hw_params(alsa->handle, hwparams);
    if (code < 0) {
        ce_logging_error("alsa: unable to set hw params for playback");
        return code;
    }

    // get the current swparams
    code = snd_pcm_sw_params_current(alsa->handle, swparams);
    if (code < 0) {
        ce_logging_error("alsa: unable to determine current swparams for playback");
        return code;
    }

    // start the transfer when the buffer is almost full:
    // (buffer_size / avail_min) * avail_min
    code = snd_pcm_sw_params_set_start_threshold(alsa->handle, swparams,
                                                (buffer_size / period_size) * period_size);
    if (code < 0) {
        ce_logging_error("alsa: unable to set start threshold mode for playback");
        return code;
    }

    // allow the transfer when at least period_size samples can be processed
    code = snd_pcm_sw_params_set_avail_min(alsa->handle, swparams, period_size);
    if (code < 0) {
        ce_logging_error("alsa: unable to set avail min for playback");
        return code;
    }

    // write the parameters to the playback device
    code = snd_pcm_sw_params(alsa->handle, swparams);
    if (code < 0) {
        ce_logging_error("alsa: unable to set sw params for playback");
        return code;
    }

    return 0;
}

static bool ce_alsa_ctor(void)
{
    ce_alsa* alsa = (ce_alsa*)ce_sound_system->impl;

    ce_logging_write("sound system: using ALSA (Advanced Linux Sound Architecture)");

    snd_lib_error_set_handler(ce_alsa_error_handler);

    const char* device = ce_alsa_choose_device();
    int code;

    code = snd_pcm_open(&alsa->handle, device, SND_PCM_STREAM_PLAYBACK, 0);
    if (code < 0) {
        ce_logging_error("alsa: playback open error: %s", snd_strerror(code));
        return false;
    }

    code = ce_alsa_set_params();
    if (code < 0) {
        ce_logging_error("alsa: setting of hw/sw params failed: %s", snd_strerror(code));
        return false;
    }

#ifndef NDEBUG
    snd_output_t* output;
    code = snd_output_stdio_attach(&output, stderr, 0);
    if (code >= 0) {
        //snd_pcm_dump(alsa->handle, output);
        snd_output_close(output);
    }
#endif

    return true;
}

static void ce_alsa_dtor(void)
{
    ce_alsa* alsa = (ce_alsa*)ce_sound_system->impl;

    if (NULL != alsa->handle) {
        snd_pcm_drain(alsa->handle);
        snd_pcm_close(alsa->handle);
    }
}

static int ce_alsa_recovery(int code)
{
    ce_alsa* alsa = (ce_alsa*)ce_sound_system->impl;

    // no data transferred or interrupt signal
    if (-EAGAIN == code || -EINTR == code) {
        return 0;
    }

    // under-run
    if (-EPIPE == code) {
        code = snd_pcm_prepare(alsa->handle);
        if (code < 0) {
            ce_logging_error("alsa: can't recovery from underrun, prepare failed");
            return code;
        }
    }

    if (-ESTRPIPE == code) {
        while (-EAGAIN == (code = snd_pcm_resume(alsa->handle))) {
            sleep(1); // wait until the suspend flag is released
        }

        if (code < 0) {
            code = snd_pcm_prepare(alsa->handle);
            if (code < 0) {
                ce_logging_error("alsa: can't recovery from suspend, prepare failed");
                return code;
            }
        }

        return 0;
    }

    return code;
}

static bool ce_alsa_write(const void* block)
{
    ce_alsa* alsa = (ce_alsa*)ce_sound_system->impl;
    const char* data = block;
    int code;

    for (size_t sample_count = CE_SOUND_SYSTEM_SAMPLES_IN_BLOCK; sample_count > 0; ) {
        code = snd_pcm_writei(alsa->handle, data, sample_count);
        if (code < 0) {
            code = ce_alsa_recovery(code);
            if (code < 0) {
                ce_logging_error("alsa: %s", snd_strerror(code));
                return false;
            }
        } else {
            data += code * CE_SOUND_SYSTEM_SAMPLE_SIZE;
            sample_count -= code;
        }
    }

    return true;
}

ce_sound_system_vtable ce_sound_system_platform(void)
{
    return (ce_sound_system_vtable){
        sizeof(ce_alsa), ce_alsa_ctor, ce_alsa_dtor, ce_alsa_write
    };
}
