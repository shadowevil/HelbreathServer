# Phase 2: Viewport System

## Overview

This phase implements a robust viewport system that separates logical game coordinates from physical screen pixels. The original game's sprites were authored for 640x480 but the game renders at 800x600, causing viewport/aspect handling issues.

**Prerequisite:** Phase 1 complete (Engine shell working)

---

## Goals

1. Define logical viewport (640x480 game units)
2. Define physical viewport (window/screen pixels)
3. Implement aspect-ratio correct scaling with letterboxing/pillarboxing
4. Create coordinate conversion helpers
5. Fix mouse input mapping
6. Add viewport debug overlay

---

## Current Problems

### Problem 1: Hardcoded Resolutions
```cpp
// Scattered throughout Game.cpp
#define SCREENX 800
#define SCREENY 600
// But sprites designed for 640x480
```

### Problem 2: Mouse Coordinates
```cpp
// Mouse position is in screen pixels
// But hit-testing uses mixed coordinate systems
if (m_stMCursor.sX >= 50 && m_stMCursor.sX <= 150) {
    // Is this in logical or physical coords?
}
```

### Problem 3: UI Positioning
```cpp
// Dialog positions are hardcoded
DrawRect(100, 200, 300, 400);  // Which coordinate system?
```

### Problem 4: World Rendering
```cpp
// Tile rendering uses offsets that assume specific resolution
int screenX = (worldX - cameraX) * 32 + OFFSET_X;
```

---

## Architecture

```
┌─────────────────────────────────────────────────────────┐
│                    Physical Viewport                     │
│              (Window: 800x600, 1920x1080, etc.)         │
│  ┌───────────────────────────────────────────────────┐  │
│  │              Letterbox/Pillarbox Area             │  │
│  │  ┌─────────────────────────────────────────────┐  │  │
│  │  │            Logical Viewport                 │  │  │
│  │  │              (640x480 game units)           │  │  │
│  │  │                                             │  │  │
│  │  │   All game logic operates in this space    │  │  │
│  │  │                                             │  │  │
│  │  └─────────────────────────────────────────────┘  │  │
│  │              Letterbox/Pillarbox Area             │  │
│  └───────────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────────┘
```

---

## Directory Structure

```
/Sources/Client/engine/
├── render_dd7/
│   ├── Viewport.h
│   ├── Viewport.cpp
│   ├── ViewportDebug.h
│   └── ViewportDebug.cpp
```

---

## Tasks

### Task 2.1: Define Viewport Types

**Files to Create:**
```
/Sources/Client/engine/render_dd7/Viewport.h
/Sources/Client/engine/render_dd7/Viewport.cpp
```

**Viewport.h:**
```cpp
#pragma once

#include <Dependencies/Shared/Types.h>

namespace Helbreath::Render {

/**
 * Logical viewport - the coordinate system game logic uses.
 * All game coordinates, UI positions, and sprite placements
 * use this coordinate system (640x480).
 */
struct LogicalViewport {
    static constexpr i32 WIDTH = 640;
    static constexpr i32 HEIGHT = 480;
    static constexpr f32 ASPECT_RATIO = static_cast<f32>(WIDTH) / HEIGHT;  // 1.333...

    i32 width = WIDTH;
    i32 height = HEIGHT;
};

/**
 * Physical viewport - actual screen/window pixels.
 * This changes based on window size and fullscreen mode.
 */
struct PhysicalViewport {
    i32 windowWidth = 800;
    i32 windowHeight = 600;

    // The actual rendering area within the window (may have letterbox)
    i32 renderX = 0;      // Left offset
    i32 renderY = 0;      // Top offset
    i32 renderWidth = 0;  // Scaled width
    i32 renderHeight = 0; // Scaled height

    // Scaling factor from logical to physical
    f32 scaleX = 1.0f;
    f32 scaleY = 1.0f;
    f32 uniformScale = 1.0f;  // Preserves aspect ratio
};

/**
 * Viewport manager handles coordinate conversion and scaling.
 */
class ViewportManager {
public:
    static ViewportManager& instance();

    // Configuration
    void setWindowSize(i32 width, i32 height);
    void setFullscreen(bool fullscreen);

    // Calculate physical viewport based on window size
    void recalculate();

    // Coordinate conversion
    [[nodiscard]] Point2D logicalToPhysical(Point2D logical) const;
    [[nodiscard]] Point2D physicalToLogical(Point2D physical) const;

    // For mouse input (physical screen coords to logical game coords)
    [[nodiscard]] Point2D screenToLogical(i32 screenX, i32 screenY) const;

    // Check if a physical point is within the render area
    [[nodiscard]] bool isInRenderArea(i32 screenX, i32 screenY) const;

    // Rectangle conversion
    [[nodiscard]] Rect logicalToPhysical(Rect logical) const;
    [[nodiscard]] Rect physicalToLogical(Rect physical) const;

    // Accessors
    [[nodiscard]] const LogicalViewport& logical() const noexcept { return m_logical; }
    [[nodiscard]] const PhysicalViewport& physical() const noexcept { return m_physical; }

    // For rendering scaled content
    [[nodiscard]] f32 scale() const noexcept { return m_physical.uniformScale; }

private:
    ViewportManager() = default;

    LogicalViewport m_logical;
    PhysicalViewport m_physical;
};

} // namespace Helbreath::Render
```

**Viewport.cpp:**
```cpp
#include "Viewport.h"
#include <algorithm>
#include <cmath>

namespace Helbreath::Render {

ViewportManager& ViewportManager::instance() {
    static ViewportManager s_instance;
    return s_instance;
}

void ViewportManager::setWindowSize(i32 width, i32 height) {
    m_physical.windowWidth = width;
    m_physical.windowHeight = height;
    recalculate();
}

void ViewportManager::recalculate() {
    const f32 windowAspect = static_cast<f32>(m_physical.windowWidth) /
                             static_cast<f32>(m_physical.windowHeight);

    const f32 logicalAspect = LogicalViewport::ASPECT_RATIO;

    if (windowAspect > logicalAspect) {
        // Window is wider than logical - pillarbox (bars on sides)
        m_physical.renderHeight = m_physical.windowHeight;
        m_physical.renderWidth = static_cast<i32>(
            m_physical.windowHeight * logicalAspect);
        m_physical.renderX = (m_physical.windowWidth - m_physical.renderWidth) / 2;
        m_physical.renderY = 0;
    } else {
        // Window is taller than logical - letterbox (bars on top/bottom)
        m_physical.renderWidth = m_physical.windowWidth;
        m_physical.renderHeight = static_cast<i32>(
            m_physical.windowWidth / logicalAspect);
        m_physical.renderX = 0;
        m_physical.renderY = (m_physical.windowHeight - m_physical.renderHeight) / 2;
    }

    // Calculate scale factors
    m_physical.scaleX = static_cast<f32>(m_physical.renderWidth) /
                        static_cast<f32>(m_logical.width);
    m_physical.scaleY = static_cast<f32>(m_physical.renderHeight) /
                        static_cast<f32>(m_logical.height);
    m_physical.uniformScale = std::min(m_physical.scaleX, m_physical.scaleY);
}

Point2D ViewportManager::logicalToPhysical(Point2D logical) const {
    return {
        static_cast<i16>(m_physical.renderX + logical.x * m_physical.scaleX),
        static_cast<i16>(m_physical.renderY + logical.y * m_physical.scaleY)
    };
}

Point2D ViewportManager::physicalToLogical(Point2D physical) const {
    return {
        static_cast<i16>((physical.x - m_physical.renderX) / m_physical.scaleX),
        static_cast<i16>((physical.y - m_physical.renderY) / m_physical.scaleY)
    };
}

Point2D ViewportManager::screenToLogical(i32 screenX, i32 screenY) const {
    // Clamp to render area first
    i32 clampedX = std::clamp(screenX, m_physical.renderX,
                              m_physical.renderX + m_physical.renderWidth - 1);
    i32 clampedY = std::clamp(screenY, m_physical.renderY,
                              m_physical.renderY + m_physical.renderHeight - 1);

    return physicalToLogical({static_cast<i16>(clampedX),
                              static_cast<i16>(clampedY)});
}

bool ViewportManager::isInRenderArea(i32 screenX, i32 screenY) const {
    return screenX >= m_physical.renderX &&
           screenX < m_physical.renderX + m_physical.renderWidth &&
           screenY >= m_physical.renderY &&
           screenY < m_physical.renderY + m_physical.renderHeight;
}

Rect ViewportManager::logicalToPhysical(Rect logical) const {
    auto topLeft = logicalToPhysical({logical.left, logical.top});
    auto bottomRight = logicalToPhysical({logical.right, logical.bottom});
    return {topLeft.x, topLeft.y, bottomRight.x, bottomRight.y};
}

Rect ViewportManager::physicalToLogical(Rect physical) const {
    auto topLeft = physicalToLogical({physical.left, physical.top});
    auto bottomRight = physicalToLogical({physical.right, physical.bottom});
    return {topLeft.x, topLeft.y, bottomRight.x, bottomRight.y};
}

} // namespace Helbreath::Render
```

---

### Task 2.2: Create Viewport Debug Overlay

**Files to Create:**
```
/Sources/Client/engine/render_dd7/ViewportDebug.h
/Sources/Client/engine/render_dd7/ViewportDebug.cpp
```

**ViewportDebug.h:**
```cpp
#pragma once

#include "Viewport.h"

namespace Helbreath::Render {

/**
 * Debug overlay to visualize viewport boundaries and scaling.
 * Toggle with a debug key (e.g., F12).
 */
class ViewportDebugOverlay {
public:
    void setEnabled(bool enabled) { m_enabled = enabled; }
    [[nodiscard]] bool isEnabled() const noexcept { return m_enabled; }

    void toggle() { m_enabled = !m_enabled; }

    // Call during render to draw debug info
    void render();

private:
    bool m_enabled = false;

    // Draw helpers (use DirectDraw or GDI)
    void drawRenderAreaBorder();
    void drawSafeAreaGuides();
    void drawScaleInfo();
    void drawMousePosition();
    void drawCoordinateGrid();
};

} // namespace Helbreath::Render
```

**Debug Overlay Features:**
- Red border around render area
- Green safe area guides (90% of logical viewport)
- Scale factor text display
- Mouse position in both coordinate systems
- Optional grid showing logical units

---

### Task 2.3: Update Input Handling

**Modify:** LegacyGameAdapter (or create InputAdapter)

```cpp
void LegacyGameAdapter::onMouseMove(int32_t screenX, int32_t screenY) {
    auto& viewport = Render::ViewportManager::instance();

    // Convert screen coordinates to logical coordinates
    auto logical = viewport.screenToLogical(screenX, screenY);

    // Update legacy mouse cursor position in logical coords
    if (m_legacyGame) {
        // Legacy code expects coordinates in 640x480 space
        m_legacyGame->m_stMCursor.sX = logical.x;
        m_legacyGame->m_stMCursor.sY = logical.y;
    }
}

void LegacyGameAdapter::onMouseButton(int32_t button, bool down) {
    // Mouse position should already be in logical coords from onMouseMove
    if (m_legacyGame) {
        if (down) {
            m_legacyGame->OnLButtonDown();  // or OnRButtonDown
        } else {
            m_legacyGame->OnLButtonUp();
        }
    }
}
```

---

### Task 2.4: Create DirectDraw Viewport Integration

**File to Create:**
```
/Sources/Client/engine/render_dd7/DD7Viewport.h
/Sources/Client/engine/render_dd7/DD7Viewport.cpp
```

**DD7Viewport.h:**
```cpp
#pragma once

#include "Viewport.h"

#ifdef _WIN32
#include <ddraw.h>
#endif

namespace Helbreath::Render {

/**
 * DirectDraw 7 viewport integration.
 * Handles surface creation and scaling for the viewport system.
 */
class DD7Viewport {
public:
    // Create surfaces matching the logical viewport size
    [[nodiscard]] bool createLogicalBackBuffer(IDirectDraw7* dd);

    // Scale and present the logical back buffer to the physical viewport
    void present(IDirectDrawSurface7* primary, IDirectDrawSurface7* logicalBack);

    // Clear letterbox/pillarbox areas
    void clearBorders(IDirectDrawSurface7* primary);

private:
#ifdef _WIN32
    // Logical back buffer (640x480)
    IDirectDrawSurface7* m_logicalBack = nullptr;

    // Helper for scaled blit
    void scaledBlit(IDirectDrawSurface7* dest, IDirectDrawSurface7* src,
                    const RECT& destRect, const RECT& srcRect);
#endif
};

} // namespace Helbreath::Render
```

**Scaling Strategy:**
```cpp
void DD7Viewport::present(IDirectDrawSurface7* primary,
                          IDirectDrawSurface7* logicalBack) {
    auto& vp = ViewportManager::instance();
    const auto& phys = vp.physical();

    // Source: entire logical back buffer
    RECT srcRect = {0, 0, LogicalViewport::WIDTH, LogicalViewport::HEIGHT};

    // Dest: scaled render area within physical viewport
    RECT destRect = {
        phys.renderX,
        phys.renderY,
        phys.renderX + phys.renderWidth,
        phys.renderY + phys.renderHeight
    };

    // DirectDraw will scale during Blt if DDBLT_WAIT is used
    // For better quality, use DDBLTFX with stretch options
    DDBLTFX fx = {};
    fx.dwSize = sizeof(fx);

    primary->Blt(&destRect, logicalBack, &srcRect, DDBLT_WAIT, &fx);
}
```

---

### Task 2.5: Update Rendering to Use Logical Coordinates

**Strategy:** All existing rendering code continues to use 640x480 coordinates. The viewport system handles scaling during present.

**Minimal Changes to CGame:**
```cpp
// Before: rendering directly to screen surface (800x600)
// After: rendering to logical back buffer (640x480)

// The only change needed is ensuring sprites and UI
// are drawn to a 640x480 surface, not the screen directly

// Example modification in CGame::UpdateScreen or render methods:
void CGame::DrawSprite(int logicalX, int logicalY, Sprite* sprite) {
    // logicalX, logicalY are in 640x480 space
    // Draw to the logical back buffer, not primary surface
    m_logicalBackBuffer->BltFast(logicalX, logicalY, sprite->surface, ...);
}
```

---

### Task 2.6: Handle Window Resize

**Add to Engine:**
```cpp
void Engine::onWindowResize(int32_t width, int32_t height) {
    // Update viewport calculations
    Render::ViewportManager::instance().setWindowSize(width, height);

    // Notify game for any game-specific handling
    if (m_app) {
        m_app->onResize(width, height);
    }

    // DirectDraw may need surface recreation for fullscreen
    // Handle in DD7 subsystem
}
```

---

### Task 2.7: Fullscreen Toggle Support

**Add to ViewportManager:**
```cpp
void ViewportManager::setFullscreen(bool fullscreen) {
    if (fullscreen) {
        // Get screen resolution
        m_physical.windowWidth = GetSystemMetrics(SM_CXSCREEN);
        m_physical.windowHeight = GetSystemMetrics(SM_CYSCREEN);
    }
    // recalculate() will handle the aspect ratio correctly
    recalculate();
}
```

---

## Testing Strategy

### Visual Tests
- [ ] Render area is centered correctly
- [ ] Letterboxing appears for wide windows
- [ ] Pillarboxing appears for tall windows
- [ ] Content is not stretched (maintains aspect ratio)
- [ ] Debug overlay shows correct values

### Input Tests
- [ ] Mouse clicks register at correct game positions
- [ ] Click on edge of render area works correctly
- [ ] Click in letterbox/pillarbox area is handled gracefully
- [ ] UI elements respond correctly regardless of resolution

### Scaling Tests
- [ ] 800x600 window (original size)
- [ ] 1280x720 window (16:9)
- [ ] 1920x1080 fullscreen
- [ ] 1024x768 window (4:3)
- [ ] Non-standard sizes

---

## Verification Checklist

### Viewport Calculation
- [ ] Aspect ratio preserved at all resolutions
- [ ] Letterbox/pillarbox calculated correctly
- [ ] Scale factor accurate

### Coordinate Conversion
- [ ] logicalToPhysical works correctly
- [ ] physicalToLogical works correctly
- [ ] screenToLogical handles edge cases
- [ ] Roundtrip conversion is accurate

### Rendering
- [ ] Game renders at logical resolution
- [ ] Scaling to physical resolution works
- [ ] No visual artifacts or stretching
- [ ] Letterbox/pillarbox areas are black

### Input
- [ ] Mouse position correctly converted
- [ ] All UI elements clickable
- [ ] World interaction works correctly

---

## Success Criteria

Phase 2 is complete when:

1. **Viewport system** correctly calculates scaling for any window size
2. **Coordinate conversion** is accurate and bidirectional
3. **Input mapping** works correctly at all resolutions
4. **Visual output** maintains correct aspect ratio
5. **Debug overlay** helps verify viewport correctness
6. **Game behavior** unchanged at original 800x600 resolution

---

## Migration Notes

### Coordinate System Summary
| System | Resolution | Used For |
|--------|------------|----------|
| Logical | 640x480 | Game logic, sprites, UI |
| Physical | Variable | Window pixels, mouse input |
| World | Tile-based | Game world positions |

### Conversion Points
1. Mouse input: Physical → Logical (in input handler)
2. Rendering: Logical → Physical (in present)
3. UI hit-testing: Already in Logical (no change needed)
4. World rendering: World → Logical (existing camera system)

---

## Next Phase

Once Phase 2 is complete, proceed to:
- **Phase 3: Split UpdateScreen**

With the viewport system in place, we can safely refactor the monolithic UpdateScreen function knowing coordinates will be handled correctly.
