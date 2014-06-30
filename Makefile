# Copyright 2012-2014 Mitchell mitchell.att.foicica.com. See LICENSE.

.SUFFIXES: .cxx .c .o .h .a

AR = ar
CXX = g++
INCLUDEDIRS = -I ../include -I ../src -I ../lexlib
CXXFLAGS = -Wall -Wno-missing-braces -Wno-char-subscripts -Wno-long-long \
           -pedantic -DCURSES -DSCI_LEXER $(INCLUDEDIRS)
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
$(scintilla): AutoComplete.o CallTip.o CaseConvert.o CaseFolder.o Catalogue.o \
              CellBuffer.o CharClassify.o ContractionState.o Decoration.o \
              Document.o Editor.o ExternalLexer.o Indicator.o KeyMap.o \
              LineMarker.o PerLine.o PositionCache.o RESearch.o RunStyles.o \
              ScintillaBase.o Selection.o Style.o UniConversion.o ViewStyle.o \
              XPM.o Accessor.o CharacterCategory.o CharacterSet.o LexerBase.o \
              LexerModule.o LexerNoExceptions.o LexerSimple.o PropSetSimple.o \
              StyleContext.o WordList.o $(lexers) ScintillaTerm.o
	$(AR) rc $@ $^
	touch $@
clean:
	rm -f *.o $(scintilla)

# Documentation.

doc: manual luadoc
manual: doc/*.md *.md | doc/bombay
	doc/bombay -d doc -t doc --title Scinterm $^
luadoc: doc/scinterm.luadoc
	luadoc -d doc -t doc --doclet doc/markdowndoc $^
cleandoc: ; rm -f doc/manual.html doc/api.html

# Release.

release_dir = scinterm_$(shell grep "^\#\#" CHANGELOG.md | head -1 | \
                               cut -d ' ' -f 2)
package = releases/$(release_dir).zip

release: doc
	hg archive $(release_dir)
	rm $(release_dir)/.hg*
	cp -rL doc $(release_dir)
	zip -r $(package) $(release_dir)
	rm -r $(release_dir)

# External dependencies.

bombay_zip = bombay.zip

$(bombay_zip):
	wget "http://foicica.com/hg/bombay/archive/tip.zip" && mv tip.zip $@
doc/bombay: | $(bombay_zip)
	mkdir $(notdir $@) && unzip -d $(notdir $@) $| && \
		mv $(notdir $@)/*/* $(dir $@)
