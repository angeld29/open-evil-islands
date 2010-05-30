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
#include "cesounddriver.h"

typedef struct {
	WAVEHDR header;
	char buffer[4096];
} ce_sounddriver_wavehdr;

typedef struct {
	WAVEFORMATEXTENSIBLE format;
	HWAVEOUT handle;
	WAVEHDR headers[16];
} ce_sounddriver_wmm;

static void ce_sounddriver_wmm_error(MMRESULT code, const char* message)
{
	char buffer[MAXERRORLENGTH];
	if (MMSYSERR_NOERROR == waveOutGetErrorText(code, buffer, sizeof(buffer))) {
		ce_logging_error("sounddriver: %s", buffer);
	}
	ce_logging_error("sounddriver: %s", message);
}

static bool ce_sounddriver_wmm_ctor(ce_sounddriver* sounddriver, va_list args)
{
	ce_unused(args);
	ce_sounddriver_wmm* wmmdriver = (ce_sounddriver_wmm*)sounddriver->impl;

	// TODO: move in soundsystem
	ce_logging_write("sounddriver: using Windows Waveform-Audio Interface");

	if (sounddriver->channels > 2) {
		ce_logging_write("sounddriver: only mono and stereo input supported");
		return false;
	}

	wmmdriver->format.Format.wFormatTag = WAVE_FORMAT_PCM;
	wmmdriver->format.Format.nChannels = sounddriver->channels;
	wmmdriver->format.Format.nSamplesPerSec = sounddriver->rate;
	wmmdriver->format.Format.wBitsPerSample  = sounddriver->bps;
	wmmdriver->format.Format.nBlockAlign = (wmmdriver->format.Format.wBitsPerSample >> 3) *
												wmmdriver->format.Format.nChannels;
	wmmdriver->format.Format.nAvgBytesPerSec = wmmdriver->format.Format.nSamplesPerSec *
											wmmdriver->format.Format.nBlockAlign;

	MMRESULT code = waveOutOpen(&wmmdriver->handle, WAVE_MAPPER,
		&wmmdriver->format.Format, 0, 0, CALLBACK_NULL | WAVE_ALLOWSYNC);
	if (MMSYSERR_NOERROR != code) {
		ce_sounddriver_wmm_error(code, "could not open waveform output device");
		return false;
	}

	return true;
}

static void ce_sounddriver_wmm_dtor(ce_sounddriver* sounddriver)
{
	ce_sounddriver_wmm* wmmdriver = (ce_sounddriver_wmm*)sounddriver->impl;
	MMRESULT code;

	if (NULL != wmmdriver->handle) {
		// TODO: waveOutUnprepareHeader
		if (MMSYSERR_NOERROR != (code = waveOutReset(wmmdriver->handle))) {
			ce_sounddriver_wmm_error(code, "could not reset waveform output device");
		}
		if (MMSYSERR_NOERROR != (code = waveOutClose(wmmdriver->handle))) {
			ce_sounddriver_wmm_error(code, "could not close waveform output device");
		}
	}
}

static void ce_sounddriver_wmm_write(ce_sounddriver* sounddriver, const void* block)
{
	ce_sounddriver_wmm* wmmdriver = (ce_sounddriver_wmm*)sounddriver->impl;
}

ce_sounddriver* ce_sounddriver_create_platform(int bps, int rate, int channels)
{
	return ce_sounddriver_new((ce_sounddriver_vtable){sizeof(ce_sounddriver_wmm),
		ce_sounddriver_wmm_ctor, ce_sounddriver_wmm_dtor, ce_sounddriver_wmm_write},
		bps, rate, channels);
}
