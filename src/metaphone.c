
/********************************************************************
*
* metaphone.c           - Library routines to implement the Metaphone
*                         algorithm.
*
* Placed into the public domain by Gary A. Parker.
* Transcribed into machine readable format by Mark Grosberg from
* the C Gazette Jun/Jul 1991, Volume 5, No 4. Pg #55
*
*********************************************************************/

#include <ctype.h>
#include <string.h>

#include "metaphone.h"

static char vsfn[26] =
{
   1,16,4,16,9,2,4,16,9,2,0,2,2,2,1,4,0,2,4,4,1,0,0,0,8,0
/* A  B C  D E F G  H I J K L M N O P Q R S T U V W X Y Z */
};

/* Testing macros */
#define vowel(x)   (vsfn[(x) - 'A'] & 1)
#define same(x)    (vsfn[(x) - 'A'] & 2)
#define varson(x)  (vsfn[(x) - 'A'] & 4)
#define frontv(x)  (vsfn[(x) - 'A'] & 8)
#define noghf(x)   (vsfn[(x) - 'A'] & 16)

#define MAX_WORD_BUF   32

bool make_metaphone(char *word, char *metaph, int maxmet)
{
  char *n, *n_start, *n_end;
  char *metaph_end;
  char  ntrans[MAX_WORD_BUF];
  bool  KSFlag;
  
  memset(metaph,0,maxmet);
  /* Transform word to upper case and remove non-alpha characters */
  for(n = ntrans + 1, n_end = ntrans + MAX_WORD_BUF - 2;
      (*word != '\0') && (n < n_end);
      word++)
   if (isalpha(*word))
      *n++ = toupper(*word);
      
  if (n == ntrans + 1)
      return false;
      
  /* Begin preprocessing */
  n_end = n;
  *n++  = '\0';
  *n    = '\0';
  n     = ntrans + 1;
  
  switch (*n)
   {
     case 'P': case 'K': case 'G':
      if (n[1] == 'N')
         *n++ = '\0';
      break;
      
     case 'A':
      if (n[1] == 'E')
         *n++ = '\0';
      break;
      
     case 'W':
      if (n[1] == 'R')
         *n++ = '\0';
      else if (n[1] == 'H')     /* bug fix - 19991121.1056 spc */
       {
         n[1] = *n;
        *n++  = '\0';
       }
      break;
      
     case 'X':
      *n = 'S';
      break;
   }
   
  /* Now, iterate over the string, stopping at the end of the string or
   * when we have computed sufficient characters.
   */
  KSFlag = false;
  for(metaph_end = metaph + maxmet, n_start = n;
      (n <= n_end) && (metaph < metaph_end);
      n++)
   {
     if (KSFlag)
      {
        KSFlag    = false;
        *metaph++ = *n;
      }
     else
      {
        /* drop duplicates except for 'CC' */
        if ((n[-1] == *n) && (*n != 'C'))
            continue;
            
        if (same(*n) || ((n == n_start) && vowel(*n)))
          *metaph++ = *n;
        else
          switch (*n)
           {
             case 'B':
               if ((n < n_end) || (n[-1] != 'M'))
                  *metaph++ = *n;
               break;
               
             case 'C':
               if ((n[-1] != 'S') || (!frontv(n[1])))
                {
                  if ((n[1] == 'I') || (n[2] == 'A'))
                    *metaph++ = 'X';
                  else if (frontv(n[1]))
                    *metaph++ = 'S';
                  else if (n[1] == 'H')
                    *metaph++ = (((n == n_start) && (!vowel(n[2]))) ||
                                 (n[-1] == 'S')) ? 'K' : 'X';
                  else
                    *metaph++ = 'K';
                }
               break;
               
             case 'D':
               *metaph++ = ((n[1] == 'G') && frontv(n[2])) ? 'J' : 'T';
               break;
               
             case 'G':
               if (((n[1]  != 'H') || vowel(n[2])) &&
                   ((n[1]  != 'N') || ((n + 1) < n_end &&
                   ((n[2]  != 'E') || n[3] != 'D'))) &&
                   ((n[-1] != 'D') || !frontv(n[1])))
                     *metaph = (frontv(n[1]) && (n[2] != 'G')) ? 'J' : 'K';
               else if (n[1] == 'H' && !noghf(n[-1]) && (n[-4]) != 'H')
                     *metaph = 'F';
               break;
               
             case 'H':
               if (!varson(n[-1]) && (!vowel(n[-1]) || vowel(n[1])))
                 *metaph++ = 'H';
               break;
               
             case 'K':
               if (n[-1] != 'C')
                 *metaph++ = 'K';
               break;
               
             case 'P':
               *metaph++ = (n[1] == 'H') ? 'F' : 'P';
               break;
               
             case 'Q':
               *metaph++ = 'K';
               break;
               
             case 'S':
               *metaph++ = ((n[1] == 'H') || ((n[1] == 'I') &&
                      (((n[2] == 'O') || (n[2] == 'A'))))) ? 'X' : 'S';
               break;
               
             case 'T':
               if ((n[1] == 'I') && ((n[2] == 'O') || (n[2] == 'A')))
                 *metaph++ = 'X';
               else if (n[1] == 'H')
                 *metaph++ = 'O';
               else if ((n[1] != 'C') || (n[2] != 'H'))
                 *metaph++ = 'T';
               break;
               
             case 'V':
               *metaph++ = 'F';
               break;
               
             case 'W':
             case 'Y':
               if (vowel(n[1]))
                 *metaph++ = *n;
               break;
               
             case 'X':
               if (n == n_start)
                 *metaph++ = 'S';
               else
                {
                  *metaph++ = 'K';
                  KSFlag    = true;
                }
               break;
               
              case 'Z':
               *metaph++ = 'S';
               break;
               
           }
      }
      
   }
   
  *metaph = '\0';
  return true;
}

