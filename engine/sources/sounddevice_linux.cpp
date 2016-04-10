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

#include "sounddevice.hpp"
#include "exception.hpp"
#include "logging.hpp"

#include <alsa/asoundlib.h>

namespace cursedearth
{
    class alsa_device_t final: public sound_device_t
    {
        static void error_handler(const char* file, int line, const char* function, int code, const char* format, ...)
        {
            ce_logging_debug("alsa: error %d (%s:%d in %s)", code, file, line, function);
            va_list args;
            va_start(args, format);
            ce_logging_debug_va(format, args);
            va_end(args);
        }

        struct snd_pcm_dtor_t
        {
            void operator ()(snd_pcm_t* pcm) const
            {
                snd_pcm_drain(pcm);
                snd_pcm_close(pcm);
            }
        };

        std::unique_ptr<snd_pcm_t, snd_pcm_dtor_t> m_handle;

        void debug_dump()
        {
            snd_output_t* output;
            int code = snd_output_stdio_attach(&output, stderr, 0);
            if (code >= 0) {
                snd_pcm_dump(m_handle.get(), output);
                snd_output_close(output);
            }
        }

        const char* choose_device()
        {
            const char* device = "default";
            switch (static_cast<int>(m_format.channel_count)) {
            case 1:
            case 2:
                break;
            default:
                assert(false && "not implemented");
            }
            return device;
        }

        snd_pcm_format_t choose_format()
        {
            snd_pcm_format_t format = SND_PCM_FORMAT_UNKNOWN;
            switch (static_cast<int>(m_format.bits_per_sample)) {
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
                assert(false && "not implemented");
            }
            return format;
        }

        void ce_alsa_set_params()
        {
            snd_pcm_hw_params_t* hwparams;
            snd_pcm_sw_params_t* swparams;

            snd_pcm_hw_params_alloca(&hwparams);
            snd_pcm_sw_params_alloca(&swparams);

            int code, dir;
            snd_pcm_uframes_t size;

            // choose all parameters
            code = snd_pcm_hw_params_any(m_handle.get(), hwparams);
            if (code < 0) {
                throw game_error("alsa", "broken configuration for playback; no configurations available");
            }

            // set hardware resampling
            code = snd_pcm_hw_params_set_rate_resample(m_handle.get(), hwparams, 1);
            if (code < 0) {
                throw game_error("alsa", "resampling setup failed for playback");
            }

            // set the interleaved read/write format
            code = snd_pcm_hw_params_set_access(m_handle.get(), hwparams, SND_PCM_ACCESS_RW_INTERLEAVED);
            if (code < 0) {
                throw game_error("alsa", "access type not available for playback");
            }

            // set the sample format
            code = snd_pcm_hw_params_set_format(m_handle.get(), hwparams, choose_format());
            if (code < 0) {
                throw game_error("alsa", "sample format not available for playback");
            }

            // set the count of channels
            code = snd_pcm_hw_params_set_channels(m_handle.get(), hwparams, m_format.channel_count);
            if (code < 0) {
                throw game_error("alsa", "channels count (%1%) not available for playbacks", m_format.channel_count);
            }

            unsigned int samples_per_second = m_format.samples_per_second;

            // set the stream rate
            code = snd_pcm_hw_params_set_rate_near(m_handle.get(), hwparams, &samples_per_second, &dir);
            if (code < 0) {
                throw game_error("alsa", "sample rate %1% Hz not available for playback", m_format.samples_per_second);
            }

            if (m_format.samples_per_second != samples_per_second) {
                ce_logging_warning("alsa: sample rate %d Hz not supported by the implementation/hardware, using %u Hz", m_format.samples_per_second, samples_per_second);
            }

            // ring buffer length in us
            unsigned int buffer_time = 500000;

            // set the buffer time
            code = snd_pcm_hw_params_set_buffer_time_near(m_handle.get(), hwparams, &buffer_time, &dir);
            if (code < 0) {
                throw game_error("alsa", "unable to set buffer time %1% for playback", buffer_time);
            }

            // period time in us
            unsigned int period_time = 100000;

            // set the period time
            code = snd_pcm_hw_params_set_period_time_near(m_handle.get(), hwparams, &period_time, &dir);
            if (code < 0) {
                throw game_error("alsa", "unable to set period time %1% for playback", period_time);
            }

            code = snd_pcm_hw_params_get_buffer_size(hwparams, &size);
            if (code < 0) {
                throw game_error("alsa", "unable to get buffer size for playback");
            }

            snd_pcm_sframes_t buffer_size = size;

            code = snd_pcm_hw_params_get_period_size(hwparams, &size, &dir);
            if (code < 0) {
                throw game_error("alsa", "unable to get period size for playback");
            }

            snd_pcm_sframes_t period_size = size;

            // write the parameters to device
            code = snd_pcm_hw_params(m_handle.get(), hwparams);
            if (code < 0) {
                throw game_error("alsa", "unable to set hw params for playback");
            }

            // get the current swparams
            code = snd_pcm_sw_params_current(m_handle.get(), swparams);
            if (code < 0) {
                throw game_error("alsa", "unable to determine current swparams for playback");
            }

            // start the transfer when the buffer is almost full: (buffer_size / avail_min) * avail_min
            code = snd_pcm_sw_params_set_start_threshold(m_handle.get(), swparams, (buffer_size / period_size) * period_size);
            if (code < 0) {
                throw game_error("alsa", "unable to set start threshold mode for playback");
            }

            // allow the transfer when at least period_size samples can be processed
            code = snd_pcm_sw_params_set_avail_min(m_handle.get(), swparams, period_size);
            if (code < 0) {
                throw game_error("alsa", "unable to set avail min for playback");
            }

            // write the parameters to the playback device
            code = snd_pcm_sw_params(m_handle.get(), swparams);
            if (code < 0) {
                throw game_error("alsa", "unable to set sw params for playback");
            }
        }

        void recovery(int code)
        {
            // no data transferred or interrupt signal
            if (-EAGAIN == code || -EINTR == code) {
                return;
            }

            // under-run
            if (-EPIPE == code) {
                code = snd_pcm_prepare(m_handle.get());
                if (code < 0) {
                    throw game_error("alsa", "can't recovery from underrun, prepare failed: %1%", snd_strerror(code));
                }
            }

            if (-ESTRPIPE == code) {
                while (-EAGAIN == (code = snd_pcm_resume(m_handle.get()))) {
                    sleep(1); // wait until the suspend flag is released
                }

                if (code < 0) {
                    code = snd_pcm_prepare(m_handle.get());
                    if (code < 0) {
                        throw game_error("alsa", "can't recovery from suspend, prepare failed: %1%", snd_strerror(code));
                    }
                }
            }

            if (code < 0) {
                throw game_error("alsa", "can't recovery: %1%", snd_strerror(code));
            }
        }

        virtual void write(const sound_block_ptr_t& block) final
        {
            auto data = block->read_raw();
            for (size_t sample_count = data.second / block->format().sample_size; sample_count > 0; ) {
                int code = snd_pcm_writei(m_handle.get(), data.first, sample_count);
                if (code < 0) {
                    recovery(code);
                } else {
                    data.first += code * block->format().sample_size;
                    sample_count -= code;
                }
            }
        }

    public:
        explicit alsa_device_t(const sound_format_t& format):
            sound_device_t(format)
        {
            ce_logging_info("sound system: using ALSA (Advanced Linux Sound Architecture)");
            snd_lib_error_set_handler(error_handler);

            snd_pcm_t* pcm;
            const char* device = choose_device();

            int code = snd_pcm_open(&pcm, device, SND_PCM_STREAM_PLAYBACK, 0);
            if (code < 0) {
                throw game_error("alsa", "playback open error: %1%", snd_strerror(code));
            }

            m_handle.reset(pcm);
            ce_alsa_set_params();

#ifndef NDEBUG
            debug_dump();
#endif
        }
    };

    sound_device_ptr_t make_sound_device(const sound_format_t& format)
    {
        return std::make_shared<alsa_device_t>(format);
    }
}
