# Phase 2: Viewport System - Micro-Phases

## Overview
Phase 2 is broken into 10 micro-phases, implementing resolution independence.

---

## Micro-Phase 2.1: Define Viewport Structs

**Goal:** Basic viewport data structures

**Changes:**
1. Create `Sources/Client/engine/render_dd7/Viewport.h`
2. Define LogicalViewport struct (640x480 constants)
3. Define PhysicalViewport struct (window dimensions)
4. No implementation yet

**Files:**
- `Sources/Client/engine/render_dd7/Viewport.h` (new)

**Verification:**
- [ ] Header compiles
- [ ] LogicalViewport::WIDTH == 640
- [ ] LogicalViewport::HEIGHT == 480

**Commit Message:** `feat(render): add viewport data structures`

---

## Micro-Phase 2.2: Create ViewportManager Singleton

**Goal:** Central viewport management

**Changes:**
1. Add ViewportManager class to Viewport.h
2. Implement singleton pattern
3. Add setWindowSize() method (stores only)
4. Add getters for logical/physical viewports

**Files:**
- `Sources/Client/engine/render_dd7/Viewport.h` (modify)
- `Sources/Client/engine/render_dd7/Viewport.cpp` (new)

**Verification:**
- [ ] ViewportManager::instance() works
- [ ] setWindowSize() stores values
- [ ] Getters return stored values

**Commit Message:** `feat(render): add ViewportManager singleton`

---

## Micro-Phase 2.3: Implement Aspect Ratio Calculation

**Goal:** Calculate scaled render area

**Changes:**
1. Implement recalculate() method
2. Calculate renderX, renderY (letterbox/pillarbox offset)
3. Calculate renderWidth, renderHeight
4. Calculate scale factors

**Files:**
- `Sources/Client/engine/render_dd7/Viewport.cpp` (modify)

**Verification:**
- [ ] 800x600 window: no letterbox (1.333 == 1.333)
- [ ] 1920x1080 window: pillarbox (16:9 > 4:3)
- [ ] 1024x768 window: no letterbox (4:3 == 4:3)
- [ ] 800x800 window: letterbox (1:1 < 4:3)

**Commit Message:** `feat(render): implement aspect ratio scaling`

---

## Micro-Phase 2.4: Implement Coordinate Conversion

**Goal:** Convert between coordinate systems

**Changes:**
1. Implement logicalToPhysical(Point2D)
2. Implement physicalToLogical(Point2D)
3. Implement screenToLogical(x, y) with clamping
4. Implement isInRenderArea(x, y)

**Files:**
- `Sources/Client/engine/render_dd7/Viewport.cpp` (modify)

**Verification:**
- [ ] (0,0) logical maps to (renderX, renderY) physical
- [ ] (640,480) logical maps to render area bottom-right
- [ ] Roundtrip conversion is accurate

**Commit Message:** `feat(render): implement coordinate conversion`

---

## Micro-Phase 2.5: Add Rectangle Conversion

**Goal:** Convert Rect between systems

**Changes:**
1. Implement logicalToPhysical(Rect)
2. Implement physicalToLogical(Rect)
3. Handle edge cases

**Files:**
- `Sources/Client/engine/render_dd7/Viewport.cpp` (modify)

**Verification:**
- [ ] Full screen rect converts correctly
- [ ] Small rect maintains proportions
- [ ] Negative coords handled

**Commit Message:** `feat(render): add rectangle coordinate conversion`

---

## Micro-Phase 2.6: Integrate with Engine

**Goal:** Engine updates viewport on resize

**Changes:**
1. Call ViewportManager::setWindowSize in Engine::initialize
2. Call ViewportManager::recalculate after window creation
3. Handle WM_SIZE to update viewport

**Files:**
- `Sources/Client/engine/core/Engine.cpp` (modify)

**Verification:**
- [ ] Viewport calculated on startup
- [ ] Window resize updates viewport
- [ ] Values accessible from ViewportManager

**Commit Message:** `feat(client/engine): integrate ViewportManager with Engine`

---

## Micro-Phase 2.7: Update Mouse Input Conversion

**Goal:** Mouse coords converted to logical

**Changes:**
1. Modify LegacyGameAdapter::onMouseMove
2. Get raw screen coords
3. Convert using ViewportManager::screenToLogical
4. Pass logical coords to legacy code

**Files:**
- `Sources/Client/game/app/LegacyGameAdapter.cpp` (modify)

**Verification:**
- [ ] Mouse at window center = (320, 240) logical
- [ ] Mouse in letterbox area = clamped to edge
- [ ] Click positions accurate in game

**Commit Message:** `feat(client/game): convert mouse input to logical coords`

---

## Micro-Phase 2.8: Create Debug Overlay Header

**Goal:** Define debug visualization

**Changes:**
1. Create `Sources/Client/engine/render_dd7/ViewportDebug.h`
2. Define ViewportDebugOverlay class
3. Add enable/disable toggle
4. Declare render method

**Files:**
- `Sources/Client/engine/render_dd7/ViewportDebug.h` (new)

**Verification:**
- [ ] Header compiles
- [ ] Overlay can be toggled

**Commit Message:** `feat(render): add ViewportDebugOverlay header`

---

## Micro-Phase 2.9: Implement Debug Overlay

**Goal:** Visualize viewport boundaries

**Changes:**
1. Create `Sources/Client/engine/render_dd7/ViewportDebug.cpp`
2. Draw red border around render area
3. Draw scale factor text
4. Draw mouse position in both coord systems

**Files:**
- `Sources/Client/engine/render_dd7/ViewportDebug.cpp` (new)

**Verification:**
- [ ] Red border visible when enabled
- [ ] Scale factor displayed correctly
- [ ] Mouse coords update in real-time

**Commit Message:** `feat(render): implement ViewportDebugOverlay`

---

## Micro-Phase 2.10: Add Debug Toggle Key

**Goal:** Toggle overlay with F12

**Changes:**
1. Handle F12 key in input processing
2. Toggle ViewportDebugOverlay
3. Don't pass F12 to game (consume it)

**Files:**
- `Sources/Client/game/app/LegacyGameAdapter.cpp` (modify)
- `Sources/Client/engine/core/Engine.cpp` (modify if needed)

**Verification:**
- [ ] F12 toggles overlay
- [ ] Overlay shows/hides correctly
- [ ] F12 doesn't affect game

**Commit Message:** `feat(client): add F12 to toggle viewport debug overlay`

---

## Phase 2 Completion Checklist

After all micro-phases:
- [ ] Viewport calculates correct scaling
- [ ] Aspect ratio preserved at all sizes
- [ ] Mouse input converted correctly
- [ ] Coordinate conversion works both ways
- [ ] Debug overlay available with F12
- [ ] Letterbox/pillarbox calculated
- [ ] Ready for Phase 3
