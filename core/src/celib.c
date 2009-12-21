#include <string.h>

#include <GL/gl.h>

#include "celib.h"

bool gl_extension_supported(const char* extension)
{
	return NULL != strstr((const char*)glGetString(GL_EXTENSIONS), extension);
}
