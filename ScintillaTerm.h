// Copyright 2012-2014 Mitchell mitchell.att.foicica.com.
// Header for Scintilla in a UNIX terminal environment.

#ifndef SCINTILLATERM_H
#define SCINTILLATERM_H

#define NCURSES_OPAQUE 0
#include <curses.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void *Scintilla;
Scintilla *scintilla_new(void (*)(Scintilla *, int, void *, void *));
WINDOW *scintilla_get_window(Scintilla *);
sptr_t scintilla_send_message(Scintilla *, unsigned int, uptr_t, sptr_t);
void scintilla_send_key(Scintilla *, int, bool, bool, bool);
void scintilla_send_mouse(Scintilla *, int, unsigned int, int, int, int, bool,
                          bool, bool);
int scintilla_get_clipboard(Scintilla *, char *);
void scintilla_refresh(Scintilla *);
void scintilla_delete(Scintilla *);

/**
 * Returns the curses `COLOR_PAIR` for the given curses foreground and
 * background `COLOR`s.
 * This is used simply to enumerate every possible color combination.
 * Note: only 256 combinations are possible due for curses portability.
 * @param f The curses foreground `COLOR`.
 * @param b The curses background `COLOR`.
 * @return int number for defining a curses `COLOR_PAIR`.
 */
#define SCI_COLOR_PAIR(f, b) ((b) * ((COLORS < 16) ? 8 : 16) + (f) + 1)

#define SCM_PRESS 1
#define SCM_DRAG 2
#define SCM_RELEASE 3

#ifdef __cplusplus
}
#endif

#endif
