# Helbreath Modernization Roadmap

## Overview

This document outlines the phased modernization strategy for the Helbreath MMORPG codebase. The goal is to incrementally upgrade the legacy C/C++ code to modern C++20 standards while preserving gameplay behavior.

---

## Phase 0: Baseline + Guardrails (Current)

### Completed
- [x] Directory restructuring
- [x] Shared dependencies setup
- [x] Portable type definitions (`Types.h`)
- [x] Modern message type enums (`MessageTypes.h`)
- [x] Binary serialization utilities (`Serialization.h`)
- [x] CLAUDE.md coding standards

### Remaining
- [ ] Recreate Visual Studio solution/projects with C++20
- [ ] Add logging and assertion framework
- [ ] Document viewport-logic dependencies
- [ ] Create baseline capture system

---

## Phase 1: Engine Shell + Loop Wrapper

### Goals
- Introduce `Engine` and `GameApp` scaffolding
- Wrap legacy `UpdateScreen()` in adapter
- Maintain identical behavior

### Tasks
- [ ] Create `/engine/core/` with Engine class
- [ ] Create `/game/app/` with GameApp class
- [ ] Implement `LegacyUpdateAdapter`
- [ ] Verify baseline match

---

## Phase 2: Viewport System

### Goals
- Implement logical (640x480) vs physical viewport
- Proper aspect-ratio scaling
- Coordinate conversion helpers

### Tasks
- [ ] Create `/engine/render_dd7/Viewport.h`
- [ ] Implement `LogicalViewport` and `PhysicalViewport`
- [ ] Add `ScreenToWorld`, `WorldToScreen` helpers
- [ ] Create `ViewportDebugOverlay`
- [ ] Update input mapping

---

## Phase 3: Split UpdateScreen

### Goals
- Break monolithic function into subsystems
- Maintain call order

### Tasks
- [ ] Extract input handling
- [ ] Extract UI update
- [ ] Extract gameplay update
- [ ] Extract render preparation
- [ ] Extract render execution

---

## Phase 4: Networking Upgrade

### Goals
- Replace XSocket with WinSock2
- Improve validation and buffering
- Reduce traffic

### Tasks
- [ ] Create `/engine/net/` interfaces
- [ ] Implement WinSock2 client
- [ ] Implement console server
- [ ] Add rate limiting
- [ ] Add message coalescing
- [ ] Add network metrics

---

## Phase 5: UI + ECS Migration

### Goals
- Build proper UI framework
- Introduce ECS-lite pattern

### Tasks
- [ ] Create `/game/ui/` widget system
- [ ] Create `/game/ecs/` entity system
- [ ] Migrate screens one at a time
- [ ] Migrate entities gradually

---

## Phase 6: Text + Fonts

### Goals
- Proper font management
- Lazy-load caching

### Tasks
- [ ] Create `/engine/text/FontManager`
- [ ] Create `/engine/text/TextRenderer`
- [ ] Create `/engine/text/bitmap/` module
- [ ] Replace ad-hoc text rendering

---

## Success Criteria

Each phase must:
1. Build successfully (Debug + Release, x86)
2. Run without crashes
3. Pass existing baseline tests
4. Maintain gameplay behavior
5. Have documented changes

---

## Risk Mitigation

- **Small commits**: Each change should be reviewable and revertible
- **Continuous testing**: Run game after each significant change
- **Baseline comparison**: Compare frame stats to detect regressions
- **Feature flags**: Use compile-time flags to enable/disable new systems

---

## Timeline

No specific timeline estimates - focus on incremental progress with continuous verification. Each phase builds on the previous and should not start until the previous is stable.
