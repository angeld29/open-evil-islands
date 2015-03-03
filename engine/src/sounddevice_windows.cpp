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
#include "logging.hpp"
#include "exception.hpp"

#include <algorithm>

#include <windows.h>
#include <mmsystem.h>
#include <mmreg.h>

namespace cursedearth
{
    class wmm_device_t final: public sound_device_t
    {
        static void error_handler(MMRESULT code)
        {
            char buffer[MAXERRORLENGTH];
            if (MMSYSERR_NOERROR == waveOutGetErrorText(code, buffer, sizeof(buffer))) {
                ce_logging_error("wmm: %s", buffer);
            }
        }

        struct header_t
        {
            WAVEHDR waveheader;
            uint8_t data[sound_options_t::max_block_size];
        };

        static const size_t s_header_count = 8;

        struct handle_dtor_t
        {
            void operator ()(PHANDLE handle)
            {
                CloseHandle(*handle);
            }
        };

        struct hwaveout_dtor_t
        {
            void operator ()(LPHWAVEOUT hwo)
            {
                MMRESULT code = waveOutClose(*hwo);
                if (MMSYSERR_NOERROR != code) {
                    error_handler(code);
                }
            }
        };

        WAVEFORMATEXTENSIBLE m_waveformat;
        header_t m_headers[s_header_count];
        HANDLE m_event;
        HWAVEOUT m_waveout;
        std::unique_ptr<HANDLE, handle_dtor_t> m_event_holder;
        std::unique_ptr<HWAVEOUT, hwaveout_dtor_t> m_waveout_holder;

        static void CALLBACK callback(HWAVEOUT, UINT message, DWORD_PTR instance, DWORD_PTR, DWORD_PTR)
        {
            if (WOM_DONE == message) {
                wmm_device_t* device = reinterpret_cast<wmm_device_t*>(instance);
                SetEvent(device->m_event);
            }
        }

        header_t* find()
        {
            for (size_t i = 0; i < s_header_count; ++i) {
                if (m_headers[i].waveheader.dwFlags & WHDR_DONE) {
                    return &m_headers[i];
                }
            }
            return NULL;
        }

        virtual void write(const sound_block_ptr_t& block)
        {
            ResetEvent(m_event);

            header_t* header = NULL;
            while (NULL == (header = find())) {
                if (WAIT_OBJECT_0 != WaitForSingleObject(m_event, INFINITE)) {
                    throw game_error("wmm", "WaitForSingleObject failed");
                }
            }

            MMRESULT code = MMSYSERR_NOERROR;
            if (header->waveheader.dwFlags & WHDR_PREPARED) {
                code = waveOutUnprepareHeader(m_waveout, &header->waveheader, sizeof(WAVEHDR));
            }

            if (MMSYSERR_NOERROR == code) {
                auto data = block->read_raw();
                std::copy_n(data.first, data.second, header->data);
                header->waveheader.dwBufferLength = data.second;
                code = waveOutPrepareHeader(m_waveout, &header->waveheader, sizeof(WAVEHDR));
                if (MMSYSERR_NOERROR == code) {
                    code = waveOutWrite(m_waveout, &header->waveheader, sizeof(WAVEHDR));
                    if (MMSYSERR_NOERROR == code) {
                        // unbelievable! :)
                    } else {
                        error_handler(code);
                        throw game_error("wmm", "waveOutWrite failed");
                    }
                } else {
                    error_handler(code);
                    throw game_error("wmm", "waveOutPrepareHeader failed");
                }
            } else {
                error_handler(code);
                throw game_error("wmm", "waveOutUnprepareHeader failed");
            }
        }

    public:
        explicit wmm_device_t(const sound_format_t& format):
            sound_device_t(format)
        {
            ce_logging_info("sound system: using Windows Waveform-Audio Interface");
            assert(format.channel_count <= 2 && "only mono and stereo output implemented");

            m_waveformat.Format.wFormatTag = WAVE_FORMAT_PCM;
            m_waveformat.Format.nChannels = format.channel_count;
            m_waveformat.Format.nSamplesPerSec = format.samples_per_second;
            m_waveformat.Format.wBitsPerSample  = format.bits_per_sample;
            m_waveformat.Format.nBlockAlign = format.sample_size;
            m_waveformat.Format.nAvgBytesPerSec = format.bytes_per_second;

            for (size_t i = 0; i < s_header_count; ++i) {
                m_headers[i].waveheader.lpData = reinterpret_cast<LPSTR>(m_headers[i].data);
                m_headers[i].waveheader.dwUser = reinterpret_cast<DWORD_PTR>(&m_headers[i]);
                m_headers[i].waveheader.dwFlags = WHDR_DONE;
            }

            m_event = CreateEvent(NULL, TRUE, FALSE, NULL);
            if (!m_event) {
                throw game_error("wmm", "CreateEvent failed");
            }
            m_event_holder.reset(&m_event);

            MMRESULT code = waveOutOpen(&m_waveout, WAVE_MAPPER, &m_waveformat.Format,
                reinterpret_cast<DWORD_PTR>(callback), reinterpret_cast<DWORD_PTR>(this), CALLBACK_FUNCTION | WAVE_ALLOWSYNC);
            if (MMSYSERR_NOERROR != code) {
                error_handler(code);
                throw game_error("wmm", "waveOutOpen failed");
            }
            m_waveout_holder.reset(&m_waveout);
        }

        virtual ~wmm_device_t()
        {
            MMRESULT code = waveOutReset(m_waveout);
            if (MMSYSERR_NOERROR != code) {
                error_handler(code);
            }

            for (size_t i = 0; i < s_header_count; ++i) {
                if (m_headers[i].waveheader.dwFlags & WHDR_PREPARED) {
                    code = waveOutUnprepareHeader(m_waveout, &m_headers[i].waveheader, sizeof(WAVEHDR));
                    if (MMSYSERR_NOERROR != code) {
                        error_handler(code);
                    }
                }
            }
        }
    };

    sound_device_ptr_t make_sound_device(const sound_format_t& format)
    {
        return std::make_shared<wmm_device_t>(format);
    }
}
