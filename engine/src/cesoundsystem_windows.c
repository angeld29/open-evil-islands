/*
 *  This file is part of Cursed Earth
 *
 *  Cursed Earth is an open source, cross-platform port of Evil Islands
 *  Copyright (C) 2009-2010 Yanis Kurganov
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

#include <windows.h>
#include <mmsystem.h>
#include <mmreg.h>

#include "celib.h"
#include "cealloc.h"
#include "celogging.h"
#include "ceerror_windows.h"
#include "cesoundsystem.h"

enum {
	CE_SOUND_SYSTEM_HEADER_COUNT = 8,
};

typedef struct {
	WAVEHDR waveheader;
	char data[CE_SOUND_SYSTEM_BLOCK_SIZE];
} ce_wmm_header;

typedef struct {
	HANDLE event;
	WAVEFORMATEXTENSIBLE waveformat;
	HWAVEOUT waveout;
	ce_wmm_header headers[CE_SOUND_SYSTEM_HEADER_COUNT];
} ce_wmm;

static void ce_wmm_error(MMRESULT code, const char* message)
{
	char buffer[MAXERRORLENGTH];
	if (MMSYSERR_NOERROR == waveOutGetErrorText(code, buffer, sizeof(buffer))) {
		ce_logging_error("wmm: %s", buffer);
	}
	if (NULL != message) {
		ce_logging_error("wmm: %s", message);
	}
}

static void CALLBACK ce_wmm_proc(HWAVEOUT waveout, UINT message,
	DWORD_PTR instance, DWORD_PTR param1, DWORD_PTR param2)
{
	ce_unused(waveout), ce_unused(param1), ce_unused(param2);
	if (WOM_DONE == message) {
		ce_wmm* wmm = (ce_wmm*)instance;
		SetEvent(wmm->event);
	}
}

static bool ce_wmm_ctor(ce_sound_system* sound_system)
{
	ce_wmm* wmm = (ce_wmm*)sound_system->impl;
	MMRESULT code = MMSYSERR_NOERROR;

	ce_logging_write("sound system: using Windows Waveform-Audio Interface");

	wmm->event = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (NULL == wmm->event) {
		ce_error_report_windows_last("wmm");
		return false;
	}

	assert(CE_SOUND_SYSTEM_CHANNEL_COUNT <= 2 && "only mono and stereo output implemented");

	wmm->waveformat.Format.wFormatTag = WAVE_FORMAT_PCM;
	wmm->waveformat.Format.nChannels = CE_SOUND_SYSTEM_CHANNEL_COUNT;
	wmm->waveformat.Format.nSamplesPerSec = CE_SOUND_SYSTEM_SAMPLE_RATE;
	wmm->waveformat.Format.wBitsPerSample  = CE_SOUND_SYSTEM_BITS_PER_SAMPLE;
	wmm->waveformat.Format.nBlockAlign = CE_SOUND_SYSTEM_SAMPLE_SIZE;
	wmm->waveformat.Format.nAvgBytesPerSec = CE_SOUND_SYSTEM_SAMPLE_RATE *
												CE_SOUND_SYSTEM_SAMPLE_SIZE;

	code = waveOutOpen(&wmm->waveout, WAVE_MAPPER,
						&wmm->waveformat.Format,
						(DWORD_PTR)ce_wmm_proc,
						(DWORD_PTR)wmm,
						CALLBACK_FUNCTION | WAVE_ALLOWSYNC);
	if (MMSYSERR_NOERROR != code) {
		ce_wmm_error(code, "could not open waveform output device");
		return false;
	}

	for (size_t i = 0; i < CE_SOUND_SYSTEM_HEADER_COUNT; ++i) {
		wmm->headers[i].waveheader.lpData = wmm->headers[i].data;
		wmm->headers[i].waveheader.dwBufferLength = CE_SOUND_SYSTEM_BLOCK_SIZE;
		wmm->headers[i].waveheader.dwUser = (DWORD_PTR)&wmm->headers[i];
		wmm->headers[i].waveheader.dwFlags = WHDR_DONE;
	}

	return true;
}

static void ce_wmm_dtor(ce_sound_system* sound_system)
{
	ce_wmm* wmm = (ce_wmm*)sound_system->impl;
	MMRESULT code = MMSYSERR_NOERROR;

	if (NULL != wmm->waveout) {
		code = waveOutReset(wmm->waveout);
		if (MMSYSERR_NOERROR != code) {
			ce_wmm_error(code, "could not reset waveform output device");
		}

		for (size_t i = 0; i < CE_SOUND_SYSTEM_HEADER_COUNT; ++i) {
			if (wmm->headers[i].waveheader.dwFlags & WHDR_PREPARED) {
				code = waveOutUnprepareHeader(wmm->waveout,
												&wmm->headers[i].waveheader,
													sizeof(WAVEHDR));
				if (MMSYSERR_NOERROR != code) {
					ce_wmm_error(code, "could not unprepare header");
				}
			}
		}

		code = waveOutClose(wmm->waveout);
		if (MMSYSERR_NOERROR != code) {
			ce_wmm_error(code, "could not close waveform output device");
		}
	}

	if (NULL != wmm->event) {
		CloseHandle(wmm->event);
	}
}

static ce_wmm_header* ce_wmm_find(ce_wmm* wmm)
{
	for (size_t i = 0; i < CE_SOUND_SYSTEM_HEADER_COUNT; ++i) {
		if (wmm->headers[i].waveheader.dwFlags & WHDR_DONE) {
			return &wmm->headers[i];
		}
	}
	return NULL;
}

static bool ce_wmm_write(ce_sound_system* sound_system, const void* block)
{
	ce_wmm* wmm = (ce_wmm*)sound_system->impl;
	ce_wmm_header* header = NULL;
	MMRESULT code = MMSYSERR_NOERROR;

	ResetEvent(wmm->event);

	while (NULL == (header = ce_wmm_find(wmm))) {
		if (WAIT_OBJECT_0 != WaitForSingleObject(wmm->event, INFINITE)) {
			ce_error_report_windows_last("wmm");
			return false;
		}
	}

	if (header->waveheader.dwFlags & WHDR_PREPARED) {
		code = waveOutUnprepareHeader(wmm->waveout,
										&header->waveheader,
											sizeof(WAVEHDR));
	}

	if (MMSYSERR_NOERROR == code) {
		memcpy(header->data, block, CE_SOUND_SYSTEM_BLOCK_SIZE);
		code = waveOutPrepareHeader(wmm->waveout,
										&header->waveheader,
											sizeof(WAVEHDR));
		if (MMSYSERR_NOERROR == code) {
			code = waveOutWrite(wmm->waveout,
									&header->waveheader,
										sizeof(WAVEHDR));
			if (MMSYSERR_NOERROR == code) {
				ce_pass(); // unbelievable! :)
			} else {
				ce_wmm_error(code, "could not write header");
			}
		} else {
			ce_wmm_error(code, "could not prepare header");
		}
	} else {
		ce_wmm_error(code, "could not unprepare header");
	}

	return MMSYSERR_NOERROR == code;
}

ce_sound_system* ce_sound_system_new_platform(void)
{
	return ce_sound_system_new((ce_sound_system_vtable){sizeof(ce_wmm),
		ce_wmm_ctor, ce_wmm_dtor, ce_wmm_write});
}
