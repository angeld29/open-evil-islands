/*
**  TRIM.C - Remove leading, trailing, & excess embedded spaces
*/

#include <ctype.h>

char *trim (char *str)
{
      char *ibuf, *obuf;

      if (str)
      {
            for (ibuf = obuf = str; *ibuf; )
            {
                  while (*ibuf && (isspace (*ibuf)))
                        ibuf++;
                  if (*ibuf && (obuf != str))
                        *(obuf++) = ' ';
                  while (*ibuf && (!isspace (*ibuf)))
                        *(obuf++) = *(ibuf++);
            }
            *obuf = NUL;
      }
      return (str);
}



/*
**
**  remove leading whitespace from a string
*/

#include <ctype.h>
#include <string.h>

char *rmlead(char *str)
{
      char *obuf;

      if (str)
      {
            for (obuf = str; *obuf && isspace(*obuf); ++obuf)
                  ;
            if (str != obuf)
                  strMove(str, obuf);
      }
      return str;
}



/*
**  remove trailing whitespace from a string
*/

#include <string.h>
#include <ctype.h>

char *rmtrail(char *str)
{
      int i;

      if (str && 0 != (i = strlen(str)))
      {
            while (--i >= 0)
            {
                  if (!isspace(str[i]))
                        break;
            }
            str[++i] = NUL;
      }
      return str;
}

/*
**  remove all whitespace from a string
*/

#include <stdio.h>
#include <ctype.h>
#include "snip_str.h"

char *rmallws(char *str)
{
      char *obuf, *nbuf;

      if (str)
      {
            for (obuf = str, nbuf = str; *obuf; ++obuf)
            {
                  if (!isspace(*obuf))
                        *nbuf++ = *obuf;
            }
            *nbuf = NUL;
      }
      return str;
}


#include <string.h>
#include <ctype.h>
#include "bacstd.h"

/*
 /-------------------------------------\
|  STRTRIMR                             |------------------------------------|
|\-------------------------------------/
|
| Removes all trailing blanks from a string.
| Blanks are defined with ISSPACE  (blank, tab, newline, return, formfeed,
| vertical tab = 0x09 - 0x0D + 0x20)
|
|----------------------------------------------------------------------------|
| CALL:
|    strtrimr(&str);
|
| HEADER:
|    ctype.h
|
| GLOBALE VARIABLES:
|    %
|
| ARGUMENTS:
|    pszStr      : String to be converted
|
| PROTOTYPE:
|    int _CfnTYPE strtrimr(char *pszStr);
|
| RETURN VALUE:
|    j-i         : No of removed blanks
|
| MODULE:
|    strtrim.c
|----------------------------------------------------------------------------|
|1994-01-08/Bac
|   All characters is checked (">" -> ">=").
|
|
|
|----------------------------------------------------------------------------|
|1992-11-09/Erik Bachmann
\---------------------------------------------------------------------------|*/
int _CfnTYPE strtrimr(char *pszStr)
{
      int   i, j;                               /* Local counters */

      /*-------------------------------------------------*/

      j = i = strlen(pszStr) - 1; /* Calculate the length of the string */

      while (isspace(pszStr[i]) && (i >= 0))

            /* WHILE string ends with a blank */
            /*1994-01-08/Bac Even if all chars are blanks (= 0) */

            pszStr[ i-- ] = '\0';               /*- Replace blank with '\0' */

      return(j - i);                            /* Return no of replacements */
}

/*
 /-------------------------------------\
|  STRTRIML                             |------------------------------------|
|\-------------------------------------/
|
| Removes all leading blanks from a string.
| Blanks are defined with ISSPACE  (blank, tab, newline, return, formfeed,
| vertical tab = 0x09 - 0x0D + 0x20)
|
|
|----------------------------------------------------------------------------|
| CALL:
|    strtriml(&str);
|
| HEADER:
|    ctype.h
|
| GLOBALE VARIABLES:
|    %
|
| ARGUMENTS:
|    pszStr      : String to be converted
|
| PROTOTYPE:
|    int _CfnTYPE strtriml(char *pszStr);
|
| RETURN VALUE:
|    i           : No of removed blanks
|
| MODULE:
|    strtrim.c
|----------------------------------------------------------------------------|
|
|
|----------------------------------------------------------------------------|
|1992-11-09/Erik Bachmann
\---------------------------------------------------------------------------|*/

int _CfnTYPE strtriml(char *pszStr)
{
      int   i = 0, j;                                 /* Local counters */

      /*-------------------------------------------------*/

      j = strlen(pszStr) - 1; /* Calculate the length of the string */

      while (isspace(pszStr[i]) && (i <= j))

            /* WHILE string starts with a blank */

            i++;                          /*- Count no of leading blanks */

      if (0 < i)                          /* IF leading blanks are found */
            strcpy(pszStr, &pszStr[i]);   /*- Shift string to the left */

      return(i);                          /* Return no of replacements */
}

/*
 /-------------------------------------\
|  STRTRIM                              |------------------------------------|
|\-------------------------------------/
| Removes all leading and trailing blanks in a string.
| Blanks are defined with ISSPACE  (blank, tab, newline, return, formfeed,
| vertical tab = 0x09 - 0x0D + 0x20)
|
|
|----------------------------------------------------------------------------|
| CALL:
|    strtrim(&str);
|
| HEADER:
|    ctype.h
|
| GLOBALE VARIABLES:
|    %
|
| ARGUMENTS:
|    pszStr      : String to be converted
|
| PROTOTYPE:
|    int _CfnTYPE strtrim(char *pszStr);
|
| RETURN VALUE:
|    iBlank      : No of removed blanks
|
| MODULE:
|    strtrim.c
|----------------------------------------------------------------------------|
|
|
|
|----------------------------------------------------------------------------|
|1992-11-09/Erik Bachmann
\---------------------------------------------------------------------------|*/

int _CfnTYPE strtrim(char *pszStr)
{
      int   iBlank;

      /*-------------------------------------------------*/

      iBlank  = strtrimr(pszStr);               /* Remove trailing blanks */
      iBlank += strtriml(pszStr);               /* Remove leading blanks */

      return(iBlank);
}



/***************************************************************************
* @(#)strtrimc.c
* @(#)strtrimcr
* @(#)      Removes all trailing occurences of specific characters in a string.
* @(#)strtrimcl
* @(#)      Removes all leading occurences of specific characters in a string.
* @(#)strtrimc
* @(#)      Removes all leading and traling occurences of specific characters
* @(#)      in a string.
*
***************************************************************************
*@(#)1993 Erik Bachmann
*
* Released to public domain 27-Oct-95
***************************************************************************/

#include <string.h>
#include "bacstd.h"

/*
 /-------------------------------------\
|  STRTRIMCR                            |------------------------------------|
|\-------------------------------------/
|
| Removes all trailing occurences of specific characters in a string.
|
|
|----------------------------------------------------------------------------|
| CALL:
|    strtrimcr(&str, ":;\\");
|
| HEADER:
|    ctype.h
|
| GLOBALE VARIABLES:
|    %
|
| ARGUMENTS:
|    pszStr      : String to be converted
|    pszSet      : String with the characters to remove
|
|
| PROTOTYPE:
|    int _CfnTYPE strtrimcr(char *szStr, char *szSet);
|
| RETURN VALUE:
|    j-i         : No of removed characters
|
| MODULE:
|    strtrim.c
|----------------------------------------------------------------------------|
|
|
|----------------------------------------------------------------------------|
|1992-11-09/Erik Bachmann
\---------------------------------------------------------------------------|*/

int _CfnTYPE strtrimcr(char *szStr, char *szSet)
{
      int   i, j;                                     /* Locale counters */

      /*-------------------------------------------------*/

      j = i = strlen(szStr) - 1;                /* Find length of string */

      while (strrchr(szSet, szStr[ i ])
                  && (0 <= i))
      {
            /* While string is terminated by one of the specified characters */
            szStr[ i-- ] = '\0';          /*- Replace character with '\0' */
      }

      return(j - i);    /* Return the difference between old and new length */
}

/*
 /-------------------------------------\
|  STRTRIMCL                            |------------------------------------|
|\-------------------------------------/
|
| Removes all leading occurences of a specific character in a string.
|
|----------------------------------------------------------------------------|
| CALL:
|    strtrimcl(&str, ";:\\");
|
| HEADER:
|    ctype.h
|
| GLOBALE VARIABLES:
|    %
|
| ARGUMENTS:
|    pszStr      : String to be converted
|    pszSet      : String with the characters to remove
|
| PROTOTYPE:
|    int _CfnTYPE strtrimcl(char *szStr, char *szSet);
|
| RETURN VALUE:
|    i           : No of removed characters
|
| MODULE:
|    strtrim.c
|----------------------------------------------------------------------------|
|
|
|----------------------------------------------------------------------------|
|1992-11-09/Erik Bachmann
\---------------------------------------------------------------------------|*/

int _CfnTYPE strtrimcl(char *szStr, char *szSet)
{
      int   i = 0, j;

      /*-------------------------------------------------*/

      j = strlen(szStr) - 1;                    /* Find length of string */

      while (strrchr(szSet, szStr[ i ])
                  && (i <= j))
      {
            /* While first character in string matches tag */

            i++;                    /*- Count no of removed chars */
      }

      if (0 < i)                    /* IF there were matches */
            strcpy(szStr, &szStr[ i ]);         /*- shift string to the left */

      return(i);                    /* Return no of matching chars */
}

/*
 /-------------------------------------\
|  STRTRIMC                             |------------------------------------|
|\-------------------------------------/
|
| Removes all leading and trailing occurences of a specific character in
|  a string.
|
|----------------------------------------------------------------------------|
| CALL:
|    strtrimc(&str, ";:\\");
|
| HEADER:
|    ctype.h
|
| GLOBALE VARIABLES:
|    %
|
| ARGUMENTS:
|    pszStr      : String to be converted
|    pszSet      : String with the characters to remove
|
| PROTOTYPE:
|    int _CfnTYPE strtrimc(char *szStr, char *szSet);
|
| RETURN VALUE:
|    iStatusFlag : No of removed characters
|
| MODULE:
|    strtrimc.c
|----------------------------------------------------------------------------|
|
|
|
|----------------------------------------------------------------------------|
|1992-11-09/Erik Bachmann
\---------------------------------------------------------------------------|*/

int _CfnTYPE strtrimc(char *szStr, char *szSet)
{
      int   iStatusFlag;

      /*-------------------------------------------------*/

      iStatusFlag =  strtrimcl(szStr, szSet);
      iStatusFlag += strtrimcr(szStr, szSet);

      return(iStatusFlag);
}

/*
 /-------------------------------------\
|  REP_LAST_CHAR                        |------------------------------------|
|\-------------------------------------/
|
| Replaces the last char on match with another specified char.
|
|
|----------------------------------------------------------------------------|
| CALL:
|    rep_last_char(str, '\n', '\0');
|
| HEADER:
|    string.h
|
| GLOBALE VARIABLES:
|    %
|
| ARGUMENTS:
|    pszStr      : String to be converted
|    cChar1      : Character to replace
|    cChar2      : Character to replace with
|
|
| PROTOTYPE:
|    int _CfnTYPE rep_last_char(char *szStr, char cChar1, char cChar2);
|
| RETURN VALUE:
|    int         : Stringlength
|
| MODULE:
|    strtrim.c
|----------------------------------------------------------------------------|
|
|
|----------------------------------------------------------------------------|
|1992-11-09/Erik Bachmann
\---------------------------------------------------------------------------|*/

int _CfnTYPE rep_last_char(char *pszStr, char cChar1, char cChar2)
{
      int   i;

      /*--------------------------------*/

      i = strlen(pszStr) - 1;

      if (pszStr[ i ] == cChar1)
            pszStr[ i ] = cChar2;

      return(i);
}     /*** rep_last_char() ***/

