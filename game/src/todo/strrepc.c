/***************************************************************************
* @(#)strrepc
* @(#)      Replaces all occurrences of a character in a string with another
* @(#)      character
*
***************************************************************************/

#include <string.h>

int _CfnTYPE strrepc(char *pszStr, char cFrom, char cTo)
{
      char  *ptr ;                              /* Pointer to string */
      int   iReturn = 0 ;                       /* No of replacements */

      /*----------------------------------------------------------------*/

      while( 0 != ( ptr = strchr( pszStr, cFrom ) ) )

      {     /* WHILE cFrom occurs in pszStr */

            pszStr[ (int) ptr - (int) pszStr ] = cTo ;

            /*- Replace next cFrom with cTo */

            iReturn++ ;                                           /*- count */
      }

      return( iReturn ) ;
}
