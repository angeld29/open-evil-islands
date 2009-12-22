#ifndef CE_GLLIB_H
#define CE_GLLIB_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

enum {
	GL_FEATURE_TEXTURE_COMPRESSION_S3TC,
	GL_FEATURE_COUNT
};

extern bool gl_query_feature(int feature);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CE_GLLIB_H */
