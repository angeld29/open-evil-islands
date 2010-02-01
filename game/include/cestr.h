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

#ifndef CE_STR_H
#define CE_STR_H

#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/// Duplicate string, returning an identical ce_alloc'd string.
extern char* cestrdup(const char* s);

/**
 *  Return a ce_alloc'd copy of at most n bytes of s. The
 *  resultant string is terminated even if no null terminator
 *  appears before s[n].
*/
extern char* cestrndup(const char* s, size_t n);

/// Convert a string into upper case in place.
extern char* cestrupr(char* s);

/// Convert a string into lower case in place.
extern char* cestrlwr(char* s);

/// Reverse a string in place.
extern char* cestrrev(char* s);

/**
 *  Replaces all occurrences of a character in a string with another character.
 *  Return number of replacements made.
*/
extern size_t cestrrepc(char* s, char from, char to);

/// Compare s1 and s2, ignoring case.
extern int cestrcasecmp(const char* s1, const char* s2);

/// Compare no more than n chars of s1 and s2, ignoring case.
extern int cestrncasecmp(const char* s1, const char* s2, size_t n);

/// Similar to strstr but this function ignores the case of both strings.
extern char* cestrcasestr(const char* haystack, const char* needle);

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
extern size_t cestrlcat(char* dst, const char* src, size_t size);

/**
 *  @brief Consistent, safe string copy.
 *  Based on http://www.gratisoft.us/todd/papers/strlcpy.html
 *
 *  Copy src to string dst of size size. At most size-1 characters
 *  will be copied. Always NULL terminates (unless size == 0).
 *  Return strlen(src); if retval >= size, truncation occurred.
*/
extern size_t cestrlcpy(char* dst, const char* src, size_t size);

/**
 *  Find the length of s, but scan at most n characters.
 *  If no '\0' terminator is found in that many characters, return n.
*/
extern size_t cestrnlen(const char* s, size_t n);

/// Find the last occurrence in string of any character in accept.
extern char* cestrrpbrk(const char* s, const char* accept);

/**
 *  Return the next delimited token from sp, terminating
 *  it with a '\0', and update sp to point past it.
*/
extern char* cestrsep(char** sp, const char* delim);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CE_STR_H */
