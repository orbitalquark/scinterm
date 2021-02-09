# Copyright 2012-2021 Mitchell. See LICENSE.

CC = gcc
CXX = g++
INCLUDEDIRS = -I ../../include -I ../../src -I ../../lexlib -I ../
CFLAGS = -DCURSES -DSCI_LEXER -Wall $(INCLUDEDIRS)
CXXFLAGS = $(CFLAGS)

scintilla = ../../bin/scintilla.a

all: jinx
jinx.o: jinx.c ; $(CC) $(CFLAGS) -c $<
jinx: jinx.o $(scintilla) ; $(CXX) $^ -o $@ -lncursesw
clean: ; rm -f jinx *.o
