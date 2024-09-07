# Copyright 2012-2024 Mitchell. See LICENSE.

.SUFFIXES: .cxx .c .o .h .a

srcdir ?= .
basedir = $(srcdir)/..

AR = ar
CC = gcc
CXX = g++
CXX_BASE_FLAGS = -std=c++17 -pedantic -DCURSES -DSCI_LEXER -I$(basedir)/include -I$(basedir)/src -Wall
ifdef DEBUG
  CXX_BASE_FLAGS += -DDEBUG -g
else
  CXX_BASE_FLAGS += -DNDEBUG -Os
endif
CURSES_FLAGS =

scintilla = $(basedir)/bin/scintilla.a
sci = AutoComplete.o CallTip.o CaseConvert.o CaseFolder.o CellBuffer.o ChangeHistory.o \
  CharacterCategoryMap.o CharacterType.o CharClassify.o ContractionState.o DBCS.o Decoration.o \
  Document.o EditModel.o Editor.o EditView.o Geometry.o Indicator.o KeyMap.o LineMarker.o \
  MarginView.o PerLine.o PositionCache.o RESearch.o RunStyles.o ScintillaBase.o Selection.o \
  Style.o UndoHistory.o UniConversion.o UniqueString.o ViewStyle.o XPM.o

vpath %.h $(srcdir) $(basedir)/src $(basedir)/include
vpath %.cxx $(srcdir) $(basedir)/src

all: $(scintilla)
$(sci) PlatCurses.o ScintillaCurses.o: %.o: %.cxx
	$(CXX) $(CXX_BASE_FLAGS) $(CXXFLAGS) $(CURSES_FLAGS) -c $<
$(scintilla): $(sci) PlatCurses.o ScintillaCurses.o
	$(AR) rc $@ $^
	touch $@
patch: $(wildcard patches/*.patch)
	@for patch in $^; do \
		echo "Applying $$patch"; \
		patch -d ../ -N -p1 < $$patch; \
	done
clean: ; rm -f *.o $(scintilla)

# Documentation.

docs: docs/index.md docs/api.md $(wildcard docs/*.md) | docs/_layouts/default.html
	for file in $(basename $^); do cat $| | docs/fill_layout.lua $$file.md > $$file.html; done
docs/index.md: README.md
	sed -e 's/^\# [[:alpha:]]\+/## Introduction/;' -e \
		's|https://[[:alpha:]]\+\.github\.io/[[:alpha:]]\+/||;' $< > $@
docs/api.md: docs/scinterm.luadoc ; ldoc --filter docs.markdowndoc.ldoc $^ > $@
cleandocs: ; rm -f docs/*.html docs/index.md docs/api.md
