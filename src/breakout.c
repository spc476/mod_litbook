
/******************************************************************
*
* breakout.c		- Program to read the Project Gutenberg
*			  King James Bible and break it into a format
*			  for use by mod_litbook
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
* History
*
* 20060713.1713 1.0.1	spc
*	Strip spaces from filenames
*
* 19991122.1706	1.0.0	spc
*	Initial release.
*
********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "types.h"
#include "util.h"
#include "nodelist.h"

/*****************************************************************/

typedef struct atom
{
  Node  node;
  Size  number;
  char *name;
  Size  size;
} *Atom;

typedef struct molecule
{
  struct atom base;
  Size        entries;
  List        sections;
} *Molecule;

/**************************************************************/

Atom		 AtomCreate		(Size,Size,char *);
void		 AtomDestroy		(Atom);
Molecule	 MoleculeCreate		(Size,char *);
void		 MoleculeDestroy	(Molecule);
Molecule	 Genesis		(FILE *);
void		 BookWrite		(Molecule);
void		 ChaptersWrite		(Molecule);
char		*my_fgets		(FILE *);
void		 setdir			(char *);
void		 resetdir		(void);

/************************************************************/

int main(int argc,char *argv[])
{
  Molecule bible;

  bible = Genesis(stdin);

#if 1
  
  BookWrite(bible);
  
#else
  {
    Molecule book;
    Molecule chapter;
    
    for (
          book = (Molecule)ListGetHead(&bible->sections) ;
          NodeValid(&book->base.node) ;
          book = (Molecule)NodeNext(&book->base.node)
        )
    {
      printf("%s: %lu chapters\n",book->base.name,(unsigned long)book->entries);
      for (
            chapter = (Molecule)ListGetHead(&book->sections) ;
            NodeValid(&chapter->base.node) ;
            chapter = (Molecule)NodeNext(&chapter->base.node)
          )
      {
        printf(
                "\tChapter %lu\t%5lu Verses\n",
                (unsigned long)chapter->base.number,
                (unsigned long)chapter->entries
              );
      }
    }
  }
#endif

  /*MoleculeDestroy(bible);*/
  printf("done\n");
  return(0);
}

/**************************************************************/

Atom AtomCreate(Size size,Size number,char *name)
{
  Atom  atom;
  char *s;
  char *d;
  
  assert(size >  0);
  assert(name != NULL);
  
  atom         = malloc(size);
  atom->number = number;
  atom->name   = dup_string(name);
  atom->size   = size;

  /*-----------------------------------
  ; strip spaces from the atom name
  ;----------------------------------*/

  for (s = d = atom->name ; *s ; s++)
    if (!isspace(*s))
      *d++ = *s;
  *d = '\0';

  return(atom);
}

/***********************************************************/

void AtomDestroy(Atom atom)
{
  assert(atom != NULL);
  free(atom->name);
  free(atom);
}

/***********************************************************/

Molecule MoleculeCreate(Size number,char *name)
{
  Molecule molecule;
  
  assert(number >  0);
  assert(name   != NULL);
  
  molecule = (Molecule)AtomCreate(sizeof(struct molecule),number,name);
  molecule->entries = 0;
  ListInit(&molecule->sections);
  return(molecule);
}

/*************************************************************/

void MoleculeDestroy(Molecule molecule)
{
  Molecule m;
  
  for(
       m = (Molecule)ListRemHead(&molecule->sections) ;
       NodeValid(&m->base.node) ;
       m = (Molecule)ListRemHead(&molecule->sections)
     )
  {
    if (m->base.size == sizeof(struct molecule))
      MoleculeDestroy(m);
    else
      AtomDestroy(&m->base);
  }
  AtomDestroy(&m->base);
}

/*************************************************************/

Molecule Genesis(FILE *fpin)
{
  Molecule  bible;
  Molecule  book;
  Molecule  chapter;
  Atom      verse;
  Size      bnum;
  Size      cnum;
  Size      vnum;
  Size      curchap;
  char     *buffer;
  
  assert(fpin != NULL);
  
  bible = MoleculeCreate(1,"Bible");
  
  while((buffer = my_fgets(fpin)) != NULL)
  {
    if (strncmp(buffer,"Book ",5) == 0)
    {
      curchap = SIZET_MAX;
      bnum    = strtoul(&buffer[5],NULL,10);
      book    = MoleculeCreate(bnum,&buffer[8]);
      bible->entries++;
      ListAddTail(&bible->sections,&book->base.node);
    }
    else
    {
      cnum = strtoul(buffer,NULL,10);
      vnum = strtoul(&buffer[4],NULL,10);
      
      if (cnum != curchap)
      {
        curchap = cnum;
        chapter = MoleculeCreate(cnum,"");
        book->entries++;
        ListAddTail(&book->sections,&chapter->base.node);
      }
      
      verse = AtomCreate(sizeof(struct atom),vnum,&buffer[8]);
      chapter->entries++;
      ListAddTail(&chapter->sections,&verse->node);
      
      free(buffer);
    }
  }
  
  return(bible);
}

/****************************************************************/

char *my_fgets(FILE *fpin)
{
  static char  rbuffer[BUFSIZ];
  static char  sbuffer[65536UL];
  char        *p;

  memset(rbuffer,0,sizeof(rbuffer));
  memset(sbuffer,0,sizeof(sbuffer));
  
  for (p = sbuffer ; ; )
  {
    if (fgets(rbuffer,sizeof(rbuffer),fpin) == NULL) break;
    if (empty_string(rbuffer)) 
    {
      return(dup_string(&sbuffer[1]));
    }

    p += sprintf(p," %s",remove_ctrl(trim_space(rbuffer)));
  }
  
  return(NULL);
}

/********************************************************************/

void BookWrite(Molecule bible)
{
  Molecule book;
  
  assert(bible != NULL);
  
  for (
        book = (Molecule)ListGetHead(&bible->sections);
        NodeValid(&book->base.node);
        book = (Molecule)NodeNext(&book->base.node)
      )
  {
    setdir(book->base.name);
    ChaptersWrite(book);
    resetdir();
  }
}

/*******************************************************************/

void ChaptersWrite(Molecule book)
{
  Molecule  chapter;
  
  assert(book != NULL);
  
  for (
        chapter = (Molecule)ListGetHead(&book->sections);
        NodeValid(&chapter->base.node);
        chapter = (Molecule)NodeNext(&chapter->base.node)
      )
  {
    Atom      verse;
    FILE     *fp;
    char      fname[BUFSIZ];
    long int *offs;
    size_t    idx;
    
    offs = malloc((chapter->entries + 2) * sizeof(long));
    
    sprintf(fname,"%lu",(unsigned long)chapter->base.number);
    fp = fopen(fname,"wb");
    
    offs[0] = (long)chapter->entries;
    
    for (
          idx = 1 , verse = (Atom)ListGetHead(&chapter->sections);
          NodeValid(&verse->node);
          verse = (Atom)NodeNext(&verse->node) , idx++
        )
    {
      offs[idx] = ftell(fp);
      fwrite(verse->name,sizeof(char),strlen(verse->name),fp);
    }
    
    offs[idx] = ftell(fp);
    fclose(fp);
    
    sprintf(fname,"%lu.index",(unsigned long)chapter->base.number);
    fp = fopen(fname,"wb");
    fwrite(offs,sizeof(long int),chapter->entries + 2,fp);
    fclose(fp);
    
    free(offs);
  }
}

/*******************************************************************/

void setdir(char *dir)
{
  int rc;
  
  assert(dir != NULL);
  
  rc = chdir(dir);
  if (rc != 0)
  {
    rc = mkdir(dir,0777);	/* no proto in Linux */
    if (rc != 0) 
    {
      perror("can't create dir---aborting");
      exit(1);
    }
    rc = chdir(dir);
    if (rc != 0)
    {
      perror("can't deal with dir---aborting");
      exit(1);
    }
  }
}

/********************************************************************/

void resetdir(void)
{
  int rc = chdir("..");
  if (rc != 0)
  {
    perror("can't get parent");
    exit(1);
  }
}

/********************************************************************/
