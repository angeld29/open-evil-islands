#ifndef CE_CEGL_H
#define CE_CEGL_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

typedef enum {
	GL_FEATURE_TEXTURE_NON_POWER_OF_TWO,
	GL_FEATURE_TEXTURE_RECTANGLE,
	GL_FEATURE_TEXTURE_COMPRESSION_S3TC,
	GL_FEATURE_COUNT
} gl_feature;

extern void gl_init(void);
extern bool gl_report_errors(void);

extern bool gl_query_feature(gl_feature feature);
extern int gl_max_texture_size(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CE_CEGL_H */
