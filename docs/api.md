## Scinterm API Documentation

A curses platform for [Scintilla][] that supports curses platforms like [ncurses][].

[Scintilla]: https://scintilla.org
[ncurses]: https://invisible-island.net/ncurses/

### Functions defined by `Scinterm`

<a id="scintilla_delete"></a>
#### `scintilla_delete`(*sci*)

Deletes the given Scintilla window.

Parameters:

- *sci*:  The Scintilla window returned by `scintilla_new()`.

Return:

- `void`

<a id="scintilla_get_clipboard"></a>
#### `scintilla_get_clipboard`(*sci*, *len*)

Returns a null-terminated copy of the text on Scintilla's internal clipboard (not the primary
and/or secondary X selections) and stores its length in *len*.
The caller is responsible for `free`ing the returned text.
Keep in mind clipboard text may contain null bytes.

Parameters:

- *sci*:  The Scintilla window returned by `scintilla_new()`.
- *len*:  (`int *`) The length of the returned text.

Return:

- `char *` clipboard text (caller is responsible for `free`ing it)

<a id="scintilla_get_window"></a>
#### `scintilla_get_window`(*sci*)

Returns the curses `WINDOW` associated with the given Scintilla window.

Parameters:

- *sci*:  The Scintilla window returned by `scintilla_new()`.

Return:

- curses `WINDOW`.

<a id="scintilla_new"></a>
#### `scintilla_new`(*callback*, *userdata*)

Creates a new Scintilla curses window.

Parameters:

- *callback*:  SCNotification callback function of the form: `void callback(Scintilla *,
   int, void *, void *)`.
- *userdata*:  (`void *`) Userdata to pass to *callback*.

Return:

- `Scintilla *`

<a id="scintilla_noutrefresh"></a>
#### `scintilla_noutrefresh`(*sci*)

Refreshes the Scintilla window on the virtual screen.
This should be done along with the normal curses `noutrefresh()`.
Note: the terminal cursor may be hidden if Scintilla thinks this window has focus
(e.g. `SCI_SETFOCUS`) and Scintilla's caret is out of view. If another non-Scintilla window
has the real focus, call `curs_set(1)` in order to show the terminal cursor for that window.

Parameters:

- *sci*:  The Scintilla window returned by `scintilla_new()`.

<a id="scintilla_refresh"></a>
#### `scintilla_refresh`(*sci*)

Refreshes the Scintilla window on the physical screen.
This should be done along with the normal curses `refresh()`.
Note: the terminal cursor may be hidden if Scintilla thinks this window has focus
(e.g. `SCI_SETFOCUS`) and Scintilla's caret is out of view. If another non-Scintilla window
has the real focus, call `curs_set(1)` in order to show the terminal cursor for that window.

Parameters:

- *sci*:  The Scintilla window returned by `scintilla_new()`.

Return:

- `void`

<a id="scintilla_send_key"></a>
#### `scintilla_send_key`(*sci*, *key*, *modifiers*)

Sends the specified key to the given Scintilla window for processing.
If it is not consumed, an SCNotification will be emitted.

Parameters:

- *sci*:  The Scintilla window returned by `scintilla_new()`.
- *key*:  (`int`) The keycode of the key, or, if Scintilla's code page is UTF-8, the UTF-8
   code point of the key.
- *modifiers*:  (`int`) Bit-mask of `SCMOD_*` modifier keys.

Return:

- `void`

<a id="scintilla_send_message"></a>
#### `scintilla_send_message`(*sci*, *iMessage*, *wParam*, *lParam*)

Sends the given message with parameters to the given Scintilla window.

Parameters:

- *sci*:  The Scintilla window returned by `scintilla_new()`.
- *iMessage*:  (`int`) The Scintilla message ID.
- *wParam*:  (`uptr_t`) The first parameter.
- *lParam*:  (`sptr_t`) The second parameter.

Return:

- `sptr_t`

<a id="scintilla_send_mouse"></a>
#### `scintilla_send_mouse`(*sci*, *event*, *button*, *modifiers*, *y*, *x*)

Sends the specified mouse event to the given Scintilla window for processing.

Parameters:

- *sci*:  The Scintilla window returned by `scintilla_new()`.
- *event*:  (`int`) The mouse event (`SCM_CLICK`, `SCM_DRAG`, or `SCM_RELEASE`).
- *button*:  (`int`) The button number pressed, or `0` if none.
- *modifiers*:  (`int`) Bit-mask of `SCMOD_*` modifier keys.
- *y*:  (`int`) The absolute y coordinate of the mouse event.
- *x*:  (`int`) The absolute x coordinate of the mouse event.

Return:

- `bool` whether or not Scintilla handled the mouse event.

<a id="scintilla_update_cursor"></a>
#### `scintilla_update_cursor`(*sci*)

Updates the curses window cursor for the Scintilla window so the terminal draws the cursor
in the correct position.
This only needs to be called when `scintilla_refresh()` or `scintilla_noutrefresh()` is not
the last curses draw command issued (for any window, not just the Scintilla window).
Note: the terminal cursor may be hidden if Scintilla thinks this window has focus
(e.g. `SCI_SETFOCUS`) and Scintilla's caret is out of view. If another non-Scintilla window
has the real focus, call `curs_set(1)` in order to show the terminal cursor for that window.

Parameters:

- *sci*:  The Scintilla window returned by `scintilla_new()`.

Return:

- `void`


