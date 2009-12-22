#include <string.h>

#include <GL/gl.h>

#include "gllib.h"

bool gl_query_feature(int feature)
{
	const char* extensions = (const char*)glGetString(GL_EXTENSIONS);
	switch (feature) {
	case GL_FEATURE_TEXTURE_COMPRESSION_S3TC:
		return NULL != strstr(extensions, "GL_EXT_texture_compression_s3tc");
	}
	return false;
}
