// Copyright 2012-2024 Mitchell. See LICENSE.

#include <dlfcn.h>
#include <locale.h>
#include <curses.h>

#include "Scintilla.h"
#include "SciLexer.h"
#include "Lexilla.h"
#include "ScintillaCurses.h"

#define SSM(m, w, l) scintilla_send_message(sci, m, w, l)

typedef void Scintilla;

void scnotification(Scintilla *view, int msg, SCNotification *n, void *userdata) {
	// printw("SCNotification received: %i", n->nmhdr.code);
}

int main(int argc, char **argv) {
	setlocale(LC_CTYPE, ""); // for displaying UTF-8 characters properly
	initscr(), raw(), cbreak(), noecho(), start_color();
	Scintilla *sci = scintilla_new(scnotification, NULL);
	char lexilla_path[] = "../../../lexilla/bin/" LEXILLA_LIB LEXILLA_EXTENSION;
	void *lexilla = dlopen(lexilla_path, RTLD_LAZY);
	CreateLexerFn lexer = (CreateLexerFn)dlsym(lexilla, LEXILLA_CREATELEXER);

	SSM(SCI_STYLESETFORE, STYLE_DEFAULT, 0xFFFFFF);
	SSM(SCI_STYLESETBACK, STYLE_DEFAULT, 0);
	SSM(SCI_STYLECLEARALL, 0, 0);
	SSM(SCI_SETILEXER, 0, (sptr_t)lexer("cpp"));
	SSM(SCI_SETKEYWORDS, 0, (sptr_t) "int char");
	SSM(SCI_STYLESETFORE, SCE_C_COMMENT, 0x00FF00);
	SSM(SCI_STYLESETFORE, SCE_C_COMMENTLINE, 0x00FF00);
	SSM(SCI_STYLESETITALIC, SCE_C_COMMENTLINE, 1);
	SSM(SCI_STYLESETFORE, SCE_C_NUMBER, 0xFFFF00);
	SSM(SCI_STYLESETFORE, SCE_C_WORD, 0xFF0000);
	SSM(SCI_STYLESETUNDERLINE, SCE_C_WORD, 1);
	SSM(SCI_STYLESETFORE, SCE_C_STRING, 0xFF00FF);
	SSM(SCI_STYLESETBOLD, SCE_C_OPERATOR, 1);
	// clang-format off
  SSM(SCI_INSERTTEXT, 0, (sptr_t)
      "int main(int argc, char **argv) {\n"
      "    // Start up the gnome\n"
      "    gnome_init(\"stest\", \"1.0\", argc, argv);\n}");
	// clang-format on
	SSM(SCI_SETPROPERTY, (uptr_t) "fold", (sptr_t) "1");
	SSM(SCI_SETMARGINWIDTHN, 0, 2);
	SSM(SCI_SETMARGINWIDTHN, 2, 1);
	SSM(SCI_SETMARGINMASKN, 2, SC_MASK_FOLDERS);
	SSM(SCI_SETMARGINSENSITIVEN, 2, 1);
	SSM(SCI_SETAUTOMATICFOLD, SC_AUTOMATICFOLD_CLICK, 0);
	SSM(SCI_SETFOCUS, 1, 0);
	scintilla_refresh(sci);

	printf("\033[?1000h"); // enable mouse press and release events
	// printf("\033[?1002h"); // enable mouse press, drag, and release events
	// printf("\033[?1003h"); // enable mouse move, press, drag, and release events
	mousemask(ALL_MOUSE_EVENTS, NULL);
	mouseinterval(0);

	// Non-UTF8 input.
	int c = 0;
	MEVENT mouse;
	WINDOW *win = scintilla_get_window(sci);
	while ((c = wgetch(win)) != 'q') {
		if (c != KEY_MOUSE) {
			if (c == KEY_UP)
				c = SCK_UP;
			else if (c == KEY_DOWN)
				c = SCK_DOWN;
			else if (c == KEY_LEFT)
				c = SCK_LEFT;
			else if (c == KEY_RIGHT)
				c = SCK_RIGHT;
			scintilla_send_key(sci, c, SCMOD_NORM);
		} else if (getmouse(&mouse) == OK) {
			int event = SCM_DRAG, button = 0;
			if (mouse.bstate & BUTTON1_PRESSED)
				event = SCM_PRESS, button = 1;
			else if (mouse.bstate & BUTTON1_RELEASED)
				event = SCM_RELEASE, button = 1;
			bool shift = mouse.bstate & BUTTON_SHIFT;
			bool ctrl = mouse.bstate & BUTTON_CTRL;
			bool alt = mouse.bstate & BUTTON_ALT;
			int modifiers = (shift ? SCMOD_SHIFT : 0) | (ctrl ? SCMOD_CTRL : 0) | (alt ? SCMOD_ALT : 0);
			scintilla_send_mouse(sci, event, button, modifiers, mouse.y, mouse.x);
		}
		scintilla_refresh(sci);
		// scintilla_update_cursor(sci); // use this when doing other curses drawing
	}
	// UTF-8 input.
	// SSM(SCI_SETCODEPAGE, SC_CP_UTF8, 0);
	// wint_t c = {0};
	// WINDOW *win = scintilla_get_window(sci);
	// while (c != 'q') {
	//   int status = wget_wch(win, &c);
	//   if (status == ERR)
	//     continue;
	//   else if (status == KEY_CODE_YES) {
	//     if (c == KEY_UP) c = SCK_UP;
	//     else if (c == KEY_DOWN) c = SCK_DOWN;
	//     else if (c == KEY_LEFT) c = SCK_LEFT;
	//     else if (c == KEY_RIGHT) c = SCK_RIGHT;
	//   }
	//   scintilla_send_key(sci, c, FALSE, FALSE, FALSE);
	//   scintilla_refresh(sci);
	// }

	printf("\033[?1000l"); // disable mouse press and release events
	// printf("\033[?1002l"); // disable mouse press, drag, and release events
	// printf("\033[?1003l"); // disable mouse move, press, drag, and release

	scintilla_delete(sci);
	endwin();

	return 0;
}
