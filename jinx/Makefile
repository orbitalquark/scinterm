# Copyright 2012-2024 Mitchell. See LICENSE.

CC = gcc
CXX = g++
CFLAGS = -DCURSES -DSCI_LEXER -Wall -I../../include -I../../src -I../../../lexilla/include \
  -I../../../lexilla/lexlib -I ../
CXXFLAGS = $(CFLAGS)

scintilla = ../../bin/scintilla.a ../../../lexilla/bin/liblexilla.a

all: jinx
jinx.o: jinx.c ; $(CC) $(CFLAGS) -c $<
jinx: jinx.o $(scintilla) ; $(CXX) $^ -o $@ -lncurses -ldl -lpthread
clean: ; rm -f jinx *.o
