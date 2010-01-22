#include <GL/gl.h>
#include <GL/glu.h>

#include "logging.h"
#include "cegl.h"

static bool features[CEGL_FEATURE_COUNT];

static bool check_extension(const char* name)
{
	return gluCheckExtension((const GLubyte*)name, glGetString(GL_EXTENSIONS));
}

static void report_extension(const char* name, bool ok)
{
	logging_write("Checking for '%s' extension... %s", name, ok ? "yes" : "no");
}

void cegl_init(void)
{
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	features[CEGL_FEATURE_TEXTURE_NON_POWER_OF_TWO] =
		check_extension("GL_ARB_texture_non_power_of_two");
	features[CEGL_FEATURE_TEXTURE_RECTANGLE] =
		check_extension("GL_ARB_texture_rectangle") ||
		check_extension("GL_EXT_texture_rectangle") ||
		check_extension("GL_NV_texture_rectangle");
	features[CEGL_FEATURE_TEXTURE_COMPRESSION_S3TC] =
		check_extension("GL_EXT_texture_compression_s3tc");

	report_extension("texture non power of two",
		features[CEGL_FEATURE_TEXTURE_NON_POWER_OF_TWO]);
	report_extension("texture rectangle",
		features[CEGL_FEATURE_TEXTURE_RECTANGLE]);
	report_extension("texture compression s3tc",
		features[CEGL_FEATURE_TEXTURE_COMPRESSION_S3TC]);
}

bool cegl_report_errors(void)
{
	bool reported = false;
	GLenum error;
	while (GL_NO_ERROR != (error = glGetError())) {
		logging_error("OpenGL error %u: %s", error, gluErrorString(error));
		reported = true;
	}
	return reported;
}

bool cegl_query_feature(cegl_feature feature)
{
	return features[feature];
}

int cegl_max_texture_size(void)
{
	GLint max_texture_size;
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max_texture_size);
	return max_texture_size;
}
