
#######################################################################
#
# Makefile		- Makefile for mod_litbook and misc. files
#
# Copyright 2022 by Sean Conner.  All Rights Reserved.
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
#
# Comments, questions and criticisms can be sent to: sean@conman.org
#
########################################################################

CC=cc -g -std=c99 -Wall -ansi -pedantic
CFLAGS=
AR=ar cr
RANLIB=ranlib
APXS=apxs

.PHONY: clean

mod_litbook.o : mod_litbook.c
	$(APXS) -i -a -c mod_litbook.c soundex.c metaphone.c

breakout    : breakout.o util.o nodelist.o
testmod     : testmod.o soundex.o metaphone.o
breakout.o  : breakout.c
metaphone.o : metaphone.c metaphone.h
nodelist.o  : nodelist.c nodelist.h
soundex.o   : soundex.c soundex.h
testmod.o   : testmod.c
util.o      : util.c util.h

clean : 
	$(RM) libsoundex.a breakout testmod *.o *~ *.lo *.la *.slo
