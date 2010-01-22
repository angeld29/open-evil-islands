#ifndef CE_MOBFILE_H
#define CE_MOBFILE_H

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

typedef struct mobfile mobfile;

extern mobfile* mobfile_open(const char* path);
extern void mobfile_close(mobfile* mob);

extern void mobfile_debug_print(mobfile* mob);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CE_MOBFILE_H */
