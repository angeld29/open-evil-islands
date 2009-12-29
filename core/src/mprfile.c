#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
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

static const float TEXTURE_UV_STEP = 1.0f / 8.0f;
static const float TEXTURE_UV_HALF_STEP = 1.0f / 16.0f;

typedef struct {
	uint32_t type;
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
	uint16_t index;
	uint16_t phases;
} anim_tile;

typedef struct {
	int8_t offset_x;
	int8_t offset_z;
	uint16_t coord_y;
	uint32_t normal;
} vertex;

typedef struct {
	uint8_t water;
	vertex* land_vertices;
	vertex* water_vertices;
	uint16_t* land_textures;
	uint16_t* water_textures;
	int16_t* water_allow;
} sector;

struct mprfile {
	float max_y;
	uint32_t sector_x_count;
	uint32_t sector_z_count;
	uint32_t texture_count;
	uint32_t texture_size;
	uint32_t tile_count;
	uint32_t tile_size;
	uint16_t material_count;
	uint32_t anim_tile_count;
	material* materials;
	uint32_t* tiles;
	anim_tile* anim_tiles;
	sector* sectors;
	GLuint* texture_ids;
};

static void normal2vector(uint32_t normal, float vector[3])
{
	vector[0] = ((normal & 0x7ff) - 1000.0f) / 1000.0f;
	vector[1] = (normal >> 22) / 1000.0f;
	vector[2] = (((normal >> 11) & 0x7ff) - 1000.0f) / 1000.0f;
};

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
	if (1 != memfile_read(&mat->type, sizeof(uint32_t), 1, mem) ||
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
	le2cpu32s(&mat->type);
	return true;
}

static bool read_anim_tile(anim_tile* at, memfile* mem)
{
	if (1 != memfile_read(&at->index, sizeof(uint16_t), 1, mem) ||
			1 != memfile_read(&at->phases, sizeof(uint16_t), 1, mem)) {
		return false;
	}
	le2cpu16s(&at->index);
	le2cpu16s(&at->phases);
	return true;
}

static bool read_header_impl(mprfile* mpr, memfile* mem)
{
	uint32_t signature;
	if (1 != memfile_read(&signature, sizeof(uint32_t), 1, mem)) {
		return false;
	}

	le2cpu32s(&signature);
	if (MP_SIGNATURE != signature) {
		return false;
	}

	if (1 != memfile_read(&mpr->max_y, sizeof(float), 1, mem) ||
			1 != memfile_read(&mpr->sector_x_count, sizeof(uint32_t), 1, mem) ||
			1 != memfile_read(&mpr->sector_z_count, sizeof(uint32_t), 1, mem) ||
			1 != memfile_read(&mpr->texture_count, sizeof(uint32_t), 1, mem) ||
			1 != memfile_read(&mpr->texture_size, sizeof(uint32_t), 1, mem) ||
			1 != memfile_read(&mpr->tile_count, sizeof(uint32_t), 1, mem) ||
			1 != memfile_read(&mpr->tile_size, sizeof(uint32_t), 1, mem) ||
			1 != memfile_read(&mpr->material_count, sizeof(uint16_t), 1, mem) ||
			1 != memfile_read(&mpr->anim_tile_count, sizeof(uint32_t), 1, mem)) {
		return false;
	}

	le2cpu32s(&mpr->sector_x_count);
	le2cpu32s(&mpr->sector_z_count);
	le2cpu32s(&mpr->texture_count);
	le2cpu32s(&mpr->texture_size);
	le2cpu32s(&mpr->tile_count);
	le2cpu32s(&mpr->tile_size);
	le2cpu16s(&mpr->material_count);
	le2cpu32s(&mpr->anim_tile_count);

	mpr->materials = malloc(mpr->material_count * sizeof(material));
	if (NULL == mpr->materials) {
		return false;
	}

	for (unsigned int i = 0; i < mpr->material_count; ++i) {
		if (!read_material(mpr->materials + i, mem)) {
			return false;
		}
	}

	mpr->tiles = malloc(mpr->tile_count * sizeof(uint32_t));
	if (NULL == mpr->tiles || (size_t)mpr->tile_count !=
			memfile_read(mpr->tiles, sizeof(uint32_t), mpr->tile_count, mem)) {
		return false;
	}

	for (unsigned int i = 0; i < mpr->tile_count; ++i) {
		le2cpu32s(mpr->tiles + i);
	}

	mpr->anim_tiles = malloc(mpr->anim_tile_count * sizeof(anim_tile));
	if (NULL == mpr->anim_tiles) {
		return false;
	}

	for (unsigned int i = 0; i < mpr->anim_tile_count; ++i) {
		if (!read_anim_tile(mpr->anim_tiles + i, mem)) {
			return false;
		}
	}

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

	int index = resfile_node_index(mp_name, res);
	if (index < 0) {
		return false;
	}

	void* data = malloc(resfile_node_size(index, res));
	if (NULL == data || !resfile_node_data(index, data, res)) {
		free(data);
		return false;
	}

	memfile* mem = memfile_open_data(data, resfile_node_size(index, res), "rb");
	if (NULL == mem) {
		free(data);
		return false;
	}

	bool ok = read_header_impl(mpr, mem);

	memfile_close(mem);
	free(data);

	return ok;
}

static bool read_vertex(vertex* ver, memfile* mem)
{
	if (1 != memfile_read(&ver->offset_x, sizeof(int8_t), 1, mem) ||
			1 != memfile_read(&ver->offset_z, sizeof(int8_t), 1, mem) ||
			1 != memfile_read(&ver->coord_y, sizeof(uint16_t), 1, mem) ||
			1 != memfile_read(&ver->normal, sizeof(uint32_t), 1, mem)) {
		return false;
	}
	le2cpu16s(&ver->coord_y);
	le2cpu32s(&ver->normal);
	return true;
}

static bool read_sector_impl(sector* sec, memfile* mem)
{
	uint32_t signature;
	if (1 != memfile_read(&signature, sizeof(uint32_t), 1, mem)) {
		return false;
	}

	le2cpu32s(&signature);
	if (SEC_SIGNATURE != signature) {
		return false;
	}

	if (1 != memfile_read(&sec->water, sizeof(uint8_t), 1, mem)) {
		return false;
	}

	sec->land_vertices = malloc(VERTEX_COUNT * sizeof(vertex));
	if (NULL == sec->land_vertices) {
		return false;
	}

	for (unsigned int i = 0; i < VERTEX_COUNT; ++i) {
		if (!read_vertex(sec->land_vertices + i, mem)) {
			return false;
		}
	}

	if (0 != sec->water) {
		sec->water_vertices = malloc(VERTEX_COUNT * sizeof(vertex));
		if (NULL == sec->water_vertices) {
			return false;
		}

		for (unsigned int i = 0; i < VERTEX_COUNT; ++i) {
			if (!read_vertex(sec->water_vertices + i, mem)) {
				return false;
			}
		}
	}

	sec->land_textures = malloc(TEXTURE_COUNT * sizeof(uint16_t));
	if (NULL == sec->land_textures || TEXTURE_COUNT != memfile_read(
			sec->land_textures, sizeof(uint16_t), TEXTURE_COUNT, mem)) {
		return false;
	}

	for (unsigned int i = 0; i < TEXTURE_COUNT; ++i) {
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
			return false;
		}

		for (unsigned int i = 0; i < TEXTURE_COUNT; ++i) {
			le2cpu16s(sec->water_textures + i);
			le2cpu16s((uint16_t*)(sec->water_allow + i));
		}
	}

	return true;
}

static bool read_sector(sector* sec, const char* name, resfile* res)
{
	int index = resfile_node_index(name, res);
	if (index < 0) {
		return false;
	}

	void* data = malloc(resfile_node_size(index, res));
	if (NULL == data || !resfile_node_data(index, data, res)) {
		free(data);
		return false;
	}

	memfile* mem = memfile_open_data(data, resfile_node_size(index, res), "rb");
	if (NULL == mem) {
		free(data);
		return false;
	}

	bool ok = read_sector_impl(sec, mem);

	memfile_close(mem);
	free(data);

	return ok;
}

static bool read_sectors(mprfile* mpr, const char* mpr_name,
							size_t mpr_name_length, resfile* res)
{
	mpr->sectors = calloc(mpr->sector_x_count *
							mpr->sector_z_count, sizeof(sector));
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

	for (unsigned int z = 0; z < mpr->sector_z_count; ++z) {
		for (unsigned int x = 0; x < mpr->sector_x_count; ++x) {
			sector* sec = mpr->sectors + (z * mpr->sector_x_count + x);

			snprintf(sec_name, sizeof(sec_name),
				"%s%03u%03u.sec", sec_tmpl_name, x, z);

			if (!read_sector(sec, sec_name, res)) {
				return false;
			}
		}
	}

	return true;
}

static bool create_texture(GLuint* texture_id, const char* name, resfile* res)
{
	int index = resfile_node_index(name, res);
	if (index < 0) {
		return false;
	}

	void* data = malloc(resfile_node_size(index, res));
	if (NULL == data || !resfile_node_data(index, data, res)) {
		free(data);
		return false;
	}

	*texture_id = mmpfile_generate_texture(0, data);

	free(data);

	return 0 != *texture_id;
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

	for (unsigned int i = 0; i < mpr->texture_count; ++i) {
		snprintf(mmp_name, sizeof(mmp_name), "%s%03u.mmp", mmp_tmpl_name, i);

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

	if (NULL != mpr->texture_ids) {
		glDeleteTextures(mpr->texture_count, mpr->texture_ids);
	}

	if (NULL != mpr->sectors) {
		for (int i = 0, n = mpr->sector_x_count *
				mpr->sector_z_count; i < n; ++i) {
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
	printf("max y: %f\n", mpr->max_y);
	printf("sector x count: %u\n", mpr->sector_x_count);
	printf("sector z count: %u\n", mpr->sector_z_count);
	printf("texture count: %u\n", mpr->texture_count);
	printf("texture size: %u\n", mpr->texture_size);
	printf("tile count: %u\n", mpr->tile_count);
	printf("tile size: %u\n", mpr->tile_size);
	printf("material count: %hu\n", mpr->material_count);
	printf("anim tile count: %u\n", mpr->anim_tile_count);

	for (unsigned int i = 0; i < mpr->material_count; ++i) {
		material* mat = mpr->materials + i;
		printf("material %u:\n", i);
		printf("\ttype: %u\n", mat->type);
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
	for (unsigned int i = 0; i < mpr->tile_count; ++i) {
		//printf("id %u: %u\n", i, mpr->tiles[i]);
	}
	for (unsigned int i = 0; i < mpr->anim_tile_count; ++i) {
		anim_tile* at = mpr->anim_tiles + i;
		printf("anim tile %u:\n", i);
		printf("\tindex: %hu\n", at->index);
		printf("\tphases: %hu\n", at->phases);
	}
	for (unsigned int i = 0, n = mpr->sector_x_count *
				mpr->sector_z_count; i < n; ++i) {
		sector* sec = mpr->sectors + i;
		if (0 != sec->water) {
			for (unsigned int j = 0; j < TEXTURE_COUNT; ++j) {
				//printf("\twater allow %d: %hd\n", j, sec->water_allow[j]);
			}
		}
	}
}

void mprfile_debug_render(int val, mprfile* mpr)
{
	glEnable(GL_TEXTURE_2D);
	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	for (unsigned int i = 0; i < mpr->sector_z_count; ++i) {
		for (unsigned int j = 0; j < mpr->sector_x_count; ++j) {
			sector* sec = mpr->sectors + (i * mpr->sector_x_count + j);
			for (unsigned int k = 0; k < VERTEX_SIDE - 2; k += 2) {
				for (unsigned int l = 0; l < VERTEX_SIDE - 2; l += 2) {
					vertex* ver1 = sec->land_vertices + ((k + 0) * VERTEX_SIDE + (l + 0));
					vertex* ver2 = sec->land_vertices + ((k + 0) * VERTEX_SIDE + (l + 1));
					vertex* ver3 = sec->land_vertices + ((k + 0) * VERTEX_SIDE + (l + 2));
					vertex* ver4 = sec->land_vertices + ((k + 1) * VERTEX_SIDE + (l + 2));
					vertex* ver5 = sec->land_vertices + ((k + 2) * VERTEX_SIDE + (l + 2));
					vertex* ver6 = sec->land_vertices + ((k + 2) * VERTEX_SIDE + (l + 1));
					vertex* ver7 = sec->land_vertices + ((k + 2) * VERTEX_SIDE + (l + 0));
					vertex* ver8 = sec->land_vertices + ((k + 1) * VERTEX_SIDE + (l + 0));
					vertex* ver9 = sec->land_vertices + ((k + 1) * VERTEX_SIDE + (l + 1));

					float coef = val % 2 ? 1.0f : 0.0f;

					GLfloat vertices[] = {
						k + i * (VERTEX_SIDE - 1) + coef * ver1->offset_x / 127.0f,
						0.025f * (ver1->coord_y / mpr->max_y),
						l + j * (VERTEX_SIDE - 1) + coef * ver1->offset_z / 127.0f,

						k + i * (VERTEX_SIDE - 1) + coef * ver2->offset_x / 127.0f,
						0.025f * (ver2->coord_y / mpr->max_y),
						l + 1 + j * (VERTEX_SIDE - 1) + coef * ver2->offset_z / 127.0f,

						k + i * (VERTEX_SIDE - 1) + coef * ver3->offset_x / 127.0f,
						0.025f * (ver3->coord_y / mpr->max_y),
						l + 2 + j * (VERTEX_SIDE - 1) + coef * ver3->offset_z / 127.0f,

						k + 1 + i * (VERTEX_SIDE - 1) + coef * ver4->offset_x / 127.0f,
						0.025f * (ver4->coord_y / mpr->max_y),
						l + 2 + j * (VERTEX_SIDE - 1) + coef * ver4->offset_z / 127.0f,

						k + 2 + i * (VERTEX_SIDE - 1) + coef * ver5->offset_x / 127.0f,
						0.025f * (ver5->coord_y / mpr->max_y),
						l + 2 + j * (VERTEX_SIDE - 1) + coef * ver5->offset_z / 127.0f,

						k + 2 + i * (VERTEX_SIDE - 1) + coef * ver6->offset_x / 127.0f,
						0.025f * (ver6->coord_y / mpr->max_y),
						l + 1 + j * (VERTEX_SIDE - 1) + coef * ver6->offset_z / 127.0f,

						k + 2 + i * (VERTEX_SIDE - 1) + coef * ver7->offset_x / 127.0f,
						0.025f * (ver7->coord_y / mpr->max_y),
						l + j * (VERTEX_SIDE - 1) + coef * ver7->offset_z / 127.0f,

						k + 1 + i * (VERTEX_SIDE - 1) + coef * ver8->offset_x / 127.0f,
						0.025f * (ver8->coord_y / mpr->max_y),
						l + j * (VERTEX_SIDE - 1) + coef * ver8->offset_z / 127.0f,

						k + 1 + i * (VERTEX_SIDE - 1) + coef * ver9->offset_x / 127.0f,
						0.025f * (ver9->coord_y / mpr->max_y),
						l + 1 + j * (VERTEX_SIDE - 1) + coef * ver9->offset_z / 127.0f
					};

					GLfloat normals[3 * 9];
					normal2vector(ver1->normal, normals + 0);
					normal2vector(ver2->normal, normals + 3);
					normal2vector(ver3->normal, normals + 6);
					normal2vector(ver4->normal, normals + 9);
					normal2vector(ver5->normal, normals + 12);
					normal2vector(ver6->normal, normals + 15);
					normal2vector(ver7->normal, normals + 18);
					normal2vector(ver8->normal, normals + 21);
					normal2vector(ver9->normal, normals + 24);

					uint16_t texture = sec->land_textures[k / 2 * TEXTURE_SIDE + l / 2];

					int idx = texture_index(texture);
					float u = (idx - idx / 8 * 8) / 8.0f;
					float v = (7 - idx / 8) / 8.0f;

					GLfloat texcoords[4][18] = {
						{ u, v + TEXTURE_UV_STEP,
						u + TEXTURE_UV_HALF_STEP, v + TEXTURE_UV_STEP,
						u + TEXTURE_UV_STEP, v + TEXTURE_UV_STEP,
						u + TEXTURE_UV_STEP, v + TEXTURE_UV_HALF_STEP,
						u + TEXTURE_UV_STEP, v,
						u + TEXTURE_UV_HALF_STEP, v,
						u, v,
						u, v + TEXTURE_UV_HALF_STEP,
						u + TEXTURE_UV_HALF_STEP, v + TEXTURE_UV_HALF_STEP },

						{ u + TEXTURE_UV_STEP, v + TEXTURE_UV_STEP,
						u + TEXTURE_UV_STEP, v + TEXTURE_UV_HALF_STEP,
						u + TEXTURE_UV_STEP, v,
						u + TEXTURE_UV_HALF_STEP, v,
						u, v,
						u, v + TEXTURE_UV_HALF_STEP,
						u, v + TEXTURE_UV_STEP,
						u + TEXTURE_UV_HALF_STEP, v + TEXTURE_UV_STEP,
						u + TEXTURE_UV_HALF_STEP, v + TEXTURE_UV_HALF_STEP },

						{ u + TEXTURE_UV_STEP, v,
						u + TEXTURE_UV_HALF_STEP, v,
						u, v,
						u, v + TEXTURE_UV_HALF_STEP,
						u, v + TEXTURE_UV_STEP,
						u + TEXTURE_UV_HALF_STEP, v + TEXTURE_UV_STEP,
						u + TEXTURE_UV_STEP, v + TEXTURE_UV_STEP,
						u + TEXTURE_UV_STEP, v + TEXTURE_UV_HALF_STEP,
						u + TEXTURE_UV_HALF_STEP, v + TEXTURE_UV_HALF_STEP },

						{ u, v,
						u, v + TEXTURE_UV_HALF_STEP,
						u, v + TEXTURE_UV_STEP,
						u + TEXTURE_UV_HALF_STEP, v + TEXTURE_UV_STEP,
						u + TEXTURE_UV_STEP, v + TEXTURE_UV_STEP,
						u + TEXTURE_UV_STEP, v + TEXTURE_UV_HALF_STEP,
						u + TEXTURE_UV_STEP, v,
						u + TEXTURE_UV_HALF_STEP, v,
						u + TEXTURE_UV_HALF_STEP, v + TEXTURE_UV_HALF_STEP },
					};

					glVertexPointer(3, GL_FLOAT, 0, vertices);
					glNormalPointer(GL_FLOAT, 0, normals);
					glTexCoordPointer(2, GL_FLOAT, 0, texcoords[texture_angle(texture)]);

					GLubyte indices[2][6] = {
						{ 7, 0, 8, 1, 3, 2 },
						{ 6, 7, 5, 8, 4, 3 }
					};

					glBindTexture(GL_TEXTURE_2D, mpr->texture_ids[texture_number(texture)]);

					glDrawElements(GL_TRIANGLE_STRIP, 6, GL_UNSIGNED_BYTE, indices[0]);
					glDrawElements(GL_TRIANGLE_STRIP, 6, GL_UNSIGNED_BYTE, indices[1]);
				}
			}
		}
	}

	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
	//glDisable(GL_BLEND);
	glDisable(GL_TEXTURE_2D);
}
