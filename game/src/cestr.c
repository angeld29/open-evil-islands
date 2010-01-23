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

#include "cealloc.h"
#include "cestr.h"

char* cestrdup(const char* s)
{
	char* p = cealloc(strlen(s) + 1);
	if (NULL != p) {
		strcpy(p, s);
	}
	return p;
}

char* cestrndup(const char* s, size_t n)
{
	char* p = cealloc(n + 1);
	if (NULL != p) {
		strncpy(p, s, n);
		p[n] = '\0';
	}
	return p;
}

char* cestrupr(char* s)
{
	for (char* p = s; *p; ++p) {
		*p = toupper(*p);
	}
	return s;
}

char* cestrlwr(char* s)
{
	for (char* p = s; *p; ++p) {
		*p = tolower(*p);
	}
	return s;
}

char* cestrrev(char* s)
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

size_t cestrrepc(char* s, char from, char to)
{
	size_t n = 0;
	for (char* p = strchr(s, from); p; p = strchr(p + 1, from), ++n) {
		*p = to;
	}
	return n;
}

int cestrcasecmp(const char* s1, const char* s2)
{
	char c1, c2;
	do {
		c1 = tolower(*s1++);
		c2 = tolower(*s2++);
	} while (c1 && c1 == c2);
	return c1 - c2;
}

int cestrncasecmp(const char* s1, const char* s2, size_t n)
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

char* cestrcasestr(const char* haystack, const char* needle)
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
 *  Based on OpenBSD source.
 *  Copyright (C) 1998 Todd C. Miller <Todd.Miller@courtesan.com>.
*/
size_t cestrlcat(char* dst, const char* src, size_t size)
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

size_t cestrlcpy(char* dst, const char* src, size_t size)
{
	size_t srclen = strlen(src);
	if (0 != size) {
		size = --size < srclen ? size : srclen;
		strncpy(dst, src, size);
		dst[size] = '\0';
	}
	return srclen;
}

size_t cestrnlen(const char* s, size_t n)
{
	size_t i;
	for (i = 0; i < n && *s; ++i, ++s) {
	}
	return i;
}

char* cestrrpbrk(const char* s, const char* accept)
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

char* cestrsep(char** sp, const char* delim)
{
	char* s = *sp;
	char* p = s + strcspn(s, delim);
	if (*p) {
		*p++ = '\0';
	}
	*sp = p;
	return s;
}
