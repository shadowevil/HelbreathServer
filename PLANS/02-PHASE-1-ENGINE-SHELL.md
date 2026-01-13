# Phase 1: Engine Shell + Loop Wrapper

## Overview

This phase introduces the foundational Engine and GameApp architecture while preserving existing behavior through adapter patterns. The goal is to establish a professional game loop structure without breaking any functionality.

**Prerequisite:** Phase 0 complete (builds working, logging functional)

---

## Goals

1. Create Engine class with proper initialization/shutdown lifecycle
2. Create GameApp interface for game-specific logic
3. Wrap legacy code in adapters for incremental migration
4. Establish proper frame timing
5. Separate platform code into dedicated modules

---

## Architecture Overview

```
┌─────────────────────────────────────────────────────────┐
│                    WinMain (Entry Point)                 │
└─────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────┐
│                    Engine (Singleton)                    │
│  ┌─────────────┐  ┌──────────────┐  ┌───────────────┐  │
│  │   Window    │  │    Timer     │  │   Subsystems  │  │
│  │  Platform   │  │   (Clock)    │  │   (Future)    │  │
│  └─────────────┘  └──────────────┘  └───────────────┘  │
└─────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────┐
│                    IGameApp (Interface)                  │
│  ┌─────────────┐  ┌──────────────┐  ┌───────────────┐  │
│  │  onInit()   │  │  onUpdate()  │  │  onRender()   │  │
│  │onShutdown() │  │ onKeyDown()  │  │  onPresent()  │  │
│  └─────────────┘  └──────────────┘  └───────────────┘  │
└─────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────┐
│              LegacyGameAdapter (Wraps CGame)             │
│  ┌─────────────────────────────────────────────────┐   │
│  │   Forwards calls to existing CGame methods       │   │
│  │   Preserves exact behavior during transition     │   │
│  └─────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────┘
```

---

## Directory Structure

```
/Sources/Client/
├── engine/
│   ├── core/
│   │   ├── Engine.h
│   │   ├── Engine.cpp
│   │   ├── Clock.h
│   │   ├── Clock.cpp
│   │   └── IGameApp.h
│   └── platform/
│       └── win32/
│           ├── Win32Window.h
│           ├── Win32Window.cpp
│           ├── Win32Main.cpp      # New entry point
│           └── Win32MessagePump.h
├── game/
│   └── app/
│       ├── LegacyGameAdapter.h
│       └── LegacyGameAdapter.cpp
└── [existing files...]

/Sources/Server/
├── engine/
│   ├── core/
│   │   ├── ServerEngine.h
│   │   ├── ServerEngine.cpp
│   │   ├── Clock.h              # Shared with client
│   │   └── IServerApp.h
│   └── platform/
│       └── win32/
│           ├── Win32Console.h
│           ├── Win32Console.cpp
│           └── Win32ServerMain.cpp
├── game/
│   └── app/
│       ├── LegacyServerAdapter.h
│       └── LegacyServerAdapter.cpp
└── [existing files...]
```

---

## Tasks

### Task 1.1: Create Clock/Timer System

**Files to Create:**
```
/Dependencies/Shared/core/Clock.h
/Dependencies/Shared/core/Clock.cpp
```

**Clock.h:**
```cpp
#pragma once

#include "../Types.h"
#include <chrono>

namespace Helbreath::Core {

class Clock {
public:
    using TimePoint = std::chrono::steady_clock::time_point;
    using Duration = std::chrono::steady_clock::duration;

    Clock();

    // Call at start of each frame
    void tick();

    // Time since last tick (frame delta)
    [[nodiscard]] f64 deltaTimeSeconds() const noexcept;
    [[nodiscard]] f64 deltaTimeMs() const noexcept;

    // Time since clock creation
    [[nodiscard]] f64 totalTimeSeconds() const noexcept;

    // Frame counting
    [[nodiscard]] u64 frameCount() const noexcept { return m_frameCount; }

    // FPS calculation (smoothed)
    [[nodiscard]] f64 fps() const noexcept;

    // Fixed timestep helpers
    [[nodiscard]] f64 accumulator() const noexcept { return m_accumulator; }
    void consumeAccumulator(f64 dt) { m_accumulator -= dt; }

private:
    TimePoint m_startTime;
    TimePoint m_lastTickTime;
    TimePoint m_currentTime;

    f64 m_deltaTime = 0.0;
    f64 m_accumulator = 0.0;

    u64 m_frameCount = 0;

    // FPS smoothing
    static constexpr usize FPS_SAMPLE_COUNT = 60;
    std::array<f64, FPS_SAMPLE_COUNT> m_frameTimes{};
    usize m_frameTimeIndex = 0;
};

} // namespace Helbreath::Core
```

**Verification:**
- [ ] Delta time is accurate (compare with system timer)
- [ ] FPS calculation is stable
- [ ] No drift over extended runtime

---

### Task 1.2: Create IGameApp Interface

**File to Create:**
```
/Sources/Client/engine/core/IGameApp.h
```

**IGameApp.h:**
```cpp
#pragma once

#include <cstdint>

namespace Helbreath::Engine {

// Forward declarations
class Engine;

/**
 * Interface for game application implementations.
 * The Engine calls these methods in a structured order.
 */
class IGameApp {
public:
    virtual ~IGameApp() = default;

    // Lifecycle
    [[nodiscard]] virtual bool onInit(Engine& engine) = 0;
    virtual void onShutdown() = 0;

    // Frame update (called every frame)
    virtual void onUpdate(double deltaTime) = 0;

    // Rendering (called every frame after update)
    virtual void onRender() = 0;

    // Input events
    virtual void onKeyDown(uint32_t keyCode) = 0;
    virtual void onKeyUp(uint32_t keyCode) = 0;
    virtual void onMouseMove(int32_t x, int32_t y) = 0;
    virtual void onMouseButton(int32_t button, bool down) = 0;

    // Window events
    virtual void onActivate(bool active) = 0;
    virtual void onResize(int32_t width, int32_t height) = 0;

    // Close request (return true to allow close)
    [[nodiscard]] virtual bool onCloseRequest() = 0;
};

} // namespace Helbreath::Engine
```

---

### Task 1.3: Create Engine Class (Client)

**Files to Create:**
```
/Sources/Client/engine/core/Engine.h
/Sources/Client/engine/core/Engine.cpp
```

**Engine.h:**
```cpp
#pragma once

#include "IGameApp.h"
#include "Clock.h"
#include <memory>
#include <string>

#ifdef _WIN32
#include <windows.h>
#endif

namespace Helbreath::Engine {

struct EngineConfig {
    std::string windowTitle = "Helbreath";
    int32_t windowWidth = 800;
    int32_t windowHeight = 600;
    bool fullscreen = false;
    bool vsync = true;
};

class Engine {
public:
    static Engine& instance();

    // Initialization
    [[nodiscard]] bool initialize(const EngineConfig& config);
    void shutdown();

    // Main loop
    void run(IGameApp& app);
    void requestQuit();

    // Accessors
    [[nodiscard]] bool isRunning() const noexcept { return m_running; }
    [[nodiscard]] const Core::Clock& clock() const noexcept { return m_clock; }
    [[nodiscard]] const EngineConfig& config() const noexcept { return m_config; }

#ifdef _WIN32
    [[nodiscard]] HWND windowHandle() const noexcept { return m_hwnd; }
    [[nodiscard]] HINSTANCE instanceHandle() const noexcept { return m_hinstance; }
#endif

private:
    Engine() = default;
    ~Engine() = default;

    Engine(const Engine&) = delete;
    Engine& operator=(const Engine&) = delete;

    // Platform-specific
    [[nodiscard]] bool createWindow();
    void destroyWindow();
    void processMessages();

    EngineConfig m_config;
    Core::Clock m_clock;
    bool m_running = false;
    IGameApp* m_app = nullptr;

#ifdef _WIN32
    HWND m_hwnd = nullptr;
    HINSTANCE m_hinstance = nullptr;
    static LRESULT CALLBACK windowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
#endif
};

} // namespace Helbreath::Engine
```

**Engine.cpp (Key Parts):**
```cpp
#include "Engine.h"
#include <Dependencies/Shared/core/Log.h>

namespace Helbreath::Engine {

Engine& Engine::instance() {
    static Engine s_instance;
    return s_instance;
}

bool Engine::initialize(const EngineConfig& config) {
    m_config = config;

    LOG_INFO("Engine initializing...");

    if (!createWindow()) {
        LOG_ERROR("Failed to create window");
        return false;
    }

    LOG_INFO("Engine initialized successfully");
    return true;
}

void Engine::run(IGameApp& app) {
    m_app = &app;
    m_running = true;

    if (!app.onInit(*this)) {
        LOG_ERROR("Game initialization failed");
        m_running = false;
        return;
    }

    LOG_INFO("Entering main loop");

    while (m_running) {
        m_clock.tick();

        // Process Windows messages
        processMessages();

        if (!m_running) break;

        // Update game
        app.onUpdate(m_clock.deltaTimeSeconds());

        // Render
        app.onRender();
    }

    LOG_INFO("Exiting main loop");
    app.onShutdown();
    m_app = nullptr;
}

void Engine::requestQuit() {
    m_running = false;
}

#ifdef _WIN32
void Engine::processMessages() {
    MSG msg;
    while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
        if (msg.message == WM_QUIT) {
            m_running = false;
            return;
        }
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}
#endif

} // namespace Helbreath::Engine
```

---

### Task 1.4: Create LegacyGameAdapter (Client)

**Files to Create:**
```
/Sources/Client/game/app/LegacyGameAdapter.h
/Sources/Client/game/app/LegacyGameAdapter.cpp
```

**LegacyGameAdapter.h:**
```cpp
#pragma once

#include "../../engine/core/IGameApp.h"
#include "../../Game.h"  // Legacy CGame

namespace Helbreath::Game {

/**
 * Adapter that wraps the legacy CGame class to work with
 * the new Engine interface. This preserves exact behavior
 * while enabling incremental migration.
 */
class LegacyGameAdapter : public Engine::IGameApp {
public:
    LegacyGameAdapter();
    ~LegacyGameAdapter() override;

    // IGameApp implementation
    [[nodiscard]] bool onInit(Engine::Engine& engine) override;
    void onShutdown() override;
    void onUpdate(double deltaTime) override;
    void onRender() override;
    void onKeyDown(uint32_t keyCode) override;
    void onKeyUp(uint32_t keyCode) override;
    void onMouseMove(int32_t x, int32_t y) override;
    void onMouseButton(int32_t button, bool down) override;
    void onActivate(bool active) override;
    void onResize(int32_t width, int32_t height) override;
    [[nodiscard]] bool onCloseRequest() override;

private:
    // Legacy game instance
    std::unique_ptr<CGame> m_legacyGame;

    // Timer simulation for legacy code
    void simulateLegacyTimer();

    // Track if we need to call UpdateScreen
    bool m_needsScreenUpdate = false;
};

} // namespace Helbreath::Game
```

**LegacyGameAdapter.cpp (Key Parts):**
```cpp
#include "LegacyGameAdapter.h"
#include "../../engine/core/Engine.h"
#include <Dependencies/Shared/core/Log.h>

// Legacy globals that need to be available
extern HWND G_hWnd;
extern class CGame* G_pGame;

namespace Helbreath::Game {

LegacyGameAdapter::LegacyGameAdapter() = default;
LegacyGameAdapter::~LegacyGameAdapter() = default;

bool LegacyGameAdapter::onInit(Engine::Engine& engine) {
    LOG_INFO("LegacyGameAdapter initializing...");

    // Set legacy globals
    G_hWnd = engine.windowHandle();

    // Create legacy game instance
    m_legacyGame = std::make_unique<CGame>();
    G_pGame = m_legacyGame.get();

    // Call legacy initialization
    // Note: Some init happens in CGame constructor, some in bInit()
    // We need to match the original sequence

    LOG_INFO("LegacyGameAdapter initialized");
    return true;
}

void LegacyGameAdapter::onShutdown() {
    LOG_INFO("LegacyGameAdapter shutting down...");

    if (m_legacyGame) {
        // Call legacy cleanup
        m_legacyGame.reset();
        G_pGame = nullptr;
    }
}

void LegacyGameAdapter::onUpdate(double deltaTime) {
    // Legacy code uses a timer at fixed intervals
    // We simulate this by accumulating time
    static double accumulatedTime = 0.0;
    accumulatedTime += deltaTime;

    // Legacy timer fires roughly every 33ms (30fps target)
    constexpr double LEGACY_TIMER_INTERVAL = 0.033;

    while (accumulatedTime >= LEGACY_TIMER_INTERVAL) {
        simulateLegacyTimer();
        accumulatedTime -= LEGACY_TIMER_INTERVAL;
    }
}

void LegacyGameAdapter::simulateLegacyTimer() {
    if (m_legacyGame) {
        // This is what the legacy timer callback does
        m_legacyGame->OnTimer();
    }
}

void LegacyGameAdapter::onRender() {
    // Legacy rendering happens inside OnTimer/UpdateScreen
    // The adapter's onUpdate already handles this via simulateLegacyTimer
    // This method exists for future separation of update/render
}

void LegacyGameAdapter::onKeyDown(uint32_t keyCode) {
    if (m_legacyGame) {
        m_legacyGame->OnKeyDown(static_cast<WPARAM>(keyCode));
    }
}

void LegacyGameAdapter::onKeyUp(uint32_t keyCode) {
    if (m_legacyGame) {
        m_legacyGame->OnKeyUp(static_cast<WPARAM>(keyCode));
    }
}

// ... other methods forward to legacy CGame ...

bool LegacyGameAdapter::onCloseRequest() {
    // Match legacy close behavior
    if (m_legacyGame) {
        // Check game mode, handle logout countdown, etc.
        // Return false to prevent immediate close if needed
    }
    return true;
}

} // namespace Helbreath::Game
```

---

### Task 1.5: Create New Entry Point (Client)

**File to Create:**
```
/Sources/Client/engine/platform/win32/Win32Main.cpp
```

**Win32Main.cpp:**
```cpp
#include "../../core/Engine.h"
#include "../../../game/app/LegacyGameAdapter.h"
#include <Dependencies/Shared/core/Log.h>

#ifdef _WIN32
#include <windows.h>

int APIENTRY WinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR lpCmdLine,
    int nCmdShow)
{
    // Initialize logging
    Helbreath::Core::Logger::instance().setOutputFile("client.log");
    Helbreath::Core::Logger::instance().setLevel(Helbreath::Core::LogLevel::Debug);

    LOG_INFO("=== Helbreath Client Starting ===");

    // Configure engine
    Helbreath::Engine::EngineConfig config;
    config.windowTitle = "Helbreath";
    config.windowWidth = 800;
    config.windowHeight = 600;
    config.fullscreen = false;

    // Initialize engine
    auto& engine = Helbreath::Engine::Engine::instance();
    if (!engine.initialize(config)) {
        LOG_FATAL("Engine initialization failed");
        return 1;
    }

    // Create and run game
    Helbreath::Game::LegacyGameAdapter game;
    engine.run(game);

    // Cleanup
    engine.shutdown();

    LOG_INFO("=== Helbreath Client Exiting ===");
    return 0;
}

#endif // _WIN32
```

---

### Task 1.6: Create Server Engine and Adapter

Similar structure for server, but simpler (no rendering):

**ServerEngine Key Differences:**
- Console window instead of graphics window
- No DirectDraw/DirectInput
- Focus on networking and game logic timing

**LegacyServerAdapter Key Methods:**
```cpp
bool onInit(ServerEngine& engine) override {
    // Mirror Initialize() from Wmain.cpp
    // - Initialize Winsock
    // - Create CGame
    // - Set up listening sockets
    // - Start timer
}

void onTick(double deltaTime) override {
    // Called at server tick rate
    // - Process network messages
    // - Run game simulation
    // - NPC processing
}

void onShutdown() override {
    // Mirror OnDestroy() from Wmain.cpp
    // - Save all players
    // - Close sockets
    // - Cleanup
}
```

---

### Task 1.7: Preserve Legacy Globals

**Challenge:** Legacy code uses many globals (G_hWnd, G_pGame, etc.)

**Solution:** Create a LegacyGlobals bridge:

**File to Create:**
```
/Sources/Client/game/LegacyGlobals.h
/Sources/Client/game/LegacyGlobals.cpp
```

```cpp
// LegacyGlobals.h
#pragma once

#ifdef _WIN32
#include <windows.h>
#endif

// Forward declarations
class CGame;

namespace Helbreath::Legacy {

// These provide access to globals for legacy code
// New code should NOT use these - use proper dependency injection

void setWindowHandle(HWND hwnd);
void setGameInstance(CGame* game);

HWND getWindowHandle();
CGame* getGameInstance();

} // namespace Helbreath::Legacy

// Legacy global declarations for backward compatibility
extern HWND G_hWnd;
extern CGame* G_pGame;
```

---

## Testing Strategy

### Unit Tests
- Clock accuracy over 1000 frames
- Engine lifecycle (init → run → shutdown)
- Adapter method forwarding

### Integration Tests
- Full game startup to main menu
- Input forwarding (keyboard, mouse)
- Timer accuracy (compare frame rates)

### Regression Tests
- Compare baseline recordings before/after
- Verify no change in game behavior

---

## Verification Checklist

### Structure
- [ ] Engine class compiles and links
- [ ] IGameApp interface defined
- [ ] LegacyGameAdapter wraps CGame
- [ ] New entry point works

### Behavior
- [ ] Game starts to main menu
- [ ] Input is responsive
- [ ] Timer fires at correct rate
- [ ] No memory leaks on shutdown

### Compatibility
- [ ] Legacy globals still work
- [ ] Existing CGame code unchanged
- [ ] Network functionality preserved

---

## Success Criteria

Phase 1 is complete when:

1. **Engine manages application lifecycle** properly
2. **LegacyGameAdapter** successfully wraps existing CGame
3. **Frame timing** matches original behavior
4. **All input** is forwarded correctly
5. **Game behavior** is identical to pre-Phase 1

---

## Migration Notes

### What Changes
- Entry point moves to new Engine-based main
- CGame is wrapped by adapter instead of called directly
- Timer is managed by Engine clock instead of Win32 multimedia timer

### What Stays the Same
- All CGame internal logic
- All rendering code
- All network handling
- All game rules and behavior

---

## Next Phase

Once Phase 1 is complete, proceed to:
- **Phase 2: Viewport System**

The Engine shell enables proper viewport management and resolution independence.
