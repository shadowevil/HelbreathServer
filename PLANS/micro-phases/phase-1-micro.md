# Phase 1: Engine Shell + Loop Wrapper - Micro-Phases

## Overview
Phase 1 is broken into 14 micro-phases, building the engine architecture incrementally.

---

## Micro-Phase 1.1: Create Clock Header

**Goal:** Define timing interface

**Changes:**
1. Create `Dependencies/Shared/core/Clock.h`
2. Define Clock class interface
3. Use std::chrono types
4. Define deltaTime and totalTime methods

**Files:**
- `Dependencies/Shared/core/Clock.h` (new)

**Verification:**
- [ ] Header compiles
- [ ] Clock class can be instantiated

**Commit Message:** `feat(core): add Clock class header for frame timing`

---

## Micro-Phase 1.2: Implement Clock

**Goal:** Working frame timer

**Changes:**
1. Create `Dependencies/Shared/core/Clock.cpp`
2. Implement tick() method
3. Implement delta time calculation
4. Implement FPS averaging

**Files:**
- `Dependencies/Shared/core/Clock.cpp` (new)
- `Dependencies/Shared/Shared.vcxitems` (modify)

**Verification:**
- [ ] Clock.tick() updates delta time
- [ ] deltaTimeSeconds() returns ~0.016 at 60fps
- [ ] fps() returns smoothed value

**Commit Message:** `feat(core): implement Clock with delta time and FPS`

---

## Micro-Phase 1.3: Create IGameApp Interface (Client)

**Goal:** Define game application contract

**Changes:**
1. Create `Sources/Client/engine/core/IGameApp.h`
2. Define lifecycle methods (onInit, onShutdown)
3. Define frame methods (onUpdate, onRender)
4. Define input methods (onKeyDown, onKeyUp, etc.)

**Files:**
- `Sources/Client/engine/core/IGameApp.h` (new)

**Verification:**
- [ ] Interface compiles
- [ ] All methods are pure virtual

**Commit Message:** `feat(client/engine): add IGameApp interface`

---

## Micro-Phase 1.4: Create Engine Header (Client)

**Goal:** Define engine interface

**Changes:**
1. Create `Sources/Client/engine/core/Engine.h`
2. Define singleton pattern
3. Define initialize/shutdown methods
4. Define run method taking IGameApp
5. Include Windows handle storage

**Files:**
- `Sources/Client/engine/core/Engine.h` (new)

**Verification:**
- [ ] Header compiles
- [ ] Engine::instance() accessible

**Commit Message:** `feat(client/engine): add Engine class header`

---

## Micro-Phase 1.5: Implement Engine - Window Creation

**Goal:** Engine creates window

**Changes:**
1. Create `Sources/Client/engine/core/Engine.cpp`
2. Implement initialize() - window creation only
3. Implement shutdown() - window destruction
4. Store HWND and HINSTANCE

**Files:**
- `Sources/Client/engine/core/Engine.cpp` (new)

**Verification:**
- [ ] Engine::initialize() creates window
- [ ] Window appears on screen
- [ ] Engine::shutdown() destroys window

**Commit Message:** `feat(client/engine): implement Engine window creation`

---

## Micro-Phase 1.6: Implement Engine - Message Pump

**Goal:** Engine processes Windows messages

**Changes:**
1. Add processMessages() private method
2. Implement PeekMessage loop
3. Handle WM_QUIT
4. Route WM_KEYDOWN/UP to future handlers

**Files:**
- `Sources/Client/engine/core/Engine.cpp` (modify)

**Verification:**
- [ ] Window responds to close button
- [ ] No hang on WM_QUIT
- [ ] Messages processed without blocking

**Commit Message:** `feat(client/engine): implement Engine message pump`

---

## Micro-Phase 1.7: Implement Engine - Main Loop

**Goal:** Engine runs game loop

**Changes:**
1. Implement run(IGameApp&) method
2. Call Clock::tick() each frame
3. Call app.onUpdate(deltaTime)
4. Call app.onRender()
5. Process messages between frames

**Files:**
- `Sources/Client/engine/core/Engine.cpp` (modify)

**Verification:**
- [ ] Loop runs continuously
- [ ] onUpdate called each frame
- [ ] onRender called each frame
- [ ] requestQuit() stops loop

**Commit Message:** `feat(client/engine): implement Engine main loop`

---

## Micro-Phase 1.8: Create LegacyGlobals Bridge

**Goal:** Provide access to legacy globals

**Changes:**
1. Create `Sources/Client/game/LegacyGlobals.h`
2. Create `Sources/Client/game/LegacyGlobals.cpp`
3. Declare extern for G_hWnd, G_pGame
4. Provide getter/setter functions

**Files:**
- `Sources/Client/game/LegacyGlobals.h` (new)
- `Sources/Client/game/LegacyGlobals.cpp` (new)

**Verification:**
- [ ] G_hWnd accessible from legacy code
- [ ] No linker errors

**Commit Message:** `refactor(client): add LegacyGlobals bridge for transition`

---

## Micro-Phase 1.9: Create LegacyGameAdapter Header

**Goal:** Define adapter interface

**Changes:**
1. Create `Sources/Client/game/app/LegacyGameAdapter.h`
2. Inherit from IGameApp
3. Hold pointer to CGame
4. Declare all interface methods

**Files:**
- `Sources/Client/game/app/LegacyGameAdapter.h` (new)

**Verification:**
- [ ] Header compiles
- [ ] Adapter satisfies IGameApp interface

**Commit Message:** `feat(client/game): add LegacyGameAdapter header`

---

## Micro-Phase 1.10: Implement LegacyGameAdapter - Lifecycle

**Goal:** Adapter initializes legacy game

**Changes:**
1. Create `Sources/Client/game/app/LegacyGameAdapter.cpp`
2. Implement onInit() - create CGame, set globals
3. Implement onShutdown() - destroy CGame
4. Do NOT call any update/render yet

**Files:**
- `Sources/Client/game/app/LegacyGameAdapter.cpp` (new)

**Verification:**
- [ ] onInit creates CGame
- [ ] G_pGame points to instance
- [ ] onShutdown cleans up

**Commit Message:** `feat(client/game): implement LegacyGameAdapter lifecycle`

---

## Micro-Phase 1.11: Implement LegacyGameAdapter - Input

**Goal:** Adapter forwards input to legacy

**Changes:**
1. Implement onKeyDown() - call CGame::OnKeyDown
2. Implement onKeyUp() - call CGame::OnKeyUp
3. Implement onMouseMove() - update m_stMCursor
4. Implement onMouseButton() - call legacy handlers

**Files:**
- `Sources/Client/game/app/LegacyGameAdapter.cpp` (modify)

**Verification:**
- [ ] Key presses reach CGame
- [ ] Mouse position updates
- [ ] Mouse clicks register

**Commit Message:** `feat(client/game): implement LegacyGameAdapter input forwarding`

---

## Micro-Phase 1.12: Implement LegacyGameAdapter - Update/Render

**Goal:** Adapter calls legacy update

**Changes:**
1. Implement onUpdate() - accumulate time, call OnTimer
2. Implement onRender() - placeholder for now
3. Match legacy timer interval (~33ms)

**Files:**
- `Sources/Client/game/app/LegacyGameAdapter.cpp` (modify)

**Verification:**
- [ ] OnTimer called at correct rate
- [ ] Game updates normally
- [ ] FPS matches original

**Commit Message:** `feat(client/game): implement LegacyGameAdapter update loop`

---

## Micro-Phase 1.13: Create New Entry Point

**Goal:** New WinMain using Engine

**Changes:**
1. Create `Sources/Client/engine/platform/win32/Win32Main.cpp`
2. Initialize logging
3. Initialize Engine
4. Create LegacyGameAdapter
5. Call Engine::run()
6. Shutdown and return

**Files:**
- `Sources/Client/engine/platform/win32/Win32Main.cpp` (new)

**Verification:**
- [ ] Compiles without errors
- [ ] NOT linked yet (old entry still active)

**Commit Message:** `feat(client/platform): add new Win32 entry point`

---

## Micro-Phase 1.14: Switch to New Entry Point

**Goal:** Game runs through Engine

**Changes:**
1. Remove old Wmain.cpp from build (don't delete)
2. Add Win32Main.cpp to build
3. Update any initialization differences
4. Verify identical behavior

**Files:**
- `Sources/Client/Client.vcxproj` (modify)

**Verification:**
- [ ] Game starts normally
- [ ] Main menu appears
- [ ] Input works
- [ ] Game plays identically

**Commit Message:** `refactor(client): switch to Engine-based entry point`

---

## Phase 1 Completion Checklist

After all micro-phases:
- [ ] Engine manages window lifecycle
- [ ] Engine runs main loop
- [ ] LegacyGameAdapter wraps CGame
- [ ] All input forwarded correctly
- [ ] Timer fires at correct rate
- [ ] Game behavior unchanged
- [ ] Old entry point preserved (not deleted)
- [ ] Ready for Phase 2
