## Changelog

[Atom Feed](https://github.com/orbitalquark/scinterm/releases.atom)

### 4.0 (05 Sep 2022)

Download:

* [Scinterm 4.0][]

Bugfixes:

* None.

Changes:

* Enable horizontal scroll with Shift+Mouse Wheel.
* Changed `scintilla_send_mouse()` API to no longer require a time argument.
* Updated to [Scintilla][] 5.3.0.

[Scinterm 4.0]: https://github.com/orbitalquark/scinterm/archive/scinterm_4.0.zip
[Scintilla]: https://scintilla.org


### 3.2 (11 Nov 2021)

Download:

* [Scinterm 3.2][]

Bugfixes:

* Fixed accidental drawing of whitespace, tab arrows, and indentation guides in margins when
  scrolling horizontally.
* Fixed accidental highlighting in margins when scrolling horizontally.
* Fixed occasional incorrect drawing when scrolling horizontally.

Changes:

* Hide the terminal cursor when Scintilla's caret is out of view.
* Updated to [Scintilla][] 5.1.4.

[Scinterm 3.2]: https://github.com/orbitalquark/scinterm/archive/scinterm_3.2.zip
[Scintilla]: https://scintilla.org

### 3.1 (26 Jun 2021)

Download:

* [Scinterm 3.1][]

Bugfixes:

* None.

Changes:

* Added support for native terminal caret and added [`scintilla_update_cursor()`][].
* Updated to [Scintilla][] 5.1.0.

[Scinterm 3.1]: https://github.com/orbitalquark/scinterm/archive/scinterm_3.1.zip
[`scintilla_update_cursor()`]: api.html#scintilla_update_cursor
[Scintilla]: https://scintilla.org

### 3.0 (27 Jul 2020)

Download:

* [Scinterm 3.0][] (Released in [Scintilla 3.21.0][])

Bugfixes:

* None.

Changes:

* [`scintilla_get_clipboard()`][] now returns a copy of clipboard text directly.

[Scinterm 3.0]: https://github.com/orbitalquark/scinterm/archive/scinterm_3.0.zip
[Scintilla 3.21.0]: https://sourceforge.net/projects/scintilla/files/scintilla/3.21.0/scintilla3210.zip/download
[`scintilla_get_clipboard()`]: api.html#scintilla_get_clipboard

### 2.0 (09 May 2020)

Download:

* [Scinterm 2.0][] (Released in [Scintilla 3.20.0][])

Bugfixes:

* Fixed overlapping window drawing bug in Windows.
* Fixed drawing of block caret in overtype mode.

Changes:

* Added userdata pointer for SCNotifications in [`scintilla_new()`][].
* Enable autoscrolling when clicking and dragging the mouse to make selections.
* Updated to [Scintilla][] 3.20.0.

[Scinterm 2.0]: https://github.com/orbitalquark/scinterm/archive/scinterm_2.0.zip
[Scintilla 3.20.0]: https://sourceforge.net/projects/scintilla/files/scintilla/3.20.0/scintilla3200.zip/download
[`scintilla_new()`]: api.html#scintilla_new
[Scintilla]: https://scintilla.sourceforge.io/LongTermDownload.html

### 1.12 (30 Aug 2019)

Download:

* [Scinterm 1.12][] (Released in [Scintilla 3.11.0][])

Bugfixes:

* None.

Changes:

* Updated to [Scintilla][] 3.11.0.

[Scinterm 1.12]: https://github.com/orbitalquark/scinterm/archive/scinterm_1.12.zip
[Scintilla 3.11.0]: https://sourceforge.net/projects/scintilla/files/scintilla/3.11.0/scintilla3110.zip/download
[Scintilla]: https://scintilla.sourceforge.io/LongTermDownload.html

### 1.11 (30 Jun 2018)

Download:

* [Scinterm 1.11][] (Released in [Scintilla 3.10.0][])

Bugfixes:

* Fixed potential crashes with newer versions of ncurses.

Changes:

* Updated to [Scintilla][] 3.10.0.

[Scinterm 1.11]: https://github.com/orbitalquark/scinterm/archive/scinterm_1.11.zip
[Scintilla 3.10.0]: https://sourceforge.net/projects/scintilla/files/scintilla/3.10.0/scintilla3100.zip/download
[Scintilla]: https://scintilla.sourceforge.io/LongTermDownload.html

### 1.10 (26 Mar 2018)

Download:

* [Scinterm 1.10][] (Released in [Scintilla 3.8.0][])

Bugfixes:

* None.

Changes:

* Update the cursor position when drawing focused views.
* Renamed *ScintillaCurses.\** to *ScintillaTerm.\**.
* Updated to [Scintilla][] 3.8.0.

[Scinterm 1.10]: https://github.com/orbitalquark/scinterm/archive/scinterm_1.10.zip
[Scintilla 3.8.0]: https://sourceforge.net/projects/scintilla/files/scintilla/3.8.0/scintilla380.zip/download
[Scintilla]: https://scintilla.sourceforge.io/LongTermDownload.html

### 1.9 (25 Jun 2017)

Download:

* [Scinterm 1.9][]

Bugfixes:

* Fixed various compiler warnings.

Changes:

* Use a curses primitive instead of implementation-specific workaround when changing text
  attributes.
* Updated to [Scintilla][] 3.7.5.

[Scinterm 1.9]: https://github.com/orbitalquark/scinterm/archive/scinterm_1.9.zip
[Scintilla]: https://scintilla.org

### 1.8 (26 Apr 2016)

Download:

* [Scinterm 1.8][]

Bugfixes:

* Many improvements to UTF-8 character handling.
* Fix building on some versions of Mac OSX.

Changes:

* Improved ncurses wide-character detection.
* Added [`scintilla_noutrefresh()`][] for more efficient refreshing.

[Scinterm 1.8]: https://github.com/orbitalquark/scinterm/archive/scinterm_1.8.zip
[Scintilla]: https://scintilla.org
[`scintilla_noutrefresh()`]: api.html#scintilla_noutrefresh

### 1.7 (03 Feb 2016)

Download:

* [Scinterm 1.7][]

Bugfixes:

* None.

Changes:

* Delay creation of curses `WINDOW`s for as long as possible.
* Updated to [Scintilla][] 3.6.3.

[Scinterm 1.7]: https://github.com/orbitalquark/scinterm/archive/scinterm_1.7.zip
[Scintilla]: https://scintilla.org

### 1.6 (18 Apr 2015)

Download:

* [Scinterm 1.6][]

Bugfixes:

* Fixed indicator drawing over multiple styles.

Changes:

* Updated to [Scintilla][] 3.5.5.

[Scinterm 1.6]: https://github.com/orbitalquark/scinterm/archive/scinterm_1.6.zip
[Scintilla]: https://scintilla.org

### 1.5 (10 Dec 2014)

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
* Added support for drawing wrap markers. (Requires a font with good glyph support, like DejaVu
  Sans Mono.)
* Added support for drawing nearly all line markers. (Requires a font with good glyph support,
  like DejaVu Sans Mono or Liberation Mono.)
* Added support for drawing indentation guides.
* Updated to [Scintilla][] 3.5.2.

[Scinterm 1.5]: https://github.com/orbitalquark/scinterm/archive/scinterm_1.5.zip
[Scintilla]: https://scintilla.org

### 1.4 (01 Sep 2014)

Download:

* [Scinterm 1.4][]

Bugfixes:

* Fixed nested folding marker drawing.

Changes:

* Updated to [Scintilla][] 3.5.0.

[Scinterm 1.4]: https://github.com/orbitalquark/scinterm/archive/scinterm_1.4.zip
[Scintilla]: https://scintilla.org

### 1.3 (09 Jul 2014)

Download:

* [Scinterm 1.3][]

Bugfixes:

* Clip long lines properly when view width is less than `COLS`.
* Fixed painting bug in resized views.

Changes:

* Updated to [Scintilla][] 3.4.2.

[Scinterm 1.3]: https://github.com/orbitalquark/scinterm/archive/scinterm_1.3.zip
[Scintilla]: https://scintilla.org

### 1.2 (05 Feb 2014)

Download:

* [Scinterm 1.2][]

Bugfixes:

* Fixed bug with calltip styles.
* Fixed various compile issues and warnings.
* Interpret UTF-8 input correctly.
* Fix crash when trying to draw a call tip larger than the window.
* Fixed bug in nested fold markers not showing up.

Changes:

* Added support for `INDIC_ROUNDBOX` and `INDIC_STRAIGHTBOX`, but with no translucency.
* Support curses, PDCurses, and X/Open Curses.
* Recognize up to 16 separate colors for terminals that support it.
* Updated to [Scintilla][] 3.3.7.

[Scinterm 1.2]: https://github.com/orbitalquark/scinterm/archive/scinterm_1.2.zip
[Scintilla]: https://scintilla.org

### 1.1 (11 Dec 2012)

Download:

* [Scinterm 1.1][]

Bugfixes:

* Fixed crash when pasting uninitialized clipboard text.
* Fixed bug with drawing around italic characters at EOLs.
* Display control characters properly.
* Fixed bug in searching autocompletion lists.

Changes:

* Expose [`SCI_COLOR_PAIR()`][] macro.

[Scinterm 1.1]: https://github.com/orbitalquark/scinterm/archive/scinterm_1.1.zip
[`SCI_COLOR_PAIR()`]: api.html#SCI_COLOR_PAIR

### 1.0 (31 Aug 2012)

Download:

* [Scinterm 1.0][]

Initial release.

[Scinterm 1.0]: https://github.com/orbitalquark/scinterm/archive/scinterm_1.0.zip
