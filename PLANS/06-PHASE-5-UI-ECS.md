# Phase 5: UI + ECS Migration

## Overview

This phase introduces a proper UI widget system and Entity-Component-System (ECS-lite) architecture to replace the tightly-coupled legacy code. Migration happens incrementally, screen by screen and entity by entity.

**Prerequisite:** Phase 4 complete (Networking modernized)

---

## Goals

1. Create a declarative UI widget system
2. Implement ECS-lite for game entities
3. Migrate UI screens one at a time
4. Migrate game entities gradually
5. Maintain exact visual and behavioral compatibility

---

## Part A: UI System

### Current UI Problems

```cpp
// Scattered button checks throughout code
if (m_stMCursor.sX >= 100 && m_stMCursor.sX <= 200 &&
    m_stMCursor.sY >= 50 && m_stMCursor.sY <= 80) {
    if (m_bMouseClicked) {
        // Handle button click
    }
}

// Hardcoded positions everywhere
DrawSprite(150, 200, m_pSprite[SPRITE_BUTTON]);

// No focus/navigation system
// No text input abstraction
```

### Target UI Architecture

```
┌─────────────────────────────────────────────────────────┐
│                    UIManager                             │
│  - Widget tree management                                │
│  - Focus/navigation                                      │
│  - Event dispatch                                        │
└─────────────────────────────────────────────────────────┘
                              │
              ┌───────────────┼───────────────┐
              │               │               │
              ▼               ▼               ▼
        ┌─────────┐     ┌─────────┐     ┌─────────┐
        │  Panel  │     │  Button │     │  Label  │
        │(Container)│   │         │     │         │
        └─────────┘     └─────────┘     └─────────┘
              │
      ┌───────┼───────┐
      ▼       ▼       ▼
   Widget  Widget  Widget
```

---

### UI Directory Structure

```
/Sources/Client/game/ui/
├── core/
│   ├── Widget.h           # Base widget class
│   ├── UIManager.h        # Widget tree management
│   ├── UIEvent.h          # Event types
│   └── UIRenderer.h       # Rendering interface
├── widgets/
│   ├── Panel.h            # Container widget
│   ├── Button.h           # Clickable button
│   ├── Label.h            # Text display
│   ├── TextBox.h          # Text input
│   ├── ListView.h         # Scrollable list
│   ├── ProgressBar.h      # Progress indicator
│   └── ImageWidget.h      # Static image
├── layouts/
│   ├── ILayout.h          # Layout interface
│   ├── AbsoluteLayout.h   # Fixed positions
│   ├── VerticalLayout.h   # Stack vertically
│   └── HorizontalLayout.h # Stack horizontally
└── dialogs/
    ├── Dialog.h           # Base dialog class
    ├── MessageBox.h       # Simple message
    ├── InventoryDialog.h  # Inventory screen
    └── CharacterDialog.h  # Character stats
```

---

### UI Tasks

#### Task 5.1: Create Base Widget Class

**Widget.h:**
```cpp
#pragma once

#include <Dependencies/Shared/Types.h>
#include <memory>
#include <vector>
#include <string>
#include <functional>

namespace Helbreath::UI {

class Widget;
using WidgetPtr = std::shared_ptr<Widget>;

struct UIEvent {
    enum class Type {
        MouseEnter,
        MouseLeave,
        MouseDown,
        MouseUp,
        Click,
        KeyDown,
        KeyUp,
        FocusGained,
        FocusLost,
        ValueChanged
    };

    Type type;
    i32 mouseX = 0;
    i32 mouseY = 0;
    u32 keyCode = 0;
    bool handled = false;
};

/**
 * Base class for all UI widgets.
 */
class Widget {
public:
    virtual ~Widget() = default;

    // Hierarchy
    void addChild(WidgetPtr child);
    void removeChild(Widget* child);
    Widget* parent() const { return m_parent; }
    const std::vector<WidgetPtr>& children() const { return m_children; }

    // Geometry (in logical coordinates)
    void setPosition(i32 x, i32 y);
    void setSize(i32 width, i32 height);
    void setBounds(const Rect& bounds);
    [[nodiscard]] Rect bounds() const { return m_bounds; }
    [[nodiscard]] Rect absoluteBounds() const;

    // Visibility
    void setVisible(bool visible) { m_visible = visible; }
    [[nodiscard]] bool isVisible() const { return m_visible; }

    // Enable/disable
    void setEnabled(bool enabled) { m_enabled = enabled; }
    [[nodiscard]] bool isEnabled() const { return m_enabled; }

    // Focus
    void setFocusable(bool focusable) { m_focusable = focusable; }
    [[nodiscard]] bool isFocusable() const { return m_focusable; }
    [[nodiscard]] bool hasFocus() const;

    // Hit testing
    [[nodiscard]] virtual bool containsPoint(i32 x, i32 y) const;
    [[nodiscard]] Widget* findWidgetAt(i32 x, i32 y);

    // Event handling
    virtual bool handleEvent(UIEvent& event);
    void setOnClick(std::function<void()> callback) { m_onClick = std::move(callback); }

    // Rendering (override in subclasses)
    virtual void render(class UIRenderer& renderer);

    // Identification
    void setName(std::string_view name) { m_name = name; }
    [[nodiscard]] const std::string& name() const { return m_name; }

protected:
    Widget* m_parent = nullptr;
    std::vector<WidgetPtr> m_children;
    Rect m_bounds{0, 0, 100, 30};
    bool m_visible = true;
    bool m_enabled = true;
    bool m_focusable = false;
    std::string m_name;

    // Event callbacks
    std::function<void()> m_onClick;
};

} // namespace Helbreath::UI
```

---

#### Task 5.2: Create Common Widgets

**Button.h:**
```cpp
#pragma once

#include "core/Widget.h"

namespace Helbreath::UI {

class Button : public Widget {
public:
    enum class State {
        Normal,
        Hovered,
        Pressed,
        Disabled
    };

    Button();

    void setText(std::string_view text) { m_text = text; }
    [[nodiscard]] const std::string& text() const { return m_text; }

    void setSprites(u32 normal, u32 hovered, u32 pressed, u32 disabled);

    bool handleEvent(UIEvent& event) override;
    void render(UIRenderer& renderer) override;

private:
    std::string m_text;
    State m_state = State::Normal;

    u32 m_spriteNormal = 0;
    u32 m_spriteHovered = 0;
    u32 m_spritePressed = 0;
    u32 m_spriteDisabled = 0;
};

} // namespace Helbreath::UI
```

**TextBox.h:**
```cpp
#pragma once

#include "core/Widget.h"

namespace Helbreath::UI {

class TextBox : public Widget {
public:
    TextBox();

    void setText(std::string_view text);
    [[nodiscard]] const std::string& text() const { return m_text; }

    void setMaxLength(usize maxLen) { m_maxLength = maxLen; }
    void setPasswordMode(bool password) { m_passwordMode = password; }

    bool handleEvent(UIEvent& event) override;
    void render(UIRenderer& renderer) override;

private:
    std::string m_text;
    usize m_cursorPos = 0;
    usize m_maxLength = 256;
    bool m_passwordMode = false;

    void insertChar(char c);
    void deleteChar();
    void moveCursor(i32 delta);
};

} // namespace Helbreath::UI
```

---

#### Task 5.3: Create UI Manager

**UIManager.h:**
```cpp
#pragma once

#include "Widget.h"
#include <memory>

namespace Helbreath::UI {

class UIManager {
public:
    static UIManager& instance();

    // Root widget management
    void setRoot(WidgetPtr root);
    Widget* root() const { return m_root.get(); }

    // Focus management
    void setFocus(Widget* widget);
    Widget* focusedWidget() const { return m_focused; }
    void focusNext();
    void focusPrevious();

    // Input processing (call from input system)
    void processMouseMove(i32 x, i32 y);
    void processMouseButton(i32 button, bool down);
    void processKeyDown(u32 keyCode);
    void processKeyUp(u32 keyCode);

    // Rendering (call from render system)
    void render(UIRenderer& renderer);

private:
    UIManager() = default;

    WidgetPtr m_root;
    Widget* m_focused = nullptr;
    Widget* m_hovered = nullptr;
    Widget* m_pressed = nullptr;

    i32 m_lastMouseX = 0;
    i32 m_lastMouseY = 0;
};

} // namespace Helbreath::UI
```

---

#### Task 5.4: Migrate Login Screen (Example)

**Before (in CGame):**
```cpp
void CGame::DrawLoginScreen() {
    // Draw background
    m_DDraw.DrawSprite(0, 0, SPRITE_LOGIN_BG);

    // Draw username field
    DrawTextBox(100, 200, m_cAccountName);

    // Draw password field
    DrawTextBox(100, 250, m_cPassword, true);

    // Draw login button
    if (m_bLoginButtonHover) {
        m_DDraw.DrawSprite(150, 300, SPRITE_BUTTON_HOVER);
    } else {
        m_DDraw.DrawSprite(150, 300, SPRITE_BUTTON_NORMAL);
    }
}
```

**After (LoginScreen.cpp):**
```cpp
class LoginScreen : public IScreen {
public:
    void onEnter() override {
        auto root = std::make_shared<Panel>();

        // Background
        auto bg = std::make_shared<ImageWidget>(SPRITE_LOGIN_BG);
        root->addChild(bg);

        // Username field
        m_usernameBox = std::make_shared<TextBox>();
        m_usernameBox->setPosition(100, 200);
        m_usernameBox->setSize(200, 25);
        m_usernameBox->setMaxLength(10);
        root->addChild(m_usernameBox);

        // Password field
        m_passwordBox = std::make_shared<TextBox>();
        m_passwordBox->setPosition(100, 250);
        m_passwordBox->setSize(200, 25);
        m_passwordBox->setMaxLength(10);
        m_passwordBox->setPasswordMode(true);
        root->addChild(m_passwordBox);

        // Login button
        auto loginBtn = std::make_shared<Button>();
        loginBtn->setPosition(150, 300);
        loginBtn->setText("Login");
        loginBtn->setOnClick([this]() { onLoginClicked(); });
        root->addChild(loginBtn);

        UIManager::instance().setRoot(root);
    }

private:
    std::shared_ptr<TextBox> m_usernameBox;
    std::shared_ptr<TextBox> m_passwordBox;

    void onLoginClicked() {
        // Trigger login with entered credentials
        std::string username = m_usernameBox->text();
        std::string password = m_passwordBox->text();
        // ... login logic
    }
};
```

---

## Part B: ECS System

### Current Entity Problems

```cpp
// Massive CClient class with everything mixed
class CClient {
    char m_cCharName[11];
    int m_iHP, m_iMP, m_iSP;
    int m_iStr, m_iDex, m_iInt;
    CItem m_ItemList[50];
    // ... hundreds more members
    // ... mixed rendering, logic, network data
};
```

### Target ECS Architecture

```
┌─────────────────────────────────────────────────────────┐
│                    EntityManager                         │
│  - Entity creation/destruction                           │
│  - Component storage                                     │
│  - Entity queries                                        │
└─────────────────────────────────────────────────────────┘
                              │
              ┌───────────────┼───────────────┐
              │               │               │
              ▼               ▼               ▼
        ┌─────────┐     ┌─────────┐     ┌─────────┐
        │Transform│     │ Sprite  │     │  Stats  │
        │Component│     │Component│     │Component│
        └─────────┘     └─────────┘     └─────────┘

┌─────────────────────────────────────────────────────────┐
│                    Systems                               │
│  MovementSystem, RenderSystem, CombatSystem, etc.       │
└─────────────────────────────────────────────────────────┘
```

---

### ECS Directory Structure

```
/Sources/Client/game/ecs/
├── core/
│   ├── Entity.h           # Entity ID and handle
│   ├── Component.h        # Component base
│   ├── EntityManager.h    # Entity/component storage
│   └── System.h           # System base class
├── components/
│   ├── TransformComponent.h
│   ├── SpriteComponent.h
│   ├── AnimationComponent.h
│   ├── StatsComponent.h
│   ├── InventoryComponent.h
│   ├── NetworkComponent.h
│   └── AIComponent.h
└── systems/
    ├── MovementSystem.h
    ├── RenderSystem.h
    ├── AnimationSystem.h
    ├── CombatSystem.h
    └── NetworkSyncSystem.h
```

---

### ECS Tasks

#### Task 5.5: Create Entity Manager

**EntityManager.h:**
```cpp
#pragma once

#include <Dependencies/Shared/Types.h>
#include <unordered_map>
#include <typeindex>
#include <any>
#include <vector>

namespace Helbreath::ECS {

using EntityId = u32;
constexpr EntityId INVALID_ENTITY = 0;

/**
 * Simple ECS-lite entity manager.
 * Uses type-erased component storage for flexibility.
 */
class EntityManager {
public:
    static EntityManager& instance();

    // Entity lifecycle
    [[nodiscard]] EntityId createEntity();
    void destroyEntity(EntityId entity);
    [[nodiscard]] bool isValid(EntityId entity) const;

    // Component management
    template<typename T, typename... Args>
    T& addComponent(EntityId entity, Args&&... args) {
        auto& storage = getOrCreateStorage<T>();
        storage[entity] = T(std::forward<Args>(args)...);
        return storage[entity];
    }

    template<typename T>
    void removeComponent(EntityId entity) {
        auto& storage = getStorage<T>();
        storage.erase(entity);
    }

    template<typename T>
    [[nodiscard]] bool hasComponent(EntityId entity) const {
        auto it = m_componentStorages.find(std::type_index(typeid(T)));
        if (it == m_componentStorages.end()) return false;
        auto& storage = std::any_cast<const std::unordered_map<EntityId, T>&>(it->second);
        return storage.contains(entity);
    }

    template<typename T>
    [[nodiscard]] T& getComponent(EntityId entity) {
        return getStorage<T>().at(entity);
    }

    template<typename T>
    [[nodiscard]] const T& getComponent(EntityId entity) const {
        return getStorage<T>().at(entity);
    }

    // Query entities with specific components
    template<typename... Components>
    [[nodiscard]] std::vector<EntityId> query() const {
        std::vector<EntityId> result;
        for (EntityId id : m_entities) {
            if ((hasComponent<Components>(id) && ...)) {
                result.push_back(id);
            }
        }
        return result;
    }

private:
    EntityManager() = default;

    template<typename T>
    std::unordered_map<EntityId, T>& getOrCreateStorage() {
        auto key = std::type_index(typeid(T));
        if (!m_componentStorages.contains(key)) {
            m_componentStorages[key] = std::unordered_map<EntityId, T>{};
        }
        return std::any_cast<std::unordered_map<EntityId, T>&>(m_componentStorages[key]);
    }

    template<typename T>
    std::unordered_map<EntityId, T>& getStorage() {
        return std::any_cast<std::unordered_map<EntityId, T>&>(
            m_componentStorages.at(std::type_index(typeid(T))));
    }

    std::vector<EntityId> m_entities;
    std::unordered_map<std::type_index, std::any> m_componentStorages;
    EntityId m_nextId = 1;
};

} // namespace Helbreath::ECS
```

---

#### Task 5.6: Create Core Components

**TransformComponent.h:**
```cpp
#pragma once

#include <Dependencies/Shared/Types.h>

namespace Helbreath::ECS {

struct TransformComponent {
    // World position (tile coordinates)
    i32 worldX = 0;
    i32 worldY = 0;

    // Sub-tile position for smooth movement
    f32 offsetX = 0.0f;
    f32 offsetY = 0.0f;

    // Direction (0-7 for 8 directions)
    u8 direction = 0;

    // For rendering calculations
    [[nodiscard]] f32 screenX() const {
        return static_cast<f32>(worldX * 32) + offsetX;
    }
    [[nodiscard]] f32 screenY() const {
        return static_cast<f32>(worldY * 32) + offsetY;
    }
};

} // namespace Helbreath::ECS
```

**SpriteComponent.h:**
```cpp
#pragma once

#include <Dependencies/Shared/Types.h>

namespace Helbreath::ECS {

struct SpriteComponent {
    u16 spriteId = 0;
    u16 frameIndex = 0;

    // Rendering properties
    i8 colorR = 0, colorG = 0, colorB = 0;  // Color tint
    u8 alpha = 255;  // Transparency
    bool flipX = false;
    bool visible = true;

    // Z-ordering
    i16 zOrder = 0;
};

} // namespace Helbreath::ECS
```

**StatsComponent.h:**
```cpp
#pragma once

#include <Dependencies/Shared/Types.h>

namespace Helbreath::ECS {

struct StatsComponent {
    // Basic stats
    i32 hp = 0;
    i32 maxHp = 0;
    i32 mp = 0;
    i32 maxMp = 0;
    i32 sp = 0;
    i32 maxSp = 0;

    // Attributes
    i16 strength = 10;
    i16 dexterity = 10;
    i16 intelligence = 10;
    i16 vitality = 10;
    i16 magic = 10;
    i16 charisma = 10;

    // Combat stats
    i32 level = 1;
    u64 experience = 0;
};

} // namespace Helbreath::ECS
```

---

#### Task 5.7: Create Systems

**System.h:**
```cpp
#pragma once

namespace Helbreath::ECS {

class EntityManager;

/**
 * Base class for ECS systems.
 */
class System {
public:
    virtual ~System() = default;

    virtual void update(EntityManager& em, f64 deltaTime) = 0;
};

} // namespace Helbreath::ECS
```

**MovementSystem.h:**
```cpp
#pragma once

#include "core/System.h"

namespace Helbreath::ECS {

class MovementSystem : public System {
public:
    void update(EntityManager& em, f64 deltaTime) override {
        // Query all entities with Transform and Velocity components
        for (EntityId id : em.query<TransformComponent, VelocityComponent>()) {
            auto& transform = em.getComponent<TransformComponent>(id);
            auto& velocity = em.getComponent<VelocityComponent>(id);

            // Update position
            transform.offsetX += velocity.vx * static_cast<f32>(deltaTime);
            transform.offsetY += velocity.vy * static_cast<f32>(deltaTime);

            // Handle tile transitions
            while (transform.offsetX >= 32.0f) {
                transform.offsetX -= 32.0f;
                transform.worldX++;
            }
            // ... similar for other directions
        }
    }
};

} // namespace Helbreath::ECS
```

---

#### Task 5.8: Bridge Legacy Entities

**LegacyEntityBridge.h:**
```cpp
#pragma once

#include "../ecs/core/EntityManager.h"
#include "../../Client.h"  // Legacy CClient

namespace Helbreath::Game {

/**
 * Bridges legacy CClient/CNpc to ECS entities.
 * Allows gradual migration without breaking existing code.
 */
class LegacyEntityBridge {
public:
    // Create ECS entity from legacy client
    static ECS::EntityId createFromLegacyClient(CClient* client);

    // Sync ECS data back to legacy client
    static void syncToLegacyClient(ECS::EntityId entity, CClient* client);

    // Create ECS entity from legacy NPC
    static ECS::EntityId createFromLegacyNpc(CNpc* npc);

private:
    // Mapping between legacy pointers and ECS entities
    static std::unordered_map<void*, ECS::EntityId> s_legacyToEntity;
};

} // namespace Helbreath::Game
```

---

## Migration Strategy

### UI Migration Order
1. Login screen (simplest, isolated)
2. Character selection screen
3. Main menu
4. In-game dialogs (inventory, character, etc.)
5. In-game HUD

### ECS Migration Order
1. Visual-only entities (effects, particles)
2. NPCs (relatively isolated)
3. Other players (more complex sync)
4. Local player (most complex)

---

## Verification

### UI Tests
- [ ] All buttons clickable
- [ ] Text input works
- [ ] Focus navigation works
- [ ] All screens display correctly

### ECS Tests
- [ ] Entity creation/destruction
- [ ] Component add/remove
- [ ] System updates run correctly
- [ ] Legacy bridge sync works

---

## Success Criteria

Phase 5 is complete when:

1. **UI system** handles all game screens
2. **ECS** manages all game entities
3. **Legacy code** still works via bridges
4. **No visual or behavioral changes**
5. **Code is more maintainable**

---

## Next Phase

Once Phase 5 is complete, proceed to:
- **Phase 6: Text + Fonts**
