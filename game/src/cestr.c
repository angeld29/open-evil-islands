#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "celib.h"
#include "cestr.h"

char* strset(char* s, char c)
{
	for (char* p = s; *p; ++p) {
		*p = c;
	}
	return s;
}

char* strnset(char* s, char c, size_t n)
{
	if (0 == n) {
		return s;
	}
	for (char* p = s; n-- > 0 && *p; ++p) {
		*p = c;
	}
	return s;
}

char* strleft(const char* s, size_t n)
{
	size_t l = strlen(s);
	if (n > l) {
		n = l;
	}
	char* p = malloc(n + 1);
	if (NULL == p) {
		return NULL;
	}
	strncpy(p, s, n);
	p[n] = '\0';
	return p;
}

char* strright(const char* s, size_t n)
{
	size_t l = strlen(s);
	if (n > l) {
		n = l;
	}
	char* p = malloc(n + 1);
	if (NULL == p) {
		return NULL;
	}
	strcpy(p, s + l - n);
	return p;
}

char* strmid(const char* s, size_t pos, size_t n)
{
	size_t l = strlen(s);
	if (pos > l) {
		return NULL;
	}
	if (n > (l - pos)) {
		n = l - pos;
	}
	char* p = malloc(n + 1);
	strncpy(p, s + pos, n);
	p[n] = '\0';
	return p;
}

char* strupr(char* s)
{
	for (char* p = s; *p; ++p) {
		*p = toupper(*p);
	}
	return s;
}

char* strlwr(char* s)
{
	for (char* p = s; *p; ++p) {
		*p = tolower(*p);
	}
	return s;
}

char* strrev(char* s)
{
	if (!*s) {
		return s;
	}
	for (char *p1 = s, *p2 = s + strlen(s) - 1; p2 > p1; ++p1, --p2) {
		*p1 ^= *p2;
		*p2 ^= *p1;
		*p1 ^= *p2;
	}
	return s;
}

size_t strrepc(char* s, char from, char to)
{
	size_t n = 0;
	for (char* p = strchr(s, from); p; p = strchr(p + 1, from), ++n) {
		*p = to;
	}
	return n;
}

#ifdef CE_NEED_STRCASECMP
int strcasecmp(const char* s1, const char* s2)
{
	char c1, c2;
	do {
		c1 = tolower(*s1++);
		c2 = tolower(*s2++);
	} while (c1 && c1 == c2);
	return c1 - c2;
}
#endif /* CE_NEED_STRCASECMP */

#ifdef CE_NEED_STRCASESTR
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
#endif /* CE_NEED_STRCASESTR */

#ifdef CE_NEED_STRDUP
char* strdup(const char* s)
{
	char* p = malloc(strlen(s) + 1);
	if (NULL != p) {
		strcpy(p, s);
	}
	return p;
}
#endif /* CE_NEED_STRDUP */

#ifdef CE_NEED_STRLCAT
size_t strlcat(char* dst, const char* src, size_t size)
{
	char* d = dst;
	const char* s = src;
	size_t n = size;
	size_t dlen;

	// Find the end of dst and adjust bytes left but don't go past end.
	while (n-- != 0 && *d) {
		++d;
	}

	dlen = d - dst;
	n = size - dlen;

	if (0 == n) {
		return dlen + strlen(s);
	}

	while (*s) {
		if (1 != n) {
			*d++ = *s;
			--n;
		}
		++s;
	}
	*d = '\0';

	return dlen + (s - src); // Count does not include NULL.
}
#endif /* CE_NEED_STRLCAT */

#ifdef CE_NEED_STRLCPY
size_t strlcpy(char* dst, const char* src, size_t size)
{
	size_t srclen = strlen(src);
	if (0 != size) {
		size = --size < srclen ? size : srclen;
		strncpy(dst, src, size);
		dst[size] = '\0';
	}
	return srclen;
}
#endif /* CE_NEED_STRLCPY */

#ifdef CE_NEED_STRNCASECMP
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
#endif /* CE_NEED_STRNCASECMP */

#ifdef CE_NEED_STRNDUP
char* strndup(const char* s, size_t n)
{
	char* p = malloc(n + 1);
	if (NULL != p) {
		strncpy(p, s, n);
		p[n] = '\0';
	}
	return p;
}
#endif /* CE_NEED_STRNDUP */

#ifdef CE_NEED_STRNLEN
size_t strnlen(const char* s, size_t n)
{
	size_t i;
	for (i = 0; i < n && *s; ++i, ++s) {
	}
	return i;
}
#endif /* CE_NEED_STRNLEN */

#ifdef CE_NEED_STRRPBRK
char* strrpbrk(const char* s, const char* accept)
{
	const char* p;
	char *p0, *p1;
	for (p = accept, p0 = p1 = NULL; *p; ++p) {
		p1 = strrchr(s, *p);
		if (p1 && p1 > p0) {
			p0 = p1;
		}
	}
	return p0;
}
#endif /* CE_NEED_STRRPBRK */

#ifdef CE_NEED_STRSEP
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
#endif /* CE_NEED_STRSEP */
