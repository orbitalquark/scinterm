# Copyright 2012-2021 Mitchell. See LICENSE.

CC = gcc
CXX = g++
CFLAGS = -DCURSES -DSCI_LEXER -Wall -I../../include -I../../src -I../../../lexilla/include \
  -I../../../lexilla/lexlib -I ../
CXXFLAGS = $(CFLAGS)

scintilla = ../../bin/scintilla.a ../../../lexilla/bin/liblexilla.a

all: jinx
jinx.o: jinx.c ; $(CC) $(CFLAGS) -c $<
jinx: jinx.o $(scintilla) ; $(CXX) $^ -o $@ -lncursesw
clean: ; rm -f jinx *.o
