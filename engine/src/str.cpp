/*
 *  This file is part of Cursed Earth.
 *
 *  Cursed Earth is an open source, cross-platform port of Evil Islands.
 *  Copyright (C) 2009-2015 Yanis Kurganov <ykurganov@users.sourceforge.net>
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

#include <cassert>
#include <cstring>
#include <cctype>
#include <algorithm>

#include "str.hpp"

namespace cursedearth
{
    char* ce_strleft(char* dst, const char* src, size_t n)
    {
        n = std::min(n, strlen(src));
        strncpy(dst, src, n);
        dst[n] = '\0';
        return dst;
    }

    char* ce_strright(char* dst, const char* src, size_t n)
    {
        size_t len = strlen(src);
        return strcpy(dst, src + len - std::min(n, len));
    }

    char* ce_strmid(char* dst, const char* src, size_t pos, size_t n)
    {
        size_t len = strlen(src);
        if (pos > len) {
            return NULL;
        }
        n = std::min(n, len - pos);
        strncpy(dst, src + pos, n);
        dst[n] = '\0';
        return dst;
    }

    char* ce_strtrim(char* dst, const char* src)
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

    char* ce_strupr(char* dst, const char* src)
    {
        char* p = dst;
        while (*src) {
            *p++ = toupper(*src++);
        }
        *p = '\0';
        return dst;
    }

    char* ce_strlwr(char* dst, const char* src)
    {
        char* p = dst;
        while (*src) {
            *p++ = tolower(*src++);
        }
        *p = '\0';
        return dst;
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

    size_t ce_strlcat(char* dst, const char* src, size_t size)
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

    size_t ce_strlcpy(char* dst, const char* src, size_t size)
    {
        size_t srclen = strlen(src);
        if (0 != size) {
            size = --size < srclen ? size : srclen;
            strncpy(dst, src, size);
            dst[size] = '\0';
        }
        return srclen;
    }

    const char* ce_strrpbrk(const char* s, const char* accept)
    {
        const char* p;
        const char *p0, *p1;
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
        if (NULL == s) {
            return NULL;
        }
        char* p = s + strcspn(s, delim);
        if (*p) {
            *p++ = '\0';
            *sp = p;
        } else {
            *sp = NULL;
        }
        return s;
    }
}
