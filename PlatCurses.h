// Copyright 2012-2022 Mitchell. See LICENSE.
// Scintilla platform for a curses (terminal) environment.

#ifndef PLAT_CURSES_H
#define PLAT_CURSES_H

namespace Scintilla::Internal {

class FontImpl : public Font {
public:
  /**
   * Sets terminal character attributes for a particular font.
   * These attributes are a union of curses attributes and stored in the font's `attrs` member.
   * The curses attributes are not constructed from various fields in *fp* since there is no
   * `underline` parameter. Instead, you need to manually set the `weight` parameter to be the
   * union of your desired attributes. Scintilla's lexers/LexLPeg.cxx has an example of this.
   */
  FontImpl(const FontParameters &fp);
  virtual ~FontImpl() noexcept override = default;

  attr_t attrs = 0;
};

class SurfaceImpl : public Surface {
  WINDOW *win = nullptr; // curses window to draw on
  PRectangle clip;

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
int term_color(ColourRGBA color);
int term_color(int color);

} // namespace Scintilla::Internal

/**
 * Returns the given Scintilla `WindowID` as a curses `WINDOW`.
 * @param w A Scintilla `WindowID`.
 * @return curses `WINDOW`.
 */
#define _WINDOW(w) reinterpret_cast<WINDOW *>(w)

/**
 * Returns a curses color pair from the given fore and back colors.
 * @param f Foreground color, either a Scintilla color or curses color.
 * @param b Background color, either a Scintilla color or curses color.
 * @return curses color pair suitable for calling `COLOR_PAIR()` with.
 */
#define term_color_pair(f, b) SCI_COLOR_PAIR(term_color(f), term_color(b))

#endif
