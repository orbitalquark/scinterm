# Changelog

[Atom Feed][] | [PGP Public Key][]

[Atom Feed]: feed
[PGP Public Key]: https://foicica.com/foicica.pgp

## 1.9 (25 Jun 2017)

Download:

* [Scinterm 1.9][] | [PGP -- 1.9][]

Bugfixes:

* Fixed various compiler warnings.

Changes:

* Use a curses primitive instead of implementation-specific workaround when
  changing text attributes.
* Updated to [Scintilla][] 3.7.5.

[Scinterm 1.9]: download/scinterm_1.9.zip
[PGP -- 1.9]: download/scinterm_1.9.zip.asc
[Scintilla]: http://scintilla.org

## 1.8 (26 Apr 2016)

Download:

* [Scinterm 1.8][] | [PGP -- 1.8][]

Bugfixes:

* Many improvements to UTF-8 character handling.
* Fix building on some version of Mac OSX.

Changes:

* Improved ncurses wide-character detection.
* Added [`scintilla_noutrefresh()`][] for more efficient refreshing.

[Scinterm 1.8]: download/scinterm_1.8.zip
[PGP -- 1.8]: download/scinterm_1.8.zip.asc
[Scintilla]: http://scintilla.org
[`scintilla_noutrefresh()`]: api.html#scintilla_noutrefresh

## 1.7 (03 Feb 2016)

Download:

* [Scinterm 1.7][] | [PGP -- 1.7][]

Bugfixes:

* None.

Changes:

* Delay creation of curses `WINDOW`s for as long as possible.
* Updated to [Scintilla][] 3.6.3.

[Scinterm 1.7]: download/scinterm_1.7.zip
[PGP -- 1.7]: download/scinterm_1.7.zip.asc
[Scintilla]: http://scintilla.org

## 1.6 (18 Apr 2015)

Download:

* [Scinterm 1.6][] | [PGP -- 1.6][]

Bugfixes:

* Fixed indicator drawing over multiple styles.

Changes:

* Updated to [Scintilla][] 3.5.5.

[Scinterm 1.6]: download/scinterm_1.6.zip
[PGP -- 1.6]: download/scinterm_1.6.zip.asc
[Scintilla]: http://scintilla.org

## 1.5 (10 Dec 2014)

Download:

* [Scinterm 1.5][] | [PGP -- 1.5][]

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
[PGP -- 1.5]: download/scinterm_1.5.zip.asc
[Scintilla]: http://scintilla.org

## 1.4 (01 Sep 2014)

Download:

* [Scinterm 1.4][] | [PGP -- 1.4][]

Bugfixes:

* Fixed nested folding marker drawing.

Changes:

* Updated to [Scintilla][] 3.5.0.

[Scinterm 1.4]: download/scinterm_1.4.zip
[PGP -- 1.4]: download/scinterm_1.4.zip.asc
[Scintilla]: http://scintilla.org

## 1.3 (09 Jul 2014)

Download:

* [Scinterm 1.3][] | [PGP -- 1.3][]

Bugfixes:

* Clip long lines properly when view width is less than `COLS`.
* Fixed painting bug in resized views.

Changes:

* Updated to [Scintilla][] 3.4.2.

[Scinterm 1.3]: download/scinterm_1.3.zip
[PGP -- 1.3]: download/scinterm_1.3.zip.asc
[Scintilla]: http://scintilla.org

## 1.2 (05 Feb 2014)

Download:

* [Scinterm 1.2][] | [PGP -- 1.2][]

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
[PGP -- 1.2]: download/scinterm_1.2.zip.asc
[Scintilla]: http://scintilla.org

## 1.1 (11 Dec 2012)

Download:

* [Scinterm 1.1][] | [PGP -- 1.1][]

Bugfixes:

* Fixed crash when pasting uninitialized clipboard text.
* Fixed bug with drawing around italic characters at EOLs.
* Display control characters properly.
* Fixed bug in searching autocompletion lists.

Changes:

* Expose [`SCI_COLOR_PAIR()`][] macro.

[Scinterm 1.1]: download/scinterm_1.1.zip
[PGP -- 1.1]: download/scinterm_1.1.zip.asc
[`SCI_COLOR_PAIR()`]: api.html#SCI_COLOR_PAIR

## 1.0 (31 Aug 2012)

Download:

* [Scinterm 1.0][] | [PGP -- 1.0][]

Initial release.

[Scinterm 1.0]: download/scinterm_1.0.zip
[PGP -- 1.0]: download/scinterm_1.0.zip.asc
