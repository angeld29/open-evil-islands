#include <string.h>
#include <assert.h>

#include "gllib.h"

bool gl_query_feature(int feature)
{
	const char* extensions = (const char*)glGetString(GL_EXTENSIONS);
	switch (feature) {
	case GL_FEATURE_TEXTURE_NON_POWER_OF_TWO:
		return NULL != strstr(extensions, "GL_ARB_texture_non_power_of_two");
	case GL_FEATURE_TEXTURE_RECTANGLE:
		return NULL != strstr(extensions, "GL_ARB_texture_rectangle") ||
			NULL != strstr(extensions, "GL_EXT_texture_rectangle") ||
			NULL != strstr(extensions, "GL_NV_texture_rectangle");
	case GL_FEATURE_TEXTURE_COMPRESSION_S3TC:
		return NULL != strstr(extensions, "GL_EXT_texture_compression_s3tc");
	default:
		assert(false);
	}
	return false;
}

GLint gl_max_texture_size(void)
{
	GLint max_texture_size;
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max_texture_size);
	return max_texture_size;
}
