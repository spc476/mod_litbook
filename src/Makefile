
#######################################################################
#
# Makefile		- Makefile for mod_litbook and misc. files
#
# Copyright 1999 by Sean Conner.  All Rights Reserved.
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

CC=cc -g -Wall -ansi -pedantic
CFLAGS=
AR=ar cr
RANLIB=ranlib

libsoundex.a : soundex.o metaphone.o
	$(AR) libsoundex.a soundex.o metaphone.o
	$(RANLIB) libsoundex.a

breakout : breakout.o util.o nodelist.o
	$(CC) -o breakout breakout.o util.o nodelist.o

testmod : testmod.o soundex.o metaphone.o
	$(CC) -o testmod testmod.o soundex.o metaphone.o

all : libsoundex.a breakout testmod

clean : 
	/bin/rm libsoundex.a breakout testmod *.o *~

#---------------------------------------------------------
# Rules to compile individual files
#---------------------------------------------------------

breakout.o : breakout.c
	$(CC) -c breakout.c

metaphone.o : metaphone.c metaphone.h
	$(CC) -c metaphone.c

nodelist.o : nodelist.c nodelist.h
	$(CC) -c nodelist.c

soundex.o : soundex.c soundex.h
	$(CC) -c soundex.c

testmod.o : testmod.c
	$(CC) -c testmod.c

util.o : util.c util.h
	$(CC) -c util.c
