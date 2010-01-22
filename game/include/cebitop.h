#ifndef CE_BITOP_H
#define CE_BITOP_H

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

extern uint8_t cebitset(uint8_t v, size_t p);
extern uint8_t cebitclr(uint8_t v, size_t p);
extern uint8_t cebitflp(uint8_t v, size_t p);
extern bool cebittst(uint8_t v, size_t p);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CE_BITOP_H */
