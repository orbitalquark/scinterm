// Copyright 2012-2025 Mitchell. See LICENSE.
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
#include <limits>

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
	if (fp.weight == FontWeight::Bold) attrs = A_BOLD;
#if defined(A_ITALIC)
	if (fp.italic) attrs |= A_ITALIC;
#endif
	if (static_cast<int>(fp.stretch) == A_UNDERLINE) attrs |= A_UNDERLINE;
}

std::shared_ptr<Font> Font::Allocate(const FontParameters &fp) {
	return std::make_shared<FontImpl>(fp);
}

// Color handling.

// These color values are arbitrary and serve only as a map of Scintilla colors to the terminal
// emulator's color palette.
ColourRGBA Colors::Black = ColourRGBA(0, 0, 0);
ColourRGBA Colors::Red = ColourRGBA(0x80, 0, 0);
ColourRGBA Colors::Green = ColourRGBA(0, 0x80, 0);
ColourRGBA Colors::Yellow = ColourRGBA(0x80, 0x80, 0);
ColourRGBA Colors::Blue = ColourRGBA(0, 0, 0x80);
ColourRGBA Colors::Magenta = ColourRGBA(0x80, 0, 0x80);
ColourRGBA Colors::Cyan = ColourRGBA(0, 0x80, 0x80);
ColourRGBA Colors::White = ColourRGBA(0xC0, 0xC0, 0xC0);
ColourRGBA Colors::LBlack = ColourRGBA(0x40, 0x40, 0x40);
ColourRGBA Colors::LRed = ColourRGBA(0xFF, 0, 0);
ColourRGBA Colors::LGreen = ColourRGBA(0, 0xFF, 0);
ColourRGBA Colors::LYellow = ColourRGBA(0xFF, 0xFF, 0);
ColourRGBA Colors::LBlue = ColourRGBA(0, 0, 0xFF);
ColourRGBA Colors::LMagenta = ColourRGBA(0xFF, 0, 0xFF);
ColourRGBA Colors::LCyan = ColourRGBA(0, 0xFF, 0xFF);
ColourRGBA Colors::LWhite = ColourRGBA(0xFF, 0xFF, 0xFF);

Colors::Colors() {
	if (!has_colors()) return;
	start_color();

	// Populate the colors map with builtin curses colors.
	// Also initialize the bold/light variants for terminals that support more than 8 colors.
	colors.emplace(Black.OpaqueRGB(), COLOR_BLACK);
	colors.emplace(Red.OpaqueRGB(), COLOR_RED);
	colors.emplace(Green.OpaqueRGB(), COLOR_GREEN);
	colors.emplace(Yellow.OpaqueRGB(), COLOR_YELLOW);
	colors.emplace(Blue.OpaqueRGB(), COLOR_BLUE);
	colors.emplace(Magenta.OpaqueRGB(), COLOR_MAGENTA);
	colors.emplace(Cyan.OpaqueRGB(), COLOR_CYAN);
	colors.emplace(White.OpaqueRGB(), COLOR_WHITE);
	if (COLORS >= 16) {
		colors.emplace(LBlack.OpaqueRGB(), COLOR_BLACK + 8);
		colors.emplace(LRed.OpaqueRGB(), COLOR_RED + 8);
		colors.emplace(LGreen.OpaqueRGB(), COLOR_GREEN + 8);
		colors.emplace(LYellow.OpaqueRGB(), COLOR_YELLOW + 8);
		colors.emplace(LBlue.OpaqueRGB(), COLOR_BLUE + 8);
		colors.emplace(LMagenta.OpaqueRGB(), COLOR_MAGENTA + 8);
		colors.emplace(LCyan.OpaqueRGB(), COLOR_CYAN + 8);
		colors.emplace(LWhite.OpaqueRGB(), COLOR_WHITE + 8);
	}
}

Colors &Colors::instance() {
	static Colors inst;
	return inst;
}

short Colors::get(const ColourRGBA &color) {
	if (const auto entry = colors.find(color.OpaqueRGB()); entry != colors.end())
		return entry->second;
	if (colors.size() >= std::numeric_limits<short>::max()) return COLOR_WHITE;
	const auto c = static_cast<short>(colors.size());
	init_color(c, color.GetRed() * 1000.0 / 255, color.GetGreen() * 1000.0 / 255,
		color.GetBlue() * 1000.0 / 255);
	colors.emplace(color.OpaqueRGB(), c);
	return c;
}

attr_t Colors::Pair(const ColourRGBA &fore, const ColourRGBA &back) {
	if (!has_colors()) return back.Opaque() == Black ? 0 : A_REVERSE;
	auto &pairs = instance().pairs;
	const auto pair = std::make_pair(instance().get(fore), instance().get(back));
	if (const auto entry = pairs.find(pair); entry != pairs.end()) return entry->second;
	if (pairs.size() >= static_cast<size_t>(COLORS)) return 0;
	const short n = pairs.size() + 1; // starts from 1, not 0
	init_pair(n, pair.first, pair.second);
	pairs.emplace(pair, COLOR_PAIR(n));
	return COLOR_PAIR(n);
}

ColourRGBA Colors::Find(const short color) {
	for (const auto &pair : instance().colors)
		if (pair.second == color) return ColourRGBA(pair.first);
	return Colors::White;
}

// Surface handling.

SurfaceImpl::~SurfaceImpl() noexcept { Release(); }

void SurfaceImpl::Init(WindowID wid) {
	Release();
	win = _WINDOW(wid);
}

void SurfaceImpl::Init(SurfaceID /*sid*/, WindowID wid) { Init(wid); }

// Pixmap surfaces are not supported, but this must return a surface because Scintilla assumes the
// allocation succeeded.
// Some surfaces can be inferred, like indent guides, which have a width of 1.
std::unique_ptr<Surface> SurfaceImpl::AllocatePixMap(int width, int /*height*/) {
	// Not supported, but cannot return a nullptr because Scintilla assumes the allocation succeeded.
	static int count = 0;
	auto surface = std::make_unique<SurfaceImpl>();
	if (width == 1) {
		// Scintilla allocates pixmap indent guides in pairs.
		surface->isIndentGuideHighlight = ++count % 2 == 0;
	}
	return surface;
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
	wattrset(win, Colors::Pair(back, Colors::White)); // invert
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
	wattrset(win, Colors::Pair(Colors::White, fill.colour));
	chtype ch = ' ';
	if (fabs(rc.left - static_cast<int>(rc.left)) > 0.1) {
		// If rc.left is a fractional value (e.g. 4.5) then whitespace dots are being drawn. Draw
		// them appropriately.
		wattrset(win, Colors::Pair(fill.colour, fill.colour));
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
		short pair = PAIR_NUMBER(attrs), fore = COLOR_WHITE, back = COLOR_BLACK;
		if (pair > 0) pair_content(pair, &fore, &back);
		if (fill.GetAlpha())
			mvwchgat(win, y, x, 1, attrs, PAIR_NUMBER(Colors::Pair(Colors::Find(fore), fill)), nullptr);
		else
			mvwchgat(win, y, x, 1, attrs | A_UNDERLINE, PAIR_NUMBER(Colors::Pair(fill, Colors::Find(back))), nullptr);
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
void SurfaceImpl::Copy(PRectangle rc, Point /*from*/, Surface &surfaceSource) {
	const ColourRGBA &fore = dynamic_cast<SurfaceImpl *>(&surfaceSource)->isIndentGuideHighlight ?
		Colors::White :
		Colors::Black;
	if (rc.left - 1 < clip.left) return;
	wattrset(win, Colors::Pair(fore, Colors::Black));
	mvwaddch(win, static_cast<int>(rc.top), static_cast<int>(rc.left - 1), '|' | A_BOLD);
}

std::unique_ptr<IScreenLineLayout> SurfaceImpl::Layout(const IScreenLine * /*screenLine*/) {
	return nullptr;
}

#if _WIN32
extern "C" {
int wcwidth(wchar_t ucs);
int wcswidth(const wchar_t *pwcs, size_t n);
}
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
	wattrset(win, attrs | Colors::Pair(fore, back));
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
	short back = COLOR_BLACK;
	if (!isCallTip) {
		auto left = static_cast<int>(rc.left);
		attr_t attrs = left >= clip.left ? mvwinch(win, static_cast<int>(rc.top), left) : 0;
		short pair = PAIR_NUMBER(attrs), unused;
		if (pair > 0)
			pair_content(pair, &unused, &back);
		else if (attrs & A_REVERSE) // !hascolors() and white terminal background
			back = COLOR_WHITE;
	}
	DrawTextNoClip(rc, font_, ybase, text, fore, Colors::Find(back));
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
	const PRectangle &rcWhole, const Font *fontForCharacter, int tFold, const void *data) {
	auto marker = reinterpret_cast<const LineMarker *>(data);
	attr_t attr = Colors::Pair(marker->fore, marker->back);
	if (tFold) attr |= A_BOLD;
	wattrset(win, attr);
	int top = static_cast<int>(rcWhole.top), left = static_cast<int>(rcWhole.left);
	switch (marker->markType) {
	case MarkerSymbol::Circle: mvwaddstr(win, top, left, "●"); return;
	case MarkerSymbol::SmallRect:
	case MarkerSymbol::RoundRect: mvwaddstr(win, top, left, "■"); return;
	case MarkerSymbol::Arrow: mvwaddstr(win, top, left, "►"); return;
	case MarkerSymbol::ShortArrow: mvwaddstr(win, top, left, "➡"); return;
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
	case MarkerSymbol::Bookmark:
	case MarkerSymbol::VerticalBookmark: mvwaddstr(win, top, left, "⚑"); return;
	case MarkerSymbol::Bar: mvwaddch(win, top, left, ACS_VLINE); return;
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
	wattrset(win, Colors::Pair(wrapColour, Colors::Black));
	mvwaddstr(
		win, static_cast<int>(rcPlace.top), static_cast<int>(rcPlace.left), isEndMarker ? "↩" : "↪");
}

// Draws the text representation of a tab arrow.
void SurfaceImpl::DrawTabArrow(PRectangle rcTab, const ViewStyle &vsDraw) {
	const ColourRGBA &fore = vsDraw.ElementColour(Element::WhiteSpace).value_or(Colors::Black);
	const ColourRGBA &back = vsDraw.ElementColour(Element::WhiteSpaceBack).value_or(Colors::Black);
	wattrset(win, Colors::Pair(fore, back));
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

void Window::Show(bool /*show*/) {} // this curses window will be painted over others as necessary

void Window::InvalidateAll() {} // this window will be completely repainted anyway

void Window::InvalidateRectangle(PRectangle /*rc*/) {} // this window will be repainted anyway

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
	std::wstring ws = WStringFromUTF8(s);
	int len = wcswidth(ws.c_str(), ws.size());
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
	snprintf(buffer, sizeof(buffer), "Assertion [%s] failed at %s %d\r\n", c, file, line);
	Platform::DebugDisplay(buffer);
	abort();
}

} // namespace Scintilla::Internal
