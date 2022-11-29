
/******************************************************************
*
* util.c                - Miscellaneous routines I've found useful
*
* Copyright 1999 by Sean Conner.  All Rights Reserved.
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*
* Comments, questions and criticisms can be sent to: sean@conman.org
*
********************************************************************/

#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdarg.h>
#include <assert.h>

#include "util.h"

/***************************************************************************/

char *spc_getenv(char *env)
{
  char *p;
  char *ret;
  
  assert(env != NULL);
  
  p = getenv(env);
  if (p == NULL)
  {
    p = "";
  }
  
  ret = dup_string(p);
  return(ret);
}

/************************************************************************/

char *up_string(char *s)
{
  char *r = s;
  
  assert(s != NULL);
  
  for ( ; *s ; s++)
    *s = toupper(*s);
  return(r);
}

/***************************************************************************/

char *down_string(char *s)
{
  char *r = s;
  
  assert(s != NULL);
  
  for ( ; *s ; s++)
    *s = tolower(*s);
  return(r);
}

/*************************************************************************/

char *dup_string(char *s)
{
  char *r;
  
  assert(s != NULL);
  
  r = malloc(strlen(s) + 1);
  if (r == NULL) return(NULL);
  strcpy(r,s);
  return(r);
}

/***************************************************************************/

int empty_string(char *s)
{
  assert(s != NULL);
  
  for ( ; *s ; s++)
  {
    if (isprint(*s)) return(FALSE);
  }
  return(TRUE);
}

/*************************************************************************/

char *remove_ctrl(char *s)
{
  assert(s != NULL);
  
  return (remove_char(s,iscntrl));
}

/*************************************************************************/

char *remove_char(char *s,int (*tstchar)(int))
{
  char *ret = s;
  char *d   = s;
  
  assert(s != NULL);
  assert(tstchar);      /* can function pointers be compared to NULL? */
  
  for ( ; *s ; s++)
  {
    if (!(*tstchar)(*s)) *d++ = *s;
  }
  *d = '\0';
  return(ret);
}

/************************************************************************/

char *trim_lspace(char *s)
{
  assert(s != NULL);
  for ( ; (*s) && (isspace(*s)) ; s++)
    ;
  return(s);
}

/*************************************************************************/

char *trim_tspace(char *s)
{
  char *p;
  
  assert(s != NULL);
  for (p = s + strlen(s) - 1 ; (p > s) && (isspace(*p)) ; p--)
    ;
  *(p+1) = '\0';
  return(s);
}

/**************************************************************************/

char *trim_space(char *s)
{
  assert(s != NULL);
  return(trim_tspace(trim_lspace(s)));
}

/***************************************************************************/

int ctohex(char c)
{
  assert(isxdigit(c));
  
  c -= '0';
  if (c > 9) c -= 7;
  assert(c >= 0);       /* warning on AIX - apparently chars are unsigned */
  assert(c <  16);
  return(c);
}

/***************************************************************************/

char hextoc(int i)
{
  char c;
  
  assert(i >= 0);
  assert(i <  16);
  
  c = i+'0';
  if (c > '9') c += 7;
  assert(isxdigit(c));
  return(c);
}

/**************************************************************************/

char *cat_string(char *dest,char *src)
{
  assert(dest  != NULL);
  assert(src   != NULL);
  assert(*dest == '\0');
  
  for ( ; (*dest = *src) ; dest++ , src++ )
    ;
  return(dest);
}

/**********************************************************************/

