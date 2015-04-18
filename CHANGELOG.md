# Changelog

[Atom Feed][]

[Atom Feed]: feed

## 1.6 (18 Apr 2015)

Download:

* [Scinterm 1.6][]

Bugfixes:

* Fixed indicator drawing over multiple styles.

Changes:

* Updated to [Scintilla][] 3.5.5.

[Scinterm 1.6]: download/scinterm_1.6.zip
[Scintilla]: http://scintilla.org

## 1.5 (10 Dec 2014)

Download:

* [Scinterm 1.5][]

Bugfixes:

* Fixed positioning of autocomplete and call tips in some edge cases.
* Margins are no longer overwritten by long lines.
* Allow UTF-8 characters as list images.

Changes:

* Added complete mouse support.
* Added support for drawing call tip arrows.
* Implemented scrollbars.
* Added support for drawing tab arrows when visualizing whitespace.
* Added support for drawing wrap markers. (Requires a font with good
  glyph support, like DejaVu Sans Mono.)
* Added support for drawing nearly all line markers. (Requires a font with good
  glyph support, like DejaVu Sans Mono or Liberation Mono.)
* Added support for drawing indentation guides.
* Updated to [Scintilla][] 3.5.2.

[Scinterm 1.5]: download/scinterm_1.5.zip
[Scintilla]: http://scintilla.org

## 1.4 (01 Sep 2014)

Download:

* [Scinterm 1.4][]

Bugfixes:

* Fixed nested folding marker drawing.

Changes:

* Updated to [Scintilla][] 3.5.0.

[Scinterm 1.4]: download/scinterm_1.4.zip
[Scintilla]: http://scintilla.org

## 1.3 (09 Jul 2014)

Download:

* [Scinterm 1.3][]

Bugfixes:

* Clip long lines properly when view width is less than `COLS`.
* Fixed painting bug in resized views.

Changes:

* Updated to [Scintilla][] 3.4.2.

[Scinterm 1.3]: download/scinterm_1.3.zip
[Scintilla]: http://scintilla.org

## 1.2 (05 Feb 2014)

Download:

* [Scinterm 1.2][]

Bugfixes:

* Fixed bug with calltip styles.
* Fixed various compile issues and warnings.
* Interpret UTF-8 input correctly.
* Fix crash when trying to draw a call tip larger than the window.
* Fixed bug in nested fold markers not showing up.

Changes:

* Added support for `INDIC_ROUNDBOX` and `INDIC_STRAIGHTBOX`, but with no
  translucency.
* Support curses, PDCurses, and X/Open Curses.
* Recognize up to 16 separate colors for terminals that support it.
* Updated to [Scintilla][] 3.3.7.

[Scinterm 1.2]: download/scinterm_1.2.zip
[Scintilla]: http://scintilla.org

## 1.1 (11 Dec 2012)

Download:

* [Scinterm 1.1][]

Bugfixes:

* Fixed crash when pasting uninitialized clipboard text.
* Fixed bug with drawing around italic characters at EOLs.
* Display control characters properly.
* Fixed bug in searching autocompletion lists.

Changes:

* Expose [`SCI_COLOR_PAIR()`][] macro.

[Scinterm 1.1]: download/scinterm_1.1.zip
[`SCI_COLOR_PAIR()`]: api.html#SCI_COLOR_PAIR

## 1.0 (31 Aug 2012)

Download:

* [Scinterm 1.0][]

Initial release.

[Scinterm 1.0]: download/scinterm_1.0.zip
