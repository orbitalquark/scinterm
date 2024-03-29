// Copyright 2012-2024 Mitchell. See LICENSE.
// Client header for Scintilla in a curses (terminal) environment.

#ifndef SCINTILLA_CURSES_H
#define SCINTILLA_CURSES_H

#include <curses.h>

#include "Scintilla.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Creates a new Scintilla window.
 * Curses does not have to be initialized before calling this function.
 * @param callback A callback function for Scintilla notifications.
 * @param userdata Userdata to pass to *callback*.
 */
void *scintilla_new(
	void (*callback)(void *sci, int iMessage, SCNotification *n, void *userdata), void *userdata);

/**
 * Returns the curses `WINDOW` associated with the given Scintilla window.
 * Curses must have been initialized prior to calling this function.
 * @param sci The Scintilla window returned by `scintilla_new()`.
 * @return curses `WINDOW`.
 */
WINDOW *scintilla_get_window(void *sci);

/**
 * Sends the given message with parameters to the given Scintilla window.
 * Curses does not have to be initialized before calling this function.
 * @param sci The Scintilla window returned by `scintilla_new()`.
 * @param iMessage The message ID.
 * @param wParam The first parameter.
 * @param lParam The second parameter.
 */
sptr_t scintilla_send_message(void *sci, unsigned int iMessage, uptr_t wParam, sptr_t lParam);

/**
 * Sends the specified key to the given Scintilla window for processing.
 * If it is not consumed, an SCNotification will be emitted.
 * Curses does not have to be initialized before calling this function.
 * @param sci The Scintilla window returned by `scintilla_new()`.
 * @param key The keycode of the key.
 * @param modifiers Bit-mask of `SCMOD_*` modifier keys.
 */
void scintilla_send_key(void *sci, int key, int modifiers);

/**
 * Sends the specified mouse event to the given Scintilla window for processing.
 * Curses must have been initialized prior to calling this function.
 * @param sci The Scintilla window returned by `scintilla_new()`.
 * @param event The mouse event (`SCM_CLICK`, `SCM_DRAG`, or `SCM_RELEASE`).
 * @param button The button number pressed, or `0` if none.
 * @param modifiers Bit-mask of `SCMOD_*` modifier keys.
 * @param y The absolute y coordinate of the mouse event.
 * @param x The absolute x coordinate of the mouse event.
 * @return whether or not Scintilla handled the mouse event
 */
bool scintilla_send_mouse(void *sci, int event, int button, int modifiers, int y, int x);

/**
 * Returns a NUL-terminated copy of the text on Scintilla's internal clipboard, not the primary
 * and/or secondary X selections.
 * The caller is responsible for `free`ing the returned text.
 * Keep in mind clipboard text may contain NUL bytes.
 * Curses does not have to be initialized before calling this function.
 * @param sci The Scintilla window returned by `scintilla_new()`.
 * @param len An optional pointer to store the length of the returned text in.
 * @return the clipboard text.
 */
char *scintilla_get_clipboard(void *sci, int *len);

/**
 * Refreshes the Scintilla window on the virtual screen.
 * This should be done along with the normal curses `noutrefresh()`, as the virtual screen is
 * updated when calling this function.
 * Curses must have been initialized prior to calling this function.
 * Note: the terminal cursor may be hidden if Scintilla thinks this window has focus
 * (e.g. `SCI_SETFOCUS`) and Scintilla's caret is out of view. If another non-Scintilla window
 * has the real focus, call `curs_set(1)` in order to show the terminal cursor for that window.
 * @param sci The Scintilla window returned by `scintilla_new()`.
 */
void scintilla_noutrefresh(void *sci);

/**
 * Refreshes the Scintilla window on the physical screen.
 * This should be done along with the normal curses `refresh()`, as the physical screen is
 * updated when calling this function.
 * Curses must have been initialized prior to calling this function.
 * Note: the terminal cursor may be hidden if Scintilla thinks this window has focus
 * (e.g. `SCI_SETFOCUS`) and Scintilla's caret is out of view. If another non-Scintilla window
 * has the real focus, call `curs_set(1)` in order to show the terminal cursor for that window.
 * @param sci The Scintilla window returned by `scintilla_new()`.
 */
void scintilla_refresh(void *sci);

/**
 * Updates the curses window cursor for the Scintilla window so the terminal draws the cursor
 * in the correct position.
 * This only needs to be called when `scintilla_refresh()` or `scintilla_noutrefresh()` is not
 * the last curses draw command issued (for any window, not just the Scintilla window).
 * Note: the terminal cursor may be hidden if Scintilla thinks this window has focus
 * (e.g. `SCI_SETFOCUS`) and Scintilla's caret is out of view. If another non-Scintilla window
 * has the real focus, call `curs_set(1)` in order to show the terminal cursor for that window.
 */
void scintilla_update_cursor(void *sci);

/**
 * Deletes the given Scintilla window.
 * Curses must have been initialized prior to calling this function.
 * @param sci The Scintilla window returned by `scintilla_new()`.
 */
void scintilla_delete(void *sci);

#define IMAGE_MAX 31

#define SCM_PRESS 1
#define SCM_DRAG 2
#define SCM_RELEASE 3

#ifdef __cplusplus
}
#endif

#endif
