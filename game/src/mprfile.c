#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <math.h>

#include <GL/gl.h>

#include "cestr.h"
#include "cemath.h"
#include "byteorder.h"
#include "memfile.h"
#include "resfile.h"
#include "texture.h"
#include "mprfile.h"

static const uint32_t MP_SIGNATURE = 0xce4af672;
static const uint32_t SEC_SIGNATURE = 0xcf4bf774;

enum {
	MATERIAL_GROUND = 1,
	MATERIAL_WATER = 3,
};

static const unsigned int VERTEX_SIDE = 33;
static const unsigned int VERTEX_COUNT = 33 * 33;

static const unsigned int TEXTURE_SIDE = 16;
static const unsigned int TEXTURE_COUNT = 16 * 16;

typedef struct {
	uint32_t type;
	float color[4];
	float selfillum;
	float wavemult;
} material;

typedef struct {
	uint16_t index;
	uint16_t count;
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
	uint32_t sector_z_count;
	uint32_t sector_x_count;
	uint32_t texture_count;
	uint32_t texture_size;
	uint32_t tile_count;
	uint32_t tile_size;
	uint16_t material_count;
	uint32_t anim_tile_count;
	material* materials;
	uint32_t* tiles; // TODO: id for bind sound ?
	anim_tile* anim_tiles;
	sector* sectors;
	bool* sector_allow;
	texture** textures;
};

static void normal2vector(uint32_t normal, float* vector)
{
	vector[0] = ((normal & 0x7ff) - 1000.0f) / 1000.0f;
	vector[1] = (normal >> 22) / 1000.0f;
	vector[2] = (((normal >> 11) & 0x7ff) - 1000.0f) / 1000.0f;
}

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

static material* find_water_material(mprfile* mpr)
{
	for (unsigned int i = 0; i < mpr->material_count; ++i) {
		material* mat = mpr->materials + i;
		if (MATERIAL_WATER == mat->type) {
			return mat;
		}
	}
	return NULL;
}

static bool read_material(material* mat, memfile* mem)
{
	float unknown[4];
	if (1 != memfile_read(&mat->type, sizeof(uint32_t), 1, mem) ||
			4 != memfile_read(mat->color, sizeof(float), 4, mem) ||
			1 != memfile_read(&mat->selfillum, sizeof(float), 1, mem) ||
			1 != memfile_read(&mat->wavemult, sizeof(float), 1, mem) ||
			4 != memfile_read(unknown, sizeof(float), 4, mem)) {
		return false;
	}
	le2cpu32s(&mat->type);
	return true;
}

static bool read_anim_tile(anim_tile* at, memfile* mem)
{
	if (1 != memfile_read(&at->index, sizeof(uint16_t), 1, mem) ||
			1 != memfile_read(&at->count, sizeof(uint16_t), 1, mem)) {
		return false;
	}
	le2cpu16s(&at->index);
	le2cpu16s(&at->count);
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
			1 != memfile_read(&mpr->sector_z_count, sizeof(uint32_t), 1, mem) ||
			1 != memfile_read(&mpr->sector_x_count, sizeof(uint32_t), 1, mem) ||
			1 != memfile_read(&mpr->texture_count, sizeof(uint32_t), 1, mem) ||
			1 != memfile_read(&mpr->texture_size, sizeof(uint32_t), 1, mem) ||
			1 != memfile_read(&mpr->tile_count, sizeof(uint32_t), 1, mem) ||
			1 != memfile_read(&mpr->tile_size, sizeof(uint32_t), 1, mem) ||
			1 != memfile_read(&mpr->material_count, sizeof(uint16_t), 1, mem) ||
			1 != memfile_read(&mpr->anim_tile_count, sizeof(uint32_t), 1, mem)) {
		return false;
	}

	le2cpu32s(&mpr->sector_z_count);
	le2cpu32s(&mpr->sector_x_count);
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

	// sec_tmpl_name + zzzxxx.sec
	char sec_name[sizeof(sec_tmpl_name) + 3 + 3 + 4];

	for (unsigned int x = 0; x < mpr->sector_x_count; ++x) {
		for (unsigned int z = 0; z < mpr->sector_z_count; ++z) {
			sector* sec = mpr->sectors + (x * mpr->sector_z_count + z);

			snprintf(sec_name, sizeof(sec_name),
				"%s%03u%03u.sec", sec_tmpl_name, z, x);

			if (!read_sector(sec, sec_name, res)) {
				return false;
			}
		}
	}

	if (NULL == (mpr->sector_allow = malloc(mpr->sector_x_count *
										mpr->sector_z_count * sizeof(bool)))) {
		return false;
	}

	for (unsigned int i = 0, n = mpr->sector_x_count *
			mpr->sector_z_count; i < n; ++i) {
		mpr->sector_allow[i] = true;
	}

	return true;
}

static bool create_texture(texture** tex, const char* name, resfile* res)
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

	*tex = texture_open(data);

	free(data);

	return NULL != *tex;
}

static bool create_textures(mprfile* mpr, const char* mpr_name,
							size_t mpr_name_length, resfile* res)
{
	mpr->textures = calloc(mpr->texture_count, sizeof(texture*));
	if (NULL == mpr->textures) {
		return false;
	}

	if (mpr_name_length < 4) {
		return false;
	}

	// mpr_name without extension
	char mmp_tmpl_name[mpr_name_length - 4 + 1];
	strlcpy(mmp_tmpl_name, mpr_name, sizeof(mmp_tmpl_name));

	// mmp_tmpl_name + nnn.mmp
	char mmp_name[sizeof(mmp_tmpl_name) + 3 + 4];

	for (unsigned int i = 0; i < mpr->texture_count; ++i) {
		snprintf(mmp_name, sizeof(mmp_name), "%s%03u.mmp", mmp_tmpl_name, i);

		if (!create_texture(mpr->textures + i, mmp_name, res)) {
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
		for (unsigned int i = 0, n = mpr->sector_x_count *
				mpr->sector_z_count; i < n; ++i) {
			sector* sec = mpr->sectors + i;
			free(sec->land_vertices);
			free(sec->water_vertices);
			free(sec->land_textures);
			free(sec->water_textures);
			free(sec->water_allow);
		}
	}

	if (NULL != mpr->textures) {
		for (unsigned int i = 0; i < mpr->texture_count; ++i) {
			texture_close(mpr->textures[i]);
		}
	}

	free(mpr->materials);
	free(mpr->tiles);
	free(mpr->anim_tiles);
	free(mpr->sectors);
	free(mpr->sector_allow);
	free(mpr->textures);

	free(mpr);

	return 0;
}

void mprfile_apply_frustum(const frustum* f, mprfile* mpr)
{
	float pmin[3], pmax[3];
	for (unsigned int x = 0; x < mpr->sector_x_count; ++x) {
		for (unsigned int z = 0; z < mpr->sector_z_count; ++z) {
			vector3_init(x * (VERTEX_SIDE - 1), 0.0f,
							z * (VERTEX_SIDE - 1), pmin);
			vector3_init(x * (VERTEX_SIDE - 1) + (VERTEX_SIDE - 1), mpr->max_y,
							z * (VERTEX_SIDE - 1) + (VERTEX_SIDE - 1), pmax);
			mpr->sector_allow[x * mpr->sector_z_count + z] =
				frustum_test_box(pmin, pmax, f);
		}
	}
}

static void render_vertices(unsigned int sector_x, unsigned int sector_z,
							vertex* vertices, uint16_t* textures,
							int16_t* water_allow, mprfile* mpr)
{
	GLfloat varray[3 * VERTEX_COUNT];
	GLfloat narray[3 * VERTEX_COUNT];
	GLfloat tcarray[2 * VERTEX_COUNT];

	static const float offset_xz_coef = 1.0f / UINT8_MAX;
	const float y_coef = mpr->max_y / UINT16_MAX;

	for (unsigned int x = 0; x < VERTEX_SIDE; ++x) {
		for (unsigned int z = 0; z < VERTEX_SIDE; ++z) {
			unsigned int i = x * VERTEX_SIDE * 3 + z * 3;
			vertex* ver = vertices + (x * VERTEX_SIDE + z);

			varray[i + 0] = x + sector_x * (VERTEX_SIDE - 1) +
								offset_xz_coef * ver->offset_x;
			varray[i + 1] = y_coef * ver->coord_y;
			varray[i + 2] = z + sector_z * (VERTEX_SIDE - 1) +
								offset_xz_coef * ver->offset_z;

			normal2vector(ver->normal, narray + i);
		}
	}

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	glVertexPointer(3, GL_FLOAT, 0, varray);
	glNormalPointer(GL_FLOAT, 0, narray);
	glTexCoordPointer(2, GL_FLOAT, 0, tcarray);

	for (unsigned int x = 0; x < VERTEX_SIDE - 2; x += 2) {
		for (unsigned int z = 0; z < VERTEX_SIDE - 2; z += 2) {
			if (NULL != water_allow &&
					-1 == water_allow[x / 2 * TEXTURE_SIDE + z / 2]) {
				continue;
			}

			uint16_t tex = textures[x / 2 * TEXTURE_SIDE + z / 2];

			int tex_idx = texture_index(tex);
			float u = (tex_idx - tex_idx / 8 * 8) / 8.0f;
			float v = (7 - tex_idx / 8) / 8.0f;

			static const float tile_uv_step = 1.0f / 8.0f;
			static const float tile_uv_half_step = 1.0f / 16.0f;

			static const float tile_border_size = 4.0f;
			const float tile_border_offset = tile_border_size /
											(0.5f * mpr->texture_size);

			float texcoords[2 * 9] = {
				u + tile_border_offset, v + tile_uv_step - tile_border_offset,
				u + tile_uv_half_step, v + tile_uv_step - tile_border_offset,
				u + tile_uv_step - tile_border_offset, v + tile_uv_step - tile_border_offset,
				u + tile_uv_step - tile_border_offset, v + tile_uv_half_step,
				u + tile_uv_step - tile_border_offset, v + tile_border_offset,
				u + tile_uv_half_step, v + tile_border_offset,
				u + tile_border_offset, v + tile_border_offset,
				u + tile_border_offset, v + tile_uv_half_step,
				u + tile_uv_half_step, v + tile_uv_half_step
			};

			static unsigned int offx[9] = { 0, 0, 0, 1, 2, 2, 2, 1, 1 };
			static unsigned int offz[9] = { 0, 1, 2, 2, 2, 1, 0, 0, 1 };

			for (unsigned int i = 0; i < 9; ++i) {
				unsigned int tci = (x + offx[i]) * VERTEX_SIDE * 2 +
													(z + offz[i]) * 2;
				tcarray[tci + 0] = texcoords[i * 2 + 0];
				tcarray[tci + 1] = texcoords[i * 2 + 1];
			}

			glMatrixMode(GL_TEXTURE);
			glLoadIdentity();
			glTranslatef(u + tile_uv_half_step,
							v + tile_uv_half_step, 0.0f);
			glRotatef(-90.0f * texture_angle(tex), 0.0f, 0.0f, 1.0f);
			glTranslatef(-u - tile_uv_half_step,
							-v - tile_uv_half_step, 0.0f);
			glMatrixMode(GL_MODELVIEW);

			GLushort vind[9];
			for (unsigned int i = 0; i < 9; ++i) {
				vind[i] = (x + offx[i]) * VERTEX_SIDE + (z + offz[i]);
			}

			GLushort indices[2][6] = {
				{ vind[7], vind[0], vind[8], vind[1], vind[3], vind[2] },
				{ vind[6], vind[7], vind[5], vind[8], vind[4], vind[3] }
			};

			if (NULL != water_allow) {
				glEnable(GL_LIGHTING);
				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				material* mat = find_water_material(mpr);
				assert(mat);
				glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, mat->color);
			}

			texture_bind(mpr->textures[texture_number(tex)]);

			glDrawElements(GL_TRIANGLE_STRIP, 6, GL_UNSIGNED_SHORT, indices[0]);
			glDrawElements(GL_TRIANGLE_STRIP, 6, GL_UNSIGNED_SHORT, indices[1]);

			texture_unbind(mpr->textures[texture_number(tex)]);

			if (NULL != water_allow) {
				glDisable(GL_BLEND);
				glDisable(GL_LIGHTING);
			}
		}
	}

	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
}

void mprfile_render(mprfile* mpr)
{
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

	for (unsigned int x = 0, i; x < mpr->sector_x_count; ++x) {
		for (unsigned int z = 0; z < mpr->sector_z_count; ++z) {
			i = x * mpr->sector_z_count + z;

			if (!mpr->sector_allow[i]) {
				continue;
			}

			sector* sec = mpr->sectors + i;

			render_vertices(x, z, sec->land_vertices,
							sec->land_textures, NULL, mpr);

			if (0 != sec->water) {
				render_vertices(x, z, sec->water_vertices,
								sec->water_textures, sec->water_allow, mpr);
			}
		}
	}

	/*glBegin(GL_LINES);
	glColor3f(1, 0, 0);
	glVertex3f(49.5f, 5, 118.5f);
	glColor3f(0, 1, 0);
	glVertex3f(50.5f, 5, 119.5f);
	glColor3f(0, 1, 0);
	glVertex3f(118.5f, 5, 49.5f);
	glColor3f(0, 0, 1);
	glVertex3f(119.5f, 5, 50.5f);
	glEnd();*/
}
