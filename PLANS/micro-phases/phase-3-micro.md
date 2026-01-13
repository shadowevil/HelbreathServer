# Phase 3: Split UpdateScreen - Micro-Phases

## Overview
Phase 3 is broken into 16 micro-phases, decomposing the monolithic update function.

---

## Micro-Phase 3.1: Create InputState Structure

**Goal:** Define input state data

**Changes:**
1. Create `Sources/Client/engine/input/InputState.h`
2. Define KeyboardState with key bitset
3. Define MouseState with position and buttons
4. Define InputState combining both

**Files:**
- `Sources/Client/engine/input/InputState.h` (new)

**Verification:**
- [ ] Structs compile
- [ ] Can track 256 keys
- [ ] Mouse state complete

**Commit Message:** `feat(input): add InputState data structures`

---

## Micro-Phase 3.2: Create InputSystem Singleton

**Goal:** Centralized input management

**Changes:**
1. Create `Sources/Client/engine/input/InputSystem.h`
2. Define singleton InputSystem
3. Add poll() method signature
4. Add state accessors

**Files:**
- `Sources/Client/engine/input/InputSystem.h` (new)

**Verification:**
- [ ] Header compiles
- [ ] Singleton accessible

**Commit Message:** `feat(input): add InputSystem singleton header`

---

## Micro-Phase 3.3: Implement InputSystem Polling

**Goal:** Read hardware input state

**Changes:**
1. Create `Sources/Client/engine/input/InputSystem.cpp`
2. Implement poll() using GetKeyboardState
3. Implement mouse position from GetCursorPos
4. Store previous state for edge detection

**Files:**
- `Sources/Client/engine/input/InputSystem.cpp` (new)

**Verification:**
- [ ] Key states accurate
- [ ] wasKeyPressed detects edges
- [ ] Mouse position updates

**Commit Message:** `feat(input): implement InputSystem polling`

---

## Micro-Phase 3.4: Integrate InputSystem with Engine

**Goal:** Engine polls input each frame

**Changes:**
1. Call InputSystem::poll() at start of frame
2. Pass input state to game app
3. Convert mouse coords through viewport

**Files:**
- `Sources/Client/engine/core/Engine.cpp` (modify)

**Verification:**
- [ ] Input polled before update
- [ ] State available in onUpdate

**Commit Message:** `feat(client/engine): integrate InputSystem into frame loop`

---

## Micro-Phase 3.5: Create IScreen Interface

**Goal:** Define screen contract

**Changes:**
1. Create `Sources/Client/game/screens/IScreen.h`
2. Define onEnter/onExit lifecycle
3. Define handleInput/update/render frame methods
4. Add name() for identification

**Files:**
- `Sources/Client/game/screens/IScreen.h` (new)

**Verification:**
- [ ] Interface compiles
- [ ] All methods pure virtual

**Commit Message:** `feat(game/screens): add IScreen interface`

---

## Micro-Phase 3.6: Create ScreenManager

**Goal:** Manage screen stack

**Changes:**
1. Create `Sources/Client/game/screens/ScreenManager.h`
2. Create `Sources/Client/game/screens/ScreenManager.cpp`
3. Implement setScreen, pushScreen, popScreen
4. Implement frame dispatch to current screen

**Files:**
- `Sources/Client/game/screens/ScreenManager.h` (new)
- `Sources/Client/game/screens/ScreenManager.cpp` (new)

**Verification:**
- [ ] Can set active screen
- [ ] Screen lifecycle called
- [ ] Frame methods dispatch

**Commit Message:** `feat(game/screens): add ScreenManager`

---

## Micro-Phase 3.7: Create LegacyScreen Wrapper

**Goal:** Wrap entire legacy game as one screen

**Changes:**
1. Create `Sources/Client/game/screens/LegacyScreen.h`
2. Create `Sources/Client/game/screens/LegacyScreen.cpp`
3. Implement IScreen forwarding to CGame
4. This wraps ALL legacy modes initially

**Files:**
- `Sources/Client/game/screens/LegacyScreen.h` (new)
- `Sources/Client/game/screens/LegacyScreen.cpp` (new)

**Verification:**
- [ ] Screen wraps CGame
- [ ] All modes work through screen
- [ ] Behavior unchanged

**Commit Message:** `feat(game/screens): add LegacyScreen wrapper`

---

## Micro-Phase 3.8: Switch Adapter to Use ScreenManager

**Goal:** Route through screen system

**Changes:**
1. Modify LegacyGameAdapter to use ScreenManager
2. Create LegacyScreen in onInit
3. Dispatch through ScreenManager in onUpdate/onRender

**Files:**
- `Sources/Client/game/app/LegacyGameAdapter.cpp` (modify)

**Verification:**
- [ ] Game routes through ScreenManager
- [ ] Behavior identical
- [ ] All modes work

**Commit Message:** `refactor(client/game): route through ScreenManager`

---

## Micro-Phase 3.9: Create RenderSystem Header

**Goal:** Define render interface

**Changes:**
1. Create `Sources/Client/engine/render_dd7/RenderSystem.h`
2. Define singleton RenderSystem
3. Declare initialize/shutdown
4. Declare beginFrame/endFrame/present

**Files:**
- `Sources/Client/engine/render_dd7/RenderSystem.h` (new)

**Verification:**
- [ ] Header compiles
- [ ] Interface defined

**Commit Message:** `feat(render): add RenderSystem header`

---

## Micro-Phase 3.10: Implement RenderSystem Basics

**Goal:** Basic DD7 management

**Changes:**
1. Create `Sources/Client/engine/render_dd7/RenderSystem.cpp`
2. Implement initialize() - just store DD pointer for now
3. Implement beginFrame/endFrame as no-ops
4. Implement present as no-op

**Files:**
- `Sources/Client/engine/render_dd7/RenderSystem.cpp` (new)

**Verification:**
- [ ] Compiles and links
- [ ] No behavior change yet

**Commit Message:** `feat(render): implement RenderSystem basics`

---

## Micro-Phase 3.11: Extract Input from LegacyScreen

**Goal:** Separate input handling

**Changes:**
1. Create handleInput() method in LegacyScreen
2. Move input processing from update to handleInput
3. Call from ScreenManager in correct order

**Files:**
- `Sources/Client/game/screens/LegacyScreen.cpp` (modify)

**Verification:**
- [ ] Input still works
- [ ] handleInput called before update

**Commit Message:** `refactor(game/screens): extract input handling from update`

---

## Micro-Phase 3.12: Identify Render Boundaries in Legacy

**Goal:** Document where rendering starts/ends

**Changes:**
1. Add comments in CGame identifying render code
2. Document: clear, draw world, draw entities, draw UI, present
3. No code changes, just documentation

**Files:**
- `Sources/Client/Game.cpp` (modify - comments only)
- `PLANS/ANALYSIS/render-boundaries.md` (new)

**Verification:**
- [ ] Render sections identified
- [ ] Documentation complete

**Commit Message:** `docs: identify render boundaries in legacy CGame`

---

## Micro-Phase 3.13: Extract Update from LegacyScreen

**Goal:** Separate game update logic

**Changes:**
1. Ensure update() only does game logic
2. No rendering calls in update
3. Track what needs to be rendered

**Files:**
- `Sources/Client/game/screens/LegacyScreen.cpp` (modify)

**Verification:**
- [ ] Update has no render calls
- [ ] Game state updates correctly

**Commit Message:** `refactor(game/screens): isolate update logic`

---

## Micro-Phase 3.14: Extract Render from LegacyScreen

**Goal:** Separate rendering

**Changes:**
1. Move render calls to render() method
2. Ensure render() only draws, no state changes
3. Call RenderSystem methods (even if no-op)

**Files:**
- `Sources/Client/game/screens/LegacyScreen.cpp` (modify)

**Verification:**
- [ ] Render isolated from update
- [ ] Visual output unchanged

**Commit Message:** `refactor(game/screens): isolate render logic`

---

## Micro-Phase 3.15: Add Frame Timing Debug

**Goal:** Verify correct frame structure

**Changes:**
1. Add timing logs in debug builds
2. Log: input time, update time, render time
3. Verify order: input → update → render → present

**Files:**
- `Sources/Client/engine/core/Engine.cpp` (modify)

**Verification:**
- [ ] Timing shows correct order
- [ ] No unexpected calls

**Commit Message:** `debug(client/engine): add frame timing verification`

---

## Micro-Phase 3.16: Clean Up and Document

**Goal:** Finalize Phase 3

**Changes:**
1. Remove unused code paths
2. Update documentation
3. Verify all tests pass

**Files:**
- Various cleanup
- `PLANS/03-PHASE-3-SPLIT-UPDATESCREEN.md` (update status)

**Verification:**
- [ ] Code clean
- [ ] Documentation current
- [ ] All behavior preserved

**Commit Message:** `refactor(client): finalize Phase 3 cleanup`

---

## Phase 3 Completion Checklist

After all micro-phases:
- [ ] InputSystem polls hardware
- [ ] ScreenManager routes screens
- [ ] LegacyScreen wraps CGame
- [ ] Input/Update/Render separated
- [ ] RenderSystem basics in place
- [ ] Frame order documented
- [ ] Ready for Phase 4
