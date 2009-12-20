#ifndef CE_STR_H
#define CE_STR_H

#include <string.h>

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/* Return a malloc'd string that contains the n leftmost characters of the s.
   The entire string is returned if n is greater than string length. */
extern char* strleft(const char* s, size_t n);

/* Return a malloc'd string that contains the n rightmost characters of the s.
   The entire string is returned if n is greater than string length. */
extern char* strright(const char* s, size_t n);

/* Return a malloc'd string, n characters long beginning at position pos.
   Return a NULL if the pos exceeds the length of the string.
   If there are less than n characters available in the string
   starting at the given position, the function returns all
   characters that are available from the specified position. */
extern char* strmid(const char* s, size_t pos, size_t n);

/* Convert a string into upper case in place. */
extern char* strupr(char* s);

/* Convert a string into lower case in place. */
extern char* strlwr(char* s);

/* Reverse a string in place. */
extern char* strrev(char* s);

#ifdef CE_NEED_STRCASECMP
/* Compare s1 and s2, ignoring case. */
extern int strcasecmp(const char* s1, const char* s2);
#endif /* CE_NEED_STRCASECMP */

#ifdef CE_NEED_STRCASESTR
/* Similar to strstr but this function ignores the case of both strings. */
extern char* strcasestr(const char* haystack, const char* needle);
#endif /* CE_NEED_STRCASESTR */

#ifdef CE_NEED_STRDUP
/* Duplicate string, returning an identical malloc'd string. */
extern char* strdup(const char* s);
#endif /* CE_NEED_STRDUP */

#ifdef CE_NEED_STRLCAT
/* strlcpy and strlcat - consistent, safe, string copy and concatenation.
   Based on http://www.gratisoft.us/todd/papers/strlcpy.html

   Appends src to string dst of size size (unlike strncat, size is the
   full size of dst, not space left). At most size-1 characters
   will be copied. Always NULL terminates (unless size <= strlen(dst)).
   Returns strlen(src) + MIN(size, strlen(initial dst)).
   If retval >= size, truncation occurred. */
extern size_t strlcat(char* dst, const char* src, size_t size);
#endif /* CE_NEED_STRLCAT */

#ifdef CE_NEED_STRLCPY
/* strlcpy and strlcat - consistent, safe, string copy and concatenation.
   Based on http://www.gratisoft.us/todd/papers/strlcpy.html

   Copy src to string dst of size size. At most size-1 characters
   will be copied. Always NULL terminates (unless size == 0).
   Return strlen(src); if retval >= size, truncation occurred. */
extern size_t strlcpy(char* dst, const char* src, size_t size);
#endif /* CE_NEED_STRLCPY */

#ifdef CE_NEED_STRNCASECMP
/* Compare no more than n chars of s1 and s2, ignoring case. */
extern int strncasecmp(const char* s1, const char* s2, size_t n);
#endif /* CE_NEED_STRNCASECMP */

#ifdef CE_NEED_STRNDUP
/* Return a malloc'd copy of at most n bytes of s. The
   resultant string is terminated even if no null terminator
   appears before s[n]. */
extern char* strndup(const char* s, size_t n);
#endif /* CE_NEED_STRNDUP */

#ifdef CE_NEED_STRNLEN
/* Find the length of s, but scan at most n characters.
   If no '\0' terminator is found in that many characters, return n. */
extern size_t strnlen(const char* s, size_t n);
#endif /* CE_NEED_STRNLEN */

#ifdef CE_NEED_STRRPBRK
/* Find the last occurrence in string of any character in accept. */
extern char* strrpbrk(const char* s, const char* accept);
#endif /* CE_NEED_STRRPBRK */

#ifdef CE_NEED_STRSEP
/* Return the next delimited token from sp, terminating
   it with a '\0', and update sp to point past it. */
extern char* strsep(char** sp, const char* delim);
#endif /* CE_NEED_STRSEP */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CE_STR_H */
