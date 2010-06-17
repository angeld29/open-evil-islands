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

#include <string.h>
#include <ctype.h>

#include "celib.h"
#include "cestr.h"

char* ce_strleft(char* restrict dst, const char* restrict src, size_t n)
{
	n = ce_min(size_t, n, strlen(src));
	strncpy(dst, src, n);
	dst[n] = '\0';
	return dst;
}

char* ce_strright(char* restrict dst, const char* restrict src, size_t n)
{
	size_t len = strlen(src);
	return strcpy(dst, src + len - ce_min(size_t, n, len));
}

char* ce_strmid(char* restrict dst, const char* restrict src, size_t pos, size_t n)
{
	size_t len = strlen(src);
	if (pos > len) {
		return NULL;
	}
	n = ce_min(size_t, n, len - pos);
	strncpy(dst, src + pos, n);
	dst[n] = '\0';
	return dst;
}

char* ce_strtrim(char* restrict dst, const char* restrict src)
{
	size_t len = strlen(src);
	if (0 != len) {
		size_t first, last;
		for (first = 0; first < len && isspace(src[first]); ++first) {
		}
		for (last = len - 1; last > 0 && isspace(src[last]); --last) {
		}
		len = first <= last ? last - first + 1 : 0;
		strncpy(dst, src + first, len);
	}
	dst[len] = '\0';
	return dst;
}

char* ce_strupr(char* restrict dst, const char* restrict src)
{
	char* p = dst;
	while (*src) {
		*p++ = toupper(*src++);
	}
	*p = '\0';
	return dst;
}

char* ce_strlwr(char* restrict dst, const char* restrict src)
{
	char* p = dst;
	while (*src) {
		*p++ = tolower(*src++);
	}
	*p = '\0';
	return dst;
}

char* ce_strrev(char* restrict dst, const char* restrict src)
{
	char* p = dst;
	const char *s = src + strlen(src);
	while (s > src) {
		*p++ = *--s;
	}
	*p = '\0';
	return dst;
}

size_t ce_strrepc(char* s, char from, char to)
{
	size_t n = 0;
	for (char* p = strchr(s, from); p; p = strchr(p + 1, from), ++n) {
		*p = to;
	}
	return n;
}

int ce_strcasecmp(const char* s1, const char* s2)
{
	char c1, c2;
	do {
		c1 = tolower(*s1++);
		c2 = tolower(*s2++);
	} while (c1 && c1 == c2);
	return c1 - c2;
}

int ce_strncasecmp(const char* s1, const char* s2, size_t n)
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

char* ce_strcasestr(const char* haystack, const char* needle)
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

/*
 *  Based on:
 *  1. OpenBSD source.
 *     Copyright (C) 1998 Todd C. Miller <Todd.Miller@courtesan.com>.
*/
size_t ce_strlcat(char* restrict dst, const char* restrict src, size_t size)
{
	char* d = dst;
	const char* s = src;
	size_t n = size;
	size_t dlen;

	// find the end of dst and adjust bytes left but don't go past end
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

	return dlen + (s - src); // count does not include NULL
}

size_t ce_strlcpy(char* restrict dst, const char* restrict src, size_t size)
{
	size_t srclen = strlen(src);
	if (0 != size) {
		size = --size < srclen ? size : srclen;
		strncpy(dst, src, size);
		dst[size] = '\0';
	}
	return srclen;
}

size_t ce_strnlen(const char* s, size_t n)
{
	size_t i;
	for (i = 0; i < n && *s; ++i, ++s) {
	}
	return i;
}

char* ce_strrpbrk(const char* s, const char* accept)
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

char* ce_strsep(char** sp, const char* delim)
{
	char* s = *sp;
	char* p = s + strcspn(s, delim);
	if (*p) {
		*p++ = '\0';
	}
	*sp = p;
	return s;
}
