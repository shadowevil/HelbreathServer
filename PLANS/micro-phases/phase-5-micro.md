# Phase 5: UI + ECS Migration - Micro-Phases

## Overview
Phase 5 is broken into 20 micro-phases, implementing the UI widget system and ECS-lite architecture.

---

## Part A: UI System (Micro-Phases 5.1-5.12)

---

## Micro-Phase 5.1: Create UIEvent Types

**Goal:** Define event structures for UI

**Changes:**
1. Create `Sources/Client/game/ui/core/UIEvent.h`
2. Define UIEvent::Type enum (MouseEnter, MouseLeave, Click, KeyDown, etc.)
3. Define UIEvent struct with type, coordinates, keycode, handled flag

**Files:**
- `Sources/Client/game/ui/core/UIEvent.h` (new)

**Verification:**
- [ ] Header compiles
- [ ] All event types defined

**Commit Message:** `feat(ui): add UIEvent types`

---

## Micro-Phase 5.2: Create Base Widget Class - Header

**Goal:** Define widget interface

**Changes:**
1. Create `Sources/Client/game/ui/core/Widget.h`
2. Define WidgetPtr using shared_ptr
3. Declare Widget class with hierarchy (parent, children)
4. Add geometry methods (setPosition, setSize, setBounds)
5. Add visibility and enabled flags
6. Declare event handling method

**Files:**
- `Sources/Client/game/ui/core/Widget.h` (new)

**Verification:**
- [ ] Header compiles
- [ ] All methods declared

**Commit Message:** `feat(ui): add Widget base class header`

---

## Micro-Phase 5.3: Implement Base Widget Class

**Goal:** Widget hierarchy and hit testing

**Changes:**
1. Create `Sources/Client/game/ui/core/Widget.cpp`
2. Implement addChild/removeChild
3. Implement absoluteBounds calculation
4. Implement containsPoint hit testing
5. Implement findWidgetAt traversal

**Files:**
- `Sources/Client/game/ui/core/Widget.cpp` (new)

**Verification:**
- [ ] Child management works
- [ ] Bounds calculation correct
- [ ] Hit testing accurate

**Commit Message:** `feat(ui): implement Widget base class`

---

## Micro-Phase 5.4: Create Panel Container Widget

**Goal:** Container for grouping widgets

**Changes:**
1. Create `Sources/Client/game/ui/widgets/Panel.h`
2. Create `Sources/Client/game/ui/widgets/Panel.cpp`
3. Inherit from Widget
4. Add background sprite support
5. Implement render traversal

**Files:**
- `Sources/Client/game/ui/widgets/Panel.h` (new)
- `Sources/Client/game/ui/widgets/Panel.cpp` (new)

**Verification:**
- [ ] Panel groups children
- [ ] Background renders
- [ ] Children render in order

**Commit Message:** `feat(ui/widgets): add Panel container`

---

## Micro-Phase 5.5: Create Button Widget

**Goal:** Clickable button with states

**Changes:**
1. Create `Sources/Client/game/ui/widgets/Button.h`
2. Create `Sources/Client/game/ui/widgets/Button.cpp`
3. Define Button::State enum (Normal, Hovered, Pressed, Disabled)
4. Implement sprite switching per state
5. Implement click event handling

**Files:**
- `Sources/Client/game/ui/widgets/Button.h` (new)
- `Sources/Client/game/ui/widgets/Button.cpp` (new)

**Verification:**
- [ ] Button renders correctly
- [ ] States change on hover/press
- [ ] Click callback fires

**Commit Message:** `feat(ui/widgets): add Button widget`

---

## Micro-Phase 5.6: Create Label Widget

**Goal:** Text display widget

**Changes:**
1. Create `Sources/Client/game/ui/widgets/Label.h`
2. Create `Sources/Client/game/ui/widgets/Label.cpp`
3. Store text string and color
4. Support text alignment (left, center, right)
5. Integrate with legacy text rendering

**Files:**
- `Sources/Client/game/ui/widgets/Label.h` (new)
- `Sources/Client/game/ui/widgets/Label.cpp` (new)

**Verification:**
- [ ] Text displays correctly
- [ ] Alignment works
- [ ] Color applied

**Commit Message:** `feat(ui/widgets): add Label widget`

---

## Micro-Phase 5.7: Create TextBox Widget

**Goal:** Text input widget

**Changes:**
1. Create `Sources/Client/game/ui/widgets/TextBox.h`
2. Create `Sources/Client/game/ui/widgets/TextBox.cpp`
3. Handle keyboard input for text entry
4. Implement cursor position tracking
5. Support password mode
6. Add max length limit

**Files:**
- `Sources/Client/game/ui/widgets/TextBox.h` (new)
- `Sources/Client/game/ui/widgets/TextBox.cpp` (new)

**Verification:**
- [ ] Text input works
- [ ] Cursor visible and movable
- [ ] Password mode shows dots
- [ ] Max length enforced

**Commit Message:** `feat(ui/widgets): add TextBox widget`

---

## Micro-Phase 5.8: Create UIManager Header

**Goal:** Central UI management

**Changes:**
1. Create `Sources/Client/game/ui/core/UIManager.h`
2. Implement singleton pattern
3. Declare root widget management
4. Declare focus management methods
5. Declare input processing methods

**Files:**
- `Sources/Client/game/ui/core/UIManager.h` (new)

**Verification:**
- [ ] Header compiles
- [ ] Singleton accessible

**Commit Message:** `feat(ui): add UIManager header`

---

## Micro-Phase 5.9: Implement UIManager Focus

**Goal:** Focus and navigation

**Changes:**
1. Create `Sources/Client/game/ui/core/UIManager.cpp`
2. Implement setRoot and root getters
3. Implement focus management (setFocus, focusedWidget)
4. Implement focusNext/focusPrevious tab navigation
5. Track focused, hovered, pressed widgets

**Files:**
- `Sources/Client/game/ui/core/UIManager.cpp` (new)

**Verification:**
- [ ] Focus can be set
- [ ] Tab navigation works
- [ ] Focus events dispatched

**Commit Message:** `feat(ui): implement UIManager focus management`

---

## Micro-Phase 5.10: Implement UIManager Input

**Goal:** Input dispatch to widgets

**Changes:**
1. Implement processMouseMove
2. Implement processMouseButton
3. Implement processKeyDown/processKeyUp
4. Dispatch events to appropriate widgets
5. Handle event bubbling

**Files:**
- `Sources/Client/game/ui/core/UIManager.cpp` (modify)

**Verification:**
- [ ] Mouse events reach widgets
- [ ] Key events reach focused widget
- [ ] Click detection works

**Commit Message:** `feat(ui): implement UIManager input dispatch`

---

## Micro-Phase 5.11: Create UIRenderer Interface

**Goal:** Abstract rendering for widgets

**Changes:**
1. Create `Sources/Client/game/ui/core/UIRenderer.h`
2. Create `Sources/Client/game/ui/core/UIRenderer.cpp`
3. Define drawing primitives (rect, sprite, text)
4. Wrap legacy DirectDraw rendering
5. Implement render traversal

**Files:**
- `Sources/Client/game/ui/core/UIRenderer.h` (new)
- `Sources/Client/game/ui/core/UIRenderer.cpp` (new)

**Verification:**
- [ ] Primitives render correctly
- [ ] Widgets can use renderer
- [ ] Clipping works

**Commit Message:** `feat(ui): add UIRenderer interface`

---

## Micro-Phase 5.12: Integrate UI with Game Loop

**Goal:** Hook UI into existing loop

**Changes:**
1. Modify LegacyGameAdapter to call UIManager
2. Process input through UIManager before legacy
3. Render UI after legacy rendering
4. Ensure proper layering

**Files:**
- `Sources/Client/game/app/LegacyGameAdapter.cpp` (modify)

**Verification:**
- [ ] UI renders on top of game
- [ ] Input reaches UI first
- [ ] Legacy input still works

**Commit Message:** `feat(client/game): integrate UIManager into game loop`

---

## Part B: ECS System (Micro-Phases 5.13-5.20)

---

## Micro-Phase 5.13: Create Entity Types

**Goal:** Basic entity identification

**Changes:**
1. Create `Sources/Client/game/ecs/core/Entity.h`
2. Define EntityId as u32
3. Define INVALID_ENTITY constant
4. Create EntityHandle struct (optional)

**Files:**
- `Sources/Client/game/ecs/core/Entity.h` (new)

**Verification:**
- [ ] Header compiles
- [ ] EntityId usable

**Commit Message:** `feat(ecs): add Entity types`

---

## Micro-Phase 5.14: Create EntityManager Header

**Goal:** Entity/component storage interface

**Changes:**
1. Create `Sources/Client/game/ecs/core/EntityManager.h`
2. Implement singleton pattern
3. Declare createEntity/destroyEntity
4. Declare template component methods (add, remove, get, has)
5. Declare query method

**Files:**
- `Sources/Client/game/ecs/core/EntityManager.h` (new)

**Verification:**
- [ ] Header compiles
- [ ] Templates declared

**Commit Message:** `feat(ecs): add EntityManager header`

---

## Micro-Phase 5.15: Implement EntityManager

**Goal:** Type-erased component storage

**Changes:**
1. Create `Sources/Client/game/ecs/core/EntityManager.cpp`
2. Implement entity creation with ID generation
3. Implement entity destruction
4. Implement type-erased component storage with std::any
5. Implement component query with variadic templates

**Files:**
- `Sources/Client/game/ecs/core/EntityManager.cpp` (new)

**Verification:**
- [ ] Entities created with unique IDs
- [ ] Components stored correctly
- [ ] Queries return matching entities

**Commit Message:** `feat(ecs): implement EntityManager`

---

## Micro-Phase 5.16: Create Transform Component

**Goal:** Position and direction

**Changes:**
1. Create `Sources/Client/game/ecs/components/TransformComponent.h`
2. Define world position (tile coordinates)
3. Define sub-tile offset for smooth movement
4. Define direction (0-7)
5. Add screen coordinate helpers

**Files:**
- `Sources/Client/game/ecs/components/TransformComponent.h` (new)

**Verification:**
- [ ] Component compiles
- [ ] Screen position calculates correctly

**Commit Message:** `feat(ecs/components): add TransformComponent`

---

## Micro-Phase 5.17: Create Sprite Component

**Goal:** Visual representation

**Changes:**
1. Create `Sources/Client/game/ecs/components/SpriteComponent.h`
2. Define sprite ID and frame index
3. Define color tint (RGB)
4. Define alpha and visibility
5. Define z-order for sorting

**Files:**
- `Sources/Client/game/ecs/components/SpriteComponent.h` (new)

**Verification:**
- [ ] Component compiles
- [ ] All rendering properties defined

**Commit Message:** `feat(ecs/components): add SpriteComponent`

---

## Micro-Phase 5.18: Create Stats Component

**Goal:** Character statistics

**Changes:**
1. Create `Sources/Client/game/ecs/components/StatsComponent.h`
2. Define HP, MP, SP (current and max)
3. Define attributes (STR, DEX, INT, VIT, MAG, CHR)
4. Define level and experience

**Files:**
- `Sources/Client/game/ecs/components/StatsComponent.h` (new)

**Verification:**
- [ ] Component compiles
- [ ] All stats represented

**Commit Message:** `feat(ecs/components): add StatsComponent`

---

## Micro-Phase 5.19: Create System Base Class

**Goal:** ECS system interface

**Changes:**
1. Create `Sources/Client/game/ecs/core/System.h`
2. Define abstract System class
3. Define update(EntityManager&, deltaTime) method
4. Create SystemManager for system registration (optional)

**Files:**
- `Sources/Client/game/ecs/core/System.h` (new)

**Verification:**
- [ ] Interface compiles
- [ ] Systems can inherit

**Commit Message:** `feat(ecs): add System base class`

---

## Micro-Phase 5.20: Create Legacy Entity Bridge

**Goal:** Connect legacy entities to ECS

**Changes:**
1. Create `Sources/Client/game/bridge/LegacyEntityBridge.h`
2. Create `Sources/Client/game/bridge/LegacyEntityBridge.cpp`
3. Implement createFromLegacyClient (CClient -> Entity)
4. Implement syncToLegacyClient (Entity -> CClient)
5. Maintain legacy pointer to EntityId mapping

**Files:**
- `Sources/Client/game/bridge/LegacyEntityBridge.h` (new)
- `Sources/Client/game/bridge/LegacyEntityBridge.cpp` (new)

**Verification:**
- [ ] Can create entity from CClient
- [ ] Data syncs both ways
- [ ] Mapping maintained correctly

**Commit Message:** `feat(ecs): add LegacyEntityBridge`

---

## Phase 5 Completion Checklist

After all micro-phases:
- [ ] Widget hierarchy works
- [ ] All basic widgets implemented (Panel, Button, Label, TextBox)
- [ ] UIManager handles focus and input
- [ ] EntityManager stores entities and components
- [ ] Core components defined (Transform, Sprite, Stats)
- [ ] Legacy bridge enables gradual migration
- [ ] Ready for Phase 6
