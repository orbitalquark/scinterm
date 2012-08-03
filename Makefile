# Copyright 2007-2012 Mitchell mitchell.att.foicica.com. See LICENSE.

.SUFFIXES: .cxx .c .o .h .a

CXX = g++
INCLUDEDIRS = -I ../include -I ../src -I ../lexlib
CXXFLAGS = -Wall -Wno-missing-braces -Wno-char-subscripts -Wno-long-long \
           -pedantic -DNCURSES -DSCI_LEXER $(INCLUDEDIRS)
ifdef DEBUG
  CXXFLAGS += -DDEBUG -g
else
  CXXFLAGS += -DNDEBUG -Os
endif
CFLAGS = CXXFLAGS

scintilla = ../bin/scintilla.a
lexers = $(addsuffix .o,$(basename $(notdir $(wildcard ../lexers/Lex*.cxx))))

vpath %.h ../src ../include ../lexlib
vpath %.cxx ../src ../lexlib ../lexers

all: $(scintilla)
.cxx.o:
	$(CXX) $(CXXFLAGS) -c $<
$(scintilla): AutoComplete.o CallTip.o Catalogue.o CellBuffer.o CharClassify.o \
              ContractionState.o Decoration.o Document.o Editor.o \
              ExternalLexer.o Indicator.o KeyMap.o LineMarker.o PerLine.o \
              PositionCache.o RESearch.o RunStyles.o ScintillaBase.o \
              Selection.o Style.o UniConversion.o ViewStyle.o XPM.o Accessor.o \
              CharacterSet.o LexerBase.o LexerModule.o LexerNoExceptions.o \
              LexerSimple.o PropSetSimple.o StyleContext.o WordList.o \
              $(lexers) ScintillaTerm.o
	ar rc $@ $^
	touch $@
clean:
	rm -f *.o $(scintilla)

# Documentation.

doc: manual luadoc
manual: doc/*.md *.md
	doc/bombay -d doc -t doc --title Scinterm --navtitle Manual $^
luadoc: doc/scinterm.luadoc
	luadoc -d doc -t doc --doclet doc/markdowndoc $^
cleandoc:
	rm -f doc/*.html
	rm -rf doc/api
