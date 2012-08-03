# Copyright 2007-2012 Mitchell mitchell.att.foicica.com. See LICENSE.

CC = gcc
CXX = g++
INCLUDEDIRS = -I ../../include -I ../../src -I ../../lexlib -I ../
CFLAGS = -DNCURSES -DSCI_LEXER -W -Wall $(INCLUDEDIRS)
CXXFLAGS = $(CFLAGS)

scintilla = ../../bin/scintilla.a
lexers = $(wildcard ../Lex*.o)

all: jinx
jinx.o: jinx.c
	$(CC) -g $(CFLAGS) -c $<
ScintillaTerm.o: ../ScintillaTerm.cxx
	$(CXX) $(CXXFLAGS) -c $<
jinx: jinx.o ScintillaTerm.o $(lexers) $(scintilla)
	$(CXX) -DNCURSES $^ -o $@ -lncursesw
clean:
	rm -f jinx *.o
