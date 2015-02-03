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
    extern char* ce_strleft(char* dst, const char* src, size_t n);

    /**
     *  Copies a string that contains the n rightmost characters of the s.
     *  The entire string is copied if n is greater than string length.
     */
    extern char* ce_strright(char* dst, const char* src, size_t n);

    /**
     *  Copies a string, n characters long beginning at position pos.
     *  Returns a NULL if the pos exceeds the length of the string, the contents
     *  of the array remain unchanged. If there are less than n characters
     *  available in the string starting at the given position, the function
     *  copies all characters that are available from the specified position.
     */
    extern char* ce_strmid(char* dst, const char* src, size_t pos, size_t n);

    /// Remove all whitespace from the start and the end.
    extern char* ce_strtrim(char* dst, const char* src);

    /// Convert a string into upper case.
    extern char* ce_strupr(char* dst, const char* src);

    /// Convert a string into lower case.
    extern char* ce_strlwr(char* dst, const char* src);

    /// Reverse a string.
    extern char* ce_strrev(char* dst, const char* src);

    /**
     *  C substring replacement.
     *  Based on https://gist.github.com/dhess/975639
     *
     *  This function returns string 'src' if string 'from' is an empty string, or
     *  if 'from' is not found in 'src'. If 'from' is found in 'src', the function
     *  returns a string whose contents are identical to 'src', except that all
     *  occurrences of 'from' in the original string 'src' are, in the new string,
     *  replaced by the string 'to'.
     *
     *  Strings 'src', 'from', and 'to' must all be null-terminated strings. If any
     *  of 'src', 'from', or 'to' are NULL, the function returns NULL, indicating an
     *  error condition. If any other error occurs, the function returns NULL.
     */
    extern char* ce_strrep(char* dst, const char* src, size_t size, const char* from, const char* to);

    /**
     *  Replaces all occurrences of a character in a string with another character.
     *  Return number of replacements made.
     */
    extern size_t ce_strrepc(char* s, char from, char to);

    /// Compare s1 and s2, ignoring case.
    extern int ce_strcasecmp(const char* s1, const char* s2);

    /// Compare no more than n chars of s1 and s2, ignoring case.
    extern int ce_strncasecmp(const char* s1, const char* s2, size_t n);

    /// Similar to strstr but this function ignores the case of both strings.
    extern char* ce_strcasestr(const char* haystack, const char* needle);

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
    extern size_t ce_strlcat(char* dst, const char* src, size_t size);

    /**
     *  @brief Consistent, safe string copy.
     *  Based on http://www.gratisoft.us/todd/papers/strlcpy.html
     *
     *  Copy src to string dst of size size. At most size-1 characters
     *  will be copied. Always NULL terminates (unless size == 0).
     *  Return strlen(src); if retval >= size, truncation occurred.
     */
    extern size_t ce_strlcpy(char* dst, const char* src, size_t size);

    /**
     *  Find the length of s, but scan at most n characters.
     *  If no '\0' terminator is found in that many characters, return n.
     */
    extern size_t ce_strnlen(const char* s, size_t n);

    /// Find the last occurrence in string of any character in accept.
    extern char* ce_strrpbrk(const char* s, const char* accept);

    /**
     *  Return the next delimited token from sp, terminating
     *  it with a '\0', and update sp to point past it.
     */
    extern char* ce_strsep(char** sp, const char* delim);
}

#endif
