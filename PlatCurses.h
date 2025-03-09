// Copyright 2012-2025 Mitchell. See LICENSE.
// Scintilla platform for a curses (terminal) environment.

#ifndef PLAT_CURSES_H
#define PLAT_CURSES_H

namespace Scintilla::Internal {

class FontImpl : public Font {
public:
	/**
	 * Sets terminal character attributes for a particular font.
	 * These attributes are a union of curses attributes and stored in the font's `attrs` member.
	 */
	FontImpl(const FontParameters &fp);
	virtual ~FontImpl() noexcept override = default;

	attr_t attrs = 0;
};

class SurfaceImpl : public Surface {
	WINDOW *win = nullptr; // curses window to draw on
	PRectangle clip;
	ColourRGBA pixmapColor;

public:
	SurfaceImpl() = default;
	~SurfaceImpl() noexcept override;

	void Init(WindowID wid) override;
	void Init(SurfaceID sid, WindowID wid) override;
	std::unique_ptr<Surface> AllocatePixMap(int width, int height) override;

	void SetMode(SurfaceMode mode) override;

	void Release() noexcept override;
	int SupportsFeature(Supports feature) noexcept override;
	bool Initialised() override;
	int LogPixelsY() override;
	int PixelDivisions() override;
	int DeviceHeightFont(int points) override;
	void LineDraw(Point start, Point end, Stroke stroke) override;
	void PolyLine(const Point *pts, size_t npts, Stroke stroke) override;
	void Polygon(const Point *pts, size_t npts, FillStroke fillStroke) override;
	void RectangleDraw(PRectangle rc, FillStroke fillStroke) override;
	void RectangleFrame(PRectangle rc, Stroke stroke) override;
	void FillRectangle(PRectangle rc, Fill fill) override;
	void FillRectangleAligned(PRectangle rc, Fill fill) override;
	void FillRectangle(PRectangle rc, Surface &surfacePattern) override;
	void RoundedRectangle(PRectangle rc, FillStroke fillStroke) override;
	void AlphaRectangle(PRectangle rc, XYPOSITION cornerSize, FillStroke fillStroke) override;
	void GradientRectangle(
		PRectangle rc, const std::vector<ColourStop> &stops, GradientOptions options) override;
	void DrawRGBAImage(
		PRectangle rc, int width, int height, const unsigned char *pixelsImage) override;
	void Ellipse(PRectangle rc, FillStroke fillStroke) override;
	void Stadium(PRectangle rc, FillStroke fillStroke, Ends ends) override;
	void Copy(PRectangle rc, Point from, Surface &surfaceSource) override;

	std::unique_ptr<IScreenLineLayout> Layout(const IScreenLine *screenLine) override;

	void DrawTextNoClip(PRectangle rc, const Font *font_, XYPOSITION ybase, std::string_view text,
		ColourRGBA fore, ColourRGBA back) override;
	void DrawTextClipped(PRectangle rc, const Font *font_, XYPOSITION ybase, std::string_view text,
		ColourRGBA fore, ColourRGBA back) override;
	void DrawTextTransparent(PRectangle rc, const Font *font_, XYPOSITION ybase,
		std::string_view text, ColourRGBA fore) override;
	void MeasureWidths(const Font *font_, std::string_view text, XYPOSITION *positions) override;
	XYPOSITION WidthText(const Font *font_, std::string_view text) override;

	void DrawTextNoClipUTF8(PRectangle rc, const Font *font_, XYPOSITION ybase, std::string_view text,
		ColourRGBA fore, ColourRGBA back) override;
	void DrawTextClippedUTF8(PRectangle rc, const Font *font_, XYPOSITION ybase,
		std::string_view text, ColourRGBA fore, ColourRGBA back) override;
	void DrawTextTransparentUTF8(PRectangle rc, const Font *font_, XYPOSITION ybase,
		std::string_view text, ColourRGBA fore) override;
	void MeasureWidthsUTF8(const Font *font_, std::string_view text, XYPOSITION *positions) override;
	XYPOSITION WidthTextUTF8(const Font *font_, std::string_view text) override;

	XYPOSITION Ascent(const Font *font_) override;
	XYPOSITION Descent(const Font *font_) override;
	XYPOSITION InternalLeading(const Font *font_) override;
	XYPOSITION Height(const Font *font_) override;
	XYPOSITION AverageCharWidth(const Font *font_) override;

	void SetClip(PRectangle rc) override;
	void PopClip() override;
	void FlushCachedState() override;
	void FlushDrawing() override;

	// Custom drawing functions for Curses.
	void DrawLineMarker(
		const PRectangle &rcWhole, const Font *fontForCharacter, int tFold, const void *data);
	void DrawWrapMarker(PRectangle rcPlace, bool isEndMarker, ColourRGBA wrapColour);
	void DrawTabArrow(PRectangle rcTab, const ViewStyle &vsDraw);

	bool isCallTip = false;
};

class ListBoxImpl : public ListBox {
	int height = 5, width = 10;
	std::vector<std::string> list;
	char types[IMAGE_MAX + 1][5]; // UTF-8 character plus terminating '\0' instead of an image
	int selection = 0;

public:
	IListBoxDelegate *delegate = nullptr;

	ListBoxImpl();
	~ListBoxImpl() override = default;

	void SetFont(const Font *font) override;
	void Create(Window &parent, int ctrlID, Point location_, int lineHeight_, bool unicodeMode_,
		Technology technology_) override;
	void SetAverageCharWidth(int width) override;
	void SetVisibleRows(int rows) override;
	int GetVisibleRows() const override;
	PRectangle GetDesiredRect() override;
	int CaretFromEdge() override;
	void Clear() noexcept override;
	void Append(char *s, int type) override;
	int Length() override;
	void Select(int n) override;
	int GetSelection() override;
	int Find(const char *prefix) override;
	std::string GetValue(int n) override;
	void RegisterImage(int type, const char *xpm_data) override;
	void RegisterRGBAImage(
		int type, int width, int height, const unsigned char *pixelsImage) override;
	void ClearRegisteredImages() override;
	void SetDelegate(IListBoxDelegate *lbDelegate) override;
	void SetList(const char *listText, char separator, char typesep) override;
	void SetOptions(ListOptions options_) override;
};

void init_colors();
/** Returns the curses `COLOR_PAIR` for the given curses foreground and background  `COLOR`s. */
inline short color_pair(short fore, short back) { return back * (COLORS < 16 ? 8 : 16) + fore + 1; }
inline short term_color(int color) { return color; }
short term_color(ColourRGBA color);
/**
 * Returns a curses attribute for drawing with the given foreground and background colors.
 * If curses does not support colors, returns an attribute that represents black or white.
 */
template <typename FT, typename BT>
attr_t color_attr(const FT &fore, const BT &back) {
	if (!has_colors()) return term_color(back) != COLOR_BLACK ? A_REVERSE : 0;
	return COLOR_PAIR(color_pair(term_color(fore), term_color(back)));
}

inline WINDOW *_WINDOW(WindowID wid) { return reinterpret_cast<WINDOW *>(wid); }

} // namespace Scintilla::Internal

#endif
