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
Scintilla *scintilla_new(void (*callback)(Scintilla *sci, int iMessage,
                                          void *wParam, void *lParam));
WINDOW *scintilla_get_window(Scintilla *sci);
sptr_t scintilla_send_message(Scintilla *sci, unsigned int iMessage,
                              uptr_t wParam, sptr_t lParam);
void scintilla_send_key(Scintilla *sci, int key, bool shift, bool ctrl,
                        bool alt);
bool scintilla_send_mouse(Scintilla *sci, int event, unsigned int time,
                          int button, int y, int x, bool shift, bool ctrl,
                          bool alt);
int scintilla_get_clipboard(Scintilla *sci, char *buffer);
void scintilla_refresh(Scintilla *sci);
void scintilla_delete(Scintilla *sci);

/**
 * Returns the curses `COLOR_PAIR` for the given curses foreground and
 * background `COLOR`s.
 * This is used simply to enumerate every possible color combination.
 * Note: only 256 combinations are possible due to curses portability.
 * @param f The curses foreground `COLOR`.
 * @param b The curses background `COLOR`.
 * @return int number for defining a curses `COLOR_PAIR`.
 */
#define SCI_COLOR_PAIR(f, b) ((b) * ((COLORS < 16) ? 8 : 16) + (f) + 1)

#define SCI_TYPEMAX 64

#define SCM_PRESS 1
#define SCM_DRAG 2
#define SCM_RELEASE 3

#ifdef __cplusplus
}
#endif

#endif
