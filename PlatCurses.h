// Copyright 2012-2026 Mitchell. See LICENSE.
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
	explicit FontImpl(const FontParameters &fp);
	~FontImpl() noexcept override = default;

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

	bool isIndentGuideHighlight = false;
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

/** Singleton curses color and color pair manager.
 * Colors and pairs do not need to be declared ahead of time; they are initialized on demand.
 */
class Colors {
	std::map<int, short> colors; // map of RGB ints to curses color numbers.
	std::map<std::pair<short, short>, short> pairs; // map of curses colors to their pair numbers
	int colorOffset = 0, pairOffset = 0;

	Colors();
	static Colors &instance();
	/** Returns the curses number for a Scintilla color, initializing it if necessary. */
	short get(const ColourRGBA &color);

public:
	static ColourRGBA Black, Red, Green, Yellow, Blue, Magenta, Cyan, White;
	static ColourRGBA LBlack, LRed, LGreen, LYellow, LBlue, LMagenta, LCyan, LWhite;

	/** Returns the curses pair number and an attribute for a Scintilla color pair, initializing it if necessary. */
	static short Pair(const ColourRGBA &fore, const ColourRGBA &back, attr_t &attr);
	/** Sets current color on a curses window. */
	static int wattr_set(WINDOW *win, const ColourRGBA &fore, const ColourRGBA &back, attr_t attr = 0);
	/** Returns the Scintilla color for a given curses color number. */
	static ColourRGBA Find(const short color);

	/** Sets the offsets for colors and color pairs generated on-demand. */
	static void SetOffsets(int colorOffset, int pairOffset);
};

inline WINDOW *_WINDOW(WindowID wid) { return reinterpret_cast<WINDOW *>(wid); }

} // namespace Scintilla::Internal

#endif
