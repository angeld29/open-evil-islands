#include <assert.h>

#if __GNUC__ == 4 && __GNUC_MINOR__ == 2
// Hack for gcc 4.2
typedef unsigned long long __le64;
typedef unsigned long long __be64;
#endif
#include <asm/byteorder.h>

#include "byteorder.h"

uint16_t cpu2le16(uint16_t v)
{
	return __cpu_to_le16(v);
}

uint32_t cpu2le32(uint32_t v)
{
	return __cpu_to_le32(v);
}

uint64_t cpu2le64(uint64_t v)
{
	return __cpu_to_le64(v);
}

uint16_t le2cpu16(uint16_t v)
{
	return __le16_to_cpu(v);
}

uint32_t le2cpu32(uint32_t v)
{
	return __le32_to_cpu(v);
}

uint64_t le2cpu64(uint64_t v)
{
	return __le64_to_cpu(v);
}

uint16_t cpu2be16(uint16_t v)
{
	return __cpu_to_be16(v);
}

uint32_t cpu2be32(uint32_t v)
{
	return __cpu_to_be32(v);
}

uint64_t cpu2be64(uint64_t v)
{
	return __cpu_to_be64(v);
}

uint16_t be2cpu16(uint16_t v)
{
	return __be16_to_cpu(v);
}

uint32_t be2cpu32(uint32_t v)
{
	return __be32_to_cpu(v);
}

uint64_t be2cpu64(uint64_t v)
{
	return __be64_to_cpu(v);
}

void cpu2le16s(uint16_t* v)
{
	__cpu_to_le16s(v);
}

void cpu2le32s(uint32_t* v)
{
	__cpu_to_le32s(v);
}

void cpu2le64s(uint64_t* v)
{
	__cpu_to_le64s(v);
}

void le2cpu16s(uint16_t* v)
{
	__le16_to_cpus(v);
}

void le2cpu32s(uint32_t* v)
{
	__le32_to_cpus(v);
}

void le2cpu64s(uint64_t* v)
{
	__le64_to_cpus(v);
}

void cpu2be16s(uint16_t* v)
{
	__cpu_to_be16s(v);
}

void cpu2be32s(uint32_t* v)
{
	__cpu_to_be32s(v);
}

void cpu2be64s(uint64_t* v)
{
#if __GNUC__ == 4 && __GNUC_MINOR__ == 2
	assert(0);
#else
	__cpu_to_be64s(v);
#endif
}

void be2cpu16s(uint16_t* v)
{
	__be16_to_cpus(v);
}

void be2cpu32s(uint32_t* v)
{
	__be32_to_cpus(v);
}

void be2cpu64s(uint64_t* v)
{
#if __GNUC__ == 4 && __GNUC_MINOR__ == 2
	assert(0);
#else
	__be64_to_cpus(v);
#endif
}
