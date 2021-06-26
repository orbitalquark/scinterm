## Introduction

Scinterm is a curses platform for [Scintilla][] that supports [ncurses][], [PDCurses][], and
X/Open Curses.

![Scinterm](images/scinterm.png)

It is highly recommended to run Scinterm in a UTF-8-aware terminal with a font that supports
many UTF-8 characters ("DejaVu Sans Mono" is one of them), since Scinterm makes use of UTF-8
characters when drawing wrap symbols, some marker symbols, and call tip arrows.

[Scintilla]: https://scintilla.org
[ncurses]: https://invisible-island.net/ncurses/
[PDCurses]: https://pdcurses.org

## Requirements

* Scinterm currently requires Scintilla 5.1.0 - 5.x.
* Scinterm 3.0 requires Scintilla 3.20.0 - 3.21.0.
* Scinterm 2.0 requires Scintilla 3.20.0 - 3.21.0.
* Scinterm 1.12 requires Scintilla 3.11.0 - 3.11.2.
* Scinterm 1.11 requires Scintilla 3.10.0 - 3.10.6.
* Scinterm 1.10 requires Scintilla 3.8.0.
* Scinterm 1.9 requires Scintilla 3.7.5 - 3.7.6.
* Scinterm 1.8 requires Scintilla 3.6.3 - 3.7.4.
* Scinterm 1.7 requires Scintilla 3.6.3 - 3.7.4.
* Scinterm 1.6 requires Scintilla 3.5.5 - 3.6.2.
* Scinterm 1.5 requires Scintilla 3.5.2 - 3.5.4.
* Scinterm 1.4 requires Scintilla 3.5.0 - 3.5.1.
* Scinterm 1.3 requires Scintilla 3.4.2 - 3.4.4.
* Scinterm 1.2 requires Scintilla 3.3.7 - 3.4.1.
* Scinterm 1.1 requires Scintilla 3.2.2 - 3.3.6.
* Scinterm 1.0 requires Scintilla 3.2.2 - 3.3.6.

## Download

Scinterm releases can be found [here][].

[here]: https://github.com/orbitalquark/scinterm/releases

## Compile

After downloading Scinterm, it is recommended to unzip it into the top-level directory of an
instance of Scintilla, similar to other Scintilla platforms like *gtk/* and *win32/*. After
that, go into the Scinterm directory and run `make patch` followed by `make` to build the usual
*../bin/scintilla.a*.

You can optionally build the demo application, jinx, by going into *jinx/* and running
`make`. Pressing the `q` key quits the demo. Note that the demo assumes [lexilla][]
is a sibling to the *../../../scintilla* directory, and that it has been built
(i.e. *../../../lexilla/bin/liblexilla.so* exists).

[lexilla]: https://www.scintilla.org/Lexilla.html

## Usage

Scinterm's Application Programming Interface [(API) documentation][] is located in the project's
*docs/* directory and covers how to create and interact with a Scintilla widget in a terminal
application.

[(API) documentation]: api.html

## Curses Compatibility

Scinterm lacks some Scintilla features due to the terminal's constraints:

* Any settings with alpha values are not supported.
* Autocompletion lists cannot show images (pixmap surfaces are not supported).  Instead, they
  show the first character in the string passed to [`SCI_REGISTERIMAGE`][].
* Buffered drawing is not supported.
* Caret settings like period, line style, and width are not supported (terminals use block
  carets with their own period definitions).
* Code pages other than UTF-8 have not been tested and it is possible some curses implementations
  do not support them.
* Drag and drop is not supported.
* Edge lines are not displayed properly (the line is drawn over by text lines).
* Extra ascent and descent for lines is not supported.
* Fold lines cannot be drawn above or below lines.
* Hotspot underlines are not drawn on mouse hover (`surface->FillRectangle()` is not supported).
* Indicators other than `INDIC_ROUNDBOX` and `INDIC_STRAIGHTBOX` are not drawn (`surface->LineTo()`
  and `surface->FillRectangle()` are not supported for drawing indicator shapes and pixmap
  surfaces are not supported). Translucent drawing and rounded corners are not supported either.
* Some complex marker types are not drawn properly or at all (pixmap surfaces are not supported
  and `surface->LineTo()` is not supported for drawing some marker shapes).
* Mouse cursor types are not supported.
* Up to 16 colors are supported, regardless of how many colors the terminal supports. They are
  (in "0xBBGGRR" format): black (`0x000000`), red (`0x000080`), green (`0x008000`), yellow
  (`0x008080`), blue (`0x800000`), magenta (`0x800080`), cyan (`0x808000`), white (`0xC0C0C0`),
  light black (`0x404040`), light red (`0x0000FF`), light green (`0x00FF00`), light yellow
  (`0x00FFFF`), light blue (`0xFF0000`), light magenta (`0xFF00FF`), light cyan (`0xFFFF00`),
  and light white (`0xFFFFFF`). Even if your terminal uses a different color map, you must use
  these color values with Scintilla; unrecognized colors are set to white by default. For some
  terminals, you may need to set a lexer style's `bold` attribute in order to use the light
  color variant.
* Some styles settings like font name, font size, and italic do not display properly (terminals
  use one only font, size and variant).
* X selections (primary and secondary) are not integrated into the clipboard.
* Zoom is not supported (terminal font size is fixed).
* When using the mouse in the Windows console, Shift+Double-click extends selections and
  quadruple-clicking inside a selection collapses it.

[`SCI_REGISTERIMAGE`]: https://scintilla.org/ScintillaDoc.html#SCI_REGISTERIMAGE

## Contribute

Scinterm is [open source][]. Feel free to report bugs and submit patches. Thanks to [everyone][]
who has contributed.

[open source]: https://github.com/orbitalquark/scinterm
[everyone]: thanks.html
