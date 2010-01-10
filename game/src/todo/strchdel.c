/*
**  STRDELCH.C - Removes specified character(s) from a string
**
**  public domain demo by Bob Stout
**
**  NOTE: The name of this funtion violates ANSI/ISO 9899:1990 sec. 7.1.3,
**        but this violation seems preferable to either violating sec. 7.13.8
**        or coming up with some hideous mixed-case or underscore infested
**        naming. Also, many SNIPPETS str---() functions duplicate existing
**        functions which are supported by various vendors, so the naming
**        violation may be required for portability.
*/

#include <stdio.h>
#include <string.h>
#include "snip_str.h"

char *strdelch(char *string, const char *lose)
{
      if (!string || !*string)
            return NULL;
      if (lose)
      {
            char *s;

            for (s = string; *s; ++s)
            {
                  if (strchr(lose, *s))
                  {
                        strMove(s, s + 1);
                        --s;
                  }
            }
      }
      return string;
}


/*
**  STRDEL.C - Removes specified characters from a string
**
**  public domain demo by Bob Stout, modifications by Dan Stubbs
**
**  NOTE: The name of this funtion violates ANSI/ISO 9899:1990 sec. 7.1.3,
**        but this violation seems preferable to either violating sec. 7.13.8
**        or coming up with some hideous mixed-case or underscore infested
**        naming. Also, many SNIPPETS str---() functions duplicate existing
**        functions which are supported by various vendors, so the naming
**        violation may be required for portability.
*/

#include <string.h>
#include "snip_str.h"

char *strdel(char *str, size_t posn, size_t len)
{
      size_t k;
      char *pos0, *pos1;

      if (str)
      {
            if (posn < strlen(str))
            {
                  pos0 = pos1 = str + posn;
                  k = strlen (pos1);
                  pos1 = (k >= len) ? pos1+len : pos1+k;
                  strMove(pos0, pos1);
            }
      }
      return str;
}

char* strdel(char *s, size_t offset, size_t count)
{
   size_t len = strlen(s);
   if(offset > len)
    // if offset beyond end of string -- just do nothing
      return s;
   if( (offset+count) > len)
        count = size - offset;
   strcpy(s+offset,s+offset+count);
   return s;
}
