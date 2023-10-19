// Copyright 2012-2023 Mitchell. See LICENSE.
// Scintilla platform for a curses (terminal) environment.

#include <cassert>
#include <cstring>
#include <cmath>

#include <stdexcept>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <optional>
#include <algorithm>
#include <memory>

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
#include "UniConversion.h" // UTF8DrawBytes
#include "Selection.h"
#include "PositionCache.h"
#include "EditModel.h"
#include "MarginView.h"
#include "EditView.h"
#include "Editor.h"
#include "AutoComplete.h"

#include "ScintillaCurses.h"
#include "PlatCurses.h"

namespace Scintilla::Internal {

// Font handling.

FontImpl::FontImpl(const FontParameters &fp) {
	if (fp.weight == FontWeight::Bold)
		attrs = A_BOLD;
	else if (fp.weight != FontWeight::Normal && fp.weight != FontWeight::SemiBold)
		attrs = static_cast<int>(fp.weight); // font attributes are stored in fp.weight
}

std::shared_ptr<Font> Font::Allocate(const FontParameters &fp) {
	return std::make_shared<FontImpl>(fp);
}

// Color handling.

namespace {

short COLOR_LBLACK = COLOR_BLACK + 8;
short COLOR_LRED = COLOR_RED + 8;
short COLOR_LGREEN = COLOR_GREEN + 8;
short COLOR_LYELLOW = COLOR_YELLOW + 8;
short COLOR_LBLUE = COLOR_BLUE + 8;
short COLOR_LMAGENTA = COLOR_MAGENTA + 8;
short COLOR_LCYAN = COLOR_CYAN + 8;
short COLOR_LWHITE = COLOR_WHITE + 8;

bool initialized_colors = false;

ColourRGBA BLACK(0, 0, 0);
ColourRGBA RED(0x80, 0, 0);
ColourRGBA GREEN(0, 0x80, 0);
ColourRGBA YELLOW(0x80, 0x80, 0);
ColourRGBA BLUE(0, 0, 0x80);
ColourRGBA MAGENTA(0x80, 0, 0x80);
ColourRGBA CYAN(0, 0x80, 0x80);
ColourRGBA WHITE(0xC0, 0xC0, 0xC0);
ColourRGBA LBLACK(0x40, 0x40, 0x40);
ColourRGBA LRED(0xFF, 0, 0);
ColourRGBA LGREEN(0, 0xFF, 0);
ColourRGBA LYELLOW(0xFF, 0xFF, 0);
ColourRGBA LBLUE(0, 0, 0xFF);
ColourRGBA LMAGENTA(0xFF, 0, 0xFF);
ColourRGBA LCYAN(0, 0xFF, 0xFF);
ColourRGBA LWHITE(0xFF, 0xFF, 0xFF);
ColourRGBA SCI_COLORS[] = {BLACK, RED, GREEN, YELLOW, BLUE, MAGENTA, CYAN, WHITE, LBLACK, LRED,
	LGREEN, LYELLOW, LBLUE, LMAGENTA, LCYAN, LWHITE};

} // namespace

/**
 * Initializes colors in curses if they have not already been initialized.
 * Creates all possible color pairs using the `SCI_COLOR_PAIR()` macro.
 * This is called automatically from `scintilla_new()`.
 */
void init_colors() {
	if (initialized_colors || !has_colors()) return;
	start_color();
	for (short back = 0; back < ((COLORS < 16) ? 8 : 16); back++)
		for (short fore = 0; fore < ((COLORS < 16) ? 8 : 16); fore++)
			init_pair(SCI_COLOR_PAIR(fore, back), fore, back);
	if (COLORS < 16) {
		// Do not distinguish between light and normal colors.
		COLOR_LBLACK -= 8;
		COLOR_LRED -= 8;
		COLOR_LGREEN -= 8;
		COLOR_LYELLOW -= 8;
		COLOR_LBLUE -= 8;
		COLOR_LMAGENTA -= 8;
		COLOR_LCYAN -= 8;
		COLOR_LWHITE -= 8;
	}
	initialized_colors = true;
}

/**
 * Returns a curses color for the given Scintilla color.
 * Recognized colors are: black (0x000000), red (0x800000), green (0x008000), yellow (0x808000),
 * blue (0x000080), magenta (0x800080), cyan (0x008080), white (0xc0c0c0), light black (0x404040),
 * light red (0xff0000), light green (0x00ff00), light yellow (0xffff00), light blue (0x0000ff),
 * light magenta (0xff00ff), light cyan (0x00ffff), and light white (0xffffff). If the color
 * is not recognized, returns `COLOR_WHITE` by default.
 * @param color Color to get a curses color for.
 * @return curses color
 */
short term_color(ColourRGBA color) {
	color = color.Opaque();
	if (color == BLACK) return COLOR_BLACK;
	if (color == RED) return COLOR_RED;
	if (color == GREEN) return COLOR_GREEN;
	if (color == YELLOW) return COLOR_YELLOW;
	if (color == BLUE) return COLOR_BLUE;
	if (color == MAGENTA) return COLOR_MAGENTA;
	if (color == CYAN) return COLOR_CYAN;
	if (color == LBLACK) return COLOR_LBLACK;
	if (color == LRED) return COLOR_LRED;
	if (color == LGREEN) return COLOR_LGREEN;
	if (color == LYELLOW) return COLOR_LYELLOW;
	if (color == LBLUE) return COLOR_LBLUE;
	if (color == LMAGENTA) return COLOR_LMAGENTA;
	if (color == LCYAN) return COLOR_LCYAN;
	if (color == LWHITE) return COLOR_LWHITE;
	return COLOR_WHITE;
}

/**
 * Returns a curses color for the given curses color.
 * This overloaded method only exists for the `term_color_pair()` macro.
 */
short term_color(short color) { return color; }

// Surface handling.

SurfaceImpl::~SurfaceImpl() noexcept { Release(); }

void SurfaceImpl::Init(WindowID wid) {
	Release();
	win = _WINDOW(wid);
}

void SurfaceImpl::Init(SurfaceID /*sid*/, WindowID wid) { Init(wid); }

std::unique_ptr<Surface> SurfaceImpl::AllocatePixMap(int /*width*/, int /*height*/) {
	// Not supported, but cannot return a nullptr because Scintilla assumes the allocation succeeded.
	return std::make_unique<SurfaceImpl>();
}

void SurfaceImpl::SetMode(SurfaceMode /*mode*/) {}

void SurfaceImpl::Release() noexcept { win = nullptr; }

int SurfaceImpl::SupportsFeature(Supports /*feature*/) noexcept {
	return 0; // feature == Supports::ThreadSafeMeasureWidths;
}

bool SurfaceImpl::Initialised() { return true; }

int SurfaceImpl::LogPixelsY() { return 1; } // N/A

int SurfaceImpl::PixelDivisions() { return 1; }

int SurfaceImpl::DeviceHeightFont(int /*points*/) { return 1; }

// Drawing lines is not implemented because more often than not, lines are being drawn for
// decoration (e.g. line markers, underlines, indicators, arrows, etc.)
void SurfaceImpl::LineDraw(Point /*start*/, Point /*end*/, Stroke /*stroke*/) {}

void SurfaceImpl::PolyLine(const Point * /*pts*/, size_t /*npts*/, Stroke /*stroke*/) {}

// Draws the character equivalent of shape outlined by the given polygon's points.
// Only called for CallTip arrows and INDIC_POINT[CHARACTER]. Assume the former. Line markers
// normally drawn as polygons are handled in `DrawLineMarker()`.
void SurfaceImpl::Polygon(const Point *pts, size_t npts, FillStroke fillStroke) {
	ColourRGBA &back = fillStroke.fill.colour;
	wattr_set(win, 0, term_color_pair(back, COLOR_WHITE), nullptr); // invert
	if (pts[0].y < pts[npts - 1].y) // up arrow
		mvwaddstr(win, static_cast<int>(pts[0].y), static_cast<int>(pts[npts - 1].x - 2), "▲");
	else if (pts[0].y > pts[npts - 1].y) // down arrow
		mvwaddstr(win, static_cast<int>(pts[0].y - 2), static_cast<int>(pts[npts - 1].x - 2), "▼");
}

// Never called. Line markers normally drawn as rectangles are handled in `DrawLineMarker()`.
void SurfaceImpl::RectangleDraw(PRectangle /*rc*/, FillStroke /*fillStroke*/) {}

// Drawing framed rectangles like fold display text, EOL annotations, and INDIC_BOX is not
// implemented.
void SurfaceImpl::RectangleFrame(PRectangle /*rc*/, Stroke /*stroke*/) {}

// Normally this clears the given portion of the screen with the given background color. In
// some cases however, it can be determined that whitespace is being drawn. If so, draw it
// appropriately instead of clearing the given portion of the screen.
void SurfaceImpl::FillRectangle(PRectangle rc, Fill fill) {
	if (!win) {
		// Drawing to a pixmap, probably the fold margin. Record the color for a later fill.
		pixmapColor = fill.colour;
		return;
	}
	wattr_set(win, 0, term_color_pair(COLOR_WHITE, fill.colour), nullptr);
	chtype ch = ' ';
	if (fabs(rc.left - static_cast<int>(rc.left)) > 0.1) {
		// If rc.left is a fractional value (e.g. 4.5) then whitespace dots are being drawn. Draw
		// them appropriately.
		// TODO: set color to vs.whitespaceColours.fore and back.
		wcolor_set(win, term_color_pair(COLOR_BLACK, COLOR_BLACK), nullptr);
		rc.right = static_cast<int>(rc.right), ch = ACS_BULLET | A_BOLD;
	}
	for (int y = static_cast<int>(rc.top); y < rc.bottom; y++)
		for (int x = static_cast<int>(std::max(rc.left, clip.left)); x < rc.right; x++)
			mvwaddch(win, y, x, ch);
}

// Note: special alignment to pixel boundaries is not needed.
void SurfaceImpl::FillRectangleAligned(PRectangle rc, Fill fill) { FillRectangle(rc, fill); }

// Instead of filling a portion of the screen with a surface pixmap, fills the the screen
// portion with that pixmap's last fill color.
void SurfaceImpl::FillRectangle(PRectangle rc, Surface &surfacePattern) {
	FillRectangle(rc, static_cast<SurfaceImpl &>(surfacePattern).pixmapColor);
}

// Never called. Line markers normally drawn as rounded rectangles are handled in
// `DrawLineMarker()`.
void SurfaceImpl::RoundedRectangle(PRectangle /*rc*/, FillStroke /*fillStroke*/) {}

// Drawing alpha rectangles is not fully supported.
// Instead, fills the background color of the given rectangle with the fill color, emulating
// INDIC_STRAIGHTBOX with no transparency.
// Called to draw INDIC_ROUNDBOX and INDIC_STRAIGHTBOX indicators, text blobs, and translucent
// line states and selections.
// Does not draw INDIC_FULLBOX correctly.
void SurfaceImpl::AlphaRectangle(PRectangle rc, XYPOSITION /*cornerSize*/, FillStroke fillStroke) {
	ColourRGBA &fill = fillStroke.fill.colour;
	for (int x = static_cast<int>(std::max(rc.left, clip.left)), y = static_cast<int>(rc.top - 1);
			 x < rc.right; x++) {
		attr_t attrs = mvwinch(win, y, x) & A_ATTRIBUTES;
		short pair = PAIR_NUMBER(attrs), fore = COLOR_WHITE, unused;
		if (pair > 0) pair_content(pair, &fore, &unused);
		mvwchgat(win, y, x, 1, attrs, term_color_pair(fore, fill), nullptr);
	}
}

void SurfaceImpl::GradientRectangle(
	PRectangle /*rc*/, const std::vector<ColourStop> & /*stops*/, GradientOptions /*options*/) {}

void SurfaceImpl::DrawRGBAImage(
	PRectangle /*rc*/, int /*width*/, int /*height*/, const unsigned char * /*pixelsImage*/) {}

// Never called. Line markers normally drawn as circles are handled in `DrawLineMarker()`.
void SurfaceImpl::Ellipse(PRectangle /*rc*/, FillStroke /*fillStroke*/) {}

// Drawing curved ends on EOL annotations is not implemented.
void SurfaceImpl::Stadium(PRectangle /*rc*/, FillStroke /*fillStroke*/, Ends /*ends*/) {}

// Draw an indentation guide.
// Only called when drawing indentation guides or during certain drawing operations when double
// buffering is enabled. Since the latter is not supported, assume the former.
void SurfaceImpl::Copy(PRectangle rc, Point /*from*/, Surface & /*surfaceSource*/) {
	// TODO: handle indent guide highlighting.
	if (rc.left - 1 < clip.left) return;
	wattr_set(win, 0, term_color_pair(COLOR_BLACK, COLOR_BLACK), nullptr);
	mvwaddch(win, static_cast<int>(rc.top), static_cast<int>(rc.left - 1), '|' | A_BOLD);
}

std::unique_ptr<IScreenLineLayout> SurfaceImpl::Layout(const IScreenLine * /*screenLine*/) {
	return nullptr;
}

#if _WIN32
#define wcwidth(_) 1 // TODO: http://www.cl.cam.ac.uk/~mgk25/ucs/wcwidth.c
#endif

/**
 * Returns the number of columns used to display the first UTF-8 character in `s`, taking
 * into account zero-width combining characters.
 * @param s The string that contains the first UTF-8 character to display.
 */
int grapheme_width(const char *s) {
	wchar_t wch;
	if (mbtowc(&wch, s, MB_CUR_MAX) < 1) return 1;
	int width = wcwidth(wch);
	return width >= 0 ? width : 1;
}

void SurfaceImpl::DrawTextNoClip(PRectangle rc, const Font *font_, XYPOSITION /*ybase*/,
	std::string_view text, ColourRGBA fore, ColourRGBA back) {
	attr_t attrs = dynamic_cast<const FontImpl *>(font_)->attrs;
	wattr_set(win, attrs, term_color_pair(fore, back), nullptr);
	if (rc.left < clip.left) {
		// Do not overwrite margin text.
		auto clip_chars = static_cast<int>(clip.left - rc.left);
		size_t offset = 0;
		for (int chars = 0; offset < text.length(); offset++) {
			if (!UTF8IsTrailByte(static_cast<unsigned char>(text[offset])))
				chars += grapheme_width(text.data() + offset);
			if (chars > clip_chars) break;
		}
		text.remove_prefix(offset);
		rc.left = clip.left;
	}
	// Do not write beyond right window boundary.
	int clip_chars = getmaxx(win) - static_cast<int>(rc.left);
	size_t bytes = 0;
	for (int chars = 0; bytes < text.length(); bytes++) {
		if (!UTF8IsTrailByte(static_cast<unsigned char>(text[bytes])))
			chars += grapheme_width(text.data() + bytes);
		if (chars > clip_chars) break;
	}
	mvwaddnstr(win, static_cast<int>(rc.top), static_cast<int>(rc.left), text.data(),
		static_cast<int>(std::min(text.length(), bytes)));
}

// Called for drawing the caret, text blobs, and `MarkerSymbol::Character` line markers.
// When drawing control characters, *rc* needs to have its pixel padding removed since curses
// has smaller resolution. Similarly when drawing line markers, *rc* needs to be reshaped.
void SurfaceImpl::DrawTextClipped(PRectangle rc, const Font *font_, XYPOSITION ybase,
	std::string_view text, ColourRGBA fore, ColourRGBA back) {
	if (rc.left >= rc.right) // when drawing text blobs
		rc.left -= 2, rc.right -= 2, rc.top -= 1, rc.bottom -= 1;
	DrawTextNoClip(rc, font_, ybase, text, fore, back);
}

// Called for drawing CallTip text and two-phase buffer text.
void SurfaceImpl::DrawTextTransparent(
	PRectangle rc, const Font *font_, XYPOSITION ybase, std::string_view text, ColourRGBA fore) {
	if (static_cast<int>(rc.top) > getmaxy(win) - 1) return;
	auto left = static_cast<int>(rc.left);
	attr_t attrs = left >= clip.left ? mvwinch(win, static_cast<int>(rc.top), left) : 0;
	short pair = PAIR_NUMBER(attrs), unused, back = COLOR_BLACK;
	if (pair > 0 && !isCallTip) pair_content(pair, &unused, &back);
	DrawTextNoClip(rc, font_, ybase, text, fore, SCI_COLORS[back]);
}

// Curses characters always have a width of 1 if they are not UTF-8 trailing bytes.
void SurfaceImpl::MeasureWidths(
	const Font * /*font_*/, std::string_view text, XYPOSITION *positions) {
	for (size_t i = 0, j = 0; i < text.length(); i++) {
		if (!UTF8IsTrailByte(static_cast<unsigned char>(text[i]))) j += grapheme_width(text.data() + i);
		positions[i] = static_cast<XYPOSITION>(j);
	}
}

XYPOSITION SurfaceImpl::WidthText(const Font * /*font_*/, std::string_view text) {
	int width = 0;
	for (size_t i = 0; i < text.length(); i++)
		if (!UTF8IsTrailByte(static_cast<unsigned char>(text[i])))
			width += grapheme_width(text.data() + i);
	return width;
}

void SurfaceImpl::DrawTextNoClipUTF8(PRectangle rc, const Font *font_, XYPOSITION ybase,
	std::string_view text, ColourRGBA fore, ColourRGBA back) {
	DrawTextNoClip(rc, font_, ybase, text, fore, back);
}

void SurfaceImpl::DrawTextClippedUTF8(PRectangle rc, const Font *font_, XYPOSITION ybase,
	std::string_view text, ColourRGBA fore, ColourRGBA back) {
	DrawTextClipped(rc, font_, ybase, text, fore, back);
}

void SurfaceImpl::DrawTextTransparentUTF8(
	PRectangle rc, const Font *font_, XYPOSITION ybase, std::string_view text, ColourRGBA fore) {
	DrawTextTransparent(rc, font_, ybase, text, fore);
}

void SurfaceImpl::MeasureWidthsUTF8(
	const Font *font_, std::string_view text, XYPOSITION *positions) {
	MeasureWidths(font_, text, positions);
}

XYPOSITION SurfaceImpl::WidthTextUTF8(const Font *font_, std::string_view text) {
	return WidthText(font_, text);
}

XYPOSITION SurfaceImpl::Ascent(const Font * /*font_*/) { return 0; }

XYPOSITION SurfaceImpl::Descent(const Font * /*font_*/) { return 0; }

XYPOSITION SurfaceImpl::InternalLeading(const Font * /*font_*/) { return 0; }

XYPOSITION SurfaceImpl::Height(const Font * /*font_*/) { return 1; }

XYPOSITION SurfaceImpl::AverageCharWidth(const Font * /*font_*/) { return 1; }

void SurfaceImpl::SetClip(PRectangle rc) { clip = rc; }

void SurfaceImpl::PopClip() { clip.left = 0, clip.top = 0, clip.right = 0, clip.bottom = 0; }

void SurfaceImpl::FlushCachedState() {}

void SurfaceImpl::FlushDrawing() {} // N/A

// Draws the text representation of a lien marker, if possible.
void SurfaceImpl::DrawLineMarker(
	const PRectangle &rcWhole, const Font *fontForCharacter, int /*tFold*/, const void *data) {
	// TODO: handle fold marker highlighting.
	auto marker = reinterpret_cast<const LineMarker *>(data);
	wattr_set(win, 0, term_color_pair(marker->fore, marker->back), nullptr);
	int top = static_cast<int>(rcWhole.top), left = static_cast<int>(rcWhole.left);
	switch (marker->markType) {
	case MarkerSymbol::Circle: mvwaddstr(win, top, left, "●"); return;
	case MarkerSymbol::SmallRect:
	case MarkerSymbol::RoundRect: mvwaddstr(win, top, left, "■"); return;
	case MarkerSymbol::Arrow: mvwaddstr(win, top, left, "►"); return;
	case MarkerSymbol::ShortArrow: mvwaddstr(win, top, left, "→"); return;
	case MarkerSymbol::ArrowDown: mvwaddstr(win, top, left, "▼"); return;
	case MarkerSymbol::Minus: mvwaddch(win, top, left, '-'); return;
	case MarkerSymbol::BoxMinus:
	case MarkerSymbol::BoxMinusConnected: mvwaddstr(win, top, left, "⊟"); return;
	case MarkerSymbol::CircleMinus:
	case MarkerSymbol::CircleMinusConnected: mvwaddstr(win, top, left, "⊖"); return;
	case MarkerSymbol::Plus: mvwaddch(win, top, left, '+'); return;
	case MarkerSymbol::BoxPlus:
	case MarkerSymbol::BoxPlusConnected: mvwaddstr(win, top, left, "⊞"); return;
	case MarkerSymbol::CirclePlus:
	case MarkerSymbol::CirclePlusConnected: mvwaddstr(win, top, left, "⊕"); return;
	case MarkerSymbol::VLine: mvwaddch(win, top, left, ACS_VLINE); return;
	case MarkerSymbol::LCorner:
	case MarkerSymbol::LCornerCurve: mvwaddch(win, top, left, ACS_LLCORNER); return;
	case MarkerSymbol::TCorner:
	case MarkerSymbol::TCornerCurve: mvwaddch(win, top, left, ACS_LTEE); return;
	case MarkerSymbol::DotDotDot: mvwaddstr(win, top, left, "…"); return;
	case MarkerSymbol::Arrows: mvwaddstr(win, top, left, "»"); return;
	case MarkerSymbol::FullRect: FillRectangle(rcWhole, marker->back); return;
	case MarkerSymbol::LeftRect: mvwaddstr(win, top, left, "▌"); return;
	case MarkerSymbol::Bookmark: mvwaddstr(win, top, left, "Σ"); return;
	default: break; // prevent warning
	}
	if (marker->markType >= MarkerSymbol::Character) {
		auto ch = static_cast<char>(
			static_cast<int>(marker->markType) - static_cast<int>(MarkerSymbol::Character));
		DrawTextClipped(
			rcWhole, fontForCharacter, rcWhole.bottom, std::string(&ch, 1), marker->fore, marker->back);
		return;
	}
}

// Draws the text representation of a wrap marker.
void SurfaceImpl::DrawWrapMarker(PRectangle rcPlace, bool isEndMarker, ColourRGBA wrapColour) {
	wattr_set(win, 0, term_color_pair(wrapColour, COLOR_BLACK), nullptr);
	mvwaddstr(
		win, static_cast<int>(rcPlace.top), static_cast<int>(rcPlace.left), isEndMarker ? "↩" : "↪");
}

// Draws the text representation of a tab arrow.
void SurfaceImpl::DrawTabArrow(PRectangle rcTab, const ViewStyle &vsDraw) {
	// TODO: set color to vs.whitespaceColours.fore and back.
	wattr_set(win, 0, term_color_pair(COLOR_BLACK, COLOR_BLACK), nullptr);
	for (int i = static_cast<int>(std::max(rcTab.left - 1, clip.left)); i < rcTab.right; i++)
		mvwaddch(win, static_cast<int>(rcTab.top), i, '-' | A_BOLD);
	char tail = vsDraw.tabDrawMode == TabDrawMode::LongArrow ? '>' : '-';
	mvwaddch(win, static_cast<int>(rcTab.top), static_cast<int>(rcTab.right), tail | A_BOLD);
}

std::unique_ptr<Surface> Surface::Allocate(Technology /*technology*/) {
	return std::make_unique<SurfaceImpl>();
}

// Window handling.

Window::~Window() noexcept {}

// Releases the window's resources.
// Since the only Windows created are AutoComplete and CallTip windows, and since those windows
// created in `ListBox::Create()` and `ScintillaCurses::CreateCallTipWindow()`, respectively,
// via `newwin()`, it is safe to use `delwin()`.
// It is important to note that even though `ScintillaCurses::wMain` is a Window, its `Destroy()`
// function is never called, hence why `scintilla_delete()` is the complement to `scintilla_new()`.
void Window::Destroy() noexcept {
	if (wid) delwin(_WINDOW(wid));
	wid = nullptr;
}

// Unlike other platforms, Scintilla paints in coordinates relative to the window in
// curses. Therefore, this function should always return the window bounds to ensure all of it
// is painted.
PRectangle Window::GetPosition() const {
	int maxx = wid ? getmaxx(_WINDOW(wid)) : 0;
	int maxy = wid ? getmaxy(_WINDOW(wid)) : 0;
	return PRectangle(0, 0, maxx, maxy);
}

void Window::SetPositionRelative(PRectangle rc, const Window *relativeTo) {
	int begx = 0, begy = 0, x = 0, y = 0;
	// Determine the relative position.
	getbegyx(_WINDOW(relativeTo->GetID()), begy, begx);
	x = begx + static_cast<int>(rc.left);
	if (x < begx) x = begx;
	y = begy + static_cast<int>(rc.top);
	if (y < begy) y = begy;
	// Correct to fit the parent if necessary.
	int sizex = static_cast<int>(rc.right - rc.left);
	int sizey = static_cast<int>(rc.bottom - rc.top);
	int screen_width = getmaxx(_WINDOW(relativeTo->GetID()));
	int screen_height = getmaxy(_WINDOW(relativeTo->GetID()));
	if (sizex > screen_width)
		x = begx; // align left
	else if (x + sizex > begx + screen_width)
		x = begx + screen_width - sizex; // align right
	if (y + sizey > begy + screen_height) {
		y = begy + screen_height - sizey; // align bottom
		if (screen_height == 1) y--; // show directly above the relative window
	}
	if (y < 0) y = begy; // align top
	// Update the location.
	mvwin(_WINDOW(wid), y, x);
}

PRectangle Window::GetClientPosition() const { return GetPosition(); }

void Window::Show(bool /*show*/) {} // TODO: ?

void Window::InvalidateAll() {} // TODO: notify repaint?

void Window::InvalidateRectangle(PRectangle /*rc*/) {} // TODO: notify repaint?

void Window::SetCursor(Cursor /*curs*/) {}

PRectangle Window::GetMonitorRect(Point /*pt*/) { return GetPosition(); }

ListBoxImpl::ListBoxImpl() {
	list.reserve(10);
	ClearRegisteredImages();
}

void ListBoxImpl::SetFont(const Font * /*font*/) {}

void ListBoxImpl::Create(Window & /*parent*/, int /*ctrlID*/, Point /*location_*/,
	int /*lineHeight_*/, bool /*unicodeMode_*/, Technology /*technology_*/) {
	wid = newwin(1, 1, 0, 0); // will be resized as items are added
}

void ListBoxImpl::SetAverageCharWidth(int /*width*/) {} // N/A

void ListBoxImpl::SetVisibleRows(int rows) {
	height = rows;
	wresize(_WINDOW(wid), height + 2, width + 2);
}

int ListBoxImpl::GetVisibleRows() const { return height; }

PRectangle ListBoxImpl::GetDesiredRect() {
	return PRectangle(0, 0, width + 2, height + 2); // add border widths
}

int ListBoxImpl::CaretFromEdge() { return 2; } // shift border and type character over

void ListBoxImpl::Clear() noexcept {
	list.clear();
	width = 0;
}

void ListBoxImpl::Append(char *s, int type) {
	if (type >= 0 && type <= IMAGE_MAX) {
		char *chtype = types[type];
		list.push_back(std::string(chtype, strlen(chtype)) + s);
	} else
		list.push_back(std::string(" ") + s);
	int len = static_cast<int>(strlen(s)); // TODO: UTF-8 awareness?
	if (width < len + 1) {
		width = len + 1; // include type character len
		wresize(_WINDOW(wid), height + 2, width + 2);
	}
}

int ListBoxImpl::Length() { return static_cast<int>(list.size()); }

void ListBoxImpl::Select(int n) {
	WINDOW *w = _WINDOW(wid);
	wclear(w);
	box(w, '|', '-');
	auto len = static_cast<int>(list.size());
	int s = n - height / 2;
	if (s + height > len) s = len - height;
	if (s < 0) s = 0;
	for (int i = s; i < s + height && i < len; i++) {
		mvwaddstr(w, i - s + 1, 1, list.at(i).c_str());
		if (i == n) mvwchgat(w, i - s + 1, 2, width - 1, A_REVERSE, 0, nullptr);
	}
	wmove(w, n - s + 1, 1); // place cursor on selected line
	wnoutrefresh(w);
	selection = n;
}

int ListBoxImpl::GetSelection() { return selection; }

// The type is the first (UTF-8) character, so start searching at the second one.
int ListBoxImpl::Find(const char *prefix) {
	size_t len = strlen(prefix);
	for (unsigned int i = 0; i < list.size(); i++) {
		const char *item = list.at(i).c_str();
		item += UTF8DrawBytes(reinterpret_cast<const char *>(item), strlen(item));
		if (strncmp(prefix, item, len) == 0) return i;
	}
	return -1;
}

// The type is the first (UTF-8) character, so return the string after it.
std::string ListBoxImpl::GetValue(int n) {
	const char *item = list.at(n).c_str();
	item += UTF8DrawBytes(reinterpret_cast<const char *>(item), strlen(item));
	return item;
}

// Register the first UTF-8 character as the type (e.g. "*", "+", or "■").
// By default, ' ' (space) is registered to all types.
void ListBoxImpl::RegisterImage(int type, const char *xpm_data) {
	if (type < 0 || type > IMAGE_MAX) return;
	int len = UTF8DrawBytes(reinterpret_cast<const char *>(xpm_data), strlen(xpm_data));
	for (int i = 0; i < len; i++) types[type][i] = xpm_data[i];
	types[type][len] = '\0';
}

void ListBoxImpl::RegisterRGBAImage(
	int /*type*/, int /*width*/, int /*height*/, const unsigned char * /*pixelsImage*/) {} // N/A

// Clear back to ' ' (space).
void ListBoxImpl::ClearRegisteredImages() {
	for (int i = 0; i <= IMAGE_MAX; i++) types[i][0] = ' ', types[i][1] = '\0';
}

void ListBoxImpl::SetDelegate(IListBoxDelegate *lbDelegate) { delegate = lbDelegate; }

void ListBoxImpl::SetList(const char *listText, char separator, char typesep) {
	Clear();
	size_t len = strlen(listText);
	char *text = new char[len + 1];
	if (!text) return;
	memcpy(text, listText, len + 1);
	char *word = text, *type = nullptr;
	for (size_t i = 0; i <= len; i++) {
		if (text[i] == separator || i == len) {
			text[i] = '\0';
			if (type) *type = '\0';
			Append(word, type ? atoi(type + 1) : -1);
			word = text + i + 1, type = nullptr;
		} else if (text[i] == typesep)
			type = text + i;
	}
	delete[] text;
}

void ListBoxImpl::SetOptions(ListOptions /*options_*/) {}

ListBox::ListBox() noexcept = default;

ListBox::~ListBox() noexcept = default;

std::unique_ptr<ListBox> ListBox::Allocate() { return std::make_unique<ListBoxImpl>(); }

Menu::Menu() noexcept : mid(nullptr) {}

void Menu::CreatePopUp() {}

void Menu::Destroy() noexcept {}

void Menu::Show(Point /*pt*/, const Window & /*w*/) {}

// System wide platform parameters.

ColourRGBA Platform::Chrome() { return ColourRGBA(0, 0, 0); }

ColourRGBA Platform::ChromeHighlight() { return ColourRGBA(0, 0, 0); }

const char *Platform::DefaultFont() { return "monospace"; }

int Platform::DefaultFontSize() { return 10; }

unsigned int Platform::DoubleClickTime() { return 500; /* ms */ }

void Platform::DebugDisplay(const char *s) noexcept { fprintf(stderr, "%s", s); }

void Platform::DebugPrintf(const char * /*format*/, ...) noexcept {}

// bool Platform::ShowAssertionPopUps(bool assertionPopUps_) noexcept { return true; }

void Platform::Assert(const char *c, const char *file, int line) noexcept {
	char buffer[2000];
	sprintf(buffer, "Assertion [%s] failed at %s %d\r\n", c, file, line);
	Platform::DebugDisplay(buffer);
	abort();
}

} // namespace Scintilla::Internal
