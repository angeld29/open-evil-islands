#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "cestr.h"

char* strleft(const char* s, size_t n)
{
	size_t l = strlen(s);
	if (n > l) {
		n = l;
	}
	char* p = (char*)malloc(n + 1);
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
	char* p = (char*)malloc(n + 1);
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
	char* p = (char*)malloc(n + 1);
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

#ifdef CE_NEED_STRDUP
char* strdup(const char* s)
{
	char* p = (char*)malloc(strlen(s) + 1);
	if (NULL != p) {
		strcpy(p, s);
	}
	return p;
}
#endif /* CE_NEED_STRDUP */

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

#ifdef CE_NEED_STRNDUP
char* strndup(const char* s, size_t n)
{
	char* p = (char*)malloc(n + 1);
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
