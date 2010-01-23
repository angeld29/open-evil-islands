/*
 *  This file is part of Cursed Earth.
 *
 *  Cursed Earth is an open source, cross-platform port of Evil Islands.
 *  Copyright (C) 2009-2010 Yanis Kurganov.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef CE_BYTEORDER_H
#define CE_BYTEORDER_H

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

extern uint16_t cecpu2le16(uint16_t v);
extern uint32_t cecpu2le32(uint32_t v);
extern uint64_t cecpu2le64(uint64_t v);

extern uint16_t cele2cpu16(uint16_t v);
extern uint32_t cele2cpu32(uint32_t v);
extern uint64_t cele2cpu64(uint64_t v);

extern uint16_t cecpu2be16(uint16_t v);
extern uint32_t cecpu2be32(uint32_t v);
extern uint64_t cecpu2be64(uint64_t v);

extern uint16_t cebe2cpu16(uint16_t v);
extern uint32_t cebe2cpu32(uint32_t v);
extern uint64_t cebe2cpu64(uint64_t v);

extern void cecpu2le16s(uint16_t* v);
extern void cecpu2le32s(uint32_t* v);
extern void cecpu2le64s(uint64_t* v);

extern void cele2cpu16s(uint16_t* v);
extern void cele2cpu32s(uint32_t* v);
extern void cele2cpu64s(uint64_t* v);

extern void cecpu2be16s(uint16_t* v);
extern void cecpu2be32s(uint32_t* v);
extern void cecpu2be64s(uint64_t* v);

extern void cebe2cpu16s(uint16_t* v);
extern void cebe2cpu32s(uint32_t* v);
extern void cebe2cpu64s(uint64_t* v);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CE_BYTEORDER_H */
