// Copyright 2012 Mitchell mitchell.att.foicica.com.
// Header for Scintilla in a UNIX terminal environment.

#ifndef SCINTILLATERM_H
#define SCINTILLATERM_H

#ifdef __cplusplus
extern "C" {
#endif

typedef void *Scintilla;
Scintilla *scintilla_new(void (*)(Scintilla *, int, void *, void *));
WINDOW *scintilla_get_window(Scintilla *);
sptr_t scintilla_send_message(Scintilla *, unsigned int, uptr_t, sptr_t);
void scintilla_send_key(Scintilla *, int, bool, bool, bool);
int scintilla_get_clipboard(Scintilla *sci, char *);
void scintilla_refresh(Scintilla *);
void scintilla_delete(Scintilla *);

/**
 * Returns the ncurses `COLOR_PAIR` for the given ncurses foreground and
 * background `COLOR`s.
 * This is used simply to enumerate every possible color combination.
 * @param f The ncurses foreground `COLOR`.
 * @param b The ncurses background `COLOR`.
 * @return int number for defining an ncurses `COLOR_PAIR`.
 */
#define SCI_COLOR_PAIR(f, b) ((b) * 8 + (f) + 1)

#ifdef __cplusplus
}
#endif

#endif
