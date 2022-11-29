
/******************************************************************
*
* soundex.c             - Library routines to implement the Soundex
*                         algorithm.
*
* Copyright 2022 by Sean Conner.  All Rights Reserved.
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

#include <stdbool.h>
#include <ctype.h>
#include <string.h>

#include "soundex.h"

/*************************************************************************/

static bool inset(char const *s,char const *set)
{
  return ((*s != '\0') && (strchr(set,toupper(*s)) != NULL));
}

/*************************************************************************/

static char const *nextchar(char const *s,char const *set)
{
  while(inset(s,set))
    s++;
  return s;
}

/*************************************************************************/

static char const *ignore(char const *s)
{
  return nextchar(s,"AEIOUWYH'");
}

/*************************************************************************/

static char const *skip(char const *s)
{
  return nextchar(s,"HW");
}

/*************************************************************************/

static bool checkset(char const **ps,char const*set,char *d,char ret)
{
  char const *s = *ps;
  
  if (!inset(s,set))
    return false;
    
  s = nextchar(s,set);
  s = skip(s);
  if (inset(s,set))
    s = nextchar(s,set);
  *ps = s;
  *d  = ret;
  return true;
}

/*************************************************************************/

static bool cs1(char const **s,char *d)
{
  return checkset(s,"BFPV",d,'1');
}

/*************************************************************************/

static bool cs2(char const **s,char *d)
{
  return checkset(s,"CGJKQSXZ",d,'2');
}

/*************************************************************************/

static bool cs3(char const **s,char *d)
{
  return checkset(s,"DT",d,'3');
}

/*************************************************************************/

static bool cs4(char const **s,char *d)
{
  return checkset(s,"L",d,'4');
}

/*************************************************************************/

static bool cs5(char const **s,char *d)
{
  return checkset(s,"MN",d,'5');
}

/*************************************************************************/

static bool cs6(char const **s,char *d)
{
  return checkset(s,"R",d,'6');
}

/*************************************************************************/

static char const *use(char const *s,char *d)
{
  s = ignore(s);
  if (cs1(&s,d))
    return s;
  else if (cs2(&s,d))
    return s;
  else if (cs3(&s,d))
    return s;
  else if (cs4(&s,d))
    return s;
  else if (cs5(&s,d))
    return s;
  else if (cs6(&s,d))
    return s;
  else
  {
    *d = '0';
    return *s == '\0' ? s : ++s;
  }
}

/*************************************************************************/

static char const *initial(char const *s,char *d)
{
  char dummy;
  
  *d = toupper(*s);
  if (cs1(&s,&dummy))
    return s;
  else if (cs2(&s,&dummy))
    return s;
  else if (cs3(&s,&dummy))
    return s;
  else if (cs4(&s,&dummy))
    return s;
  else if (cs5(&s,&dummy))
    return s;
  else if (cs6(&s,&dummy))
    return s;
  else
    return ++s;
}

/*************************************************************************/

SOUNDEX (Soundex)(char const *name)
{
  SOUNDEX code;
  name = initial(name,&code.cval[0]);
  name = use(name,&code.cval[1]);
  name = use(name,&code.cval[2]);
         use(name,&code.cval[3]);
  return code;
}

/**********************************************************************/

int (SoundexCompare)(SOUNDEX s1,SOUNDEX s2)
{
  return(s1.value - s2.value);
}

/***********************************************************************/

int (SoundexEqu)(SOUNDEX s1,SOUNDEX s2)
{
  return(s1.value == s2.value);
}

/***********************************************************************/

char *(SoundexString)(char *dest,SOUNDEX sdx)
{
  dest[0] = sdx.cval[0];
  dest[1] = sdx.cval[1];
  dest[2] = sdx.cval[2];
  dest[3] = sdx.cval[3];
  dest[4] = '\0';
  return(dest);
}

/************************************************************************/
