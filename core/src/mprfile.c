#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <assert.h>
#include <math.h>

#include <GL/gl.h>

#include "cestr.h"
#include "byteorder.h"
#include "memfile.h"
#include "resfile.h"
#include "mmpfile.h"
#include "mprfile.h"

enum {
	MP_SIGNATURE = 0xce4af672,
	SEC_SIGNATURE = 0xcf4bf774
};

enum {
	TOT_UNDEF = 0,
	TOT_TERRAIN = 1,
	TOT_WATER_NOTEXTURE = 2,
	TOT_WATER = 3,
	TOT_GRASS = 4,
};

enum {
	VERTEX_SIDE = 33,
	VERTEX_COUNT = 33 * 33
};

enum {
	TEXTURE_SIDE = 16,
	TEXTURE_COUNT = 16 * 16
};

typedef struct {
	int32_t type;
	float colour_r;
	float colour_g;
	float colour_b;
	float colour_a;
	float self_illumination; // diffuse_color_r ?
	float wave_multiplier;   // diffuse_color_g ?
	float warp_speed;        // diffuse_color_b ?
	float reserved1;         // specular_color_r ?
	float reserved2;         // specular_color_g ?
	float reserved3;         // specular_color_b ?
} material;

typedef struct {
	int16_t index;
	int16_t phases;
} anim_tile;

typedef struct {
	int8_t offset_x;
	int8_t offset_y;
	uint16_t coord_z;
	uint32_t normal;
} vertex;

typedef struct {
	int8_t water;
	vertex* land_vertices;
	vertex* water_vertices;
	uint16_t* land_textures;
	uint16_t* water_textures;
	int16_t* water_allow;
} sector;

struct mprfile {
	float max_z;
	int32_t sector_x_count;
	int32_t sector_y_count;
	int32_t texture_count;
	int32_t texture_size;
	int32_t tile_count;
	int32_t tile_size;
	int16_t material_count;
	int32_t anim_tile_count;
	material* materials;
	int32_t* tiles;
	anim_tile* anim_tiles;
	sector* sectors;
	GLuint* texture_ids;
};

/*static void normal2vector(uint32_t normal, float vector[3])
{
	vector[2] = (normal >> 22) / 1000.0f;
	vector[0] = (((normal >> 11) & 0x7ff) - 1000.0f) / 1000.0f;
	vector[1] = ((normal & 0x7ff) - 1000.0f) / 1000.0f;
};*/

/*static uint32_t vector2normal(float vector[3])
{
	assert(isgreaterequal(vector[2], 0.0f) && islessequal(vector[2], 1.0f));
	assert(isgreaterequal(vector[0], -1.0f) && islessequal(vector[0], 1.0f));
	assert(isgreaterequal(vector[1], -1.0f) && islessequal(vector[1], 1.0f));
	return ((uint32_t)(vector[2] * 1000.0f) << 22 ) |
			((uint32_t)(vector[0] * 1000.0f + 1000.0f) << 11) |
			(uint32_t)(vector[1] * 1000.0f + 1000.0f);
};*/

static inline uint8_t texture_index(uint16_t value)
{
	return value & 0x003f;
}

static inline uint8_t texture_number(uint16_t value)
{
	return (value & 0x3fc0) >> 6;
}

static inline uint8_t texture_angle(uint16_t value)
{
	return (value & 0xc000) >> 14;
}

static bool read_material(material* mat, memfile* mem)
{
	if (1 != memfile_read(&mat->type, sizeof(int32_t), 1, mem) ||
			1 != memfile_read(&mat->colour_r, sizeof(float), 1, mem) ||
			1 != memfile_read(&mat->colour_g, sizeof(float), 1, mem) ||
			1 != memfile_read(&mat->colour_b, sizeof(float), 1, mem) ||
			1 != memfile_read(&mat->colour_a, sizeof(float), 1, mem) ||
			1 != memfile_read(&mat->self_illumination, sizeof(float), 1, mem) ||
			1 != memfile_read(&mat->wave_multiplier, sizeof(float), 1, mem) ||
			1 != memfile_read(&mat->warp_speed, sizeof(float), 1, mem) ||
			1 != memfile_read(&mat->reserved1, sizeof(float), 1, mem) ||
			1 != memfile_read(&mat->reserved2, sizeof(float), 1, mem) ||
			1 != memfile_read(&mat->reserved3, sizeof(float), 1, mem)) {
		return false;
	}
	le2cpu32s((uint32_t*)&mat->type);
	return true;
}

static bool read_anim_tile(anim_tile* at, memfile* mem)
{
	if (1 != memfile_read(&at->index, sizeof(int16_t), 1, mem) ||
			1 != memfile_read(&at->phases, sizeof(int16_t), 1, mem)) {
		return false;
	}
	le2cpu16s((uint16_t*)&at->index);
	le2cpu16s((uint16_t*)&at->phases);
	return true;
}

static bool read_header(mprfile* mpr, const char* mpr_name,
						size_t mpr_name_length, resfile* res)
{
	if (0 == mpr_name_length) {
		return false;
	}

	// make mp_name by truncation of one last mpr_name character
	char mp_name[mpr_name_length];
	strlcpy(mp_name, mpr_name, sizeof(mp_name));

	int mp_index = resfile_node_index(mp_name, res);
	if (mp_index < 0) {
		return false;
	}

	memfile* mem = resfile_node_memfile(mp_index, res);
	if (NULL == mem) {
		return false;
	}

	uint32_t signature;
	if (1 != memfile_read(&signature, sizeof(uint32_t), 1, mem)) {
		memfile_close(mem);
		return false;
	}

	le2cpu32s(&signature);
	if (MP_SIGNATURE != signature) {
		memfile_close(mem);
		return false;
	}

	if (1 != memfile_read(&mpr->max_z, sizeof(float), 1, mem) ||
			1 != memfile_read(&mpr->sector_x_count, sizeof(int32_t), 1, mem) ||
			1 != memfile_read(&mpr->sector_y_count, sizeof(int32_t), 1, mem) ||
			1 != memfile_read(&mpr->texture_count, sizeof(int32_t), 1, mem) ||
			1 != memfile_read(&mpr->texture_size, sizeof(int32_t), 1, mem) ||
			1 != memfile_read(&mpr->tile_count, sizeof(int32_t), 1, mem) ||
			1 != memfile_read(&mpr->tile_size, sizeof(int32_t), 1, mem) ||
			1 != memfile_read(&mpr->material_count, sizeof(int16_t), 1, mem) ||
			1 != memfile_read(&mpr->anim_tile_count, sizeof(int32_t), 1, mem)) {
		memfile_close(mem);
		return false;
	}

	le2cpu32s((uint32_t*)&mpr->sector_x_count);
	le2cpu32s((uint32_t*)&mpr->sector_y_count);
	le2cpu32s((uint32_t*)&mpr->texture_count);
	le2cpu32s((uint32_t*)&mpr->texture_size);
	le2cpu32s((uint32_t*)&mpr->tile_count);
	le2cpu32s((uint32_t*)&mpr->tile_size);
	le2cpu16s((uint16_t*)&mpr->material_count);
	le2cpu32s((uint32_t*)&mpr->anim_tile_count);

	mpr->materials = malloc(mpr->material_count * sizeof(material));
	if (NULL == mpr->materials) {
		memfile_close(mem);
		return false;
	}

	for (int i = 0; i < mpr->material_count; ++i) {
		if (!read_material(mpr->materials + i, mem)) {
			memfile_close(mem);
			return false;
		}
	}

	mpr->tiles = malloc(mpr->tile_count * sizeof(int32_t));
	if (NULL == mpr->tiles || (size_t)mpr->tile_count !=
			memfile_read(mpr->tiles, sizeof(int32_t), mpr->tile_count, mem)) {
		memfile_close(mem);
		return false;
	}

	for (int i = 0; i < mpr->tile_count; ++i) {
		le2cpu32s((uint32_t*)(mpr->tiles + i));
	}

	mpr->anim_tiles = malloc(mpr->anim_tile_count * sizeof(anim_tile));
	if (NULL == mpr->anim_tiles) {
		memfile_close(mem);
		return false;
	}

	for (int i = 0; i < mpr->anim_tile_count; ++i) {
		if (!read_anim_tile(mpr->anim_tiles + i, mem)) {
			memfile_close(mem);
			return false;
		}
	}

	memfile_close(mem);
	return true;
}

static bool read_vertex(vertex* ver, memfile* mem)
{
	if (1 != memfile_read(&ver->offset_x, sizeof(int8_t), 1, mem) ||
			1 != memfile_read(&ver->offset_y, sizeof(int8_t), 1, mem) ||
			1 != memfile_read(&ver->coord_z, sizeof(uint16_t), 1, mem) ||
			1 != memfile_read(&ver->normal, sizeof(uint32_t), 1, mem)) {
		return false;
	}
	le2cpu16s(&ver->coord_z);
	le2cpu32s(&ver->normal);
	return true;
}

static bool read_sector(sector* sec, const char* sec_name, resfile* res)
{
	int sec_index = resfile_node_index(sec_name, res);
	if (sec_index < 0) {
		return false;
	}

	memfile* mem = resfile_node_memfile(sec_index, res);
	if (NULL == mem) {
		return false;
	}

	uint32_t signature;
	if (1 != memfile_read(&signature, sizeof(uint32_t), 1, mem)) {
		memfile_close(mem);
		return false;
	}

	le2cpu32s(&signature);
	if (SEC_SIGNATURE != signature) {
		memfile_close(mem);
		return false;
	}

	if (1 != memfile_read(&sec->water, sizeof(int8_t), 1, mem)) {
		memfile_close(mem);
		return false;
	}

	sec->land_vertices = malloc(VERTEX_COUNT * sizeof(vertex));
	if (NULL == sec->land_vertices) {
		memfile_close(mem);
		return false;
	}

	for (int i = 0; i < VERTEX_COUNT; ++i) {
		if (!read_vertex(sec->land_vertices + i, mem)) {
			memfile_close(mem);
			return false;
		}
	}

	if (0 != sec->water) {
		sec->water_vertices = malloc(VERTEX_COUNT * sizeof(vertex));
		if (NULL == sec->water_vertices) {
			memfile_close(mem);
			return false;
		}

		for (int i = 0; i < VERTEX_COUNT; ++i) {
			if (!read_vertex(sec->water_vertices + i, mem)) {
				memfile_close(mem);
				return false;
			}
		}
	}

	sec->land_textures = malloc(TEXTURE_COUNT * sizeof(uint16_t));
	if (NULL == sec->land_textures || TEXTURE_COUNT != memfile_read(
			sec->land_textures, sizeof(uint16_t), TEXTURE_COUNT, mem)) {
		memfile_close(mem);
		return false;
	}

	for (int i = 0; i < TEXTURE_COUNT; ++i) {
		le2cpu16s(sec->land_textures + i);
	}

	if (0 != sec->water) {
		sec->water_textures = malloc(TEXTURE_COUNT * sizeof(uint16_t));
		sec->water_allow = malloc(TEXTURE_COUNT * sizeof(int16_t));

		if (NULL == sec->water_textures || NULL == sec->water_allow ||
				TEXTURE_COUNT != memfile_read(sec->water_textures,
					sizeof(uint16_t), TEXTURE_COUNT, mem) ||
				TEXTURE_COUNT != memfile_read(sec->water_allow,
					sizeof(int16_t), TEXTURE_COUNT, mem)) {
			memfile_close(mem);
			return false;
		}

		for (int i = 0; i < TEXTURE_COUNT; ++i) {
			le2cpu16s(sec->land_textures + i);
			le2cpu16s((uint16_t*)(sec->water_allow + i));
		}
	}

	memfile_close(mem);
	return true;
}

static bool read_sectors(mprfile* mpr, const char* mpr_name,
							size_t mpr_name_length, resfile* res)
{
	mpr->sectors = calloc(mpr->sector_x_count *
							mpr->sector_y_count, sizeof(sector));
	if (NULL == mpr->sectors) {
		return false;
	}

	if (mpr_name_length < 4) {
		return false;
	}

	// mpr_name without extension
	char sec_tmpl_name[mpr_name_length - 4 + 1];
	strlcpy(sec_tmpl_name, mpr_name, sizeof(sec_tmpl_name));

	// sec_tmpl_name + xxxyyy.sec
	char sec_name[sizeof(sec_tmpl_name) + 3 + 3 + 4];

	for (int i = 0; i < mpr->sector_x_count; ++i) {
		for (int j = 0; j < mpr->sector_y_count; ++j) {
			sector* sec = mpr->sectors + (i * mpr->sector_y_count + j);

			snprintf(sec_name, sizeof(sec_name),
				"%s%03d%03d.sec", sec_tmpl_name, i, j);

			if (!read_sector(sec, sec_name, res)) {
				return false;
			}
		}
	}

	return true;
}

static bool create_texture(GLuint* texture_id,
			const char* mmp_name, resfile* res)
{
	int mmp_index = resfile_node_index(mmp_name, res);
	if (mmp_index < 0) {
		return false;
	}

	memfile* mem = resfile_node_memfile(mmp_index, res);
	if (NULL == mem) {
		return false;
	}

	*texture_id = mmpfile_create_texture(0, mem);

	if (0 == *texture_id) {
		memfile_close(mem);
		return false;
	}

	memfile_close(mem);
	return true;
}

static bool create_textures(mprfile* mpr, const char* mpr_name,
							size_t mpr_name_length, resfile* res)
{
	mpr->texture_ids = malloc(mpr->texture_count * sizeof(GLuint));
	if (NULL == mpr->texture_ids) {
		return false;
	}

	if (mpr_name_length < 4) {
		return false;
	}

	// mpr_name without extension
	char mmp_tmpl_name[mpr_name_length - 4 + 1];
	strlcpy(mmp_tmpl_name, mpr_name, sizeof(mmp_tmpl_name));

	// mmp_tmpl_name + xxx.mmp
	char mmp_name[sizeof(mmp_tmpl_name) + 3 + 4];

	for (int i = 0; i < mpr->texture_count; ++i) {
		snprintf(mmp_name, sizeof(mmp_name), "%s%03d.mmp", mmp_tmpl_name, i);

		if (!create_texture(mpr->texture_ids + i, mmp_name, res)) {
			return false;
		}
	}

	return true;
}

mprfile* mprfile_open(resfile* mpr_res, resfile* textures_res)
{
	mprfile* mpr = calloc(1, sizeof(mprfile));
	if (NULL == mpr) {
		return NULL;
	}

	const char* mpr_name = resfile_name(mpr_res);
	size_t mpr_name_length = strlen(mpr_name);

	if (!read_header(mpr, mpr_name, mpr_name_length, mpr_res)) {
		mprfile_close(mpr);
		return NULL;
	}

	if (!read_sectors(mpr, mpr_name, mpr_name_length, mpr_res)) {
		mprfile_close(mpr);
		return NULL;
	}

	if (!create_textures(mpr, mpr_name, mpr_name_length, textures_res)) {
		mprfile_close(mpr);
		return NULL;
	}

	return mpr;
}

int mprfile_close(mprfile* mpr)
{
	if (NULL == mpr) {
		return 0;
	}

	if (NULL != mpr->sectors) {
		for (int i = 0, n = mpr->sector_x_count *
				mpr->sector_y_count; i < n; ++i) {
			sector* sec = mpr->sectors + i;
			free(sec->land_vertices);
			free(sec->water_vertices);
			free(sec->land_textures);
			free(sec->water_textures);
			free(sec->water_allow);
		}
	}

	free(mpr->materials);
	free(mpr->tiles);
	free(mpr->anim_tiles);
	free(mpr->sectors);
	free(mpr->texture_ids);

	free(mpr);

	return 0;
}

void mprfile_debug_print(mprfile* mpr)
{
	printf("max z: %f\n", mpr->max_z);
	printf("sector x count: %d\n", mpr->sector_x_count);
	printf("sector y count: %d\n", mpr->sector_y_count);
	printf("texture count: %d\n", mpr->texture_count);
	printf("texture size: %d\n", mpr->texture_size);
	printf("tile count: %d\n", mpr->tile_count);
	printf("tile size: %d\n", mpr->tile_size);
	printf("material count: %hd\n", mpr->material_count);
	printf("anim tile count: %d\n", mpr->anim_tile_count);

	for (int i = 0; i < mpr->material_count; ++i) {
		material* mat = mpr->materials + i;
		printf("material %d:\n", i);
		printf("\ttype: %d\n", mat->type);
		printf("\tcolour_r: %f\n", mat->colour_r);
		printf("\tcolour_g: %f\n", mat->colour_g);
		printf("\tcolour_b: %f\n", mat->colour_b);
		printf("\tcolour_a: %f\n", mat->colour_a);
		printf("\tself_illumination: %f\n", mat->self_illumination);
		printf("\twave_multiplier: %f\n", mat->wave_multiplier);
		printf("\twarp_speed: %f\n", mat->warp_speed);
		printf("\treserved1: %f\n", mat->reserved1);
		printf("\treserved2: %f\n", mat->reserved2);
		printf("\treserved3: %f\n", mat->reserved3);
	}
	for (int i = 0; i < mpr->tile_count; ++i) {
		//printf("id %d: %d\n", i, mpr->tiles[i]);
	}
	for (int i = 0; i < mpr->anim_tile_count; ++i) {
		anim_tile* at = mpr->anim_tiles + i;
		printf("anim tile %d:\n", i);
		printf("\tindex: %hd\n", at->index);
		printf("\tphases: %hd\n", at->phases);
	}
	for (int i = 0, n = mpr->sector_x_count *
				mpr->sector_y_count; i < n; ++i) {
		sector* sec = mpr->sectors + i;
		printf("sector %d:\n", i);
		printf("\twater: %hhd\n", sec->water);
		for (int j = 0; j < VERTEX_COUNT; ++j) {
			//float vector[3] = { 0.0f };
			//normal2vector(sec->land_vertices[j].normal, vector);
			/*printf("\tland vertex %3d: %hhd %hhd %hu %f %f %f\n", j,
				sec->land_vertices[j].offset_x,
				sec->land_vertices[j].offset_y,
				sec->land_vertices[j].coord_z,
				vector[0], vector[1], vector[2]);*/
		}
		if (0 != sec->water) {
			for (int j = 0; j < VERTEX_COUNT; ++j) {
				/*printf("\twater vertex %3d: %hhd %hhd %hu %u\n", j,
					sec->water_vertices[j].offset_x,
					sec->water_vertices[j].offset_y,
					sec->water_vertices[j].coord_z,
					sec->water_vertices[j].normal);*/
			}
		}
		for (int j = 0; j < TEXTURE_COUNT; ++j) {
			/*printf("\tland textures %3d: %2hhu %hhu %hhu\n", j,
				texture_index(sec->land_textures[j]),
				texture_number(sec->land_textures[j]),
				texture_angle(sec->land_textures[j]));*/
		}
		if (0 != sec->water) {
			for (int j = 0; j < TEXTURE_COUNT; ++j) {
				//printf("\t water textures %3d: %hd\n", j, sec->water_textures[j]);
			}
			for (int j = 0; j < TEXTURE_COUNT; ++j) {
				//printf("\twater allow %d: %hd\n", j, sec->water_allow[j]);
			}
		}
	}
}

void mprfile_debug_render(int val, mprfile* mpr)
{
	for (int i = 0; i < mpr->sector_x_count; ++i) {
		for (int j = 0; j < mpr->sector_y_count; ++j) {
			sector* sec = mpr->sectors + (i * mpr->sector_y_count + j);
			for (int k = 0; k < VERTEX_SIDE - 2; k += 2) {
				for (int l = 0; l < VERTEX_SIDE - 2; l += 2) {
					glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
					glBindTexture(GL_TEXTURE_2D, mpr->texture_ids[texture_number(sec->land_textures[(k / 2) * TEXTURE_SIDE + l / 2])]);
					glBegin(GL_TRIANGLE_STRIP);
					glTexCoord2f(1.0f / 16.0f, 1.0f / 16.0f);
					glVertex3f(k + j * (VERTEX_SIDE - 1), l + i * (VERTEX_SIDE - 1),
							0.025f * (sec->land_vertices[k * VERTEX_SIDE + l].coord_z / mpr->max_z));
					glVertex3f(k + 1 + j * (VERTEX_SIDE - 1), l + i * (VERTEX_SIDE - 1),
							0.025f * (sec->land_vertices[(k + 1) * VERTEX_SIDE + l].coord_z / mpr->max_z));
					glVertex3f(k + j * (VERTEX_SIDE - 1), l + 1 + i * (VERTEX_SIDE - 1),
							0.025f * (sec->land_vertices[k * VERTEX_SIDE + (l + 1)].coord_z / mpr->max_z));
					glVertex3f(k + 1 + j * (VERTEX_SIDE - 1), l + 1 + i * (VERTEX_SIDE - 1),
							0.025f * (sec->land_vertices[(k + 1) * VERTEX_SIDE + (l + 1)].coord_z / mpr->max_z));
					glVertex3f(k + j * (VERTEX_SIDE - 1), l + 2 + i * (VERTEX_SIDE - 1),
							0.025f * (sec->land_vertices[k * VERTEX_SIDE + (l + 2)].coord_z / mpr->max_z));
					glVertex3f(k + 1 + j * (VERTEX_SIDE - 1), l + 2 + i * (VERTEX_SIDE - 1),
							0.025f * (sec->land_vertices[(k + 1) * VERTEX_SIDE + (l + 2)].coord_z / mpr->max_z));
					glEnd();
					glBegin(GL_TRIANGLE_STRIP);
					glVertex3f(k + 1 + j * (VERTEX_SIDE - 1), l + i * (VERTEX_SIDE - 1),
							0.025f * (sec->land_vertices[(k + 1) * VERTEX_SIDE + l].coord_z / mpr->max_z));
					glVertex3f(k + 2 + j * (VERTEX_SIDE - 1), l + i * (VERTEX_SIDE - 1),
							0.025f * (sec->land_vertices[(k + 2) * VERTEX_SIDE + l].coord_z / mpr->max_z));
					glVertex3f(k + 1 + j * (VERTEX_SIDE - 1), l + 1 + i * (VERTEX_SIDE - 1),
							0.025f * (sec->land_vertices[(k + 1) * VERTEX_SIDE + (l + 1)].coord_z / mpr->max_z));
					glVertex3f(k + 2 + j * (VERTEX_SIDE - 1), l + 1 + i * (VERTEX_SIDE - 1),
							0.025f * (sec->land_vertices[(k + 2) * VERTEX_SIDE + (l + 1)].coord_z / mpr->max_z));
					glVertex3f(k + 1 + j * (VERTEX_SIDE - 1), l + 2 + i * (VERTEX_SIDE - 1),
							0.025f * (sec->land_vertices[(k + 1) * VERTEX_SIDE + (l + 2)].coord_z / mpr->max_z));
					glVertex3f(k + 2 + j * (VERTEX_SIDE - 1), l + 2 + i * (VERTEX_SIDE - 1),
							0.025f * (sec->land_vertices[(k + 2) * VERTEX_SIDE + (l + 2)].coord_z / mpr->max_z));
					glEnd();
				}
			}
		}
	}
}
