
/******************************************************************
*
* testmod.c             - Program to interactively test the algorithms
*                         used in mod_litbook.
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
* -----------------------------------------------------------------
*
* History:
*
* 19991122.1712 1.0.0   spc
*       Initial release
*
********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <ctype.h>
#include <assert.h>

#include <unistd.h>

#include "soundex.h"
#include "metaphone.h"

#define BUMPSIZE        10

/***************************************************************/

struct bookname
{
  char    *s1;
  char    *s2;
  SOUNDEX  sdx;
  char    *mp;
};

struct bookrequest
{
  char   *name;
  size_t  c1;
  size_t  v1;
  size_t  c2;
  size_t  v2;
  int     redirect;
};

/*************************************************************/

static void      translate_request      (struct bookrequest *,char *);
static void      read_booklist          (char *);
static void      print_request          (struct bookrequest *);
static int       show_chapter           (size_t,size_t,size_t);
static int       sort_abrev             (const void *,const void *);
static int       sort_fullname          (const void *,const void *);
static int       sort_sounds            (const void *,const void *);
static int       sort_metaphone         (const void *,const void *);
static int       find_abrev             (const void *,const void *);
static int       find_fullname          (const void *,const void *);
static int       find_sounds            (const void *,const void *);
static int       find_metaphone         (const void *,const void *);
static char     *trim_lspace            (char *);
static char     *trim_tspace            (char *);
static char     *trim_space             (char *);
static char     *m_strdup               (char *);

static void      dump_list              (struct bookname **);

/*************************************************************/

static struct bookname   *books     = NULL;
static struct bookname  **abrev     = NULL;
static struct bookname  **fullname  = NULL;
static struct bookname  **sounds    = NULL;
static struct bookname  **metaphone = NULL;
static size_t             maxbook   = 0;

/************************************************************/

int main(int argc,char *argv[])
{
  char               buffer[BUFSIZ];
  struct bookrequest br;
  
  if (argc < 3)
  {
    fprintf(stderr,"%s <booklist> <bookdir>\n",argv[0]);
    exit(1);
  }
  
  read_booklist(argv[1]);
  dump_list(abrev);
#if 0
  dump_list(fullname);
  dump_list(sounds);
#endif
  chdir(argv[2]);
  
  while(fgets(buffer,sizeof(buffer),stdin))
  {
    char *p = strchr(buffer,'\n'); if (p) *p = '\0';
    
    translate_request(&br,buffer);
    if (br.name == NULL)
    {
      printf("error in request\n");
      continue;
    }
#if 1
    print_request(&br);
#else
    if (br.redirect)
      printf("%s->",buffer);
    printf(
            "%s.%lu:%lu-%lu:%lu found\n",
            br.name,
            (unsigned long)br.c1,
            (unsigned long)br.v1,
            (unsigned long)br.c2,
            (unsigned long)br.v2
          );
    /*print_request(&br);*/
#endif
  }
  
  return(0);
}

/********************************************************************/

static void translate_request(struct bookrequest *pbr,char *r)
{
  char    buffer[BUFSIZ];
  char   *p;
  size_t  size;
  struct bookname **pres;
  
  assert(pbr != NULL);
  assert(r   != NULL);
  
  pbr->name     = NULL;
  pbr->c1       = 1;
  pbr->v1       = 1;
  pbr->c2       = INT_MAX;
  pbr->v2       = INT_MAX;
  pbr->redirect = 0;
  
  for (p = buffer , size = 0; (*r) && (!ispunct(*r)) ; r++ , size++)
  {
    *p++ = tolower(*r);
    *p   = '\0';
  }
  
  if (size < 2) return;
  if (isdigit(buffer[0]))
    buffer[1] = toupper(buffer[1]);
  else
    buffer[0] = toupper(buffer[0]);
    
  pres = bsearch(buffer,fullname,maxbook,sizeof(struct bookname *),find_fullname);
  if (pres == NULL)
  {
    pbr->redirect = 1;
    pres = bsearch(buffer,abrev,maxbook,sizeof(struct bookname *),find_abrev);
    if (pres == NULL)
    {
      SOUNDEX sdx = Soundex(buffer);
      
      pres = bsearch(&sdx,sounds,maxbook,sizeof(struct bookname *),find_sounds);
      if (pres == NULL)
      {
        char mp[BUFSIZ];
        int  rc;
        
        rc = make_metaphone(buffer,mp,BUFSIZ);
        if (rc == 0) return;
        pres = bsearch(mp,metaphone,maxbook,sizeof(struct bookname *),find_metaphone);
        if (pres == NULL) return;
      }
    }
  }
  
  pbr->name = (*pres)->s2;
  
  if (*r == '\0') return;               /* 1. G or G- */
  if (*r == '-')  return;
  
  pbr->c1 = strtoul(r+1,&r,10);
  if (*r == '\0')                       /* 2. G.a */
  {
    pbr->c2 = pbr->c1;
    return;
  }
  
  if ((*r == '.') || (*r == ':'))
  {
    pbr->v1 = strtoul(r+1,&r,10);       /* 3. G.a.b */
    if (*r++ != '-')
    {
      pbr->c2 = pbr->c1;
      pbr->v2 = pbr->v1;
      return;
    }
    if (!isdigit(*r)) return;           /* 4. G.a.b- */
    
    pbr->c2 = strtoul(r,&r,10);         /* 5. G.a.b-x */
    if ((*r != '.') && (*r != ':'))
    {
      pbr->v2 = pbr->c2;
      pbr->c2 = pbr->c1;
      return;
    }
    
    pbr->v2 = strtoul(r+1,&r,10);       /* 6. G.a.b-x.y */
    return;
  }
  
  if (*r++ != '-')                      /* 2. G.a */
  {
    pbr->c2 = pbr->c1;
    return;
  }
  
  if (!isdigit(*r)) return;             /* 7. G.a- */
  
  pbr->c2 = strtoul(r,&r,10);           /* 8. G.a-x */
  if ((*r != '.') && (*r != ':')) return;
  
  pbr->v2 = strtoul(r+1,&r,10);         /* 9. G.a-x.y */
}

/********************************************************************/

static void print_request(struct bookrequest *pbr)
{
  int    rc;
  size_t i;
  
  assert(pbr != NULL);
  
  rc = chdir(pbr->name);
  if (rc != 0)
  {
    printf("error\n");
    return;
  }
  
  printf("\n%s\n",pbr->name);
  
  if (pbr->c1 == pbr->c2)
    show_chapter(pbr->c1,pbr->v1,pbr->v2);
  else
  {
    for (i = pbr->c1 ; i <= pbr->c2 ; i++ )
    {
      if (i == pbr->c1)
      {
        if (show_chapter(i,pbr->v1,INT_MAX)) break;
      }
      else if (i == pbr->c2)
      {
        if (show_chapter(i,1,pbr->v2)) break;
      }
      else
      {
        if (show_chapter(i,1,INT_MAX)) break;
      }
    }
  }
  
  chdir("..");
}

/********************************************************************/

static int show_chapter(size_t chapter,size_t vlow,size_t vhigh)
{
  long int *iarray;
  FILE     *fp;
  char      fname[BUFSIZ];
  long      max;
  long      i;
  long      s;
  char     *p;
  
  assert(chapter > 0);
  assert(vlow    > 0);
  
  sprintf(fname,"%lu.index",(unsigned long)chapter);
  fp = fopen(fname,"rb");
  
  if (fp == NULL) return(1);
  
  fread(&max,sizeof(max),1,fp);
  if (max < 1)
  {
    fclose(fp);
    return(1);
  }
  
  if (vlow > max)
  {
    fclose(fp);
    return(1);
  }
  
  if (vhigh > max) vhigh = max;
  
  iarray = malloc((max + 1) * sizeof(long));
  if (iarray == NULL)
  {
    fclose(fp);
    return(1);
  }
  
  fread(iarray,sizeof(long),max + 1,fp);
  fclose(fp);
  
  sprintf(fname,"%lu",(unsigned long)chapter);
  fp = fopen(fname,"rb");
  
  printf("Chapter %lu\n\n",(unsigned long)chapter);
  if (vlow > 1)
    printf("\t.\n\t.\n\t.\n");
    
  for (i = vlow ; i <= vhigh ; i++)
  {
    s = iarray[i] - iarray[i-1];
    p = malloc(s);
    if (p == NULL)
    {
      fclose(fp);
      free(iarray);
      return(0);
    }
    
    fseek(fp,iarray[i-1],SEEK_SET);
    fread(p,sizeof(char),s,fp);
    printf("%lu. ",(unsigned long)i);
    fwrite(p,sizeof(char),s,stdout);
    printf("\n\n");
    free(p);
  }
  fclose(fp);
  free(iarray);
  return(0);
}

/********************************************************************/

static void read_booklist(char *fname)
{
  size_t  size = 0;
  FILE   *fp;
  char    buffer[BUFSIZ];
  int     i;
  
  assert(fname != NULL);
  
  fp = fopen(fname,"r");
  if (fp == NULL) return;
  
  while(fgets(buffer,BUFSIZ,fp))
  {
    char *s1;
    char *s2;
    char *s11;
    char *s22;
    char  mp[BUFSIZ];
    
    if (maxbook == size)
    {
      size  += BUMPSIZE;
      books  = realloc(books,size * sizeof(struct bookname));
    }
    
    s1 = strtok(buffer,",");
    s2 = strtok(NULL,",\n");
    assert(s1 != NULL);
    assert(s2 != NULL);
    
    s1 = trim_space(s1);
    s2 = trim_space(s2);
    s11 = m_strdup(s1);
    s22 = m_strdup(s2);
    
    books[maxbook].s1  = s11;
    books[maxbook].s2  = s22;
    books[maxbook].sdx = isdigit(*s22) ? Soundex(s22+1) : Soundex(s22) ;
    make_metaphone(s22,mp,sizeof(mp));
    books[maxbook].mp  = m_strdup(mp);
    maxbook++;
  }
  
  abrev     = malloc(maxbook * sizeof(struct bookname *));
  fullname  = malloc(maxbook * sizeof(struct bookname *));
  sounds    = malloc(maxbook * sizeof(struct bookname *));
  metaphone = malloc(maxbook * sizeof(struct bookname *));
  
  for (i = 0 ; i < maxbook ; i++)
    abrev[i] = fullname[i] = sounds[i] = metaphone[i] = &books[i];
    
  qsort(abrev,    maxbook,sizeof(struct bookname *),sort_abrev);
  qsort(fullname, maxbook,sizeof(struct bookname *),sort_fullname);
  qsort(sounds,   maxbook,sizeof(struct bookname *),sort_sounds);
  qsort(metaphone,maxbook,sizeof(struct bookname *),sort_metaphone);
}

/**************************************************************/

static int sort_abrev(const void *o1,const void *o2)
{
  const struct bookname *pbn1;
  const struct bookname *pbn2;
  
  assert(o1 != NULL);
  assert(o2 != NULL);
  
  pbn1 = *((struct bookname **)o1);
  pbn2 = *((struct bookname **)o2);
  
  return(strcmp(pbn1->s1,pbn2->s1));
}

/*****************************************************************/

static int sort_fullname(const void *o1,const void *o2)
{
  const struct bookname *pbn1;
  const struct bookname *pbn2;
  
  assert(o1 != NULL);
  assert(o2 != NULL);
  
  pbn1 = *((struct bookname **)o1);
  pbn2 = *((struct bookname **)o2);
  
  return(strcmp(pbn1->s2,pbn2->s2));
}

/*******************************************************************/

static int sort_sounds(const void *o1,const void *o2)
{
  const struct bookname *pbn1;
  const struct bookname *pbn2;
  
  assert(o1 != NULL);
  assert(o2 != NULL);
  
  pbn1 = *((struct bookname **)o1);
  pbn2 = *((struct bookname **)o2);
  
  return(SoundexCompare(pbn1->sdx,pbn2->sdx));
}

/********************************************************************/

static int sort_metaphone(const void *o1,const void *o2)
{
  const struct bookname *pbn1;
  const struct bookname *pbn2;
  
  assert(o1 != NULL);
  assert(o2 != NULL);
  
  pbn1 = *((struct bookname **)o1);
  pbn2 = *((struct bookname **)o2);
  
  return(strcmp(pbn1->mp,pbn2->mp));
}

/********************************************************************/

static int find_abrev(const void *key,const void *datum)
{
  const struct bookname *pbn = *((struct bookname **)datum);
  const char            *k   = key;
  
  assert(pbn != NULL);
  assert(k   != NULL);
  
  return(strcmp(k,pbn->s1));
}

/*******************************************************************/

static int find_fullname(const void *key,const void *datum)
{
  const struct bookname *pbn = *((struct bookname **)datum);
  const char            *k   = key;
  
  assert(pbn != NULL);
  assert(k   != NULL);
  
  return(strcmp(k,pbn->s2));
}

/********************************************************************/

static int find_sounds(const void *key,const void *datum)
{
  const struct bookname *pbn = *((struct bookname **)datum);
  const SOUNDEX         *k   = key;
  
  assert(pbn != NULL);
  assert(k   != NULL);
  
  return(SoundexCompare(*k,pbn->sdx));
}

/**********************************************************************/

static int find_metaphone(const void *key,const void *datum)
{
  const struct bookname *pbn = *((struct bookname **)datum);
  const char            *k   = key;
  
  assert(pbn != NULL);
  assert(k   != NULL);
  
  return(strcmp(k,pbn->mp));
}

/***********************************************************************/

static char *trim_lspace(char *s)
{
  assert(s != NULL);
  for ( ; (*s) && (isspace(*s)) ; s++)
    ;
  return(s);
}

/******************************************************************/

static char *trim_tspace(char *s)
{
  char *p;
  
  assert(s != NULL);
  for (p = s + strlen(s) - 1 ; (p > s) && (isspace(*p)) ; p--)
    ;
  *(p+1) = '\0';
  return(s);
}

/*******************************************************************/

static char *trim_space(char *s)
{
  assert(s != NULL);
  return(trim_tspace(trim_lspace(s)));
}

/*******************************************************************/

static char *m_strdup(char *s)
{
  char *t;
  
  assert(s != NULL);
  
  t = malloc(strlen(s) + 1);
  if (t) strcpy(t,s);
  return(t);
}

/*******************************************************************/

static void dump_list(struct bookname **pa)
{
  int  i;
  char buffer[BUFSIZ];
  
  assert(pa != NULL);
  
  printf("Dump list\n");
  for (i = 0 ; i < maxbook ; i++)
  {
    printf(
            "\t%s\t, %s(%s)[%s]\n",
            pa[i]->s1,
            pa[i]->s2,
            SoundexString(buffer,pa[i]->sdx),
            pa[i]->mp
          );
  }
}

