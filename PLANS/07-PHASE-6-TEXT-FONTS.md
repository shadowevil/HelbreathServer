# Phase 6: Text + Fonts

## Overview

This phase implements proper text rendering and font management to replace the ad-hoc text drawing scattered throughout the codebase. Includes support for bitmap fonts, lazy-loading, and caching.

**Prerequisite:** Phase 5 complete (UI and ECS in place)

---

## Goals

1. Create FontManager with lazy-load caching
2. Create TextRenderer for DirectDraw7
3. Isolate bitmap font handling
4. Support multiple font styles and sizes
5. Enable proper text layout and alignment

---

## Current Problems

```cpp
// Ad-hoc text rendering everywhere
void CGame::DrawText(int x, int y, char* text) {
    // Direct GDI or sprite-based rendering
    // No caching, no proper font management
    HDC hdc;
    m_pSurface->GetDC(&hdc);
    TextOut(hdc, x, y, text, strlen(text));
    m_pSurface->ReleaseDC(hdc);
}

// Multiple font loading in different places
// No consistent sizing or styling
// Performance issues from frequent DC access
```

---

## Architecture

```
┌─────────────────────────────────────────────────────────┐
│                    FontManager                           │
│  - Register fonts by name                                │
│  - Lazy-load on first use                                │
│  - Cache font handles by (font, size)                    │
└─────────────────────────────────────────────────────────┘
                              │
              ┌───────────────┼───────────────┐
              │               │               │
              ▼               ▼               ▼
        ┌─────────┐     ┌─────────┐     ┌─────────┐
        │ GDIFont │     │BitmapFont│    │ Future  │
        │ Backend │     │ Backend  │    │ Backends│
        └─────────┘     └─────────┘     └─────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────┐
│                    TextRenderer                          │
│  - Draw text to DirectDraw surfaces                      │
│  - Text layout and wrapping                              │
│  - Color, shadow, outline effects                        │
└─────────────────────────────────────────────────────────┘
```

---

## Directory Structure

```
/Sources/Client/engine/text/
├── FontManager.h
├── FontManager.cpp
├── TextRenderer.h
├── TextRenderer.cpp
├── Font.h              # Font interface
├── FontMetrics.h       # Font measurement data
├── gdi/
│   ├── GDIFont.h       # Windows GDI font wrapper
│   └── GDIFont.cpp
├── bitmap/
│   ├── BitmapFont.h    # Sprite-based font
│   ├── BitmapFont.cpp
│   └── BitmapFontLoader.h
└── layout/
    ├── TextLayout.h    # Text measurement and wrapping
    └── TextLayout.cpp
```

---

## Tasks

### Task 6.1: Create Font Interface

**Font.h:**
```cpp
#pragma once

#include <Dependencies/Shared/Types.h>
#include <string>
#include <string_view>

namespace Helbreath::Text {

/**
 * Metrics for a rendered glyph.
 */
struct GlyphMetrics {
    i32 width = 0;      // Glyph width in pixels
    i32 height = 0;     // Glyph height in pixels
    i32 bearingX = 0;   // Horizontal offset from cursor
    i32 bearingY = 0;   // Vertical offset from baseline
    i32 advance = 0;    // Cursor advance after glyph
};

/**
 * Overall font metrics.
 */
struct FontMetrics {
    i32 height = 0;         // Line height
    i32 ascent = 0;         // Height above baseline
    i32 descent = 0;        // Height below baseline
    i32 lineGap = 0;        // Extra spacing between lines
    i32 averageWidth = 0;   // Average character width
};

/**
 * Font style flags.
 */
enum class FontStyle : u8 {
    Normal = 0,
    Bold = 1 << 0,
    Italic = 1 << 1,
    Underline = 1 << 2,
    Strikethrough = 1 << 3
};

inline FontStyle operator|(FontStyle a, FontStyle b) {
    return static_cast<FontStyle>(static_cast<u8>(a) | static_cast<u8>(b));
}

inline bool hasStyle(FontStyle styles, FontStyle test) {
    return (static_cast<u8>(styles) & static_cast<u8>(test)) != 0;
}

/**
 * Font handle - identifies a loaded font.
 */
struct FontHandle {
    u32 id = 0;

    [[nodiscard]] bool isValid() const noexcept { return id != 0; }
    explicit operator bool() const noexcept { return isValid(); }
};

/**
 * Abstract font interface.
 * Implementations provide actual rendering capabilities.
 */
class IFont {
public:
    virtual ~IFont() = default;

    // Font information
    [[nodiscard]] virtual const FontMetrics& metrics() const = 0;
    [[nodiscard]] virtual GlyphMetrics getGlyphMetrics(char32_t codepoint) const = 0;

    // Text measurement
    [[nodiscard]] virtual i32 measureWidth(std::string_view text) const = 0;

    // Rendering (implementation-specific)
    virtual void renderText(
        void* targetSurface,
        i32 x, i32 y,
        std::string_view text,
        u32 color) = 0;
};

} // namespace Helbreath::Text
```

---

### Task 6.2: Create Font Manager

**FontManager.h:**
```cpp
#pragma once

#include "Font.h"
#include <memory>
#include <string>
#include <unordered_map>

namespace Helbreath::Text {

/**
 * Font registration entry.
 */
struct FontRegistration {
    std::string name;           // Logical name (e.g., "main", "title")
    std::string path;           // File path or system font name
    bool isBitmap = false;      // True for bitmap fonts
};

/**
 * Font cache key.
 */
struct FontCacheKey {
    std::string name;
    u16 size;
    FontStyle style;

    bool operator==(const FontCacheKey& other) const {
        return name == other.name && size == other.size && style == other.style;
    }
};

struct FontCacheKeyHash {
    std::size_t operator()(const FontCacheKey& key) const {
        return std::hash<std::string>{}(key.name) ^
               (std::hash<u16>{}(key.size) << 1) ^
               (std::hash<u8>{}(static_cast<u8>(key.style)) << 2);
    }
};

/**
 * Manages font registration, loading, and caching.
 * Fonts are lazily loaded on first use.
 */
class FontManager {
public:
    static FontManager& instance();

    // Font registration (call during initialization)
    void registerFont(const FontRegistration& registration);
    void registerSystemFont(std::string_view name, std::string_view systemName);
    void registerBitmapFont(std::string_view name, std::string_view path);

    // Get font (loads if not cached)
    [[nodiscard]] FontHandle getFont(
        std::string_view name,
        u16 size = 12,
        FontStyle style = FontStyle::Normal);

    // Access font by handle
    [[nodiscard]] IFont* font(FontHandle handle);
    [[nodiscard]] const IFont* font(FontHandle handle) const;

    // Font existence check
    [[nodiscard]] bool hasFont(std::string_view name) const;

    // Cache management
    void clearCache();
    [[nodiscard]] usize cacheSize() const { return m_fontCache.size(); }

    // Default font
    void setDefaultFont(std::string_view name) { m_defaultFontName = name; }
    [[nodiscard]] FontHandle defaultFont(u16 size = 12);

private:
    FontManager() = default;

    // Font loading
    std::unique_ptr<IFont> loadFont(const FontRegistration& reg, u16 size, FontStyle style);
    std::unique_ptr<IFont> loadGDIFont(const std::string& name, u16 size, FontStyle style);
    std::unique_ptr<IFont> loadBitmapFont(const std::string& path, u16 size);

    // Registration storage
    std::unordered_map<std::string, FontRegistration> m_registrations;

    // Font cache
    std::unordered_map<FontCacheKey, std::unique_ptr<IFont>, FontCacheKeyHash> m_fontCache;

    // Handle mapping
    std::unordered_map<u32, IFont*> m_handleToFont;
    u32 m_nextHandle = 1;

    std::string m_defaultFontName = "default";
};

} // namespace Helbreath::Text
```

**FontManager.cpp (Key Parts):**
```cpp
FontHandle FontManager::getFont(std::string_view name, u16 size, FontStyle style) {
    // Check if registration exists
    auto regIt = m_registrations.find(std::string(name));
    if (regIt == m_registrations.end()) {
        LOG_WARN("Font not registered: {}", name);
        return FontHandle{0};
    }

    // Check cache
    FontCacheKey key{std::string(name), size, style};
    auto cacheIt = m_fontCache.find(key);
    if (cacheIt != m_fontCache.end()) {
        // Return cached handle
        for (auto& [handle, font] : m_handleToFont) {
            if (font == cacheIt->second.get()) {
                return FontHandle{handle};
            }
        }
    }

    // Load font
    LOG_DEBUG("Loading font: {} size={} style={}", name, size, static_cast<int>(style));
    auto font = loadFont(regIt->second, size, style);
    if (!font) {
        LOG_ERROR("Failed to load font: {}", name);
        return FontHandle{0};
    }

    // Cache and create handle
    IFont* fontPtr = font.get();
    m_fontCache[key] = std::move(font);

    u32 handle = m_nextHandle++;
    m_handleToFont[handle] = fontPtr;

    return FontHandle{handle};
}
```

---

### Task 6.3: Create GDI Font Backend

**gdi/GDIFont.h:**
```cpp
#pragma once

#include "../Font.h"

#ifdef _WIN32
#include <windows.h>
#endif

namespace Helbreath::Text {

/**
 * Windows GDI-based font implementation.
 * Uses CreateFont and TextOut for rendering.
 */
class GDIFont : public IFont {
public:
    GDIFont(const std::string& fontName, u16 size, FontStyle style);
    ~GDIFont() override;

    [[nodiscard]] const FontMetrics& metrics() const override { return m_metrics; }
    [[nodiscard]] GlyphMetrics getGlyphMetrics(char32_t codepoint) const override;
    [[nodiscard]] i32 measureWidth(std::string_view text) const override;

    void renderText(
        void* targetSurface,
        i32 x, i32 y,
        std::string_view text,
        u32 color) override;

private:
    void calculateMetrics();

#ifdef _WIN32
    HFONT m_hFont = nullptr;
    HDC m_measureDC = nullptr;  // For text measurement
#endif

    FontMetrics m_metrics;
    std::string m_fontName;
    u16 m_size;
    FontStyle m_style;
};

} // namespace Helbreath::Text
```

---

### Task 6.4: Create Bitmap Font Backend

**bitmap/BitmapFont.h:**
```cpp
#pragma once

#include "../Font.h"
#include <unordered_map>
#include <vector>

namespace Helbreath::Text {

/**
 * Glyph data for bitmap font.
 */
struct BitmapGlyph {
    u16 x, y;           // Position in atlas
    u16 width, height;  // Size in atlas
    i16 offsetX, offsetY;
    i16 advance;
};

/**
 * Bitmap font loaded from sprite atlas.
 * Each character is a sprite in the atlas.
 */
class BitmapFont : public IFont {
public:
    // Load from custom format
    [[nodiscard]] static std::unique_ptr<BitmapFont> loadFromFile(
        const std::string& path);

    // Load from standard formats (BMFont, etc.)
    [[nodiscard]] static std::unique_ptr<BitmapFont> loadBMFont(
        const std::string& fntPath);

    [[nodiscard]] const FontMetrics& metrics() const override { return m_metrics; }
    [[nodiscard]] GlyphMetrics getGlyphMetrics(char32_t codepoint) const override;
    [[nodiscard]] i32 measureWidth(std::string_view text) const override;

    void renderText(
        void* targetSurface,
        i32 x, i32 y,
        std::string_view text,
        u32 color) override;

    // Atlas access for rendering
    void* atlasTexture() const { return m_atlasData; }
    i32 atlasWidth() const { return m_atlasWidth; }
    i32 atlasHeight() const { return m_atlasHeight; }

private:
    BitmapFont() = default;

    FontMetrics m_metrics;
    std::unordered_map<char32_t, BitmapGlyph> m_glyphs;

    // Atlas data (DirectDraw surface or raw pixels)
    void* m_atlasData = nullptr;
    i32 m_atlasWidth = 0;
    i32 m_atlasHeight = 0;
};

} // namespace Helbreath::Text
```

---

### Task 6.5: Create Text Renderer

**TextRenderer.h:**
```cpp
#pragma once

#include "FontManager.h"
#include <string_view>

namespace Helbreath::Text {

/**
 * Text alignment options.
 */
enum class TextAlign : u8 {
    Left,
    Center,
    Right
};

enum class TextVAlign : u8 {
    Top,
    Middle,
    Bottom
};

/**
 * Text rendering options.
 */
struct TextStyle {
    u32 color = 0xFFFFFF;       // RGB color
    u32 shadowColor = 0x000000; // Shadow color (0 = no shadow)
    i8 shadowOffsetX = 1;
    i8 shadowOffsetY = 1;
    u32 outlineColor = 0;       // Outline color (0 = no outline)
    TextAlign align = TextAlign::Left;
    TextVAlign valign = TextVAlign::Top;
};

/**
 * High-level text rendering interface.
 */
class TextRenderer {
public:
    static TextRenderer& instance();

    // Initialize with DirectDraw
    void initialize(void* directDraw);

    // Simple text rendering
    void drawText(
        void* surface,
        i32 x, i32 y,
        std::string_view text,
        FontHandle font,
        u32 color = 0xFFFFFF);

    // Styled text rendering
    void drawText(
        void* surface,
        i32 x, i32 y,
        std::string_view text,
        FontHandle font,
        const TextStyle& style);

    // Text in rectangle with alignment
    void drawTextInRect(
        void* surface,
        const Rect& rect,
        std::string_view text,
        FontHandle font,
        const TextStyle& style);

    // Word-wrapped text
    void drawWrappedText(
        void* surface,
        const Rect& rect,
        std::string_view text,
        FontHandle font,
        const TextStyle& style);

    // Measurement (for layout)
    [[nodiscard]] i32 measureWidth(std::string_view text, FontHandle font) const;
    [[nodiscard]] i32 measureHeight(std::string_view text, FontHandle font, i32 maxWidth) const;

private:
    TextRenderer() = default;

    void* m_directDraw = nullptr;
};

} // namespace Helbreath::Text
```

**TextRenderer.cpp (Key Parts):**
```cpp
void TextRenderer::drawText(
    void* surface,
    i32 x, i32 y,
    std::string_view text,
    FontHandle fontHandle,
    const TextStyle& style) {

    IFont* font = FontManager::instance().font(fontHandle);
    if (!font) {
        LOG_WARN("Invalid font handle for text rendering");
        return;
    }

    // Draw shadow first if enabled
    if (style.shadowColor != 0) {
        font->renderText(
            surface,
            x + style.shadowOffsetX,
            y + style.shadowOffsetY,
            text,
            style.shadowColor);
    }

    // Draw outline if enabled
    if (style.outlineColor != 0) {
        const i8 offsets[][2] = {{-1,0}, {1,0}, {0,-1}, {0,1}};
        for (auto& [ox, oy] : offsets) {
            font->renderText(surface, x + ox, y + oy, text, style.outlineColor);
        }
    }

    // Draw main text
    font->renderText(surface, x, y, text, style.color);
}

void TextRenderer::drawTextInRect(
    void* surface,
    const Rect& rect,
    std::string_view text,
    FontHandle fontHandle,
    const TextStyle& style) {

    IFont* font = FontManager::instance().font(fontHandle);
    if (!font) return;

    i32 textWidth = font->measureWidth(text);
    i32 textHeight = font->metrics().height;

    // Calculate position based on alignment
    i32 x = rect.left;
    i32 y = rect.top;

    switch (style.align) {
        case TextAlign::Center:
            x = rect.left + (rect.width() - textWidth) / 2;
            break;
        case TextAlign::Right:
            x = rect.right - textWidth;
            break;
        default:
            break;
    }

    switch (style.valign) {
        case TextVAlign::Middle:
            y = rect.top + (rect.height() - textHeight) / 2;
            break;
        case TextVAlign::Bottom:
            y = rect.bottom - textHeight;
            break;
        default:
            break;
    }

    drawText(surface, x, y, text, fontHandle, style);
}
```

---

### Task 6.6: Create Text Layout Helper

**layout/TextLayout.h:**
```cpp
#pragma once

#include "../Font.h"
#include <string>
#include <vector>

namespace Helbreath::Text {

/**
 * Represents a line of laid-out text.
 */
struct TextLine {
    std::string text;
    i32 width;
    i32 x, y;  // Computed position
};

/**
 * Handles text measurement, wrapping, and layout.
 */
class TextLayout {
public:
    explicit TextLayout(IFont* font);

    // Set constraints
    void setMaxWidth(i32 maxWidth);
    void setLineSpacing(f32 spacing);

    // Layout text
    void layout(std::string_view text);

    // Results
    [[nodiscard]] const std::vector<TextLine>& lines() const { return m_lines; }
    [[nodiscard]] i32 totalWidth() const { return m_totalWidth; }
    [[nodiscard]] i32 totalHeight() const { return m_totalHeight; }

private:
    void wrapLine(std::string_view line);
    i32 findWrapPoint(std::string_view text, i32 startWidth);

    IFont* m_font;
    i32 m_maxWidth = 0;
    f32 m_lineSpacing = 1.0f;

    std::vector<TextLine> m_lines;
    i32 m_totalWidth = 0;
    i32 m_totalHeight = 0;
};

} // namespace Helbreath::Text
```

---

### Task 6.7: Integrate with Existing Code

**Migration Helper:**
```cpp
// Legacy text rendering replacement
// Before:
void CGame::DrawText(int x, int y, char* text, int color) {
    HDC hdc;
    m_pBackBuffer->GetDC(&hdc);
    SetTextColor(hdc, color);
    TextOut(hdc, x, y, text, strlen(text));
    m_pBackBuffer->ReleaseDC(hdc);
}

// After:
void CGame::DrawText(int x, int y, const char* text, uint32_t color) {
    auto& renderer = Helbreath::Text::TextRenderer::instance();
    renderer.drawText(
        m_pBackBuffer,
        x, y,
        text,
        FontManager::instance().defaultFont(12),
        color);
}
```

---

### Task 6.8: Register Game Fonts

**Font Initialization:**
```cpp
void initializeFonts() {
    auto& fm = FontManager::instance();

    // Register system fonts
    fm.registerSystemFont("default", "Arial");
    fm.registerSystemFont("title", "Times New Roman");
    fm.registerSystemFont("mono", "Courier New");

    // Register bitmap fonts (if using sprite-based)
    fm.registerBitmapFont("game", "data/fonts/game.fnt");
    fm.registerBitmapFont("damage", "data/fonts/damage.fnt");

    // Set default
    fm.setDefaultFont("default");
}
```

---

## Performance Considerations

### Font Caching
- Cache loaded fonts by (name, size, style) key
- Avoid reloading fonts each frame
- Clear cache on resolution change if needed

### Batch Rendering
- Group text draws by font to minimize state changes
- Use sprite batching for bitmap fonts

### GDI Usage
- Minimize GetDC/ReleaseDC calls
- Cache measurement DC
- Consider pre-rendering common strings

---

## Verification

### Visual Tests
- [ ] All game text displays correctly
- [ ] Font sizes match original
- [ ] Colors render correctly
- [ ] Shadows and outlines work

### Performance Tests
- [ ] Text rendering doesn't impact FPS
- [ ] Font loading time acceptable
- [ ] Memory usage reasonable

### Functional Tests
- [ ] All text wrapping works
- [ ] Alignment options work
- [ ] Unicode support (if needed)

---

## Success Criteria

Phase 6 is complete when:

1. **FontManager** handles all font loading/caching
2. **TextRenderer** replaces ad-hoc text drawing
3. **Bitmap fonts** supported for game-specific fonts
4. **Performance** same or better than original
5. **All game text** renders correctly

---

## Project Completion

With Phase 6 complete, the modernization project has achieved:

1. **C++20 codebase** with modern practices
2. **RAII everywhere** - no manual memory management
3. **Clean architecture** - separated concerns
4. **Proper viewport** - resolution independence
5. **Modern networking** - WinSock2, no message pump
6. **UI system** - widget-based
7. **ECS** - entity management
8. **Text system** - proper font management

The game now has a solid foundation for future development while maintaining full backward compatibility with original game behavior.
