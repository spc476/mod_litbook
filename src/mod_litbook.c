
/******************************************************************
*
* mod_litbook.c		- Apache module for serving up literature
*			  according to certain conventions. 
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
* 19991127.1800	1.0.8	spc/myg	(bugfix)
*	Just goes to show that sometimes simple changes often times
*	aren't (this relates to the changes for 1.0.4).  
*
*	It seems that clt_linecount() was counting the lengths of lines
*	just a bit too good and passing that value in to fgets() resulted
*	in a line being read that was exactly that length would result
*	in the line being stored without the trailing '\n', which would
*	then cause strtok() to choke, breaking out earlier (see change
*	for 1.0.5).  
*
*	Also, if the last line of the translation file does contain a 
*	newline, an empty line is read in causing things to get out of
*	whack yet again.  So added code to check for an empty line,
*	adjusting the maximum book count appropriately.  Added the routine
*	empty_string() for that (to handle what may appear to be a blank
*	line but one that may actually contain whitespace).
*
*	Also return an error if the number of lines read in is not the
*	expected number of lines to be read in.
*
* 19991127.1711	1.0.7	spc/myg	(bugfix)
*	1. Fixed problem in clt_linecount() where if the last line
*	   doesn't end in a '\n' and is the longest line, the wrong
*	   longest line length would be returned. (thanks myg).
*
*	2. in config_litbooktrans(), the metaphone value was being
*	   corrupted (pointing to a locally allocated array instead
*	   of making a copy) (thank myg).
*
* 19991127.0545	1.0.6	spc	(bugfix)
*	If the server isn't on port 80, redirects were not being properly
*	redirected.  Fixed, but wonder about the cleanliness of the fix.
*	Can I actually look at r->server or not?  I assume so ... 
*
*	Also found the proper include file to get the prototype to mkdir().
*
* 19991126.2214	1.0.5	spc	(bugfix)
*	You know, I should TEST changes before commiting them.  Especially
*	SIMPLE THIS CAN'T BREAK type changes.
*
*	Basically, the change to 1.0.3->1.0.4 introduced two bugs.  The
*	first was related to the second but kept the second one hidden because
*	of a core dump.
*
*	The first was in config_litbooktrans() as I'm parsing the file.
*	The strtok() function was returning NULL when I wasn't expecting
*	it to return NULL at all.  Now I check the return code (gotta
*	remember---data is coming in from external source that I (as the
*	program) don't control so checking the data is paramount).
*
*	The second has to do with the changes made in the 1.0.3 to 1.0.4
*	transition.  As per comments from Mark I rewrote the code to count
*	lines but didn't handle the end of the file properly.  It does
*	now.
*
*	All of this came about when azagthoth@azagthoth-reborn.com had
*	problems with the webserver crashing after installing the module.
*	I first thought it might have been the Linux 2.2 kernel as another
*	project I worked on failed on a 2.2 kernel.  Imagine my surprise
*	when I actually tested the module on my 2.0 kernel and it still
*	failed!  Oops.
*
* 19991121.0024	1.0.4a	spc	(comment)
*	I found out that the ptemp doesn't exist, ptrans does, but it's
*	statically defined in http_main.c so I can't use it.  I suppose
*	I could use malloc()/free() but I'd rather stick with the Apache
*	API if I can. 
*
*	Sigh.
*
* 19991120.1711	1.0.4a	spc	(hack)
*	Okay, where does Apache keep ptrans?  The transient pool that was
*	mentioned in the documentation?  Where?  Where?  
*
*	Sigh.
*
* 19991120.1436	1.0.4	spc	(reliability/bug fix)
*	Fixed up clt_linecount() to return the size of the longest
*	line, and fixed up config_litbooktrans() to use this, allocate
*	a buffer large enough to handle this.  All this at the urgings
*	of myg.
*
*	See what we go through to make bug-free software? 8-)
*
*	Also defined my own BUFSIZ constant.  I perhaps tend to overuse
*	the constant perhaps?  But then why does ANSI C make it available?
*
* 19991120.0200	1.0.3	spc	(comment)
*	myg noted that clt_linecount() has a potential bug if the 
*	translation file has a line longer than BUFSIZ chars then the
*	routine will return a bad line count.  This also affects 
*	config_litbooktrans() since it too assumes lines to be smaller
*	than BUFSIZ chars in size.
*
*	Working around this would either require major hacking (since
*	the Apache mem API doesn't include a realloc() command, I would
*	have to do something similar in hr_show_chapter() (see comment)
*	but across two commands.  I could allocate memory from the tmp
*	pool, but it gets real messy real quick.
*
*	If the system you are on has a BUFSIZ smaller than 80, complain
*	to your vendor.  While it would be nice to avoid making assumptions,
*	I'm not entirely sure what to do, other than make sure the trans-
*	lation file is less than 80 characters wide.
*
*	Now, it won't overwrite anything, but it will probably start
*	to act a bit odd ...
*
*	One possible solution:  have clt_linecount() return the length
*	of the longest line (in addition to the number of lines), allocate
*	a buffer that big out of the tmp pool and go from there ...
*
* 19991118.2300	1.0.3	spc	(feature/hack)
*	Added a directive to get around one of the gross hacks in the code
*	(namely, a specification to the directory the handler is covering).
*	Also added a directive (consider it marked for obsolescense until
*	I get a way to specify an HTML template file) to mark the title
*	of the output pages.
*
* 19991117.0500	1.0.2	spc	(cosmetic)
*	Changed the background to a slight offwhite (smoke white I think
*	is the term ... )
*
* 19991113.2355	1.0.1	spc	(bugfix)
*	Check for references like ``book.0:0'' and return an error.
*	Also fixed a bug where I was allocating memory for each line
*	being sent out (not that bad as it's all freed after the request,
*	but still ... 
*
*	Also reorganized the code and cleaned it up a bit.
*
* 19991111.1745	1.0.0	spc
*	Initial version.  Support for the Bible (Old and New Testaments)
*	only.
*
*******************************************************************/

/*#define NDEBUG*/

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <ctype.h>
#include <assert.h>
#include <errno.h>

#include <sys/stat.h>
#include <unistd.h>

#include "soundex.h"
#include "metaphone.h"

#include "httpd.h"
#include "http_config.h"
#include "http_request.h"
#include "http_core.h"
#include "http_protocol.h"
#include "http_main.h"
#include "http_log.h"
#include "util_script.h"
#include "http_conf_globals.h"

#define MBUFSIZ		512

/*****************************************************************/

struct bookname
{
  char    *abrev;
  char    *fullname;
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

struct litconfig
{
  char             *bookroot;
  char             *bookdir;
  char             *booktld;
  char		   *booktitle;
  struct bookname  *books;
  struct bookname **abrev;
  struct bookname **fullname;
  struct bookname **soundex;
  struct bookname **metaphone;
  size_t            maxbook;
};

/*****************************************************************/

	/*-----------------------------------------------
	; Apache module hooks
	;------------------------------------------------*/

static void	  *create_dir_config	(pool *,char *);
static int	   handle_request	(request_rec *);

	/*----------------------------------------------
	; Callbacks during command processing
	;-----------------------------------------------*/
	
static const char *config_litbookdir	(cmd_parms *,void *,char *);
static const char *config_litbooktrans	(cmd_parms *,void *,char *);
static const char *config_litbookindex	(cmd_parms *,void *,char *);
static const char *config_litbooktld	(cmd_parms *,void *,char *); /* o? */
static const char *config_litbooktitle	(cmd_parms *,void *,char *); /* o  */

	/*--------------------------------------------------------
	; Subroutines called by the command processing routines.
	; These are only used during configuration
	;---------------------------------------------------------*/

static void	   clt_linecount	(FILE *,size_t *,size_t *);
static int	   clt_sort_abrev	(const void *,const void *);
static int	   clt_sort_fullname	(const void *,const void *);
static int	   clt_sort_soundex	(const void *,const void *);
static int	   clt_sort_metaphone	(const void *,const void *);

	/*------------------------------------------------
	; Subroutines called by handle_request().
	;------------------------------------------------*/

static void        hr_print_request	(
                                          struct bookrequest *,
					  struct litconfig   *,
					  request_rec        *
					);
static int         hr_show_chapter	(
                                          size_t,
					  size_t,
					  size_t,
					  struct litconfig *,
					  char             *,
					  request_rec      *
					);
static void	   hr_translate_request	(
                                          struct bookrequest *,
					  struct litconfig   *,
					  char               *
					);
static char	  *hr_redirect_request	(struct bookrequest *,pool *);
static int	   hr_find_abrev	(const void *,const void *);
static int	   hr_find_fullname	(const void *,const void *);
static int	   hr_find_soundex	(const void *,const void *);
static int	   hr_find_metaphone	(const void *,const void *);

	/*-------------------------------------------------
	; some misc. routines I needed and couldn't find in
	; the Apache API.  Maybe they'll exist in 2.0
	;--------------------------------------------------*/
	
static char	  *trim_lspace		(char *);
static char	  *trim_tspace		(char *);
static char	  *trim_space		(char *);
static int         empty_string		(char *);

/******************************************************************
*	DATA SECTION
******************************************************************/

static const handler_rec handler_table[] =
{
  { "litbook-handler"	, handle_request } ,
  { NULL		, NULL		 }
};

static command_rec command_table[] =
{
  {
    "LitbookDir",
    config_litbookdir,
    NULL,
    RSRC_CONF | OR_OPTIONS,
    TAKE1,
    "Specifies base location of book contents"
  },
  
  {
    "LitbookTranslation",
    config_litbooktrans,
    NULL,
    RSRC_CONF | OR_OPTIONS,
    TAKE1,
    "Specifies the location of book/chapter titles and abbreviations"
  },
  
  {
    "LitbookIndex",
    config_litbookindex,
    NULL,
    RSRC_CONF | OR_OPTIONS,
    TAKE1,
    "The URL for the main indexpage for this book"
  },
  
  {
    "LitbookTLD",
    config_litbooktld,
    NULL,
    RSRC_CONF | OR_OPTIONS,
    TAKE1,
    "Same value as the Location directive (see docs)"
  },
  
  {
    "LitbookTitle",
    config_litbooktitle,
    NULL,
    RSRC_CONF | OR_OPTIONS,
    TAKE1,
    "Set the title of pages output by this module"
  },
  
  {
    NULL,
    NULL,
    NULL,
    0,
    0,
    NULL
  }
};

module MODULE_VAR_EXPORT litbook_module =
{
  STANDARD_MODULE_STUFF,
  NULL,				/* F module init 		*/
  create_dir_config,		/* F per-directory config create*/
  NULL,				/* F dir config merger		*/
  NULL,				/* F server config creator	*/
  NULL,				/* F server config merger	*/
  command_table,		/* S command table		*/
  handler_table,		/* S list of handlers		*/
  NULL,				/* F filename-to-URI translation*/
  NULL,				/* F check/validate user	*/
  NULL,				/* F check user_id valid here	*/
  NULL,				/* F check access by host	*/
  NULL,				/* F MIME type checker/setter	*/
  NULL,				/* F fixups			*/
  NULL,				/* F logger			*/
  NULL,				/* F header parser		*/
  NULL,				/* F process initalizer		*/
  NULL,				/* F process exit/cleanup	*/
  NULL,				/* F post read_request handler	*/
};

/***********************************************************************
*	CONFIGURATION HOOKS 
***********************************************************************/

static void *create_dir_config(pool *p,char *dirspec)
{
  struct litconfig *plc;
  
  assert(p       != NULL);
  
  plc            = ap_palloc(p,sizeof(struct litconfig));
  plc->bookroot  = NULL;
  plc->bookdir   = NULL;
  plc->booktld   = NULL;
  plc->booktitle = NULL;
  plc->books     = NULL;
  plc->abrev     = NULL;
  plc->fullname  = NULL;
  plc->soundex   = NULL;
  plc->maxbook   = 0;
  return(plc);
}

/*********************************************************************/

static const char *config_litbookdir(cmd_parms *cmd,void *mconfig,char *arg)
{
  struct litconfig *plc;
  struct stat       dstatus;
  char             *err;
  
  assert(cmd     != NULL);
  assert(mconfig != NULL);
  assert(arg     != NULL);
  
  plc = mconfig;

  if (stat(arg,&dstatus) == -1)
  {
    err = ap_pstrcat(
                      cmd->pool,
                      cmd->cmd->name,
                      " : " ,
                      arg ,
                      " " ,
                      strerror(errno),
                      NULL
                    );
    return(err);
  }
  if (!S_ISDIR(dstatus.st_mode))
  {
    err = ap_pstrcat(
                      cmd->pool,
                      cmd->cmd->name,
                      " : ",
                      arg ,
                      " is not a directory.",
                      NULL
                    );
    return(err);
  }
  if (access(arg,X_OK) == -1)
  {
    err = ap_pstrcat(
                      cmd->pool,
                      cmd->cmd->name,
                      " : ",
                      arg ,
                      " ",
                      strerror(errno),
                      NULL
                    );
    return(err);
  }
  plc->bookdir = ap_pstrdup(cmd->pool,arg);
  return(NULL);
}

/*******************************************************************/

static const char *config_litbooktrans(cmd_parms *cmd,void *mconfig,char *arg)
{
  struct litconfig *plc;
  char             *err;
  FILE             *fp;
  char             *buffer;
  int               i;
  size_t            lsize;
  
  assert(cmd     != NULL);
  assert(mconfig != NULL);
  assert(arg     != NULL);
  
  plc = mconfig;
  
  if (access(arg,R_OK) == -1)
  {
    err = ap_pstrcat(
                      cmd->pool,
                      cmd->cmd->name,
                      " : " ,
                      arg,
                      " " ,
                      strerror(errno),
                      NULL
                    );
    return(err);
  }
  
  fp = ap_pfopen(cmd->pool,arg,"r");
  if (fp == NULL)
  {
    err = ap_pstrcat(
                      cmd->pool,
                      cmd->cmd->name,
                      " : " ,
                      arg,
                      " cannot open even though we have access",
                      NULL
                    );
    return(err);
  }

  clt_linecount(fp,&plc->maxbook,&lsize);	/* because we can't realloc */  
  buffer = ap_palloc(cmd->pool,lsize + 1);	/* ptrans is static! */
  if (buffer == NULL)
  {
    return("NO MEMORY!");
  }
  
  plc->books     = ap_palloc(cmd->pool,plc->maxbook * sizeof(struct bookname));
  plc->abrev     = ap_palloc(cmd->pool,plc->maxbook * sizeof(struct bookname *));
  plc->fullname  = ap_palloc(cmd->pool,plc->maxbook * sizeof(struct bookname *));
  plc->soundex   = ap_palloc(cmd->pool,plc->maxbook * sizeof(struct bookname *));
  plc->metaphone = ap_palloc(cmd->pool,plc->maxbook * sizeof(struct bookname *));

  for (i = 0 ; (i < plc->maxbook) && (fgets(buffer,lsize,fp)) ; i++)
  {
    char *abrev;
    char *fulln;
    char  mp[MBUFSIZ];
    int   rc;

    if (empty_string(buffer))
    {
      plc->maxbook--;
      break;
    }
    
    abrev = strtok(buffer,",");
    fulln = strtok(NULL,",\n");
    
    if ((abrev == NULL) || (fulln == NULL)) break;
        
    abrev = ap_pstrdup(cmd->pool,trim_space(abrev));
    fulln = ap_pstrdup(cmd->pool,trim_space(fulln));
    rc    = make_metaphone(fulln,mp,sizeof(mp));
    
    plc->books[i].abrev    = abrev;
    plc->books[i].fullname = fulln;
    plc->books[i].sdx      = isdigit(*fulln) ? Soundex(fulln+1) : Soundex(fulln);
    plc->books[i].mp       = ap_pstrdup(cmd->pool,mp);
    
    plc->abrev[i] = plc->fullname [i] = plc->soundex[i] 
                  = plc->metaphone[i] = &plc->books [i];
  }
  
  ap_pfclose(cmd->pool,fp);
  
  if (i != plc->maxbook)
  {
    char tmpbuf[MBUFSIZ];
    
    sprintf(tmpbuf,"%lu",(unsigned long)i);
    
    err = ap_pstrcat(
                      cmd->pool,
                      cmd->cmd->name,
                      " : " ,
                      "translation file ",
                      arg,
                      " is corrupted on or around line ",
                      tmpbuf,
                      NULL
                    );
    return(err);
  }
  
  qsort(plc->abrev,    plc->maxbook,sizeof(struct bookname *),clt_sort_abrev);
  qsort(plc->fullname, plc->maxbook,sizeof(struct bookname *),clt_sort_fullname);
  qsort(plc->soundex,  plc->maxbook,sizeof(struct bookname *),clt_sort_soundex);
  qsort(plc->metaphone,plc->maxbook,sizeof(struct bookname *),clt_sort_metaphone);
  
  return(NULL);
}

/********************************************************************/

static const char *config_litbookindex(cmd_parms *cmd,void *mconfig,char *arg)
{
  assert(cmd     != NULL);
  assert(mconfig != NULL);
  assert(arg     != NULL);

  ((struct litconfig *)mconfig)->bookroot = ap_pstrdup(cmd->pool,arg);
  return(NULL);
}

/*******************************************************************/

static const char *config_litbooktld(cmd_parms *cmd,void *mconfig,char *arg)
{
  assert(cmd     != NULL);
  assert(mconfig != NULL);
  assert(arg     != NULL);
  
  ((struct litconfig *)mconfig)->booktld = ap_pstrdup(cmd->pool,arg);
  return(NULL);
}

/*******************************************************************/

static const char *config_litbooktitle(cmd_parms *cmd,void *mconfig,char *arg)
{
  assert(cmd     != NULL);
  assert(mconfig != NULL);
  assert(arg     != NULL);
  
  ((struct litconfig *)mconfig)->booktitle = ap_pstrdup(cmd->pool,arg);
  return(NULL);
}

/******************************************************************
*	CONFIGURATION SUBROUTINES
******************************************************************/

static void clt_linecount(FILE *fp,size_t *pcount,size_t *plsize)
{
  size_t lcount;
  size_t lsize;
  size_t lmax;
  int    c;
  
  assert(fp     != NULL);
  assert(pcount != NULL);
  assert(plsize != NULL);
  
  lcount = lsize = lmax = 0;
  
  while((c = fgetc(fp)) != EOF)
  {
    lsize++;
    
    if (c == '\n')
    {
      lcount++;
      if (lsize > lmax)
        lmax = lsize;
      lsize = 0;
    }
  }
  
  /*----------------------------------------------------
  ; (1.0.5) if lsize > 0 then the last line in the file 
  ; does not end with a '\n'.  Adjust accordingly.
  ;
  ; (1.0.7) if the last line doesn't end with a '\n' and 
  ; it's the longest line, lmax isn't the maximum line 
  ; size.
  ;
  ; (1.0.8) adjust lsize by 1 to adjust for
  ; fgets()/strtok() interaction.
  ;-----------------------------------------------------*/
  
  *pcount = lcount + ((lsize > 0) ? 1 : 0) ;
  *plsize = 1 + ((lsize > lmax) ? lsize : lmax);
  fseek(fp,0,SEEK_SET);
}

/**********************************************************************/

static int clt_sort_abrev(const void *o1,const void *o2)
{
  assert(o1 != NULL);
  assert(o2 != NULL);
  
  return(
          strcmp(
                  (*((struct bookname **)o1))->abrev,
                  (*((struct bookname **)o2))->abrev
                )
        );
}

/*******************************************************************/

static int clt_sort_fullname(const void *o1,const void *o2)
{
  assert(o1 != NULL);
  assert(o2 != NULL);

  return(
          strcmp(
                  (*((struct bookname **)o1))->fullname,
                  (*((struct bookname **)o2))->fullname
                )
        );
}

/*********************************************************************/

static int clt_sort_soundex(const void *o1,const void *o2)
{
  assert(o1 != NULL);
  assert(o2 != NULL);

  return(
          SoundexCompare(
                          (*((struct bookname **)o1))->sdx,
                          (*((struct bookname **)o2))->sdx
                        )
        );
}

/********************************************************************/

static int clt_sort_metaphone(const void *o1,const void *o2)
{
  assert(o1 != NULL);
  assert(o2 != NULL);

  return(
          strcmp(
                  (*((struct bookname **)o1))->mp,
                  (*((struct bookname **)o2))->mp
                )
        );
}

/*****************************************************************
*	HANDLER HOOK
******************************************************************/

static int handle_request(request_rec *r)
{
  struct litconfig   *plc;
  struct bookrequest  br;
  
  assert(r != NULL);
  
  plc = ap_get_module_config(r->per_dir_config,&litbook_module);
  
  if (r->method_number == M_OPTIONS)
  {
    r->allowed |= ((1 << M_GET) | (1 << M_POST));
    return(DECLINED);
  }
  
  /*------------------------------------------------------------
  ; if there's no path to search down, redirect (permanently) 
  ; to the book index (which is elsewhere ... )
  ;
  ; TODO:	If we set a handler for `/' then there doesn't 
  ; 		seem to be a r->path_info set.  I think it's
  ;		in r->uri or some other similar field we have
  ;		to then check.  I wonder if there's a way to
  ;		get the location of the handler in which we've
  ;		been installed in ...
  ;
  ;		(1.0.3) see comment below
  ;------------------------------------------------------------*/
  
  if (
       (r->path_info[0] == '\0')
       || ((r->path_info[0] == '/') && (r->path_info[1] == '\0'))
     )
  {
    if (plc->bookroot == NULL)
      return(HTTP_NOT_FOUND);
    ap_table_setn(r->headers_out,"Location",plc->bookroot);
    return(HTTP_MOVED_PERMANENTLY);
  }
  
  /*--------------------------------------------------------------
  ; Translate the request.  If it isn't in canonical form, do that
  ; redirect thang.  If it isn't found, do that not found thang.
  ;
  ; TODO:	Related to the one above---we don't know (or rather,
  ;		I don't know at this time) how to determine (well,
  ;		not without a lot of hassle) what the top level
  ;		directory is that we're handling.  See ``hack.''
  ;
  ; (1.0.3) Added directive LitbookTLD to easily get the location of the
  ; handler.  Not pretty, but saves some code.  Hopefully Apache 2.0 will
  ; have a better module design.
  ;
  ; (1.0.3) Also added directive LitbookTitle so I don't have to embed the
  ; title here either.  This WILL go away once I figure out how to handle
  ; HTML template.
  ;
  ; (1.0.6) Get the hostname AND the port to redirect to.
  ;--------------------------------------------------------------*/
  
  hr_translate_request(&br,plc,&r->path_info[1]);
  if (br.name == NULL) return(HTTP_NOT_FOUND);
  if (br.redirect)
  {
    char tportnum[MBUFSIZ];

    if (r->server->port != 80)
      sprintf(tportnum,":%lu",(unsigned long)r->server->port);
    else
      tportnum[0] = '\0';
     
    ap_table_setn(
                   r->headers_out,
                   "Location",
                   ap_pstrcat(
		               r->pool,
			       "http://",
			       r->server->server_hostname,
			       tportnum,
			       plc->booktld,	/* hack, still */
			       hr_redirect_request(&br,r->pool),
			       NULL
			     )
                 );
    return(HTTP_MOVED_PERMANENTLY);
  }
  
  /*-------------------------------------------------------------
  ; Handle the request now that we have everything in place
  ;
  ; TODO:	How to remove the HTML formatting from the source
  ;		code to an external file.
  ;
  ;		More immediate:  better <META> tags.
  ;-------------------------------------------------------------*/

  r->content_type = "text/html";
  ap_soft_timeout("send literary content",r);
  ap_send_http_header(r);
  
  if (r->header_only)
  {
    ap_kill_timeout(r);
    return(OK);
  }
  
  ap_rputs(DOCTYPE_HTML_3_2,r);
  ap_rprintf(
              r,
              "<html>\n"
              "<head>\n"
              "  <title>%s</title>\n"
              "</head>\n"
              "\n"
              "<body bgcolor=\"#efefef\" text=\"#000000\">\n"
              "\n",
              plc->booktitle
          );

  hr_print_request(&br,plc,r);
  
  ap_rputs(
            "\n"
            "</body>\n"
            "</html>\n"
            "\n",
            r
          );
  ap_kill_timeout(r);
  return(OK);
}

/*******************************************************************
*	HANDLER SUBROUTINES
*******************************************************************/

static void hr_print_request(
                              struct bookrequest *pbr,
                              struct litconfig   *plc,
                              request_rec        *r
                            )
{
  int    rc;
  size_t i;
  
  assert(pbr != NULL);
  assert(plc != NULL);
  assert(r   != NULL);
  
  ap_rprintf(r,"<h1 align=center>%s</h1>\n",pbr->name);
  
  if (pbr->c1 == pbr->c2)
    hr_show_chapter(pbr->c1,pbr->v1,pbr->v2,plc,pbr->name,r);
  else
  {
    for (i = pbr->c1 ; i <= pbr->c2 ; i++)
    {
      if (i == pbr->c1)
      {
        if (hr_show_chapter(i,pbr->v1,INT_MAX,plc,pbr->name,r)) break;
      }
      else if (i == pbr->c2)
      {
        if (hr_show_chapter(i,1,pbr->v2,plc,pbr->name,r)) break;
      }
      else
      {
        if (hr_show_chapter(i,1,INT_MAX,plc,pbr->name,r)) break;
      }
    }
  }
}

/***********************************************************************/

static int hr_show_chapter(
                            size_t            chapter,
                            size_t            vlow,
                            size_t            vhigh,
                            struct litconfig *plc,
                            char             *name,
                            request_rec      *r
                          )
{
  long int *iarray;
  FILE     *fp;
  char      fname[MBUFSIZ];
  long      max;
  long      i;
  long      s;
  long      maxs = 0;
  char     *p;
  
  assert(chapter >  0);
  assert(vlow    >  0);
  assert(plc     != NULL);
  assert(r       != NULL);
  
  sprintf(fname,"%s/%s/%lu.index",plc->bookdir,name,(unsigned long)chapter);
  fp = ap_pfopen(r->pool,fname,"rb");
  if (fp == NULL) return(1);
  
  fread(&max,sizeof(max),1,fp);
  
  if (max < 1)
  {
    ap_pfclose(r->pool,fp);
    return(1);
  }
  
  if (vlow > max)
  {
    ap_pfclose(r->pool,fp);
    return(1);
  }
  
  if (vhigh > max) vhigh = max;
  
  iarray = ap_palloc(r->pool,(max + 1) * sizeof(long));
  if (iarray == NULL)
  {
    ap_pfclose(r->pool,fp);
    return(1);
  }
  
  fread(iarray,sizeof(long),max + 1,fp);
  ap_pfclose(r->pool,fp);
  
  sprintf(fname,"%s/%s/%lu",plc->bookdir,name,(unsigned long)chapter);
  fp = ap_pfopen(r->pool,fname,"rb");
  
  if (fp == NULL) return(1);
  
  ap_rprintf(r,"<h2 align=center>Chapter %lu</h2>\n",(unsigned long)chapter);
  if (vlow > 1)
    ap_rprintf(r,"<p align=center>.<br>.<br>.</p>\n");
  
  for (i = vlow ; i <= vhigh ; i++)
  {
    s = iarray[i] - iarray[i-1];
    
    /*-------------------------------------------------------------
    ; In the original (non-module) version of this code, I allocate
    ; memory for each line, read it in, the free it up.  Since Apache
    ; doesn't have a free() call per say, and since I don't want to 
    ; end up allocating memory for each line (it could be a significant
    ; number of lines being read) I reuse the memory I allocate if it's
    ; big enough.  It should only waste a few lines worth of memory.
    ;-----------------------------------------------------------------*/
    
    if (s > maxs) 
    {
      p = ap_palloc(r->pool,s + 1);
      if (p == NULL)
      {
        ap_pfclose(r->pool,fp);
        return(0);
      }
      maxs = s;
    }
    
    p[s] = '\0';
    fseek(fp,iarray[i-1],SEEK_SET);
    fread(p,sizeof(char),s,fp);
    ap_rprintf(r,"%lu. ",(unsigned long)i);
    ap_rputs(p,r);
    ap_rputs("\n\n<p>\n\n",r);
  }
  
  ap_pfclose(r->pool,fp);
  return(0);
}

/**********************************************************************/

static void hr_translate_request(
                                  struct bookrequest *pbr,
                                  struct litconfig   *plc,
                                  char               *r
                                )
{
  char              buffer[MBUFSIZ];
  char             *p;
  size_t            size;
  struct bookname **pres;
  char             *or = r;
  
  assert(pbr != NULL);
  assert(r   != NULL);
  
  pbr->name     = NULL;
  pbr->c1       = 1;
  pbr->v1       = 1;
  pbr->c2       = INT_MAX;
  pbr->v2       = INT_MAX;
  pbr->redirect = 0;
  
  for (p = buffer , size = 0 ; (*r) && (!ispunct(*r)) ; r++ , size++)
  {
    *p++ = tolower(*r);
    *p   = '\0';
  }
  
  if (size < 2) return;
  if (isdigit(buffer[0]))
    buffer[1] = toupper(buffer[1]);
  else
    buffer[0] = toupper(buffer[0]);
  
  /*--------------------------------------------------------
  ; quickly:
  ;
  ;	if not found fullname
  ;	  if not found abrev
  ;	    if not found soundex
  ;	      if not found metaphone
  ;             return(NOT FOUND);
  ;---------------------------------------------------------*/

  pres = bsearch(
                  buffer,
                  plc->fullname,
                  plc->maxbook,
                  sizeof(struct bookname *),
                  hr_find_fullname
                );
  if (pres == NULL)
  {
    pres = bsearch(
                    buffer,
                    plc->abrev,
                    plc->maxbook,
                    sizeof(struct bookname *),
                    hr_find_abrev
                  );
    if (pres == NULL)
    {
      SOUNDEX sdx = Soundex(buffer);
      
      pres = bsearch(
                      &sdx,
                      plc->soundex,
                      plc->maxbook,
                      sizeof(struct bookname *),
                      hr_find_soundex
                    );
      if (pres == NULL)
      {
        char mp[MBUFSIZ];
        int  rc;
        
        rc = make_metaphone(buffer,mp,sizeof(mp));
        if (rc == 0) return;
        pres = bsearch(
                        mp,
                        plc->metaphone,
                        plc->maxbook,
                        sizeof(struct bookname *),
                        hr_find_metaphone
                      );
        if (pres == NULL) return;
      }
    }
  }
  
  pbr->name     = (*pres)->fullname;
  pbr->redirect = strncmp(or,pbr->name,strlen(pbr->name));
  
  if ((*r == '\0') || (*r == '-')) return;	/* 1. G or G- */
  
  pbr->c1 = strtol(r+1,&r,10);
  if (pbr->c1 == 0)
  {
    pbr->name = NULL;
    return;
  }
  
  if (*r == '\0')				/* 2. G.a */
  {
    pbr->c2 = pbr->c1;
    return;
  }
  
  if ((*r == '.') || (*r == ':'))
  {
    if (*r == '.') pbr->redirect = 1;
    pbr->v1 = strtol(r+1,&r,10);		/* 3. G.a.b */
    if (pbr->v1 == 0)
    {
      pbr->name = NULL;
      return;
    }
    
    if (*r++ != '-')
    {
      pbr->c2 = pbr->c1;
      pbr->v2 = pbr->v1;
      if (*(r-1)) pbr->redirect = 1;
      return;
    }
    
    if (!isdigit(*r))				/* 4. G.a.b- */
    {
      if (*r) pbr->redirect = *r;
      return;
    }
    
    pbr->c2 = strtol(r,&r,10);			/* 5. G.a.b-x */
    if (pbr->c2 == 0)
    {
      pbr->name = NULL;
      return;
    }
    
    if ((*r != '.') && (*r != ':'))
    {
      pbr->v2       = pbr->c2;
      pbr->c2       = pbr->c1;
      if (*r) pbr->redirect = *r;
      return;
    }

    if (*r == '.') pbr->redirect = 1;
    
    pbr->v2       = strtol(r+1,&r,10);		/* 6. G.a.b-x.y */
    if (pbr->v2 == 0)
    {
      pbr->name = NULL;
      return;
    }
    
    if (*r) pbr->redirect = *r;
    return;
  }
  
  if (*r++ != '-')				/* 2. G.a */
  {
    pbr->c2 = pbr->c1;
    if (*(r-1)) pbr->redirect = *(r-1);
    return;
  }
  
  if (!isdigit(*r))				/* 7. G.a- */
  {
    if (*r) pbr->redirect = *r;
    return;
  }
  
  pbr->c2 = strtol(r,&r,10);			/* 8. G.a-x */
  if (pbr->c2 == 0)
  {
    pbr->name = NULL;
    return;
  }
  
  if ((*r != '.') && (*r != ':'))
  {
    if (*r) pbr->redirect = *r;
    return;
  }
  
  if (*r == '.') pbr->redirect = 1;
  pbr->v2       = strtol(r+1,&r,10);		/* 9. G.a-x.y */
  if (pbr->v2 == 0)
  {
    pbr->name = NULL;
    return;
  }
  if (*r) pbr->redirect = *r;
}

/*******************************************************************/

static char *hr_redirect_request(struct bookrequest *pbr,pool *p)
{
  char tc1[64];
  char tc2[64];
  char tv1[64];
  char tv2[64];
  
  sprintf(tc1,"%lu",(unsigned long)pbr->c1);
  sprintf(tc2,"%lu",(unsigned long)pbr->c2);
  sprintf(tv1,"%lu",(unsigned long)pbr->v1);
  sprintf(tv2,"%lu",(unsigned long)pbr->v2);
  
  if ((pbr->c2 == INT_MAX) && (pbr->v2 == INT_MAX))
  {
    if ((pbr->c1 == 1) && (pbr->v1 == 1))
      return(ap_pstrdup(p,pbr->name));
    
    if ((pbr->c1 != 1) && (pbr->v2 == 1))
      return(ap_pstrcat(p,pbr->name,".",tc1,"-",NULL));
    
    return(ap_pstrcat(p,pbr->name,".",tc1,":",tv1,"-",NULL));
  }
  
  if (pbr->c1 == pbr->c2)
  {
    if ((pbr->v1 == 1) && (pbr->v2 == INT_MAX))
      return(ap_pstrcat(p,pbr->name,".",tc1,":1",NULL));
  
    if (pbr->v1 == pbr->v2)
      return(ap_pstrcat(p,pbr->name,".",tc1,":",tv1,NULL));
    
    return(ap_pstrcat(p,pbr->name,".",tc1,":",tv1,"-",tc2,":",tv2,NULL));
  }
  
  if (pbr->v1 == 1)
  {
    if (pbr->v2 == INT_MAX)
      return(ap_pstrcat(p,pbr->name,".",tc1,"-",tc2,NULL));
    else
      return(ap_pstrcat(p,pbr->name,".",tc1,"-",tc2,":",tv2,NULL));
  }
  
  return(ap_pstrcat(p,pbr->name,".",tc1,":",tv1,"-",tc1,":",tv2,NULL));
}

/**********************************************************************/

static int hr_find_abrev(const void *key,const void *datum)
{
  assert(datum != NULL);
  assert(key   != NULL);

  return(strcmp(key,(*((struct bookname **)datum))->abrev));
}

/**********************************************************************/

static int hr_find_fullname(const void *key,const void *datum)
{
  assert(datum != NULL);
  assert(key   != NULL);

  return(strcmp(key,(*((struct bookname **)datum))->fullname));
}

/*********************************************************************/

static int hr_find_soundex(const void *key,const void *datum)
{
  assert(datum != NULL);
  assert(key   != NULL);

  return(SoundexCompare(*((SOUNDEX *)key),(*((struct bookname **)datum))->sdx));
}

/**********************************************************************/

static int hr_find_metaphone(const void *key,const void *datum)
{
  assert(datum != NULL);
  assert(key   != NULL);

  return(strcmp(key,(*((struct bookname **)datum))->mp));
}

/************************************************************************
*	MISC UTIL SUBROUTINES
************************************************************************/

static char *trim_lspace(char *s)
{
  assert(s != NULL);
  for ( ; (*s) && (isspace(*s)) ; s++)
    ;
  return(s);
}

/********************************************************************/

static char *trim_tspace(char *s)
{
  char *p;
  
  assert(s != NULL);
  for (p = s + strlen(s) - 1 ; (p > s) && (isspace(*p)) ; p--)
    ;
  p[1] = '\0';
  return(s);
}

/********************************************************************/

static char *trim_space(char *s)
{
  assert(s != NULL);
  return(trim_tspace(trim_lspace(s)));
}

/********************************************************************/

static int empty_string(char *s)
{
  assert(s != NULL);
  
  for ( ; *s ; s++)
  {
    if (isprint(*s)) return(0);
  }
  return(1);
}

/*********************************************************************/
