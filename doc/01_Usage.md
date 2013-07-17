# Usage

## Compiling

After downloading Scinterm, it is recommended to unzip it into the top-level
directory of an instance of Scintilla, similar to other Scintilla platforms like
`gtk/` and `win32/`. After that, go into the Scinterm directory and run `make`
to build the usual `../bin/scintilla.a`.

## Curses Compatibility

Scinterm lacks some Scintilla features due to curses' non-existant graphics
capabilities:

* Any settings with alpha values are not supported.
* Autocompletion lists cannot show images (pixmap surfaces are not supported).
  Instead, they show the first character in the string passed to
  [`SCI_REGISTERIMAGE`][].
* Buffered and two-phase drawing is not supported.
* Call tip arrows are not displayed (`surface->Polygon()` is not supported for
  drawing arrow shapes).
* Caret settings like period, line style, and width are not supported
  (terminals use block carets with their own period definitions).
* Code pages other than UTF-8 have not been tested and it is possible some
  curses implementations do not support them.
* Edge lines are not displayed properly (the line is drawn over by text lines).
* Extra ascent and descent for lines is not supported.
* Fold lines are not supported (`surface->LineTo()` is not supported).
* Indentation guides are not visible (pixmap surfaces are not supported).
* Indicators other than `INDIC_ROUNDBOX` and `INDIC_STRAIGHTBOX` are not drawn
  (`surface->LineTo()` is not supported for drawing indicator shapes and pixmap
  surfaces are not supported). Translucent drawing and rounded corners are not
  supported either.
* Insert mode caret is not drawn properly (no way to detect it from within
  `surface->FillRectangle()`).
* Margins are overwritten by long lines when scrolling to the right.
* Marker types other than `SC_MARK_CHARACTER` are not drawn (pixmap surfaces are
  not supported and `surface->LineTo()` is not supported for drawing marker
  shapes).
* Mouse interactions, cursor types, and hotspots are not supported.
* Up to 16 colors are supported: black (`0x000000`), red (`0x800000`), green
  (`0x008000`), yellow (`0x808000`), blue (`0x000080`), magenta (`0x800080`),
  cyan (`0x008080`), white (`0xC0C0C0`), light black (`0x404040`), light red
  (`0xFF0000`), light green (`0x00FF00`), light yellow (`0xFFFF00`), light blue
  (`0x0000FF`), light magenta (`0xFF00FF`), light cyan (`0x00FFFF`), and light
  white (`0xFFFFFF`). Even if your terminal uses a different color map, you must
  use these color values with Scintilla; unrecognized colors are set to white by
  default. For some terminals, you may need to set a lexer style's `bold`
  attribute in order to use the light color variant.
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

`jinx` is an example of using Scintilla with curses. You can build it by going
into `jinx/` and running `make`.
