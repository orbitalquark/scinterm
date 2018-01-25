# Copyright 2012-2018 Mitchell mitchell.att.foicica.com. See LICENSE.

CC = gcc
CXX = g++
INCLUDEDIRS = -I ../../include -I ../../src -I ../../lexlib -I ../
CFLAGS = -DCURSES -DSCI_LEXER -D_XOPEN_SOURCE_EXTENDED -W -Wall $(INCLUDEDIRS) \
         -Wno-unused-parameter
CXXFLAGS = $(CFLAGS)

scintilla = ../../bin/scintilla.a
lexers = $(wildcard ../Lex*.o)

all: jinx
jinx.o: jinx.c
	$(CC) $(CFLAGS) -c $<
jinx: jinx.o $(lexers) $(scintilla)
	$(CXX) -DCURSES $^ -o $@ -lncursesw
clean:
	rm -f jinx *.o
