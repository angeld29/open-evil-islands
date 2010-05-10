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

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <limits.h>
#include <assert.h>

#include "celib.h"
#include "cealloc.h"
#include "celogging.h"
#include "cedisplay.h"

ce_displaymode* ce_displaymode_new(int width, int height, int bpp, int rate)
{
	ce_displaymode* mode = ce_alloc(sizeof(ce_displaymode));
	mode->width = width;
	mode->height = height;
	mode->bpp = bpp;
	mode->rate = rate;
	return mode;
}

void ce_displaymode_del(ce_displaymode* mode)
{
	if (NULL != mode) {
		ce_free(mode, sizeof(ce_displaymode));
	}
}

ce_displaymng* ce_displaymng_new(ce_displaymng_vtable vtable, size_t size, ...)
{
	ce_displaymng* displaymng = ce_alloc(sizeof(ce_displaymng) + size);
	displaymng->modes = ce_vector_new();
	displaymng->rotation = CE_DISPLAY_ROTATION_NONE;
	displaymng->reflection = CE_DISPLAY_REFLECTION_NONE;
	displaymng->vtable = vtable;
	displaymng->size = size;
	va_list args;
	va_start(args, size);
	(*vtable.ctor)(displaymng, args);
	va_end(args);
	return displaymng;
}

void ce_displaymng_del(ce_displaymng* displaymng)
{
	if (NULL != displaymng) {
		(*displaymng->vtable.dtor)(displaymng);
		ce_vector_for_each(displaymng->modes, ce_displaymode_del);
		ce_vector_del(displaymng->modes);
		ce_free(displaymng, sizeof(ce_displaymng) + displaymng->size);
	}
}

void ce_displaymng_restore(ce_displaymng* displaymng)
{
	(*displaymng->vtable.restore)(displaymng);
}

void ce_displaymng_change(ce_displaymng* displaymng,
							int width, int height, int bpp, int rate,
							ce_display_rotation rotation,
							ce_display_reflection reflection)
{
	if (ce_vector_empty(displaymng->modes)) {
		ce_logging_warning("displaymng: no modes found");
		return;
	}

	int best_width = width, best_height = height;
	int best_bpp = bpp, best_rate = rate;

	for (int i = 0; i < displaymng->modes->count; ++i) {
		ce_displaymode* mode = displaymng->modes->items[i];
		if (width <= 0) best_width = ce_max(best_width, mode->width);
		if (height <= 0) best_height = ce_max(best_height, mode->height);
		if (bpp <= 0) best_bpp = ce_max(best_bpp, mode->bpp);
		if (rate <= 0) best_rate = ce_max(best_rate, mode->rate);
	}

	width = best_width, height = best_height;
	bpp = best_bpp, rate = best_rate;

	int best_size_score = INT_MAX;
	int best_bpp_score = INT_MAX, best_rate_score = INT_MAX;

	// pass 1: find best width and height
	for (int i = 0, score; i < displaymng->modes->count; ++i) {
		ce_displaymode* mode = displaymng->modes->items[i];
		score = (width - mode->width) * (width - mode->width) +
				(height - mode->height) * (height - mode->height);
		if (score < best_size_score) {
			best_width = mode->width;
			best_height = mode->height;
			best_size_score = score;
		}
	}

	// pass 2: find best bpp and rate
	for (int i = 0, score; i < displaymng->modes->count; ++i) {
		ce_displaymode* mode = displaymng->modes->items[i];
		if (best_width == mode->width && best_height == mode->height) {
			score = abs(bpp - mode->bpp);
			if (score < best_bpp_score) {
				best_bpp = mode->bpp;
				best_bpp_score = score;
			}
			score = abs(rate - mode->rate);
			if (score < best_rate_score) {
				best_rate = mode->rate;
				best_rate_score = score;
			}
		}
	}

	if (width != best_width || height != best_height ||
			bpp != best_bpp || rate != best_rate) {
		ce_logging_warning("displaymng: mode %dx%d:%d@%d not found",
			width, height, bpp, rate);
		ce_logging_warning("displaymng: using best match %dx%d:%d@%d",
			best_width, best_height, best_bpp, best_rate);
	}

	// pass 3: find index
	int index;
	for (index = 0; index < displaymng->modes->count; ++index) {
		ce_displaymode* mode = displaymng->modes->items[index];
		if (best_width == mode->width && best_height == mode->height &&
				best_bpp == mode->bpp && best_rate == mode->rate) {
			break;
		}
	}

	if (index == displaymng->modes->count) {
		assert(false);
		index = 0;
	}

	// correct rotation and reflection
	ce_display_rotation best_rotation = CE_DISPLAY_ROTATION_NONE;
	ce_display_reflection best_reflection = CE_DISPLAY_REFLECTION_NONE;

	// exclusive mode!
	if (rotation & CE_DISPLAY_ROTATION_0 &&
			displaymng->rotation & CE_DISPLAY_ROTATION_0) {
		best_rotation = CE_DISPLAY_ROTATION_0;
	} else if (rotation & CE_DISPLAY_ROTATION_90 &&
			displaymng->rotation & CE_DISPLAY_ROTATION_90) {
		best_rotation = CE_DISPLAY_ROTATION_90;
	} else if (rotation & CE_DISPLAY_ROTATION_180 &&
			displaymng->rotation & CE_DISPLAY_ROTATION_180) {
		best_rotation = CE_DISPLAY_ROTATION_180;
	} else if (rotation & CE_DISPLAY_ROTATION_270 &&
			displaymng->rotation & CE_DISPLAY_ROTATION_270) {
		best_rotation = CE_DISPLAY_ROTATION_270;
	}

	if (reflection & CE_DISPLAY_REFLECTION_X &&
			displaymng->reflection & CE_DISPLAY_REFLECTION_X) {
		best_reflection |= CE_DISPLAY_REFLECTION_X;
	}

	if (reflection & CE_DISPLAY_REFLECTION_Y &&
			displaymng->reflection & CE_DISPLAY_REFLECTION_Y) {
		best_reflection |= CE_DISPLAY_REFLECTION_Y;
	}

	(*displaymng->vtable.change)(displaymng, index, best_rotation, best_reflection);
}
