# Copyright 2012-2021 Mitchell. See LICENSE.

.SUFFIXES: .cxx .c .o .h .a

AR = ar
CC = gcc
CXX = g++
CFLAGS = -std=c99 -pedantic -Wall
CXXFLAGS = -std=c++17 -pedantic -DCURSES -DSCI_LEXER -I../include -I../src -Wall
ifdef DEBUG
  CXXFLAGS += -DDEBUG -g
else
  CXXFLAGS += -DNDEBUG -Os
endif
CURSES_FLAGS =

scintilla = ../bin/scintilla.a
sci = AutoComplete.o CallTip.o CaseConvert.o CaseFolder.o CellBuffer.o \
      CharacterCategory.o CharClassify.o ContractionState.o DBCS.o \
      Decoration.o Document.o EditModel.o Editor.o EditView.o Geometry.o \
      Indicator.o KeyMap.o LineMarker.o MarginView.o PerLine.o PositionCache.o \
      RESearch.o RunStyles.o ScintillaBase.o Selection.o Style.o \
      UniConversion.o UniqueString.o ViewStyle.o XPM.o

vpath %.h ../src ../include
vpath %.cxx ../src

all: $(scintilla)
$(sci) ScintillaCurses.o: %.o: %.cxx
	$(CXX) $(CXXFLAGS) $(CURSES_FLAGS) -c $<
$(scintilla): $(sci) ScintillaCurses.o
	$(AR) rc $@ $^
	touch $@
patch: $(wildcard patches/*.patch)
	@for patch in $^; do \
		echo "Applying $$patch"; \
		patch -d ../ -N -p1 < $$patch; \
	done
clean: ; rm -f *.o $(scintilla)

# Documentation.

docs: docs/index.md docs/api.md $(wildcard docs/*.md) | \
      docs/_layouts/default.html
	for file in $(basename $^); do \
		cat $| | docs/fill_layout.lua $$file.md > $$file.html; \
	done
docs/index.md: README.md
	sed -e 's/^\# [[:alpha:]]\+/## Introduction/;' -e \
		's|https://[[:alpha:]]\+\.github\.io/[[:alpha:]]\+/||;' $< > $@
docs/api.md: docs/scinterm.luadoc ; luadoc --doclet docs/markdowndoc $^ > $@
cleandocs: ; rm -f docs/*.html docs/index.md docs/api.md
