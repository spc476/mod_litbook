
/******************************************************************
*
* soundex.c		- Library routines to implement the Soundex
*			  algorithm.
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

#include <ctype.h>
#include <string.h>
#include "soundex.h"

#ifdef DDT
#  define D(x)	x
#else
#  define D(x)
#  define NDEBUG
#endif
#include <assert.h>

/*************************************************************************/

static char *ignore = "AEIOUWYH";
static char *use[6] =
{
  "BPFV",
  "CSKGJQXZ",
  "DT",
  "L",
  "MN",
  "R"
};

/*************************************************************************/

SOUNDEX (Soundex)(char *word)
{
  SOUNDEX sdx;
  char	  c;
  char	  last;
  int	  idx  = 1;
  int	  i;

  assert(word != NULL);

  sdx.cval[0] = toupper(*word++);
  sdx.cval[1] = sdx.cval[2] = sdx.cval[3] = '0';
  last	      = sdx.cval[0];

Soundex_10:
  c = toupper(*word++);
  if (c == 0) goto Soundex_end;
  if (strchr(ignore,c) != NULL) goto Soundex_10;
  for (i = 0 ; i < 6 ; i++)
  {
    if (strchr(use[i],c) != NULL)
    {
      if (strchr(use[i],last) != NULL) goto Soundex_10;
      last = c;
      sdx.cval[idx++] = '1' + i;
      if (idx == 4) goto Soundex_end;
    }
  }
  goto Soundex_10;

Soundex_end:
  return(sdx);
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

