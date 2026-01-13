# Phase 3: Split UpdateScreen

## Overview

This phase breaks apart the monolithic `UpdateScreen()` function (and related game loop code) into distinct, manageable subsystems while preserving exact behavior. The goal is separation of concerns without changing functionality.

**Prerequisite:** Phase 2 complete (Viewport system working)

---

## Goals

1. Identify all responsibilities in UpdateScreen
2. Extract input handling into InputSystem
3. Extract UI update logic into UISystem
4. Extract gameplay update into GameplaySystem
5. Extract rendering into RenderSystem
6. Maintain exact call order for behavioral compatibility

---

## Current State Analysis

### Client UpdateScreen() Flow (Approximate)
```
UpdateScreen()
├── Process DirectInput (keyboard/mouse state)
├── Check game mode (menu, loading, game, etc.)
├── Handle mode-specific logic:
│   ├── GAMEMODE_ONMAINMENU
│   │   ├── Update menu animations
│   │   ├── Check button hover/clicks
│   │   └── Draw menu
│   ├── GAMEMODE_ONLOADING
│   │   ├── Update loading progress
│   │   └── Draw loading screen
│   ├── GAMEMODE_ONMAINGAME
│   │   ├── Process player input
│   │   ├── Update camera
│   │   ├── Process network messages
│   │   ├── Update game objects
│   │   ├── Update UI dialogs
│   │   ├── Draw world tiles
│   │   ├── Draw objects/NPCs/players
│   │   ├── Draw effects
│   │   ├── Draw UI
│   │   └── Draw chat/text
│   └── [other modes...]
├── Present back buffer
└── Update frame counter
```

### Server OnTimer() Flow
```
OnTimer()
├── Update game time
├── Process message queue (MsgProcess)
├── NPC processing (NpcProcess)
├── Delayed events (DelayEventProcess)
├── Force recall processing
├── Weather updates
├── Event scheduling
└── Auto-save checks
```

---

## Architecture After Refactor

```
┌─────────────────────────────────────────────────────────┐
│                    Game Frame                            │
├─────────────────────────────────────────────────────────┤
│  1. Input Phase                                          │
│     └── InputSystem::poll()                              │
│         ├── Read keyboard state                          │
│         ├── Read mouse state                             │
│         └── Queue input events                           │
├─────────────────────────────────────────────────────────┤
│  2. Update Phase                                         │
│     ├── UISystem::update()                               │
│     │   ├── Process input for UI                         │
│     │   ├── Update widget states                         │
│     │   └── Handle button clicks                         │
│     └── GameplaySystem::update()                         │
│         ├── Process network messages                     │
│         ├── Update player state                          │
│         ├── Update camera                                │
│         └── Update game objects                          │
├─────────────────────────────────────────────────────────┤
│  3. Render Phase                                         │
│     └── RenderSystem::render()                           │
│         ├── Clear back buffer                            │
│         ├── Draw world                                   │
│         ├── Draw entities                                │
│         ├── Draw effects                                 │
│         ├── Draw UI                                      │
│         └── Draw debug overlays                          │
├─────────────────────────────────────────────────────────┤
│  4. Present Phase                                        │
│     └── RenderSystem::present()                          │
│         └── Flip/blit to screen                          │
└─────────────────────────────────────────────────────────┘
```

---

## Directory Structure

```
/Sources/Client/
├── engine/
│   ├── input/
│   │   ├── InputSystem.h
│   │   ├── InputSystem.cpp
│   │   ├── InputState.h
│   │   └── InputEvents.h
│   └── render_dd7/
│       ├── RenderSystem.h
│       └── RenderSystem.cpp
├── game/
│   ├── systems/
│   │   ├── UISystem.h
│   │   ├── UISystem.cpp
│   │   ├── GameplaySystem.h
│   │   └── GameplaySystem.cpp
│   └── screens/
│       ├── IScreen.h
│       ├── ScreenManager.h
│       ├── MainMenuScreen.h
│       ├── LoadingScreen.h
│       └── GameScreen.h
```

---

## Tasks

### Task 3.1: Create Input System

**Files to Create:**
```
/Sources/Client/engine/input/InputSystem.h
/Sources/Client/engine/input/InputState.h
/Sources/Client/engine/input/InputEvents.h
```

**InputState.h:**
```cpp
#pragma once

#include <Dependencies/Shared/Types.h>
#include <array>
#include <bitset>

namespace Helbreath::Input {

// Key codes (matching Windows virtual key codes for compatibility)
enum class Key : u8 {
    Unknown = 0,
    Escape = 0x1B,
    Enter = 0x0D,
    Space = 0x20,
    Left = 0x25,
    Up = 0x26,
    Right = 0x27,
    Down = 0x28,
    // ... all keys
    F1 = 0x70,
    F12 = 0x7B,
    // Total 256 possible keys
};

enum class MouseButton : u8 {
    Left = 0,
    Right = 1,
    Middle = 2,
    X1 = 3,
    X2 = 4,
    Count = 5
};

struct MouseState {
    i32 x = 0;              // Logical coordinates
    i32 y = 0;
    i32 deltaX = 0;         // Movement since last frame
    i32 deltaY = 0;
    i32 wheelDelta = 0;     // Scroll wheel
    std::bitset<5> buttons; // Button states
};

struct KeyboardState {
    std::bitset<256> keys;  // Key states (pressed = 1)
    std::bitset<256> previousKeys; // Last frame's state

    [[nodiscard]] bool isKeyDown(Key key) const {
        return keys[static_cast<u8>(key)];
    }

    [[nodiscard]] bool isKeyUp(Key key) const {
        return !keys[static_cast<u8>(key)];
    }

    [[nodiscard]] bool wasKeyPressed(Key key) const {
        auto k = static_cast<u8>(key);
        return keys[k] && !previousKeys[k];
    }

    [[nodiscard]] bool wasKeyReleased(Key key) const {
        auto k = static_cast<u8>(key);
        return !keys[k] && previousKeys[k];
    }
};

struct InputState {
    KeyboardState keyboard;
    MouseState mouse;
};

} // namespace Helbreath::Input
```

**InputSystem.h:**
```cpp
#pragma once

#include "InputState.h"

namespace Helbreath::Input {

class InputSystem {
public:
    static InputSystem& instance();

    // Call at start of frame to poll hardware
    void poll();

    // Access current state
    [[nodiscard]] const InputState& state() const noexcept { return m_state; }
    [[nodiscard]] const KeyboardState& keyboard() const noexcept { return m_state.keyboard; }
    [[nodiscard]] const MouseState& mouse() const noexcept { return m_state.mouse; }

    // Convenience methods
    [[nodiscard]] bool isKeyDown(Key key) const {
        return m_state.keyboard.isKeyDown(key);
    }

    [[nodiscard]] bool wasKeyPressed(Key key) const {
        return m_state.keyboard.wasKeyPressed(key);
    }

    [[nodiscard]] bool isMouseButtonDown(MouseButton button) const {
        return m_state.mouse.buttons[static_cast<u8>(button)];
    }

private:
    InputSystem() = default;

    void pollKeyboard();
    void pollMouse();

    InputState m_state;

#ifdef _WIN32
    // DirectInput handles (if using DInput)
    // Or raw Win32 input
#endif
};

} // namespace Helbreath::Input
```

---

### Task 3.2: Create Screen/Scene Interface

**File to Create:**
```
/Sources/Client/game/screens/IScreen.h
```

**IScreen.h:**
```cpp
#pragma once

#include <cstdint>

namespace Helbreath::Game {

// Forward declarations
class ScreenManager;

/**
 * Interface for game screens (menu, loading, game, etc.)
 * Each screen handles its own input, update, and render.
 */
class IScreen {
public:
    virtual ~IScreen() = default;

    // Called when screen becomes active
    virtual void onEnter() = 0;

    // Called when screen is deactivated
    virtual void onExit() = 0;

    // Called when screen is paused (another screen pushed on top)
    virtual void onPause() {}

    // Called when screen is resumed (screen on top was popped)
    virtual void onResume() {}

    // Frame update
    virtual void handleInput() = 0;
    virtual void update(double deltaTime) = 0;
    virtual void render() = 0;

    // Screen identification
    [[nodiscard]] virtual const char* name() const = 0;
};

} // namespace Helbreath::Game
```

---

### Task 3.3: Create Screen Manager

**Files to Create:**
```
/Sources/Client/game/screens/ScreenManager.h
/Sources/Client/game/screens/ScreenManager.cpp
```

**ScreenManager.h:**
```cpp
#pragma once

#include "IScreen.h"
#include <memory>
#include <vector>
#include <string>

namespace Helbreath::Game {

/**
 * Manages screen stack and transitions.
 * Supports push/pop for modal screens and set for direct transitions.
 */
class ScreenManager {
public:
    static ScreenManager& instance();

    // Screen transitions
    void setScreen(std::unique_ptr<IScreen> screen);
    void pushScreen(std::unique_ptr<IScreen> screen);
    void popScreen();

    // Register screens by name for easy switching
    void registerScreen(const std::string& name, std::unique_ptr<IScreen> screen);
    void setScreen(const std::string& name);

    // Frame processing
    void handleInput();
    void update(double deltaTime);
    void render();

    // Accessors
    [[nodiscard]] IScreen* currentScreen() const;
    [[nodiscard]] bool hasScreen() const { return !m_screenStack.empty(); }

private:
    ScreenManager() = default;

    std::vector<std::unique_ptr<IScreen>> m_screenStack;
    std::unordered_map<std::string, std::unique_ptr<IScreen>> m_registeredScreens;
};

} // namespace Helbreath::Game
```

---

### Task 3.4: Create Legacy Screen Adapter

**File to Create:**
```
/Sources/Client/game/screens/LegacyGameScreen.h
```

**LegacyGameScreen.h:**
```cpp
#pragma once

#include "IScreen.h"
#include "../../Game.h"  // Legacy CGame

namespace Helbreath::Game {

/**
 * Wraps the legacy CGame rendering for the main game screen.
 * This preserves exact behavior while fitting into the new architecture.
 */
class LegacyGameScreen : public IScreen {
public:
    explicit LegacyGameScreen(CGame* legacyGame);

    void onEnter() override;
    void onExit() override;

    void handleInput() override;
    void update(double deltaTime) override;
    void render() override;

    [[nodiscard]] const char* name() const override { return "LegacyGameScreen"; }

private:
    CGame* m_legacyGame;  // Non-owning pointer to legacy game

    // Extract specific parts of UpdateScreen
    void processLegacyInput();
    void updateLegacyGame(double deltaTime);
    void renderLegacyGame();
};

} // namespace Helbreath::Game
```

**Implementation Pattern:**
```cpp
void LegacyGameScreen::handleInput() {
    // For now, input is still handled by legacy code
    // This method exists for future extraction
    processLegacyInput();
}

void LegacyGameScreen::update(double deltaTime) {
    // Legacy code uses fixed timestep internally
    // Just call the relevant update portions
    updateLegacyGame(deltaTime);
}

void LegacyGameScreen::render() {
    // Call legacy rendering
    // This is the bulk of UpdateScreen's rendering code
    renderLegacyGame();
}

void LegacyGameScreen::renderLegacyGame() {
    // Extract the rendering portion of UpdateScreen
    // Initially this may just call into CGame methods
    // Over time, individual pieces get extracted

    // Example: m_legacyGame->DrawWorld();
    // Example: m_legacyGame->DrawEntities();
    // Example: m_legacyGame->DrawUI();
}
```

---

### Task 3.5: Extract Gameplay System (Client)

**Files to Create:**
```
/Sources/Client/game/systems/GameplaySystem.h
/Sources/Client/game/systems/GameplaySystem.cpp
```

**GameplaySystem.h:**
```cpp
#pragma once

namespace Helbreath::Game {

class CGame;  // Forward declaration of legacy class

/**
 * Handles gameplay logic update separate from rendering.
 * This wraps the non-rendering parts of legacy game update.
 */
class GameplaySystem {
public:
    explicit GameplaySystem(CGame* legacyGame);

    // Called each frame
    void update(double deltaTime);

private:
    CGame* m_legacyGame;

    // Individual update components (to be extracted over time)
    void processNetworkMessages();
    void updatePlayer(double deltaTime);
    void updateCamera();
    void updateGameObjects(double deltaTime);
    void updateEffects(double deltaTime);
};

} // namespace Helbreath::Game
```

---

### Task 3.6: Extract Render System

**Files to Create:**
```
/Sources/Client/engine/render_dd7/RenderSystem.h
/Sources/Client/engine/render_dd7/RenderSystem.cpp
```

**RenderSystem.h:**
```cpp
#pragma once

#include "Viewport.h"

#ifdef _WIN32
#include <ddraw.h>
#endif

namespace Helbreath::Render {

/**
 * Manages DirectDraw7 rendering pipeline.
 */
class RenderSystem {
public:
    static RenderSystem& instance();

    // Initialization
    [[nodiscard]] bool initialize(HWND hwnd);
    void shutdown();

    // Frame rendering
    void beginFrame();
    void endFrame();
    void present();

    // Clear operations
    void clearBackBuffer(uint32_t color = 0);
    void clearLetterbox();

    // Surface access for legacy code
    [[nodiscard]] IDirectDrawSurface7* backBuffer() const { return m_backBuffer; }
    [[nodiscard]] IDirectDrawSurface7* primarySurface() const { return m_primary; }

    // State
    [[nodiscard]] bool isInitialized() const noexcept { return m_initialized; }

private:
    RenderSystem() = default;
    ~RenderSystem();

    bool m_initialized = false;

#ifdef _WIN32
    IDirectDraw7* m_directDraw = nullptr;
    IDirectDrawSurface7* m_primary = nullptr;
    IDirectDrawSurface7* m_backBuffer = nullptr;  // Logical resolution
    IDirectDrawClipper* m_clipper = nullptr;
#endif
};

} // namespace Helbreath::Render
```

---

### Task 3.7: Update Game Loop Integration

**Modify LegacyGameAdapter:**
```cpp
void LegacyGameAdapter::onUpdate(double deltaTime) {
    // New structured approach:

    // 1. Input (handled by Engine before this call)

    // 2. Update via screen manager
    ScreenManager::instance().handleInput();
    ScreenManager::instance().update(deltaTime);
}

void LegacyGameAdapter::onRender() {
    // 3. Render via screen manager
    auto& render = Render::RenderSystem::instance();

    render.beginFrame();
    render.clearBackBuffer();

    ScreenManager::instance().render();

    render.endFrame();
    render.present();
}
```

---

## Extraction Strategy

### Step-by-Step Extraction

1. **First Pass:** Create empty systems that just call legacy code
2. **Second Pass:** Move code from legacy methods into system methods
3. **Third Pass:** Refactor moved code to use new infrastructure
4. **Verification:** After each step, verify identical behavior

### Code Movement Example

**Before (in CGame):**
```cpp
void CGame::UpdateScreen() {
    // ... 1000+ lines of mixed code
    DrawMap();
    DrawNPCs();
    DrawPlayers();
    DrawEffects();
    DrawUI();
    // ...
}
```

**After (distributed):**
```cpp
// In RenderSystem
void WorldRenderer::render() {
    DrawMap();  // Moved from CGame
}

void EntityRenderer::render() {
    DrawNPCs();   // Moved from CGame
    DrawPlayers(); // Moved from CGame
}

void EffectRenderer::render() {
    DrawEffects(); // Moved from CGame
}

// In UISystem
void UISystem::render() {
    DrawUI();  // Moved from CGame
}
```

---

## Verification

### Behavioral Tests
- [ ] Game mode transitions work identically
- [ ] All UI elements functional
- [ ] Combat and skills work correctly
- [ ] NPCs behave normally
- [ ] Network synchronization unchanged

### Performance Tests
- [ ] Frame rate same or better
- [ ] No additional memory allocation per frame
- [ ] CPU usage unchanged

### Visual Tests
- [ ] Rendering output identical (compare screenshots)
- [ ] No flicker or artifacts
- [ ] Animation timing unchanged

---

## Success Criteria

Phase 3 is complete when:

1. **UpdateScreen is decomposed** into Input, Update, Render phases
2. **Screen manager** handles different game modes
3. **Systems are separated** but behavior identical
4. **Legacy code wrapped** in new structure
5. **No regressions** in any functionality

---

## Next Phase

Once Phase 3 is complete, proceed to:
- **Phase 4: Networking Upgrade**

With clean separation of systems, networking can be upgraded without affecting rendering or gameplay code.
