#include <stddef.h>
#include <ctype.h>

#include "strcasestr.h"

char* strcasestr(const char* haystack, const char* needle)
{
	for (char *p = (char*)haystack, *startn = NULL, *np = NULL; *p; ++p) {
		if (np) {
			if (tolower(*p) == tolower(*np)) {
				if (!*++np) {
					return startn;
				}
			} else {
				np = NULL;
			}
		} else if (tolower(*p) == tolower(*needle)) {
			np = (char*)needle + 1;
			startn = p;
		}
	}
	return NULL;
}
