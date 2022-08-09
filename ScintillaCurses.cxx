// Copyright 2012-2022 Mitchell. See LICENSE.
// Note: setlocale(LC_CTYPE, "") must be called before initializing curses in order to display
// UTF-8 characters properly in ncursesw.

#include <cassert>
#include <cstring>
#include <cmath>

#include <string>
#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include <memory>
#include <chrono>

#include <curses.h>

#include "ScintillaTypes.h"
#include "ScintillaMessages.h"
#include "ScintillaStructures.h"
#include "ILoader.h"
#include "ILexer.h"

#include "Debugging.h"
#include "Geometry.h"
#include "Platform.h"

#include "CharacterCategoryMap.h"
#include "Position.h"
#include "UniqueString.h"
#include "SplitVector.h"
#include "Partitioning.h"
#include "RunStyles.h"
#include "ContractionState.h"
#include "CellBuffer.h"
#include "CallTip.h"
#include "KeyMap.h"
#include "Indicator.h"
#include "LineMarker.h"
#include "Style.h"
#include "ViewStyle.h"
#include "CharClassify.h"
#include "Decoration.h"
#include "CaseFolder.h"
#include "Document.h"
#include "UniConversion.h"
#include "Selection.h"
#include "PositionCache.h"
#include "EditModel.h"
#include "MarginView.h"
#include "EditView.h"
#include "Editor.h"
#include "AutoComplete.h"
#include "ScintillaBase.h"

#include "ScintillaCurses.h"
#include "PlatCurses.h"

namespace Scintilla::Internal {

namespace {

// Custom drawing function for line markers.
void DrawLineMarker(Surface *surface, const PRectangle &rcWhole, const Font *fontForCharacter,
  int tFold, MarginType marginStyle, const void *data) {
  reinterpret_cast<SurfaceImpl *>(surface)->DrawLineMarker(rcWhole, fontForCharacter, tFold, data);
}

// Custom drawing function for wrap markers.
void DrawWrapVisualMarker(
  Surface *surface, PRectangle rcPlace, bool isEndMarker, ColourRGBA wrapColour) {
  reinterpret_cast<SurfaceImpl *>(surface)->DrawWrapMarker(rcPlace, isEndMarker, wrapColour);
}

// Custom drawing function for tab arrows.
void DrawTabArrow(
  Surface *surface, PRectangle rcTab, int ymid, const ViewStyle &vsDraw, Stroke stroke) {
  reinterpret_cast<SurfaceImpl *>(surface)->DrawTabArrow(rcTab, vsDraw);
}

// Uses the given UTF-8 code point to fill the given UTF-8 byte sequence and length.
// This algorithm was inspired by Paul Evans' libtermkey.
// (http://www.leonerd.org.uk/code/libtermkey)
void toutf8(int code, char *s, int *len) {
  if (code < 0x80)
    *len = 1;
  else if (code < 0x800)
    *len = 2;
  else if (code < 0x10000)
    *len = 3;
  else if (code < 0x200000)
    *len = 4;
  else if (code < 0x4000000)
    *len = 5;
  else
    *len = 6;
  for (int b = *len - 1; b > 0; b--) s[b] = 0x80 | (code & 0x3F), code >>= 6;
  if (*len == 1)
    s[0] = code & 0x7F;
  else if (*len == 2)
    s[0] = 0xC0 | (code & 0x1F);
  else if (*len == 3)
    s[0] = 0xE0 | (code & 0x0F);
  else if (*len == 4)
    s[0] = 0xF0 | (code & 0x07);
  else if (*len == 5)
    s[0] = 0xF8 | (code & 0x03);
  else if (*len == 6)
    s[0] = 0xFC | (code & 0x01);
}

} // namespace

class ScintillaCurses : public ScintillaBase {
  std::unique_ptr<Surface> sur; // window surface to draw on
  int width = 0, height = 0; // window dimensions
  void (*callback)(void *, int, SCNotification *, void *); // SCNotification cb
  void *userdata; // userdata for SCNotification callbacks
  int scrollBarVPos, scrollBarHPos; // positions of the scroll bars
  int scrollBarHeight = 1, scrollBarWidth = 1; // scroll bar height and width
  SelectionText clipboard; // current clipboard text
  bool capturedMouse; // whether or not the mouse is currently captured
  unsigned int autoCompleteLastClickTime; // last click time in the AC box
  bool draggingVScrollBar, draggingHScrollBar; // a scrollbar is being dragged
  int dragOffset; // the distance to the position of the scrollbar being dragged

public:
  ScintillaCurses(void (*callback_)(void *, int, SCNotification *, void *), void *userdata_);
  virtual ~ScintillaCurses() override;

private:
  void Initialise() override;

  void StartDrag() override;

  void SetVerticalScrollPos() override;
  void SetHorizontalScrollPos() override;
  bool ModifyScrollBars(Sci::Line nMax, Sci::Line nPage) override;

  void Copy() override;
  void Paste() override;
  void ClaimSelection() override;

  void NotifyChange() override;
  void NotifyParent(NotificationData scn) override;

  int KeyDefault(Keys key, KeyMod modifiers) override;

  void CopyToClipboard(const SelectionText &selectedText) override;

  bool FineTickerRunning(TickReason reason) override;
  void FineTickerStart(TickReason reason, int millis, int tolerance) override;
  void FineTickerCancel(TickReason reason) override;

  void SetMouseCapture(bool on) override;
  bool HaveMouseCapture() override;

  std::string UTF8FromEncoded(std::string_view encoded) const override;
  std::string EncodedFromUTF8(std::string_view utf8) const override;

  sptr_t DefWndProc(Message iMessage, uptr_t wParam, sptr_t lParam) override;

  void CreateCallTipWindow(PRectangle rc) override;

  void AddToPopUp(const char *label, int cmd = 0, bool enabled = true) override;

public:
  sptr_t WndProc(Message iMessage, uptr_t wParam, sptr_t lParam) override;

  // Access methods for C interface.

  WINDOW *GetWINDOW();

  void UpdateCursor();

  void NoutRefresh();
  void Refresh();

  void KeyPress(int key, bool shift, bool ctrl, bool alt);

  bool MousePress(int button, int y, int x, bool shift, bool ctrl, bool alt);
  bool MouseMove(int y, int x, bool shift, bool ctrl, bool alt);
  void MouseRelease(int y, int x, int ctrl);

  char *GetClipboard(int *len);
};

// Creates a new Scintilla instance on a curses `WINDOW`, but does not create that `WINDOW`
// until absolutely necessary. When it is created, it will initially be full-screen.
ScintillaCurses::ScintillaCurses(
  void (*callback_)(void *, int, SCNotification *, void *), void *userdata_)
    : sur(Surface::Allocate(Technology::Default)), callback(callback_), userdata(userdata_) {

  // Defaults for curses.
  marginView.wrapMarkerPaddingRight = 0; // no padding for margin wrap markers
  marginView.customDrawWrapMarker = DrawWrapVisualMarker; // draw text markers

  view.tabWidthMinimumPixels = 0; // no proportional fonts
  view.drawOverstrikeCaret = false; // always draw normal caret
  view.bufferedDraw = false; // draw directly to the screen
  view.tabArrowHeight = 0; // no additional tab arrow height
  view.customDrawTabArrow = DrawTabArrow; // draw text arrows for tabs
  view.customDrawWrapMarker = DrawWrapVisualMarker; // draw text wrap markers

  mouseSelectionRectangularSwitch = true; // easier rectangular selection
  doubleClickCloseThreshold = Point(0, 0); // double-clicks only in same cell
  horizontalScrollBarVisible = false; // no horizontal scroll bar
  scrollWidth = 5 * width; // reasonable default for any horizontal scroll bar

  vs.SetElementRGB(Element::SelectionText, 0x000000); // black on white selection
  vs.SetElementRGB(Element::SelectionAdditionalText, 0x000000);
  vs.SetElementRGB(Element::SelectionAdditionalBack, 0xFFFFFF);
  vs.SetElementRGB(Element::Caret, 0xFFFFFF); // white caret
  vs.caret.style = CaretStyle::Curses; // block carets
  vs.leftMarginWidth = 0, vs.rightMarginWidth = 0; // no margins
  vs.ms[1].width = 1; // marker margin width should be 1
  vs.extraDescent = -1; // hack to make lineHeight 1 instead of 2

  // Set default marker foreground and background colors.
  for (int i = 0; i <= MARKER_MAX; i++) {
    vs.markers[i].fore = ColourRGBA(0xC0, 0xC0, 0xC0);
    vs.markers[i].back = ColourRGBA(0, 0, 0);
    if (i >= 25) vs.markers[i].markType = MarkerSymbol::Empty;
    vs.markers[i].customDraw = DrawLineMarker;
  }

  // Use '+' and '-' fold markers.
  vs.markers[static_cast<int>(MarkerOutline::FolderOpen)].markType = MarkerSymbol::Minus;
  vs.markers[static_cast<int>(MarkerOutline::Folder)].markType = MarkerSymbol::Plus;
  vs.markers[static_cast<int>(MarkerOutline::FolderOpenMid)].markType = MarkerSymbol::Minus;
  vs.markers[static_cast<int>(MarkerOutline::FolderEnd)].markType = MarkerSymbol::Plus;

  displayPopupMenu = PopUp::Never; // no context menu

  vs.marginNumberPadding = 0; // no number margin padding
  vs.ctrlCharPadding = 0; // no ctrl character text blob padding
  vs.lastSegItalicsOffset = 0; // no offset for italic characters at EOLs

  ac.widthLBDefault = 10; // more sane bound for autocomplete width
  ac.heightLBDefault = 10; // more sane bound for autocomplete  height

  ct.colourBG = ColourRGBA(0, 0, 0); // black background color
  ct.colourUnSel = ColourRGBA(0xC0, 0xC0, 0xC0); // white text
  ct.insetX = 2; // border and arrow widths are 1 each
  ct.widthArrow = 1; // arrow width is 1 character
  ct.borderHeight = 1; // no extra empty lines in border height
  ct.verticalOffset = 0; // no extra offset of calltip from line
}

ScintillaCurses::~ScintillaCurses() {
  if (wMain.GetID()) delwin(GetWINDOW());
}

void ScintillaCurses::Initialise() {}

// Disable drag and drop since it is not implemented.
void ScintillaCurses::StartDrag() {
  inDragDrop = DragDrop::none;
  SetDragPosition(SelectionPosition(Sci::invalidPosition));
}

void ScintillaCurses::SetVerticalScrollPos() {
  if (!wMain.GetID() || !verticalScrollBarVisible) return;
  WINDOW *w = GetWINDOW();
  int maxy = getmaxy(w), maxx = getmaxx(w);
  // Draw the gutter.
  wattr_set(w, 0, term_color_pair(COLOR_WHITE, COLOR_BLACK), nullptr);
  for (int i = 0; i < maxy; i++) mvwaddch(w, i, maxx - 1, ACS_CKBOARD);
  // Draw the bar.
  scrollBarVPos = static_cast<float>(topLine) / (MaxScrollPos() + LinesOnScreen() - 1) * maxy;
  wattr_set(w, 0, term_color_pair(COLOR_BLACK, COLOR_WHITE), nullptr);
  for (int i = scrollBarVPos; i < scrollBarVPos + scrollBarHeight; i++)
    mvwaddch(w, i, maxx - 1, ' ');
}

void ScintillaCurses::SetHorizontalScrollPos() {
  if (!wMain.GetID() || !horizontalScrollBarVisible) return;
  WINDOW *w = GetWINDOW();
  int maxy = getmaxy(w), maxx = getmaxx(w);
  // Draw the gutter.
  wattr_set(w, 0, term_color_pair(COLOR_WHITE, COLOR_BLACK), nullptr);
  for (int i = 0; i < maxx; i++) mvwaddch(w, maxy - 1, i, ACS_CKBOARD);
  // Draw the bar.
  scrollBarHPos = static_cast<float>(xOffset) / scrollWidth * maxx;
  wattr_set(w, 0, term_color_pair(COLOR_BLACK, COLOR_WHITE), nullptr);
  for (int i = scrollBarHPos; i < scrollBarHPos + scrollBarWidth; i++)
    mvwaddch(w, maxy - 1, i, ' ');
}

// The height is based on the given size of a page and the total number of pages. The width is
// based on the width of the view and the view's scroll width property.
bool ScintillaCurses::ModifyScrollBars(Sci::Line nMax, Sci::Line nPage) {
  if (!wMain.GetID()) return false;
  WINDOW *w = GetWINDOW();
  int maxy = getmaxy(w), maxx = getmaxx(w);
  int height = roundf(static_cast<float>(nPage) / nMax * maxy);
  scrollBarHeight = std::clamp(height, 1, maxy);
  int width = roundf(static_cast<float>(maxx) / scrollWidth * maxx);
  scrollBarWidth = std::clamp(width, 1, maxx);
  return true;
}

// Internal copy; primary and secondary X selections are unaffected.
void ScintillaCurses::Copy() {
  if (!sel.Empty()) CopySelectionRange(&clipboard);
}

// Pastes from internal clipboard, not from primary or secondary X selections.
void ScintillaCurses::Paste() {
  if (clipboard.Empty()) return;
  ClearSelection(multiPasteMode == MultiPaste::Each);
  InsertPasteShape(clipboard.Data(), static_cast<int>(clipboard.Length()),
    !clipboard.rectangular ? PasteShape::stream : PasteShape::rectangular);
  EnsureCaretVisible();
}

void ScintillaCurses::ClaimSelection() {}

void ScintillaCurses::NotifyChange() {}

void ScintillaCurses::NotifyParent(NotificationData scn) {
  if (callback)
    (*callback)(
      reinterpret_cast<void *>(this), 0, reinterpret_cast<SCNotification *>(&scn), userdata);
}

int ScintillaCurses::KeyDefault(Keys key, KeyMod modifiers) {
  if ((IsUnicodeMode() || static_cast<int>(key) < 256) && modifiers == KeyMod::Norm) {
    if (IsUnicodeMode()) {
      char utf8[6];
      int len;
      toutf8(static_cast<int>(key), utf8, &len);
      InsertCharacter(std::string(utf8, len), CharacterSource::DirectInput);
      return 1;
    }
    auto ch = static_cast<char>(key);
    InsertCharacter(std::string(&ch, 1), CharacterSource::DirectInput);
    return 1;
  }
  NotificationData scn = {};
  scn.nmhdr.code = Notification::Key;
  scn.ch = static_cast<int>(key);
  scn.modifiers = modifiers;
  return (NotifyParent(scn), 0);
}

// Internal copy; primary and secondary X selections are unaffected.
void ScintillaCurses::CopyToClipboard(const SelectionText &selectedText) {
  clipboard.Copy(selectedText);
}

bool ScintillaCurses::FineTickerRunning(TickReason reason) { return false; }

void ScintillaCurses::FineTickerStart(TickReason reason, int millis, int tolerance) {}

void ScintillaCurses::FineTickerCancel(TickReason reason) {}

void ScintillaCurses::SetMouseCapture(bool on) { capturedMouse = on; }

// Used to handle mouse clicks, drags, and releases.
bool ScintillaCurses::HaveMouseCapture() { return capturedMouse; }

std::string ScintillaCurses::UTF8FromEncoded(std::string_view encoded) const {
  return std::string(encoded);
}

std::string ScintillaCurses::EncodedFromUTF8(std::string_view utf8) const {
  return std::string(utf8);
}

sptr_t ScintillaCurses::DefWndProc(Message iMessage, uptr_t wParam, sptr_t lParam) { return 0; }

void ScintillaCurses::CreateCallTipWindow(PRectangle rc) {
  if (!wMain.GetID()) return;
  if (!ct.wCallTip.Created()) {
    rc.right -= 1; // remove right-side padding
    int begx = 0, begy = 0, maxx = 0, maxy = 0;
    getbegyx(GetWINDOW(), begy, begx);
    int xoffset = begx - rc.left, yoffset = begy - rc.top;
    if (xoffset > 0) rc.left += xoffset, rc.right += xoffset;
    if (yoffset > 0) rc.top += yoffset, rc.bottom += yoffset;
    getmaxyx(GetWINDOW(), maxy, maxx);
    if (rc.Width() > maxx) rc.right = rc.left + maxx;
    if (rc.Height() > maxy) rc.bottom = rc.top + maxy;
    ct.wCallTip = newwin(rc.Height(), rc.Width(), rc.top, rc.left);
  }
  WindowID wid = ct.wCallTip.GetID();
  std::unique_ptr<Surface> sur = Surface::Allocate(Technology::Default);
  if (sur) {
    sur->Init(wid);
    dynamic_cast<SurfaceImpl *>(sur.get())->isCallTip = true;
    ct.PaintCT(sur.get());
    wattr_set(_WINDOW(wid), 0, term_color_pair(COLOR_WHITE, COLOR_BLACK), nullptr);
    box(_WINDOW(wid), '|', '-');
    wnoutrefresh(_WINDOW(wid));
  }
}

void ScintillaCurses::AddToPopUp(const char *label, int cmd, bool enabled) {}

sptr_t ScintillaCurses::WndProc(Message iMessage, uptr_t wParam, sptr_t lParam) {
  try {
    switch (iMessage) {
    case Message::GetDirectFunction: return reinterpret_cast<sptr_t>(scintilla_send_message);
    case Message::GetDirectPointer: return reinterpret_cast<sptr_t>(this);
    // Ignore attempted changes of the following unsupported properties.
    case Message::SetBufferedDraw:
    case Message::SetWhitespaceSize:
    case Message::SetPhasesDraw:
    case Message::SetExtraAscent:
    case Message::SetExtraDescent: return 0;
    // Pass to Scintilla.
    default: return ScintillaBase::WndProc(iMessage, wParam, lParam);
    }
  } catch (std::bad_alloc &) {
    errorStatus = Status::BadAlloc;
  } catch (...) {
    errorStatus = Status::Failure;
  }
  return 0;
}

WINDOW *ScintillaCurses::GetWINDOW() {
  if (!wMain.GetID()) {
    init_colors();
    wMain = newwin(0, 0, 0, 0);
    WINDOW *w = _WINDOW(wMain.GetID());
    keypad(w, TRUE);
    if (sur) sur->Init(w);
    getmaxyx(w, height, width);
    InvalidateStyleRedraw(); // needed to fully initialize Scintilla
  }
  return _WINDOW(wMain.GetID());
}

// Update even if it's not visible, as the container may have a use for it.
void ScintillaCurses::UpdateCursor() {
  int pos = WndProc(Message::GetCurrentPos, 0, 0);
  if (!SelectionEmpty() && !FlagSet(vs.caret.style, CaretStyle::BlockAfter) &&
    (pos > WndProc(Message::GetAnchor, 0, 0)))
    pos = WndProc(Message::PositionBefore, pos, 0); // draw inside selection
  Point point = LocationFromPosition(pos);
  auto y = static_cast<int>(point.y), x = static_cast<int>(point.x);
  WINDOW *win = GetWINDOW();
  bool in_view = x >= 0 && x <= getmaxx(win) && y >= 0 && y <= getmaxy(win);
  if (in_view) wmove(win, y, x), wrefresh(win);
  if (hasFocus && FlagSet(vs.caret.style, CaretStyle::Curses)) curs_set(in_view ? 1 : 0);
}

// Repaints the Scintilla window on the virtual screen.
// If an autocompletion list, user list, or calltip is active, redraw it over the buffer's
// contents.
// It is the application's responsibility to call the curses `doupdate()` in order to refresh
// the physical screen. To paint to the physical screen instead, use `Refresh()`.
void ScintillaCurses::NoutRefresh() {
  WINDOW *w = GetWINDOW();
  rcPaint.top = 0, rcPaint.left = 0; // paint from (0, 0), not (begy, begx)
  getmaxyx(w, rcPaint.bottom, rcPaint.right);
  if (rcPaint.bottom != height || rcPaint.right != width)
    height = rcPaint.bottom, width = rcPaint.right, ChangeSize();
  Paint(sur.get(), rcPaint);
  SetVerticalScrollPos(), SetHorizontalScrollPos();
  wnoutrefresh(w);
  if (ac.Active())
    ac.lb->Select(ac.lb->GetSelection()); // redraw
  else if (ct.inCallTipMode)
    CreateCallTipWindow(PRectangle(0, 0, 0, 0)); // redraw
#if PDCURSES
  else
    touchwin(w); // pdcurses has problems after drawing overlapping windows
#endif
  if (hasFocus) UpdateCursor();
}

// Repaints the Scintilla window on the physical screen.
// If an autocompletion list, user list, or calltip is active, redraw it over the buffer's
// contents.
// To paint to the virtual screen instead, use `NoutRefresh()`.
void ScintillaCurses::Refresh() {
  NoutRefresh();
  doupdate();
}

// Sends a key to Scintilla.
// Usually if a key is consumed, the screen should be repainted. However, when autocomplete is
// active, that window is consuming the keys and any repainting of the main Scintilla window
// will overwrite the autocomplete window.
void ScintillaCurses::KeyPress(int key, bool shift, bool ctrl, bool alt) {
  KeyDownWithModifiers(static_cast<Keys>(key), ModifierFlags(shift, ctrl, alt), nullptr);
}

// Handles a mouse button press, with coordinates relative to this window.
// Returns whether or not the press was handled.
bool ScintillaCurses::MousePress(int button, int y, int x, bool shift, bool ctrl, bool alt) {
  const auto now = std::chrono::system_clock::now().time_since_epoch();
  unsigned int time = std::chrono::duration_cast<std::chrono::milliseconds>(now).count();
  GetWINDOW(); // ensure the curses `WINDOW` has been created
  if (ac.Active() && (button == 1 || button == 4 || button == 5)) {
    // Select an autocompletion list item if possible or scroll the list.
    WINDOW *w = _WINDOW(ac.lb->GetID()), *parent = GetWINDOW();
    int begy = getbegy(w) - getbegy(parent); // y is relative to the view
    int begx = getbegx(w) - getbegx(parent); // x is relative to the view
    int maxy = getmaxy(w) - 1, maxx = getmaxx(w) - 1; // ignore border
    int ry = y - begy, rx = x - begx; // relative to list box
    if (ry > 0 && ry < maxy && rx > 0 && rx < maxx) {
      if (button == 1) {
        // Select a list item.
        // The currently selected item is normally displayed in the middle.
        int middle = ac.lb->GetVisibleRows() / 2;
        int n = ac.lb->GetSelection(), ny = middle;
        if (n < middle)
          ny = n; // the currently selected item is near the beginning
        else if (n >= ac.lb->Length() - middle)
          ny = (n - 1) % ac.lb->GetVisibleRows(); // it's near the end
        // Compute the index of the item to select.
        int offset = ry - ny - 1; // -1 ignores list box border
        if (offset == 0 && time - autoCompleteLastClickTime < Platform::DoubleClickTime()) {
          ListBoxImpl *listbox = reinterpret_cast<ListBoxImpl *>(ac.lb.get());
          if (listbox->delegate) {
            ListBoxEvent event(ListBoxEvent::EventType::doubleClick);
            listbox->delegate->ListNotify(&event);
          }
        } else
          ac.lb->Select(n + offset);
        autoCompleteLastClickTime = time;
      } else {
        // Scroll the list.
        int n = ac.lb->GetSelection();
        if (button == 4 && n > 0)
          ac.lb->Select(n - 1);
        else if (button == 5 && n < ac.lb->Length() - 1)
          ac.lb->Select(n + 1);
      }
      return true;
    }
    if (ry == 0 || ry == maxy || rx == 0 || rx == maxx) return true; // ignore border click
  } else if (ct.inCallTipMode && button == 1) {
    // Send the click to the CallTip.
    WINDOW *w = _WINDOW(ct.wCallTip.GetID()), *parent = GetWINDOW();
    int begy = getbegy(w) - getbegy(parent); // y is relative to the view
    int begx = getbegx(w) - getbegx(parent); // x is relative to the view
    int maxy = getmaxy(w) - 1, maxx = getmaxx(w) - 1; // ignore border
    int ry = y - begy, rx = x - begx; // relative to list box
    if (ry >= 0 && ry <= maxy && rx >= 0 && rx <= maxx) {
      ct.MouseClick(Point(rx, ry));
      return (CallTipClick(), true);
    }
  }

  if (button == 1) {
    if (verticalScrollBarVisible && x == getmaxx(GetWINDOW()) - 1) {
      // Scroll the vertical scrollbar.
      if (y < scrollBarVPos) return (ScrollTo(topLine - LinesOnScreen()), true);
      if (y >= scrollBarVPos + scrollBarHeight) return (ScrollTo(topLine + LinesOnScreen()), true);
      draggingVScrollBar = true, dragOffset = y - scrollBarVPos;
    } else if (horizontalScrollBarVisible && y == getmaxy(GetWINDOW()) - 1) {
      // Scroll the horizontal scroll bar.
      if (x < scrollBarHPos) return (HorizontalScrollTo(xOffset - getmaxx(GetWINDOW()) / 2), true);
      if (x >= scrollBarHPos + scrollBarWidth)
        return (HorizontalScrollTo(xOffset + getmaxx(GetWINDOW()) / 2), true);
      draggingHScrollBar = true, dragOffset = x - scrollBarHPos;
    } else {
      // Have Scintilla handle the click.
      ButtonDownWithModifiers(Point(x, y), time, ModifierFlags(shift, ctrl, alt));
      return true;
    }
  } else if (button == 4 || button == 5) {
    // Scroll the view (horizontally if shift is pressed).
    int offset = std::max((!shift ? getmaxy(GetWINDOW()) : getmaxx(GetWINDOW())) / 4, 1);
    if (button == 4) offset *= -1;
    return (!shift ? ScrollTo(topLine + offset) : HorizontalScrollTo(xOffset + offset), true);
  }
  return false;
}

// Handles a mouse move, with coordinates relative to this window.
// Returns whether or not the press was handled.
bool ScintillaCurses::MouseMove(int y, int x, bool shift, bool ctrl, bool alt) {
  GetWINDOW(); // ensure the curses `WINDOW` has been created
  if (!draggingVScrollBar && !draggingHScrollBar) {
    ButtonMoveWithModifiers(Point(x, y), 0, ModifierFlags(shift, ctrl, alt));
  } else if (draggingVScrollBar) {
    int maxy = getmaxy(GetWINDOW()) - scrollBarHeight, pos = y - dragOffset;
    if (pos >= 0 && pos <= maxy) ScrollTo(pos * MaxScrollPos() / maxy);
    return true;
  } else if (draggingHScrollBar) {
    int maxx = getmaxx(GetWINDOW()) - scrollBarWidth, pos = x - dragOffset;
    if (pos >= 0 && pos <= maxx)
      HorizontalScrollTo(pos * (scrollWidth - maxx - scrollBarWidth) / maxx);
    return true;
  }
  return HaveMouseCapture();
}

// Handles a mouse button release, with coordinates relative to this window.
void ScintillaCurses::MouseRelease(int y, int x, int ctrl) {
  const auto now = std::chrono::system_clock::now().time_since_epoch();
  unsigned int time = std::chrono::duration_cast<std::chrono::milliseconds>(now).count();
  GetWINDOW(); // ensure the curses `WINDOW` has been created
  if (draggingVScrollBar || draggingHScrollBar)
    draggingVScrollBar = false, draggingHScrollBar = false;
  else if (HaveMouseCapture()) {
    ButtonUpWithModifiers(Point(x, y), time, ModifierFlags(ctrl, false, false));
    // TODO: ListBoxEvent event(ListBoxEvent::EventType::selectionChange);
    // TODO: listbox->delegate->ListNotify(&event);
  }
}

// Returns a NUL-terminated copy of the text on the internal clipboard, not the primary and/or
// secondary X selections.
// The caller is responsible for `free`ing the returned text.
char *ScintillaCurses::GetClipboard(int *len) {
  if (len) *len = clipboard.Length();
  char *text = new char[clipboard.Length() + 1];
  memcpy(text, clipboard.Data(), clipboard.Length() + 1);
  return text;
}

} // namespace Scintilla::Internal

using ScintillaCurses = Scintilla::Internal::ScintillaCurses;

// Link with C. Documentation in ScintillaCurses.h.
extern "C" {

void *scintilla_new(void (*callback)(void *, int, SCNotification *, void *), void *userdata) {
  return reinterpret_cast<void *>(new ScintillaCurses(callback, userdata));
}

WINDOW *scintilla_get_window(void *sci) {
  return reinterpret_cast<ScintillaCurses *>(sci)->GetWINDOW();
}

sptr_t scintilla_send_message(void *sci, unsigned int iMessage, uptr_t wParam, sptr_t lParam) {
  return reinterpret_cast<ScintillaCurses *>(sci)->WndProc(
    static_cast<Scintilla::Message>(iMessage), wParam, lParam);
}

void scintilla_send_key(void *sci, int key, bool shift, bool ctrl, bool alt) {
  reinterpret_cast<ScintillaCurses *>(sci)->KeyPress(key, shift, ctrl, alt);
}

bool scintilla_send_mouse(
  void *sci, int event, int button, int y, int x, bool shift, bool ctrl, bool alt) {
  auto scicurses = reinterpret_cast<ScintillaCurses *>(sci);
  WINDOW *w = scicurses->GetWINDOW();
  int begy = getbegy(w), begx = getbegx(w);
  int maxy = getmaxy(w), maxx = getmaxx(w);
  // Ignore most events outside the window.
  if ((x < begx || x > begx + maxx - 1 || y < begy || y > begy + maxy - 1) && button != 4 &&
    button != 5 && event != SCM_DRAG)
    return false;
  y = y - begy, x = x - begx;
  if (event == SCM_PRESS) return scicurses->MousePress(button, y, x, shift, ctrl, alt);
  if (event == SCM_DRAG) return scicurses->MouseMove(y, x, shift, ctrl, alt);
  if (event == SCM_RELEASE) return (scicurses->MouseRelease(y, x, ctrl), true);
  return false;
}

char *scintilla_get_clipboard(void *sci, int *len) {
  return reinterpret_cast<ScintillaCurses *>(sci)->GetClipboard(len);
}

void scintilla_noutrefresh(void *sci) { reinterpret_cast<ScintillaCurses *>(sci)->NoutRefresh(); }

void scintilla_refresh(void *sci) { reinterpret_cast<ScintillaCurses *>(sci)->Refresh(); }

void scintilla_update_cursor(void *sci) {
  reinterpret_cast<ScintillaCurses *>(sci)->UpdateCursor();
}

void scintilla_delete(void *sci) { delete reinterpret_cast<ScintillaCurses *>(sci); }
}
