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

#ifndef CE_STR_HPP
#define CE_STR_HPP

#include <cstddef>

namespace cursedearth
{
    /**
     *  Copies a string that contains the n leftmost characters of the s.
     *  The entire string is copied if n is greater than string length.
     */
    char* ce_strleft(char* dst, const char* src, size_t n);

    /**
     *  Copies a string that contains the n rightmost characters of the s.
     *  The entire string is copied if n is greater than string length.
     */
    char* ce_strright(char* dst, const char* src, size_t n);

    /**
     *  Copies a string, n characters long beginning at position pos.
     *  Returns a NULL if the pos exceeds the length of the string, the contents
     *  of the array remain unchanged. If there are less than n characters
     *  available in the string starting at the given position, the function
     *  copies all characters that are available from the specified position.
     */
    char* ce_strmid(char* dst, const char* src, size_t pos, size_t n);

    /// Remove all whitespace from the start and the end.
    char* ce_strtrim(char* dst, const char* src);

    /// Convert a string into upper case.
    char* ce_strupr(char* dst, const char* src);

    /// Convert a string into lower case.
    char* ce_strlwr(char* dst, const char* src);

    /// Compare s1 and s2, ignoring case.
    int ce_strcasecmp(const char* s1, const char* s2);

    /**
     *  @brief Consistent, safe string concatenation.
     *  Based on http://www.gratisoft.us/todd/papers/strlcpy.html
     *
     *  Appends src to string dst of size size (unlike strncat, size is the
     *  full size of dst, not space left). At most size-1 characters
     *  will be copied. Always NULL terminates (unless size <= strlen(dst)).
     *  Returns strlen(src) + MIN(size, strlen(initial dst)).
     *  If retval >= size, truncation occurred.
     */
    size_t ce_strlcat(char* dst, const char* src, size_t size);

    /**
     *  @brief Consistent, safe string copy.
     *  Based on http://www.gratisoft.us/todd/papers/strlcpy.html
     *
     *  Copy src to string dst of size size. At most size-1 characters
     *  will be copied. Always NULL terminates (unless size == 0).
     *  Return strlen(src); if retval >= size, truncation occurred.
     */
    size_t ce_strlcpy(char* dst, const char* src, size_t size);

    /// Find the last occurrence in string of any character in accept.
    const char* ce_strrpbrk(const char* s, const char* accept);

    /**
     *  Return the next delimited token from sp, terminating
     *  it with a '\0', and update sp to point past it.
     */
    char* ce_strsep(char** sp, const char* delim);
}

#endif
