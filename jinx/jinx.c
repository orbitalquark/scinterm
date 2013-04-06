// Copyright 2012-2013 Mitchell mitchell.att.foicica.com. See LICENSE.

#include <locale.h>
#include <curses.h>

#include "Scintilla.h"
#include "SciLexer.h"
#include "ScintillaTerm.h"

#define SSM(m, w, l) scintilla_send_message(sci, m, w, l)

void scnotification(Scintilla *view, struct SCNotification *scn) {
  //printw("SCNotification received: %i", scn->nmhdr.code);
}

int main(int argc, char **argv) {
  initscr(), raw(), cbreak(), noecho(), start_color();
  setlocale(LC_CTYPE, ""); // for displaying UTF-8 characters properly
  Scintilla *sci = scintilla_new(scnotification);
  curs_set(0); // Scintilla draws its own cursor

  SSM(SCI_STYLESETFORE, STYLE_DEFAULT, 0xFFFFFF);
  SSM(SCI_STYLESETBACK, STYLE_DEFAULT, 0);
  SSM(SCI_STYLECLEARALL, 0, 0);
  SSM(SCI_SETLEXER, SCLEX_CPP, 0);
  SSM(SCI_SETKEYWORDS, 0, (sptr_t)"int char");
  SSM(SCI_STYLESETFORE, SCE_C_COMMENT, 0x00FF00);
  SSM(SCI_STYLESETFORE, SCE_C_COMMENTLINE, 0x00FF00);
  SSM(SCI_STYLESETFORE, SCE_C_NUMBER, 0xFFFF00);
  SSM(SCI_STYLESETFORE, SCE_C_WORD, 0xFF0000);
  SSM(SCI_STYLESETFORE, SCE_C_STRING, 0xFF00FF);
  SSM(SCI_STYLESETBOLD, SCE_C_OPERATOR, 1);
  SSM(SCI_INSERTTEXT, 0, (sptr_t)
      "int main(int argc, char **argv) {\n"
      "    // Start up the gnome\n"
      "    gnome_init(\"stest\", \"1.0\", argc, argv);\n}");
  SSM(SCI_SETPROPERTY, (uptr_t)"fold", (sptr_t)"1");
  SSM(SCI_SETMARGINWIDTHN, 2, 1);
  SSM(SCI_SETMARGINMASKN, 2, SC_MASK_FOLDERS);
  SSM(SCI_SETFOCUS, 1, 0);
  scintilla_refresh(sci);

  int c = 0;
  WINDOW *win = scintilla_get_window(sci);
  while ((c = wgetch(win)) != 'q') {
    if (c == KEY_UP) c = SCK_UP;
    else if (c == KEY_DOWN) c = SCK_DOWN;
    else if (c == KEY_LEFT) c = SCK_LEFT;
    else if (c == KEY_RIGHT) c = SCK_RIGHT;
    scintilla_send_key(sci, c, FALSE, FALSE, FALSE);
    scintilla_refresh(sci);
  }
  scintilla_delete(sci);
  endwin();

  return 0;
}
