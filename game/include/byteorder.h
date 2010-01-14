#ifndef CE_BYTEORDER_H
#define CE_BYTEORDER_H

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

extern uint16_t cpu2le16(uint16_t v);
extern uint32_t cpu2le32(uint32_t v);
extern uint64_t cpu2le64(uint64_t v);

extern uint16_t le2cpu16(uint16_t v);
extern uint32_t le2cpu32(uint32_t v);
extern uint64_t le2cpu64(uint64_t v);

extern uint16_t cpu2be16(uint16_t v);
extern uint32_t cpu2be32(uint32_t v);
extern uint64_t cpu2be64(uint64_t v);

extern uint16_t be2cpu16(uint16_t v);
extern uint32_t be2cpu32(uint32_t v);
extern uint64_t be2cpu64(uint64_t v);

extern void cpu2le16s(uint16_t* v);
extern void cpu2le32s(uint32_t* v);
extern void cpu2le64s(uint64_t* v);

extern void le2cpu16s(uint16_t* v);
extern void le2cpu32s(uint32_t* v);
extern void le2cpu64s(uint64_t* v);

extern void cpu2be16s(uint16_t* v);
extern void cpu2be32s(uint32_t* v);
extern void cpu2be64s(uint64_t* v);

extern void be2cpu16s(uint16_t* v);
extern void be2cpu32s(uint32_t* v);
extern void be2cpu64s(uint64_t* v);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CE_BYTEORDER_H */
