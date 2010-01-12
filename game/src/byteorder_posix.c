#include <assert.h>

#if __GNUC__ == 4 && __GNUC_MINOR__ == 2
// Hack for gcc 4.2
typedef unsigned long long __le64;
typedef unsigned long long __be64;
#endif
#include <asm/byteorder.h>

#include "byteorder.h"

uint16_t cpu2le16(uint16_t val)
{
	return __cpu_to_le16(val);
}

uint32_t cpu2le32(uint32_t val)
{
	return __cpu_to_le32(val);
}

uint64_t cpu2le64(uint64_t val)
{
	return __cpu_to_le64(val);
}

uint16_t le2cpu16(uint16_t val)
{
	return __le16_to_cpu(val);
}

uint32_t le2cpu32(uint32_t val)
{
	return __le32_to_cpu(val);
}

uint64_t le2cpu64(uint64_t val)
{
	return __le64_to_cpu(val);
}

uint16_t cpu2be16(uint16_t val)
{
	return __cpu_to_be16(val);
}

uint32_t cpu2be32(uint32_t val)
{
	return __cpu_to_be32(val);
}

uint64_t cpu2be64(uint64_t val)
{
	return __cpu_to_be64(val);
}

uint16_t be2cpu16(uint16_t val)
{
	return __be16_to_cpu(val);
}

uint32_t be2cpu32(uint32_t val)
{
	return __be32_to_cpu(val);
}

uint64_t be2cpu64(uint64_t val)
{
	return __be64_to_cpu(val);
}

void cpu2le16s(uint16_t* val)
{
	__cpu_to_le16s(val);
}

void cpu2le32s(uint32_t* val)
{
	__cpu_to_le32s(val);
}

void cpu2le64s(uint64_t* val)
{
	__cpu_to_le64s(val);
}

void le2cpu16s(uint16_t* val)
{
	__le16_to_cpus(val);
}

void le2cpu32s(uint32_t* val)
{
	__le32_to_cpus(val);
}

void le2cpu64s(uint64_t* val)
{
	__le64_to_cpus(val);
}

void cpu2be16s(uint16_t* val)
{
	__cpu_to_be16s(val);
}

void cpu2be32s(uint32_t* val)
{
	__cpu_to_be32s(val);
}

void cpu2be64s(uint64_t* val)
{
#if __GNUC__ == 4 && __GNUC_MINOR__ == 2
	assert(0);
#else
	__cpu_to_be64s(val);
#endif
}

void be2cpu16s(uint16_t* val)
{
	__be16_to_cpus(val);
}

void be2cpu32s(uint32_t* val)
{
	__be32_to_cpus(val);
}

void be2cpu64s(uint64_t* val)
{
#if __GNUC__ == 4 && __GNUC_MINOR__ == 2
	assert(0);
#else
	__be64_to_cpus(val);
#endif
}
