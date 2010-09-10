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

#include <stdint.h>
#include <string.h>
#include <assert.h>

#include "celib.h"
#include "cealloc.h"
#include "celogging.h"
#include "ceevent.h"
#include "cesoundsystem.h"
#include "cesoundmixer.h"

struct ce_sound_mixer* ce_sound_mixer;

static void ce_sound_mixer_create_buffer_react(ce_event* event)
{
	ce_sound_buffer* sound_buffer = ((ce_event_ptr*)event->impl)->ptr;
	ce_vector_push_back(ce_sound_mixer->sound_buffers, sound_buffer);
}

static void ce_sound_mixer_destroy_buffer_react(ce_event* event)
{
	ce_sound_buffer* sound_buffer = ((ce_event_ptr*)event->impl)->ptr;
	ce_vector_remove_all(ce_sound_mixer->sound_buffers, sound_buffer);
	ce_sound_buffer_del(sound_buffer);
}

static void ce_sound_mixer_exit(ce_event* CE_UNUSED(event))
{
	ce_sound_mixer->done = true;
}

static void ce_sound_mixer_convert_sample_s16_s16(int16_t* sample1, const int16_t* sample2,
													const ce_sound_format* sound_format1,
													const ce_sound_format* sound_format2)
{
	for (size_t i = 0; i < sound_format1->channel_count; ++i) {
		if (0 == i || i < sound_format2->channel_count) {
			sample1[i] = sample2[i];
		} else {
			sample1[i] = sample1[i - 1];
		}
	}
}

static void ce_sound_mixer_convert_sample(void* sample1, const void* sample2,
											const ce_sound_format* sound_format1,
											const ce_sound_format* sound_format2)
{
	if (16 == sound_format1->bits_per_sample &&
			16 == sound_format2->bits_per_sample) {
		ce_sound_mixer_convert_sample_s16_s16(sample1, sample2, sound_format1, sound_format2);
	} else {
		assert(false && "not implemented");
	}
}

static void ce_sound_mixer_mix_sample_s16(int16_t* sample, const int16_t* other,
											const ce_sound_format* sound_format)
{
	for (size_t i = 0; i < sound_format->channel_count; ++i) {
		int32_t value = (int32_t)sample[i] + (int32_t)other[i];
		sample[i] = ce_clamp(int32_t, value, INT16_MIN, INT16_MAX);
	}
}

static void ce_sound_mixer_mix_sample(void* sample, const void* other,
										const ce_sound_format* sound_format)
{
	switch (sound_format->bits_per_sample) {
	case 16:
		ce_sound_mixer_mix_sample_s16(sample, other, sound_format);
		break;
	default:
		assert(false && "not implemented");
	}
}

static void ce_sound_mixer_exec(void* CE_UNUSED(arg))
{
	while (!ce_sound_mixer->done) {
		char sample[ce_sound_system->sound_format.sample_size];

		char* block = ce_sound_system_map_block();
		memset(block, 0, CE_SOUND_SYSTEM_BLOCK_SIZE);

		for (size_t i = 0; i < CE_SOUND_SYSTEM_SAMPLES_IN_BLOCK;
							++i, block += ce_sound_system->sound_format.sample_size) {
			for (size_t j = 0; j < ce_sound_mixer->sound_buffers->count; ++j) {
				ce_sound_buffer* sound_buffer = ce_sound_mixer->sound_buffers->items[j];
				if (ce_sound_buffer_is_one_sample_ready(sound_buffer)) {
					char sample2[sound_buffer->sound_format.sample_size];
					ce_sound_buffer_read_one_sample(sound_buffer, sample2);
					ce_sound_mixer_convert_sample(sample, sample2,
						&ce_sound_system->sound_format, &sound_buffer->sound_format);
					ce_sound_mixer_mix_sample(block, sample, &ce_sound_system->sound_format);
				}
			}
		}

		ce_sound_system_unmap_block();
		ce_event_manager_process_events();
	}
}

void ce_sound_mixer_init(void)
{
	ce_sound_mixer = ce_alloc_zero(sizeof(struct ce_sound_mixer));
	ce_sound_mixer->sound_buffers = ce_vector_new();
	ce_sound_mixer->thread = ce_thread_new(ce_sound_mixer_exec, NULL);
}

void ce_sound_mixer_term(void)
{
	if (NULL != ce_sound_mixer) {
		ce_event_manager_post_call(ce_sound_mixer->thread->id, ce_sound_mixer_exit);
		ce_thread_wait(ce_sound_mixer->thread);
		ce_thread_del(ce_sound_mixer->thread);
		if (!ce_vector_empty(ce_sound_mixer->sound_buffers)) {
			ce_logging_warning("sound mixer: some buffers have not been unregistered");
		}
		ce_vector_for_each(ce_sound_mixer->sound_buffers, ce_sound_buffer_del);
		ce_vector_del(ce_sound_mixer->sound_buffers);
		ce_free(ce_sound_mixer, sizeof(struct ce_sound_mixer));
	}
}

ce_sound_buffer* ce_sound_mixer_create_buffer(void)
{
	ce_sound_buffer* sound_buffer = ce_sound_buffer_new(CE_SOUND_SYSTEM_BLOCK_SIZE *
														CE_SOUND_SYSTEM_BLOCK_COUNT);
	ce_event_manager_post_ptr(ce_sound_mixer->thread->id,
		ce_sound_mixer_create_buffer_react, sound_buffer);
	return sound_buffer;
}

void ce_sound_mixer_destroy_buffer(ce_sound_buffer* sound_buffer)
{
	ce_event_manager_post_ptr(ce_sound_mixer->thread->id,
		ce_sound_mixer_destroy_buffer_react, sound_buffer);
}
