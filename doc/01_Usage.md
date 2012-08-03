# Usage

## Compiling

After downloading Scinterm, it is recommended to unzip it into the top-level
directory of an instance of Scintilla, similar to other Scintilla platforms like
`gtk/` and `win32/`. After that, go into the Scinterm directory and run `make`
to build the usual `../bin/scintilla.a`.

## Ncurses Compatibility

Since ncurses is quite low-level in terms of graphics capability compared to GUI
toolkits like GTK and Qt, Scinterm lacks some Scintilla features:

* Any settings with alpha values are not supported.
* Autocompletion lists cannot show images (pixmap surfaces are not supported).
  Instead, they show the first character in the string passed to
  [`SCI_REGISTERIMAGE`][].
* Buffered and two-phase drawing is not supported.
* Call tip arrows are not displayed (`surface->Polygon()` is not supported for
  drawing arrow shapes).
* Caret settings like period, line style, and width are not supported
  (terminals use block carets with their own period definitions).
* Code pages other than UTF-8 have not been tested and it is possible ncurses
  does not support them.
* Edge lines are not displayed properly (the line is drawn over by text lines).
* Extra ascent and decent for lines is not supported.
* Fold lines are not supported (`surface->LineTo()` is not supported).
* Indentation guides are not visible (pixmap surfaces are not supported).
* Indicators are not displayed (they would be drawn over by text lines).
* Insert mode caret is not drawn properly (no way to detect it from within
  `surface->FillRectangle()`).
* Margins are overwritten by long lines when scrolling to the right.
* Marker types other than `SC_MARK_CHARACTER` are not drawn (pixmap surfaces are
  not supported and `surface->LineTo()` is not supported for drawing marker
  shapes and ).
* Mouse interactions, cursor types, and hotspots are not supported.
* Only 8 colors are supported: black (`0x000000`), red (`0xFF0000`), green
  (`0x00FF00`), yellow (`0xFFFF00`), blue (`0x0000FF`), magenta (`0xFF00FF`),
  cyan (`0x00FFFF`), and white (`0xFFFFFF`). Even if your terminal uses a
  different color map, you must use these color values with Scintilla;
  unrecognized colors are set to white by default. Lexers can use up to 8 more
  colors by setting the lexer style's `bold` attribute.
* Scroll bars are not supported.
* Some styles settings like font name, font size, and italic do not display
  properly (terminals use one only font, size and variant).
* Viewing whitespace does not show the 'Tab' character arrows
  (`surface->LineTo()` is not supported for drawing arrows).
* Visual wrap flags are not supported (`surface->LineTo()` is not supported).
* X selections (primary and secondary) are not integrated into the clipboard.
* Zoom is not supported (terminal font size is fixed).

[`SCI_REGISTERIMAGE`]: http://scintilla.org/ScintillaDoc.html#SCI_REGISTERIMAGE

## `jinx`

`jinx` is an example of using Scintilla with ncurses. You can build it by going
into `jinx/` and running `make`.
