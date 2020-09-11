# Copyright 2012-2020 Mitchell mitchell.att.foicica.com. See LICENSE.

.SUFFIXES: .cxx .c .o .h .a

AR = ar
CC = gcc
CXX = g++
INCLUDEDIRS = -I ../include -I ../src -I ../lexlib
CFLAGS = -std=c99 -pedantic -Wall
CXXFLAGS = -std=c++17 -pedantic -DCURSES -DSCI_LEXER $(INCLUDEDIRS) -Wall
ifdef DEBUG
  CXXFLAGS += -DDEBUG -g
else
  CXXFLAGS += -DNDEBUG -Os
endif
CURSES_FLAGS =

scintilla = ../bin/scintilla.a
sci = AutoComplete.o CallTip.o CaseConvert.o CaseFolder.o Catalogue.o \
      CellBuffer.o CharacterCategory.o CharClassify.o ContractionState.o \
      DBCS.o Decoration.o Document.o EditModel.o Editor.o EditView.o \
      ExternalLexer.o Indicator.o KeyMap.o LineMarker.o MarginView.o PerLine.o \
      PositionCache.o RESearch.o RunStyles.o ScintillaBase.o Selection.o \
      Style.o UniConversion.o UniqueString.o ViewStyle.o XPM.o \
      Accessor.o CharacterSet.o DefaultLexer.o LexerBase.o LexerModule.o \
      LexerNoExceptions.o LexerSimple.o PropSetSimple.o StyleContext.o \
      WordList.o
lexers = $(addsuffix .o,$(basename $(sort $(notdir $(wildcard ../lexers/Lex*.cxx)))))

vpath %.h ../src ../include ../lexlib
vpath %.cxx ../src ../lexlib ../lexers

all: $(scintilla)
$(sci) $(lexers) ScintillaCurses.o: %.o: %.cxx
	$(CXX) $(CXXFLAGS) $(CURSES_FLAGS) -c $<
$(scintilla): $(sci) $(lexers) ScintillaCurses.o
	$(AR) rc $@ $^
	touch $@
clean: ; rm -f *.o $(scintilla)

# Documentation.

docs: docs/index.md docs/api.md $(wildcard docs/*.md) | \
      docs/_layouts/default.html
	for file in $(basename $^); do \
		cat $| | docs/fill_layout.lua $$file.md > $$file.html; \
	done
docs/index.md: README.md
	sed 's/^\# [[:alpha:]]\+/## Introduction/;' $< > $@
	sed -i 's|https://[[:alpha:]]\+\.github\.io/[[:alpha:]]\+/||;' $@
docs/api.md: docs/scinterm.luadoc ; luadoc --doclet docs/markdowndoc $^ > $@
cleandocs: ; rm -f docs/*.html docs/index.md docs/api.md
