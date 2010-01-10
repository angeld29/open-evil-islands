#include <ctype.h>

#include "strncasecmp.h"

int strncasecmp(const char* s1, const char* s2, size_t n)
{
	char c1, c2;
	if (0 == n) {
		return 0;
	}
	do {
		c1 = tolower(*s1++);
		c2 = tolower(*s2++);
	} while (--n > 0 && c1 && c1 == c2);
	return c1 - c2;
}
