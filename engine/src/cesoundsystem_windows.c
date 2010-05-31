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

typedef struct {
	WAVEHDR header;
	char buffer[4096];
} ce_soundsystem_wavehdr;

typedef struct {
	WAVEFORMATEXTENSIBLE format;
	HWAVEOUT handle;
	WAVEHDR headers[16];
} ce_soundsystem_wmm;

static void ce_soundsystem_wmm_error(MMRESULT code, const char* message)
{
	char buffer[MAXERRORLENGTH];
	if (MMSYSERR_NOERROR == waveOutGetErrorText(code, buffer, sizeof(buffer))) {
		ce_logging_error("soundsystem: %s", buffer);
	}
	ce_logging_error("soundsystem: %s", message);
}

static bool ce_soundsystem_wmm_ctor(ce_soundsystem* soundsystem, va_list args)
{
	ce_unused(args);
	ce_soundsystem_wmm* wmmsystem = (ce_soundsystem_wmm*)soundsystem->impl;

	ce_logging_write("soundsystem: using Windows Waveform-Audio Interface");

	assert(2 == soundsystem->channels && "only mono and stereo input supported");

	wmmsystem->format.Format.wFormatTag = WAVE_FORMAT_PCM;
	wmmsystem->format.Format.nChannels = soundsystem->channels;
	wmmsystem->format.Format.nSamplesPerSec = soundsystem->rate;
	wmmsystem->format.Format.wBitsPerSample  = soundsystem->bps;
	wmmsystem->format.Format.nBlockAlign = soundsystem->sample_size;
	wmmsystem->format.Format.nAvgBytesPerSec = soundsystem->rate *
												soundsystem->sample_size;

	MMRESULT code = waveOutOpen(&wmmsystem->handle, WAVE_MAPPER,
		&wmmsystem->format.Format, 0, 0, CALLBACK_NULL | WAVE_ALLOWSYNC);
	if (MMSYSERR_NOERROR != code) {
		ce_soundsystem_wmm_error(code, "could not open waveform output device");
		return false;
	}

	return true;
}

static void ce_soundsystem_wmm_dtor(ce_soundsystem* soundsystem)
{
	ce_soundsystem_wmm* wmmsystem = (ce_soundsystem_wmm*)soundsystem->impl;
	MMRESULT code;

	if (NULL != wmmsystem->handle) {
		// TODO: waveOutUnprepareHeader
		if (MMSYSERR_NOERROR != (code = waveOutReset(wmmsystem->handle))) {
			ce_soundsystem_wmm_error(code, "could not reset waveform output device");
		}
		if (MMSYSERR_NOERROR != (code = waveOutClose(wmmsystem->handle))) {
			ce_soundsystem_wmm_error(code, "could not close waveform output device");
		}
	}
}

static void ce_soundsystem_wmm_write(ce_soundsystem* soundsystem, const void* block)
{
	ce_soundsystem_wmm* wmmsystem = (ce_soundsystem_wmm*)soundsystem->impl;
}

ce_soundsystem* ce_soundsystem_new_platform(void)
{
	return ce_soundsystem_new((ce_soundsystem_vtable){sizeof(ce_soundsystem_wmm),
		ce_soundsystem_wmm_ctor, ce_soundsystem_wmm_dtor, ce_soundsystem_wmm_write});
}
