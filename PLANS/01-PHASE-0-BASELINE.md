# Phase 0: Baseline + Build Setup

## Overview

This phase establishes the foundation for modernization by creating a working C++20 build system, adding essential diagnostics, and documenting the current architecture.

**Duration Estimate:** Foundation work - complete before any code changes

---

## Goals

1. Create modern Visual Studio 2022 projects with C++20 and valid x86 configurations
2. Establish logging and assertion infrastructure
3. Document critical architecture dependencies (viewport, network, game loop)
4. Create baseline capture system for regression detection
5. Ensure both client and server build and run correctly

---

## Prerequisites

- Visual Studio 2022 with C++ workload installed
- Windows SDK 10.0 or later
- DirectX SDK (for DirectDraw 7 headers/libs)

---

## Tasks

### Task 0.1: Create Modern Solution Structure

**Files to Create:**
```
/Helbreath.sln                     # Master solution
/Sources/Server/HGServer.vcxproj   # Server project (updated)
/Sources/Client/Client.vcxproj     # Client project (updated)
/Dependencies/Shared/Shared.vcxitems  # Shared items project
```

**Project Settings (Both Projects):**
```xml
<PropertyGroup>
    <PlatformToolset>v143</PlatformToolset>
    <ConfigurationType>Application</ConfigurationType>
    <CharacterSet>MultiByte</CharacterSet>
    <LanguageStandard>stdcpp20</LanguageStandard>
</PropertyGroup>

<PropertyGroup Condition="'$(Configuration)'=='Debug'">
    <UseDebugLibraries>true</UseDebugLibraries>
    <RuntimeLibrary>MultiThreadedDebug</RuntimeLibrary>
</PropertyGroup>

<PropertyGroup Condition="'$(Configuration)'=='Release'">
    <UseDebugLibraries>false</UseDebugLibraries>
    <RuntimeLibrary>MultiThreaded</RuntimeLibrary>
    <WholeProgramOptimization>true</WholeProgramOptimization>
</PropertyGroup>

<ItemDefinitionGroup>
    <ClCompile>
        <WarningLevel>Level4</WarningLevel>
        <TreatWarningAsError>false</TreatWarningAsError>  <!-- Enable later -->
        <PreprocessorDefinitions>WIN32;_WINDOWS;%(PreprocessorDefinitions)</PreprocessorDefinitions>
        <AdditionalIncludeDirectories>
            $(ProjectDir)..\..\Dependencies\Shared;
            %(AdditionalIncludeDirectories)
        </AdditionalIncludeDirectories>
    </ClCompile>
</ItemDefinitionGroup>
```

**Server-Specific:**
```xml
<ItemDefinitionGroup>
    <Link>
        <SubSystem>Windows</SubSystem>
        <AdditionalDependencies>ws2_32.lib;winmm.lib;%(AdditionalDependencies)</AdditionalDependencies>
    </Link>
</ItemDefinitionGroup>
```

**Client-Specific:**
```xml
<ItemDefinitionGroup>
    <ClCompile>
        <AdditionalIncludeDirectories>
            $(ProjectDir)..\..\Dependencies\Client\Includes;
            %(AdditionalIncludeDirectories)
        </AdditionalIncludeDirectories>
    </ClCompile>
    <Link>
        <SubSystem>Windows</SubSystem>
        <AdditionalLibraryDirectories>
            $(ProjectDir)..\..\Dependencies\Client\Libs;
            %(AdditionalLibraryDirectories)
        </AdditionalLibraryDirectories>
        <AdditionalDependencies>
            ddraw.lib;dinput.lib;dsound.lib;dxguid.lib;
            ws2_32.lib;winmm.lib;imm32.lib;
            %(AdditionalDependencies)
        </AdditionalDependencies>
    </Link>
</ItemDefinitionGroup>
```

**Verification:**
- [ ] Solution opens in VS2022 without errors
- [ ] Both configurations (Debug/Release) are available
- [ ] Platform is x86 (Win32)

---

### Task 0.2: Add Logging Infrastructure

**Files to Create:**
```
/Dependencies/Shared/core/Log.h
/Dependencies/Shared/core/Log.cpp
/Dependencies/Shared/core/Assert.h
```

**Log.h Design:**
```cpp
#pragma once

#include <string_view>
#include <source_location>
#include <format>

namespace Helbreath::Core {

enum class LogLevel : uint8_t {
    Trace = 0,
    Debug = 1,
    Info = 2,
    Warning = 3,
    Error = 4,
    Fatal = 5
};

class Logger {
public:
    static Logger& instance();

    void setLevel(LogLevel level);
    void setOutputFile(std::string_view path);
    void enableConsole(bool enable);

    void log(LogLevel level,
             std::string_view message,
             std::source_location loc = std::source_location::current());

    template<typename... Args>
    void logFmt(LogLevel level,
                std::format_string<Args...> fmt,
                Args&&... args,
                std::source_location loc = std::source_location::current()) {
        log(level, std::format(fmt, std::forward<Args>(args)...), loc);
    }

private:
    Logger() = default;
    // Implementation details...
};

// Convenience macros
#define LOG_TRACE(msg) Helbreath::Core::Logger::instance().log(Helbreath::Core::LogLevel::Trace, msg)
#define LOG_DEBUG(msg) Helbreath::Core::Logger::instance().log(Helbreath::Core::LogLevel::Debug, msg)
#define LOG_INFO(msg)  Helbreath::Core::Logger::instance().log(Helbreath::Core::LogLevel::Info, msg)
#define LOG_WARN(msg)  Helbreath::Core::Logger::instance().log(Helbreath::Core::LogLevel::Warning, msg)
#define LOG_ERROR(msg) Helbreath::Core::Logger::instance().log(Helbreath::Core::LogLevel::Error, msg)
#define LOG_FATAL(msg) Helbreath::Core::Logger::instance().log(Helbreath::Core::LogLevel::Fatal, msg)

} // namespace Helbreath::Core
```

**Assert.h Design:**
```cpp
#pragma once

#include <source_location>
#include <string_view>

namespace Helbreath::Core {

[[noreturn]] void assertFailed(
    std::string_view expression,
    std::string_view message,
    std::source_location loc = std::source_location::current());

} // namespace Helbreath::Core

#define HB_ASSERT(expr) \
    do { \
        if (!(expr)) { \
            Helbreath::Core::assertFailed(#expr, ""); \
        } \
    } while(false)

#define HB_ASSERT_MSG(expr, msg) \
    do { \
        if (!(expr)) { \
            Helbreath::Core::assertFailed(#expr, msg); \
        } \
    } while(false)

#ifdef _DEBUG
    #define HB_DEBUG_ASSERT(expr) HB_ASSERT(expr)
#else
    #define HB_DEBUG_ASSERT(expr) ((void)0)
#endif
```

**Verification:**
- [ ] Logging outputs to file and console
- [ ] Source location captured correctly
- [ ] Assert triggers debugger break in Debug mode

---

### Task 0.3: Document Viewport-Logic Dependencies

**File to Create:**
```
/PLANS/ANALYSIS/viewport-dependencies.md
```

**Areas to Document:**

1. **Client Game.cpp - Mouse Hit Testing**
   - Find all `m_stMCursor` usage
   - Document coordinate systems used
   - Map pixel coordinates to game world coordinates

2. **Client Game.cpp - UI Element Positioning**
   - Dialog positions (hardcoded 640x480 or 800x600?)
   - Button hit areas
   - Text rendering positions

3. **Client Game.cpp - World Rendering**
   - Tile rendering offsets
   - Sprite positioning
   - Camera/viewport calculations

4. **Client Game.cpp - DirectDraw Surface Sizes**
   - Primary surface dimensions
   - Back buffer dimensions
   - Sprite sheet assumptions

**Template for Each Finding:**
```markdown
## [Location]: Game.cpp:LineNumber

**Code:**
```cpp
// Relevant code snippet
```

**Coordinate System:** [Screen/World/Logical]
**Hardcoded Values:** [List any magic numbers]
**Dependencies:** [What other systems use this]
**Migration Notes:** [How to make resolution-independent]
```

---

### Task 0.4: Document Network Message Flow

**File to Create:**
```
/PLANS/ANALYSIS/network-messages.md
```

**Document Structure:**
```markdown
# Network Protocol Documentation

## Connection Flow
1. Client connects to Login Server (port 2500)
2. Authentication handshake
3. Character selection
4. Game server connection (port 9907)
5. Game data initialization

## Message Categories

### Login Messages (Client → Login Server)
| Message ID | Name | Direction | Payload |
|------------|------|-----------|---------|
| 0x0FC94201 | REQUEST_LOGIN | C→S | account, password |
| ... | ... | ... | ... |

### Game Messages (Client ↔ Game Server)
| Message ID | Name | Direction | Payload |
|------------|------|-----------|---------|
| 0x0FA314D5 | COMMAND_MOTION | C→S | action, x, y, dir |
| ... | ... | ... | ... |

## Message Handlers

### Server: CGame::OnClientSocketEvent()
- Routes WM_ONCLIENTSOCKETEVENT messages
- Calls specific handlers based on message type

### Client: CGame::OnGameSocketEvent()
- Handles FD_READ, FD_CLOSE events
- Dispatches to message processors
```

---

### Task 0.5: Create Baseline Capture System

**Files to Create:**
```
/Dependencies/Shared/debug/Baseline.h
/Dependencies/Shared/debug/Baseline.cpp
```

**Baseline.h Design:**
```cpp
#pragma once

#include "../Types.h"
#include <string>
#include <chrono>

namespace Helbreath::Debug {

struct FrameStats {
    u32 frameNumber = 0;
    f64 deltaTimeMs = 0.0;
    f64 fpsAverage = 0.0;

    // Viewport state
    i32 viewportX = 0;
    i32 viewportY = 0;
    i32 viewportWidth = 0;
    i32 viewportHeight = 0;

    // Game state
    i32 playerX = 0;
    i32 playerY = 0;
    i32 playerHP = 0;
    i32 playerMP = 0;

    // Network state
    u32 messagesSent = 0;
    u32 messagesReceived = 0;
    u32 bytesTransferred = 0;
};

class BaselineRecorder {
public:
    void startRecording(std::string_view filename);
    void stopRecording();
    void recordFrame(const FrameStats& stats);

    bool isRecording() const { return m_recording; }

private:
    bool m_recording = false;
    std::string m_filename;
    // File handle, etc.
};

class BaselineComparator {
public:
    bool loadBaseline(std::string_view filename);
    bool compareFrame(const FrameStats& current);
    std::string getLastDifference() const;

private:
    // Stored baseline data
};

} // namespace Helbreath::Debug
```

**Verification:**
- [ ] Can record frame stats to file
- [ ] Can load and compare against baseline
- [ ] Differences are clearly reported

---

### Task 0.6: Fix Immediate Build Errors

**Common Issues to Address:**

1. **Missing Windows SDK paths**
   - Ensure DirectX headers are found
   - Add proper include directories

2. **Deprecated function warnings**
   - `sprintf` → `sprintf_s` or `std::format`
   - `strcpy` → `strcpy_s` or `std::string`
   - Document but don't fix all (incremental)

3. **Type conversion warnings**
   - `DWORD` ↔ `int` conversions
   - Size type mismatches
   - Document for later phases

4. **Unresolved externals**
   - Verify all libraries linked
   - Check for missing implementations

**Create Build Error Tracking:**
```
/PLANS/TRACKING/build-errors.md
```

---

### Task 0.7: Create BUILDING.md

**File to Create:**
```
/BUILDING.md
```

**Content:**
```markdown
# Building Helbreath

## Requirements

- **Visual Studio 2022** (v17.x) with C++ Desktop Development workload
- **Windows SDK** 10.0.19041.0 or later
- **Platform:** x86 (32-bit) only

## Quick Start

1. Open `Helbreath.sln` in Visual Studio 2022
2. Select configuration: `Debug` or `Release`
3. Select platform: `x86`
4. Build → Build Solution (F7)

## Output Locations

| Project | Debug Output | Release Output |
|---------|-------------|----------------|
| Server | `Binaries/Server/Debug/HGServer.exe` | `Binaries/Server/Release/HGServer.exe` |
| Client | `Binaries/Game/Debug/Game.exe` | `Binaries/Game/Release/Game.exe` |

## Running

### Server
1. Copy configuration files from `Files/` to output directory
2. Run `HGServer.exe`
3. Server listens on ports 2500 (login) and 9907 (game)

### Client
1. Copy game data from `Helbreath/` to client output directory
2. Edit `LOGIN.CFG` with server address
3. Run `Game.exe`

## Troubleshooting

### "Cannot find ddraw.h"
Ensure DirectX SDK is installed and paths are configured in project settings.

### Linker errors for DirectX functions
Add DirectX library directory to Additional Library Directories.

### "Cannot open include file: 'windows.h'"
Install Windows SDK via Visual Studio Installer.

## Project Structure

See `CLAUDE.md` for detailed architecture documentation.
```

---

## Verification Checklist

### Build Verification
- [ ] Server builds in Debug configuration
- [ ] Server builds in Release configuration
- [ ] Client builds in Debug configuration
- [ ] Client builds in Release configuration
- [ ] No errors (warnings acceptable for Phase 0)

### Runtime Verification
- [ ] Server starts without crash
- [ ] Server creates listening sockets
- [ ] Server accepts connections (test with telnet)
- [ ] Client starts without crash
- [ ] Client initializes DirectDraw
- [ ] Client displays main menu

### Documentation Verification
- [ ] Viewport dependencies documented
- [ ] Network message flow documented
- [ ] Build instructions complete
- [ ] All configuration files identified

---

## Success Criteria

Phase 0 is complete when:

1. **Both projects build successfully** on a clean checkout
2. **Server runs** and accepts socket connections
3. **Client runs** and displays graphics
4. **Logging infrastructure** is functional
5. **Documentation** covers critical dependencies
6. **Baseline capture** can record and compare frames

---

## Next Phase

Once Phase 0 is complete, proceed to:
- **Phase 1: Engine Shell + Loop Wrapper**

The foundation established here enables safe incremental refactoring.
