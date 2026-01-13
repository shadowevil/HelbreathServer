# Phase 6: Text + Fonts - Micro-Phases

## Overview
Phase 6 is broken into 14 micro-phases, implementing proper font management and text rendering.

---

## Micro-Phase 6.1: Create Font Metrics Types

**Goal:** Define font measurement structures

**Changes:**
1. Create `Sources/Client/engine/text/FontMetrics.h`
2. Define GlyphMetrics struct (width, height, bearing, advance)
3. Define FontMetrics struct (height, ascent, descent, lineGap)
4. Define FontStyle enum (Normal, Bold, Italic, etc.)

**Files:**
- `Sources/Client/engine/text/FontMetrics.h` (new)

**Verification:**
- [ ] Header compiles
- [ ] All metrics types defined

**Commit Message:** `feat(text): add FontMetrics types`

---

## Micro-Phase 6.2: Create Font Interface

**Goal:** Abstract font rendering

**Changes:**
1. Create `Sources/Client/engine/text/Font.h`
2. Define FontHandle struct
3. Define IFont abstract class
4. Declare metrics(), getGlyphMetrics()
5. Declare measureWidth(), renderText()

**Files:**
- `Sources/Client/engine/text/Font.h` (new)

**Verification:**
- [ ] Interface compiles
- [ ] All methods declared

**Commit Message:** `feat(text): add IFont interface`

---

## Micro-Phase 6.3: Create FontManager Header

**Goal:** Font registration and caching interface

**Changes:**
1. Create `Sources/Client/engine/text/FontManager.h`
2. Define FontRegistration struct
3. Define FontCacheKey struct and hash
4. Declare singleton FontManager class
5. Declare register, getFont, font access methods

**Files:**
- `Sources/Client/engine/text/FontManager.h` (new)

**Verification:**
- [ ] Header compiles
- [ ] Cache key hashing works

**Commit Message:** `feat(text): add FontManager header`

---

## Micro-Phase 6.4: Implement FontManager Registration

**Goal:** Font registration system

**Changes:**
1. Create `Sources/Client/engine/text/FontManager.cpp`
2. Implement singleton instance
3. Implement registerFont, registerSystemFont, registerBitmapFont
4. Store registrations in map
5. Implement hasFont check

**Files:**
- `Sources/Client/engine/text/FontManager.cpp` (new)

**Verification:**
- [ ] Fonts can be registered
- [ ] hasFont returns correct result

**Commit Message:** `feat(text): implement FontManager registration`

---

## Micro-Phase 6.5: Implement FontManager Caching

**Goal:** Lazy-load font caching

**Changes:**
1. Implement getFont with cache lookup
2. Implement font loading on cache miss
3. Store loaded fonts in cache by key
4. Implement handle mapping
5. Implement clearCache

**Files:**
- `Sources/Client/engine/text/FontManager.cpp` (modify)

**Verification:**
- [ ] First getFont loads font
- [ ] Second getFont returns cached
- [ ] clearCache removes all

**Commit Message:** `feat(text): implement FontManager caching`

---

## Micro-Phase 6.6: Create GDI Font Header

**Goal:** Windows font wrapper

**Changes:**
1. Create `Sources/Client/engine/text/gdi/GDIFont.h`
2. Inherit from IFont
3. Declare Windows-specific members (HFONT, HDC)
4. Use #ifdef _WIN32 guards

**Files:**
- `Sources/Client/engine/text/gdi/GDIFont.h` (new)

**Verification:**
- [ ] Header compiles on Windows
- [ ] Platform guards work

**Commit Message:** `feat(text/gdi): add GDIFont header`

---

## Micro-Phase 6.7: Implement GDI Font Creation

**Goal:** Create Windows fonts

**Changes:**
1. Create `Sources/Client/engine/text/gdi/GDIFont.cpp`
2. Implement constructor with CreateFont
3. Apply style (bold, italic) to font creation
4. Create measurement DC
5. Implement destructor cleanup

**Files:**
- `Sources/Client/engine/text/gdi/GDIFont.cpp` (new)

**Verification:**
- [ ] Font handle created
- [ ] Styles applied correctly
- [ ] Resources cleaned up

**Commit Message:** `feat(text/gdi): implement GDIFont creation`

---

## Micro-Phase 6.8: Implement GDI Font Metrics

**Goal:** Text measurement

**Changes:**
1. Implement calculateMetrics using GetTextMetrics
2. Implement getGlyphMetrics using GetCharWidth
3. Implement measureWidth using GetTextExtentPoint32
4. Cache metrics for performance

**Files:**
- `Sources/Client/engine/text/gdi/GDIFont.cpp` (modify)

**Verification:**
- [ ] Metrics populated correctly
- [ ] Width measurement accurate
- [ ] Glyph metrics work

**Commit Message:** `feat(text/gdi): implement GDIFont metrics`

---

## Micro-Phase 6.9: Implement GDI Font Rendering

**Goal:** Render text to surfaces

**Changes:**
1. Implement renderText method
2. Get DC from DirectDraw surface
3. Set text color and background mode
4. Call TextOut or ExtTextOut
5. Release DC properly

**Files:**
- `Sources/Client/engine/text/gdi/GDIFont.cpp` (modify)

**Verification:**
- [ ] Text renders to surface
- [ ] Color applied correctly
- [ ] DC released

**Commit Message:** `feat(text/gdi): implement GDIFont rendering`

---

## Micro-Phase 6.10: Create Bitmap Font Header

**Goal:** Sprite-based fonts

**Changes:**
1. Create `Sources/Client/engine/text/bitmap/BitmapFont.h`
2. Define BitmapGlyph struct (atlas position, size, offset)
3. Inherit from IFont
4. Declare static load methods (loadFromFile, loadBMFont)

**Files:**
- `Sources/Client/engine/text/bitmap/BitmapFont.h` (new)

**Verification:**
- [ ] Header compiles
- [ ] Glyph structure defined

**Commit Message:** `feat(text/bitmap): add BitmapFont header`

---

## Micro-Phase 6.11: Implement Bitmap Font Loading

**Goal:** Load font from file

**Changes:**
1. Create `Sources/Client/engine/text/bitmap/BitmapFont.cpp`
2. Implement loadFromFile for custom format
3. Parse glyph definitions
4. Load atlas texture
5. Calculate font metrics from glyphs

**Files:**
- `Sources/Client/engine/text/bitmap/BitmapFont.cpp` (new)

**Verification:**
- [ ] File parsing works
- [ ] Glyphs stored correctly
- [ ] Atlas loaded

**Commit Message:** `feat(text/bitmap): implement BitmapFont loading`

---

## Micro-Phase 6.12: Create TextRenderer

**Goal:** High-level text API

**Changes:**
1. Create `Sources/Client/engine/text/TextRenderer.h`
2. Define TextAlign and TextVAlign enums
3. Define TextStyle struct (color, shadow, outline)
4. Declare drawText, drawTextInRect, drawWrappedText
5. Create `Sources/Client/engine/text/TextRenderer.cpp`
6. Implement simple drawText using FontManager

**Files:**
- `Sources/Client/engine/text/TextRenderer.h` (new)
- `Sources/Client/engine/text/TextRenderer.cpp` (new)

**Verification:**
- [ ] Basic text renders
- [ ] FontManager lookup works

**Commit Message:** `feat(text): add TextRenderer`

---

## Micro-Phase 6.13: Implement TextRenderer Effects

**Goal:** Shadow and outline

**Changes:**
1. Implement shadow rendering (offset text)
2. Implement outline rendering (4-direction offset)
3. Implement alignment in drawTextInRect
4. Implement word wrapping in drawWrappedText

**Files:**
- `Sources/Client/engine/text/TextRenderer.cpp` (modify)

**Verification:**
- [ ] Shadows render correctly
- [ ] Outlines visible
- [ ] Alignment works
- [ ] Text wraps at width

**Commit Message:** `feat(text): implement TextRenderer effects`

---

## Micro-Phase 6.14: Integrate with Legacy Code

**Goal:** Replace legacy text rendering

**Changes:**
1. Register game fonts in initialization
2. Modify CGame::DrawText to use TextRenderer
3. Update UI widgets to use new text system
4. Remove direct GDI usage from game code

**Files:**
- `Sources/Client/game/app/LegacyGameAdapter.cpp` (modify)
- `Sources/Client/Game.cpp` (modify if exists)

**Verification:**
- [ ] All text displays correctly
- [ ] No visual changes from original
- [ ] Performance same or better

**Commit Message:** `refactor(text): integrate new text system with legacy code`

---

## Phase 6 Completion Checklist

After all micro-phases:
- [ ] FontManager caches loaded fonts
- [ ] GDI fonts work on Windows
- [ ] Bitmap fonts load and render
- [ ] TextRenderer provides high-level API
- [ ] Shadow and outline effects work
- [ ] Text alignment and wrapping work
- [ ] Legacy code uses new system
- [ ] Performance maintained

---

## Project Completion

With Phase 6 complete, the full modernization is achieved:
- C++20 codebase with modern practices
- RAII everywhere - no manual memory management
- Clean architecture with separated concerns
- Proper viewport with resolution independence
- Modern WinSock2 networking
- Widget-based UI system
- ECS for entity management
- Proper font and text management
