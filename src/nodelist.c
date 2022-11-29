
/******************************************************************
*
* nodelist.c            - Generic double liked-list routines heavily
*                         based upon the ones found in the AmigaOS.
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
*********************************************************************/

#include <stddef.h>
#include <assert.h>
#include "nodelist.h"

/*********************************************************************/

void (ListInit)(List *pl)
{
  assert(pl != NULL);
  
  pl->lh_Head     = (Node *)&pl->lh_Tail;
  pl->lh_Tail     = NULL;
  pl->lh_TailPred = (Node *)&pl->lh_Head;
}

/*********************************************************************/

void (ListAddHead)(List *pl,Node *pn)
{
  assert(pl != NULL);
  assert(pn != NULL);
  
  NodeInsert((Node *)&pl->lh_Head,pn);
}

/**********************************************************************/

void (ListAddTail)(List *pl,Node *pn)
{
  assert(pl != NULL);
  assert(pn != NULL);
  
  NodeInsert(pl->lh_TailPred,pn);
}

/*********************************************************************/

Node *(ListGetHead)(List *pl)
{
  assert(pl          != NULL);
  assert(pl->lh_Head != NULL);
  
  return(pl->lh_Head);
}

/**********************************************************************/

Node *(ListGetTail)(List *pl)
{
  assert(pl              != NULL);
  assert(pl->lh_TailPred != NULL);
  
  return(pl->lh_TailPred);
}

/********************************************************************/

Node *(ListRemHead)(List *pl)
{
  Node *pn;
  
  assert(pl != NULL);
  
  pn = ListGetHead(pl);
  assert(pn != NULL);
  if (NodeValid(pn))
    NodeRemove(pn);
  return(pn);
}

/*********************************************************************/

Node *(ListRemTail)(List *pl)
{
  Node *pn;
  
  assert(pl != NULL);
  
  pn = ListGetTail(pl);
  assert(pn != NULL);
  if (NodeValid(pn))
    NodeRemove(pn);
  return(pn);
}

/**********************************************************************/

int (ListEmpty)(List *pl)
{
  assert(pl != NULL);
  return(pl->lh_Head == (Node *)&pl->lh_Tail);
}

/***********************************************************************/

void (NodeInsert)(Node *pn,Node *pntoa)
{
  Node *pnn;
  
  assert(pn    != NULL);
  assert(pntoa != NULL);
  
  pnn            = pn->ln_Succ;
  pntoa->ln_Succ = pnn;
  pntoa->ln_Pred = pn;
  pn->ln_Succ    = pntoa;
  pnn->ln_Pred   = pntoa;
}

/***********************************************************************/

void (NodeRemove)(Node *pn)
{
  Node *pns;
  Node *pnp;
  
  assert(pn          != NULL);
  assert(pn->ln_Succ != NULL);
  assert(pn->ln_Pred != NULL);
  
  pns = pn->ln_Succ;
  pnp = pn->ln_Pred;
  
  pns->ln_Pred = pnp;
  pnp->ln_Succ = pns;
}

/*************************************************************************/

Node *(NodeNext)(Node *pn)
{
  assert(pn != NULL);
  
  if (NodeValid(pn))
    pn = pn->ln_Succ;
  return(pn);
}

/*********************************************************************/

Node *(NodePrev)(Node *pn)
{
  assert(pn != NULL);
  
  if (NodeValid(pn))
    pn = pn->ln_Pred;
  return(pn);
}

/********************************************************************/

Node *(NodeNextW)(Node *pn)
{
  assert(pn != NULL);
  
  if (NodeValid(pn))
  {
    pn = pn->ln_Succ;
    if (!NodeValid(pn))
    {
      pn = *((Node **)(pn) - 1);
    }
  }
  return(pn);
}

/********************************************************************/

Node *(NodePrevW)(Node *pn)
{
  assert(pn != NULL);
  
  if (NodeValid(pn))
  {
    pn = pn->ln_Pred;
    if (!NodeValid(pn))
    {
      pn = *((Node **)(pn + 1));
    }
  }
  return(pn);
}

/*********************************************************************/

int (NodeValid)(Node *pn)
{
  assert(pn != NULL);
  
  if (pn->ln_Succ == NULL) return(0);
  if (pn->ln_Pred == NULL) return(0);
  return(1);
}

/**********************************************************************/

