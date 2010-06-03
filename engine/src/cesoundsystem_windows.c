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

#include <windows.h>
#include <mmsystem.h>
#include <mmreg.h>

#include "celib.h"
#include "cealloc.h"
#include "celogging.h"
#include "cesoundsystem.h"

#include "ceerror_win32.h"

enum {
	CE_SOUNDSYSTEM_HEADER_COUNT = 8,
};

typedef struct {
	WAVEHDR waveheader;
	char data[];
} ce_soundsystem_header;

typedef struct {
	HANDLE event;
	WAVEFORMATEXTENSIBLE waveformat;
	HWAVEOUT waveout;
	ce_vector* headers;
} ce_soundsystem_wmm;

static void ce_soundsystem_wmm_error(MMRESULT code, const char* message)
{
	char buffer[MAXERRORLENGTH];
	if (MMSYSERR_NOERROR == waveOutGetErrorText(code, buffer, sizeof(buffer))) {
		ce_logging_error("soundsystem: %s", buffer);
	}
	if (NULL != message) {
		ce_logging_error("soundsystem: %s", message);
	}
}

static void CALLBACK ce_soundsystem_wmm_proc(HWAVEOUT waveout, UINT message,
	DWORD_PTR instance, DWORD_PTR param1, DWORD_PTR param2)
{
	ce_unused(waveout), ce_unused(param1), ce_unused(param2);
	if (WOM_DONE == message) {
		ce_soundsystem_wmm* wmmsystem = (ce_soundsystem_wmm*)instance;
		SetEvent(wmmsystem->event);
	}
}

static bool ce_soundsystem_wmm_ctor(ce_soundsystem* soundsystem, va_list args)
{
	ce_unused(args);
	ce_soundsystem_wmm* wmmsystem = (ce_soundsystem_wmm*)soundsystem->impl;
	MMRESULT code = MMSYSERR_NOERROR;

	ce_logging_write("soundsystem: using Windows Waveform-Audio Interface");

	wmmsystem->headers = ce_vector_new_reserved(CE_SOUNDSYSTEM_HEADER_COUNT);
	ce_vector_resize(wmmsystem->headers, CE_SOUNDSYSTEM_HEADER_COUNT);

	for (int i = 0; i < wmmsystem->headers->count; ++i) {
		wmmsystem->headers->items[i] = ce_alloc_zero(CE_SOUNDSYSTEM_BLOCK_SIZE +
												sizeof(ce_soundsystem_header));
		ce_soundsystem_header* header = wmmsystem->headers->items[i];
		header->waveheader.lpData = header->data;
		header->waveheader.dwBufferLength = CE_SOUNDSYSTEM_BLOCK_SIZE;
		header->waveheader.dwUser = (DWORD_PTR)header;
		header->waveheader.dwFlags = WHDR_DONE;
	}

	wmmsystem->event = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (NULL == wmmsystem->event) {
		ce_error_report_windows_last("soundsystem");
		return false;
	}

	assert(2 == CE_SOUNDSYSTEM_CHANNEL_COUNT && "only mono and stereo output implemented");

	wmmsystem->waveformat.Format.wFormatTag = WAVE_FORMAT_PCM;
	wmmsystem->waveformat.Format.nChannels = CE_SOUNDSYSTEM_CHANNEL_COUNT;
	wmmsystem->waveformat.Format.nSamplesPerSec = CE_SOUNDSYSTEM_SAMPLE_RATE;
	wmmsystem->waveformat.Format.wBitsPerSample  = CE_SOUNDSYSTEM_BITS_PER_SAMPLE;
	wmmsystem->waveformat.Format.nBlockAlign = CE_SOUNDSYSTEM_SAMPLE_SIZE;
	wmmsystem->waveformat.Format.nAvgBytesPerSec = CE_SOUNDSYSTEM_SAMPLE_RATE *
													CE_SOUNDSYSTEM_SAMPLE_SIZE;

	code = waveOutOpen(&wmmsystem->waveout, WAVE_MAPPER,
						&wmmsystem->waveformat.Format,
						(DWORD_PTR)ce_soundsystem_wmm_proc,
						(DWORD_PTR)wmmsystem,
						CALLBACK_FUNCTION | WAVE_ALLOWSYNC);
	if (MMSYSERR_NOERROR != code) {
		ce_soundsystem_wmm_error(code, "could not open waveform output device");
		return false;
	}

	return true;
}

static void ce_soundsystem_wmm_dtor(ce_soundsystem* soundsystem)
{
	ce_soundsystem_wmm* wmmsystem = (ce_soundsystem_wmm*)soundsystem->impl;
	MMRESULT code = MMSYSERR_NOERROR;

	if (NULL != wmmsystem->waveout) {
		code = waveOutReset(wmmsystem->waveout);
		if (MMSYSERR_NOERROR != code) {
			ce_soundsystem_wmm_error(code, "could not reset waveform output device");
		}

		for (int i = 0; i < wmmsystem->headers->count; ++i) {
			ce_soundsystem_header* header = wmmsystem->headers->items[i];
			if (header->waveheader.dwFlags & WHDR_PREPARED) {
				code = waveOutUnprepareHeader(wmmsystem->waveout,
												&header->waveheader,
													sizeof(WAVEHDR));
				if (MMSYSERR_NOERROR != code) {
					ce_soundsystem_wmm_error(code, "could not unprepare header");
				}
			}
		}

		code = waveOutClose(wmmsystem->waveout);
		if (MMSYSERR_NOERROR != code) {
			ce_soundsystem_wmm_error(code, "could not close waveform output device");
		}
	}

	if (NULL != wmmsystem->event) {
		CloseHandle(wmmsystem->event);
	}

	for (int i = 0; i < wmmsystem->headers->count; ++i) {
		ce_free(wmmsystem->headers->items[i], CE_SOUNDSYSTEM_BLOCK_SIZE +
												sizeof(ce_soundsystem_header));
	}

	ce_vector_del(wmmsystem->headers);
}

static void* ce_soundsystem_wmm_find(ce_soundsystem_wmm* wmmsystem)
{
	for (int i = 0; i < wmmsystem->headers->count; ++i) {
		ce_soundsystem_header* header = wmmsystem->headers->items[i];
		if (header->waveheader.dwFlags & WHDR_DONE) {
			return header;
		}
	}
	return NULL;
}

static bool ce_soundsystem_wmm_write(ce_soundsystem* soundsystem, const void* block)
{
	ce_soundsystem_wmm* wmmsystem = (ce_soundsystem_wmm*)soundsystem->impl;
	ce_soundsystem_header* header = NULL;
	MMRESULT code = MMSYSERR_NOERROR;

	ResetEvent(wmmsystem->event);

	while (NULL == (header = ce_soundsystem_wmm_find(wmmsystem))) {
		if (WAIT_OBJECT_0 != WaitForSingleObject(wmmsystem->event, INFINITE)) {
			ce_error_report_windows_last("soundsystem");
			return false;
		}
	}

	if (header->waveheader.dwFlags & WHDR_PREPARED) {
		code = waveOutUnprepareHeader(wmmsystem->waveout,
										&header->waveheader,
											sizeof(WAVEHDR));
	}

	if (MMSYSERR_NOERROR == code) {
		memcpy(header->data, block, CE_SOUNDSYSTEM_BLOCK_SIZE);
		code = waveOutPrepareHeader(wmmsystem->waveout,
										&header->waveheader,
											sizeof(WAVEHDR));
		if (MMSYSERR_NOERROR == code) {
			code = waveOutWrite(wmmsystem->waveout,
									&header->waveheader,
										sizeof(WAVEHDR));
			if (MMSYSERR_NOERROR == code) {
				ce_pass(); // unbelievable! :)
			} else {
				ce_soundsystem_wmm_error(code, "could not write header");
			}
		} else {
			ce_soundsystem_wmm_error(code, "could not prepare header");
		}
	} else {
		ce_soundsystem_wmm_error(code, "could not unprepare header");
	}

	return MMSYSERR_NOERROR == code;
}

ce_soundsystem* ce_soundsystem_new_platform(void)
{
	return ce_soundsystem_new((ce_soundsystem_vtable){sizeof(ce_soundsystem_wmm),
		ce_soundsystem_wmm_ctor, ce_soundsystem_wmm_dtor, ce_soundsystem_wmm_write});
}
