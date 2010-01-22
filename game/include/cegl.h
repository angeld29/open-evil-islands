#ifndef CE_GL_H
#define CE_GL_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

typedef enum {
	CEGL_FEATURE_TEXTURE_NON_POWER_OF_TWO,
	CEGL_FEATURE_TEXTURE_RECTANGLE,
	CEGL_FEATURE_TEXTURE_COMPRESSION_S3TC,
	CEGL_FEATURE_COUNT
} cegl_feature;

extern void cegl_init(void);
extern bool cegl_report_errors(void);

extern bool cegl_query_feature(cegl_feature feature);
extern int cegl_max_texture_size(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CE_GL_H */
