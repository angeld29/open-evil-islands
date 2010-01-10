#include <string.h>

#include "strsep.h"

char* strsep(char** sp, const char* delim)
{
	char* s = *sp;
	char* p = s + strcspn(s, delim);
	if (*p) {
		*p++ = '\0';
	}
	*sp = p;
	return s;
}
