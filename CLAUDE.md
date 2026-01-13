# Claude Code Prompt — Modernize Legacy Windows MMORPG (C/C++) While Preserving Logic (C++20, RAII, Strings)

You are acting as a senior game-engine + network engineer modernizing a legacy Windows MMORPG codebase written in old C/C++. The current project is bug-riddled, tightly coupled, and uses obsolete project configurations.

## Non-Negotiable Constraints
- **Platform:** Windows only (for now), but **code must clearly separate platform-specific pieces**.
- **Architecture/Target:** **x86** (32-bit) only.
- **Language Standard:** **C++20** throughout touched code (client + server + shared).
- **Rendering API:** **DirectDraw 7** must remain (no D3D/OpenGL migration in this phase).
- **Behavior:** Preserve *functional gameplay logic* and *rendering results* as closely as possible while improving correctness, maintainability, and performance.
- **Build System:** Existing project files/configurations are invalid; **recreate project files** cleanly with modern, valid settings (Visual Studio recommended).
- **No "rewrite from scratch."** This is an incremental modernization with continuous verification.

## Platform Separation Rules (Strict)
- **All Windows-specific APIs/usages must be wrapped** in:
  - `#ifdef _WIN32` / `#endif`
- Prefer a small, isolated `/engine/platform/win32/` layer and keep the rest platform-agnostic.
- Any file that includes Windows headers should be clearly in a platform folder or explicitly gated by `_WIN32`.
- Use **portable types** (`std::uint32_t`, `std::size_t`, etc.).
  - Avoid Win32 typedefs like `DWORD`, `WORD`, `BYTE`, `LPARAM`, etc. **except** when required for Windows API calls or structs.
  - Convert immediately at the boundary layer: Windows types in → portable types out.

## Modern C++ Requirements (Strict)
- Use **RAII everywhere**:
  - No manual `new/delete` in modernized code.
  - No "naked" `Release()` scattered around: wrap COM-style resources (DirectDraw interfaces, surfaces) in RAII holders.
- Use modern ownership:
  - `std::unique_ptr` by default.
  - `std::shared_ptr` only for shared ownership with clear justification.
  - Prefer value types and references where safe.
- Eliminate C string usage:
  - Replace `char*`, `TCHAR*`, raw buffers with `std::string`, `std::string_view`, `std::vector<std::byte>`.
  - Use `std::span` and `std::string_view` for non-owning views.
  - Avoid unsafe functions (`strcpy`, `sprintf`, etc.). Use `std::format` (C++20) or safe alternatives.
- Prefer `enum class`, `constexpr`, `noexcept`, `[[nodiscard]]`, `std::optional`, `std::variant`, and strong typing.

## Current System Facts (Assume True)
- Client uses **DirectDraw7** and sprites authored for **640×480**.
- Game currently renders at **800×600**; viewport/aspect handling is incorrect and **viewport is tightly coupled to logic**.
- Networking uses **XSocket** (legacy wrapper over old WinSock) and relies heavily on **Windows Message Pump** patterns.
- Main loop is a monolithic **`UpdateScreen()`** that handles input, UI, dialogs, gameplay update, and rendering.
- `Game.cpp` is extremely large and contains intertwined systems.

## High-Level Objectives
1. **Stabilize and modernize the codebase** to professional standards (C++20 + RAII) without changing gameplay behavior.
2. **Decouple engine vs. game logic**:
   - Engine: windowing, DirectDraw device/surfaces, input, timing, text/font system, resource management.
   - Game: rules, entities, UI screens, networking protocol handling, simulation.
3. Introduce a **professional update/render pipeline** replacing the monolithic `UpdateScreen()` while preserving logic.
4. Replace XSocket with **WinSock2** (incrementally) and remove dependence on Window Message Pump logic for networking.
5. Create clean modular systems:
   - **UI system**
   - **Entity/Component (ECS-lite is fine)**
   - **Networking message system**
   - **Text rendering + font management**, including lazy-load caching
   - **Bitmap font module**
6. Improve performance, reduce unnecessary network messages, and keep the server **console-only x86**.

---

## Project Directory Structure

```
HelbreathServer/
├───Binaries
│   ├───Game              # Client executable output
│   └───Server            # Server executable output
├───Debug                 # Debug build artifacts
├───Dependencies
│   ├───Client
│   │   ├───Includes      # Client-specific headers (DirectDraw, DirectInput, etc.)
│   │   └───Libs          # Client-specific libraries
│   ├───Server
│   │   ├───Includes      # Server-specific headers
│   │   └───Libs          # Server-specific libraries
│   └───Shared            # Shared headers/code between client and server
├───PLANS                 # Architecture plans and documentation
├───Release               # Release build artifacts
└───Sources
    ├───Client
    │   ├───Debug
    │   └───Release
    └───Server
        ├───Debug
        └───Release
```

---

## Deliverables (You Must Produce)
### A) Build + Project Modernization
- Recreate solution/projects with correct modern settings for **Windows x86** and **C++20**:
  - consistent runtime library selection
  - warnings enabled (high warning level); treat warnings as errors where feasible
  - clear Debug/Release configs
- Provide a `BUILDING.md` describing toolchain, configs, and how to run client/server.

### B) Refactored Architecture (Folder + Module Layout)
Implement a clean structure similar to:

```
/engine
  /core        (time, logging, config, utilities; portable)
  /platform
    /win32      (Win32 windowing + message handling; all Windows headers here)
  /render_dd7   (DDraw init, surfaces, blitting, present, viewport; Windows-gated)
  /input        (DirectInput modernization; Windows-gated)
  /assets       (resource manager, paths, caching; portable)
  /text         (font manager, bitmap fonts, text draw; mostly portable, DD7 backend gated)
/game
  /app         (main, screen manager)
  /ui          (widgets, layouts, screens)
  /ecs         (entities, components, systems)
  /net         (protocol, message definitions, serializer)
  /logic       (game rules, state machines)
/server
  /net         (WinSock2 server; Windows-gated)
  /logic       (game server rules, validation)
  /main        (console entry)
/shared
  /net         (shared message IDs, structs, serialization rules; portable types)
  /math
  /util
/tests
/tools
```

### C) Update/Render Pipeline
Replace `UpdateScreen()` with:
- `Engine::Run()` with a structured loop:
  - `PollInput()`
  - `Update(dt)`
  - `Render()`
  - `Present()`
- Game side uses a **Screen/Scene manager**:
  - `IScreen::HandleInput()`
  - `IScreen::Update(dt)`
  - `IScreen::Render(renderer)`
- Preserve legacy flow by **wrapping** old logic first, then splitting responsibilities progressively.

### D) Viewport & Resolution Correctness (Critical)
Implement a robust viewport system:
- Sprites authored at **640×480** (logical resolution).
- Output supports windowed/fullscreen at various sizes.
- Must support **aspect-ratio correct scaling** with letterboxing/pillarboxing as needed.
- Make viewport/coordinate dependency explicit:
  - separate **LogicalViewport (game units)** from **PhysicalViewport (window pixels)**.
  - conversion helpers: `ScreenToWorld`, `WorldToScreen`, `ClipRect`, etc.
- Fix the 800×600 behavior by choosing a canonical logical mapping (prefer 640×480) and mapping to physical resolutions without logic drift.
- Provide a `ViewportDebugOverlay` for verification (safe area, bounds, scale factor).

### E) Networking Modernization
- Replace XSocket incrementally with **WinSock2**:
  - client: non-blocking socket + event-driven IO **without relying on Window Message Pump**
  - server: **console-only x86** WinSock2 server
- Introduce a **message system**:
  - stable message IDs
  - binary serialization/deserialization (bounds checked)
  - validation, rate limiting, coalescing to reduce traffic
- Provide metrics:
  - messages per second
  - average payload size
  - dropped/invalid messages
- Keep protocol behavior compatible unless explicitly documented.

### F) UI System
- Build a proper UI framework:
  - widgets: `Button`, `Label`, `Panel`, `List`, `Textbox` (as needed)
  - focus/navigation, hit-testing
  - decoupled from rendering details (UI emits draw commands)

### G) ECS / Entity System
- ECS-lite:
  - `EntityId`
  - Components (as needed): `Transform`, `Sprite`, `Animation`, `Collider`, etc.
  - Systems: `MovementSystem`, `RenderSystem`, `AnimationSystem`
- Preserve behavior by bridging from legacy entity structures first.

### H) Text Rendering + Font Management
Implement:
- `FontManager`:
  - lazy registration of fonts by file path or system name
  - first use loads and caches by (font-id, size)
  - subsequent use returns cached handles
- `TextRenderer` suitable for DirectDraw7 usage
- Bitmap fonts isolated into `/engine/text/bitmap/` with a clean API

---

## Refactor Strategy (Incremental With Safety Rails)
### Phase 0 — Baseline + Guardrails
1. Make the project build on Windows x86 with recreated project files (C++20).
2. Add logging + assertions (minimal) and crash diagnostics.
3. Add baseline capture:
   - record frame stats (resolution, viewport params, key state variables)
   - optional screenshot dumps (if feasible)
4. Document:
   - where viewport affects logic (mouse hit tests, world coords, UI coords, etc.)
   - all network message types and directions

### Phase 1 — Engine Shell + Loop Wrapper
1. Introduce `Engine` and `GameApp` scaffolding.
2. Wrap legacy `UpdateScreen()` in an adapter:
   - `LegacyUpdateAdapter::UpdateAndRender()`
3. Keep behavior matching baseline.

### Phase 2 — Viewport System (Before Major Decoupling)
1. Implement logical vs physical viewport + aspect-correct scaling.
2. Update input mapping and UI hit tests using conversions.
3. Add windowed/fullscreen toggle and scalable window sizes with aspect lock.

### Phase 3 — Split UpdateScreen Responsibilities
Break `UpdateScreen()` into:
- input
- UI update
- gameplay update
- render prep
- render
Move each into dedicated modules while preserving call order.

### Phase 4 — Networking Upgrade
1. Introduce `NetClient`/`NetServer` behind interfaces; WinSock2 implementation gated by `_WIN32`.
2. Keep protocol structures but improve:
   - validation, buffering, dispatch
3. Reduce traffic:
   - coalesce repeated updates
   - tick-rate control
   - avoid sending unchanged data

### Phase 5 — UI + ECS Migration
1. Create UI primitives; migrate one screen at a time.
2. Introduce ECS-lite; migrate entities gradually.

### Phase 6 — Text + Fonts
1. Implement `FontManager` caching + bitmap font module.
2. Replace ad-hoc text rendering with `TextRenderer`.

---

## Coding Standards (Apply Everywhere You Touch)
- Naming:
  - Types: `PascalCase`
  - Functions/methods: `camelCase`
  - Members: `m_member` (or another consistent convention across the repo)
  - Replace abbreviations (e.g., `btnL` → `mouseLeftButton`)
- RAII wrappers for all lifetime-managed resources (DirectDraw objects, sockets, file handles).
- No C-style strings; use `std::string`/`std::string_view`/`std::span`.
- Portable integer types (`std::uint32_t`, etc.); Windows typedefs only at boundaries.
- Keep functions small; avoid circular dependencies; maintain clear module boundaries.

---

## Testing & Verification Requirements
- Automated checks:
  - message serialization round-trip tests
  - viewport conversion tests
  - font cache behavior tests
- Runtime debug overlays (toggleable):
  - viewport bounds
  - FPS/frame time
  - network stats
- Every phase must build and run; do not allow long-lived broken states.

---

## What You Should Output As You Work
For each significant change, provide:
1. What changed (files/modules)
2. Why
3. How verified (baseline/tests/overlays)
4. Next step (small, incremental)

---

## Immediate Tasks To Start With (Do These First)
1. Recreate x86 Visual Studio solution/projects with C++20 and valid configs.
2. Implement Engine shell + loop, initially calling legacy `UpdateScreen()` through an adapter.
3. Implement viewport system (logical 640×480 mapping) and correct scaling/aspect.
4. Begin isolating DirectDraw7 initialization and surfaces into `/engine/render_dd7/` (Windows-gated).

Begin now. Preserve behavior. Use incremental commits and keep the game running.

---

## Shared Dependencies Strategy

The `Dependencies/Shared` folder contains code that **must be kept in sync** between client and server:

### Network Protocol (`Dependencies/Shared/net/`)
- `NetMessages.h` - Message IDs and types (shared between client/server)
- `MessageTypes.h` - Enum definitions for all message categories
- `Serialization.h` - Binary serialization/deserialization utilities
- `PacketStructs.h` - Packet structure definitions

### Game Constants (`Dependencies/Shared/game/`)
- `GlobalDef.h` - Game-wide constants and limits
- `ItemDef.h` - Item type definitions and constants
- `SkillDef.h` - Skill IDs and constants
- `MagicDef.h` - Magic/spell definitions
- `ActionID.h` - Action identifiers

### Utilities (`Dependencies/Shared/util/`)
- `Types.h` - Portable type definitions
- `Math.h` - Shared math utilities
- `StrTok.h` - String tokenization utilities

### Synchronization Rules
1. **Any change to shared files requires updating BOTH client and server projects**
2. **Version numbers must be incremented in `NetMessages.h` when protocol changes**
3. **All shared code must be platform-agnostic (no Windows types)**
4. **Use `#pragma once` for all shared headers**

---

## Legacy Code Reference

### Original Server Files (HGServer/)
The original server implementation with the following key files:
- `Game.cpp/h` - Main game logic (57K+ lines)
- `Client.cpp/h` - Player client management
- `Npc.cpp/h` - NPC AI and behavior
- `Map.cpp/h` - Map management
- `XSocket.cpp/h` - Legacy networking

### Original Client Files (Client/)
The original client implementation with DirectDraw7 rendering.

### Configuration Files (Files/)
- `GServer.cfg` - Server configuration
- `GMaps.cfg` - Map definitions
- `GameConfigs/` - Item, NPC, Magic, Skill definitions
