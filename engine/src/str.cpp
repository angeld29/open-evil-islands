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
#include <vector>

#include "lib.hpp"
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

    char* ce_strrep(char* dst, const char* src_orig, size_t size, const char* from, const char* to)
    {
        if (NULL == dst || NULL == src_orig || 0 == size || NULL == from || NULL == to) {
            return NULL;
        }

        size_t src_len = strlen(src_orig);
        if (0 == src_len) {
            dst[0] = '\0';
            return dst;
        }

        std::vector<char> src_buffer(src_len + 1);
        char* src = src_buffer.data();
        memcpy(src, src_orig, src_buffer.size());

        size_t from_len = strlen(from);
        if (0 == from_len) {
             ce_strlcpy(dst, src, size);
             return dst;
        }

        // Two-pass approach: figure out how much space to allocate for
        // the new string, pre-allocate it, then perform replacement(s).

        size_t count = 0;
        const char* src_pos = src;
        assert(from_len); // otherwise, strstr(src,from) will return src

        do {
            src_pos = strstr(src_pos, from);
            if (NULL != src_pos) {
                src_pos += from_len;
                ++count;
            }
        } while (src_pos);

        if (0 == count) {
            ce_strlcpy(dst, src, size);
            return dst;
        }

        // The following size arithmetic is extremely cautious, to guard against size_t overflows.
        assert(src_len >= count * from_len);
        assert(0 != count);

        size_t src_without_from_len = src_len - count * from_len;
        size_t to_len = strlen(to);
        size_t newstr_len = src_without_from_len + count * to_len;

        if ((0 != to_len && ((newstr_len <= src_without_from_len) || (newstr_len + 1 == 0))) || (size < newstr_len + 1)) {
            // overflow
            return NULL;
        }

        const char* start_substr = src;
        char* dst_pos = dst;
        for (size_t i = 0; i != count; ++i) {
            const char* end_substr = strstr(start_substr, from);
            assert(NULL != end_substr);
            size_t substr_len = end_substr - start_substr;
            memcpy(dst_pos, start_substr, substr_len);
            dst_pos += substr_len;
            memcpy(dst_pos, to, to_len);
            dst_pos += to_len;
            start_substr = end_substr + from_len;
        }

        // copy remainder of src, including trailing '\0'
        size_t remains = src_len - (start_substr - src) + 1;
        assert(dst_pos + remains == dst + newstr_len + 1);
        memcpy(dst_pos, start_substr, remains);
        assert(strlen(dst) == newstr_len);

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
