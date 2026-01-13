# Phase 0: Baseline + Build Setup - Micro-Phases

## Overview
Phase 0 is broken into 12 micro-phases, each completing a single focused task.

---

## Micro-Phase 0.1: Create Solution File

**Goal:** Single VS2022 solution file that opens without errors

**Changes:**
1. Create `Helbreath.sln` at repository root
2. Configure for VS2022 format
3. Add placeholder for Server and Client projects

**Files:**
- `Helbreath.sln` (new)

**Verification:**
- [ ] Solution opens in VS2022
- [ ] No errors or warnings on open

**Commit Message:** `build: create VS2022 solution file`

---

## Micro-Phase 0.2: Create Server Project File

**Goal:** Server project file with C++20 settings

**Changes:**
1. Update `Sources/Server/HGserver.vcxproj` with modern settings
2. Set `LanguageStandard` to `stdcpp20`
3. Set `PlatformToolset` to `v143`
4. Configure Debug/Release for x86

**Files:**
- `Sources/Server/HGserver.vcxproj` (modify)
- `Sources/Server/HGserver.vcxproj.filters` (modify)

**Verification:**
- [ ] Project loads in solution
- [ ] Properties show C++20, v143, x86

**Commit Message:** `build(server): configure C++20 and VS2022 toolset`

---

## Micro-Phase 0.3: Create Client Project File

**Goal:** Client project file with C++20 settings

**Changes:**
1. Update `Sources/Client/Client.vcxproj` with modern settings
2. Set `LanguageStandard` to `stdcpp20`
3. Set `PlatformToolset` to `v143`
4. Configure Debug/Release for x86
5. Add DirectX include/lib paths

**Files:**
- `Sources/Client/Client.vcxproj` (modify)
- `Sources/Client/Client.vcxproj.filters` (modify)

**Verification:**
- [ ] Project loads in solution
- [ ] Properties show C++20, v143, x86
- [ ] DirectX paths configured

**Commit Message:** `build(client): configure C++20 and VS2022 toolset`

---

## Micro-Phase 0.4: Add Shared Items Project

**Goal:** Shared code accessible from both projects

**Changes:**
1. Create `Dependencies/Shared/Shared.vcxitems`
2. Reference from Server and Client projects
3. Add existing shared headers

**Files:**
- `Dependencies/Shared/Shared.vcxitems` (new)
- `Sources/Server/HGserver.vcxproj` (modify - add reference)
- `Sources/Client/Client.vcxproj` (modify - add reference)

**Verification:**
- [ ] Shared files appear in both projects
- [ ] No duplicate symbol errors

**Commit Message:** `build: add shared items project for common code`

---

## Micro-Phase 0.5: Fix Server Build Errors

**Goal:** Server compiles without errors

**Changes:**
1. Fix any include path issues
2. Fix any missing dependencies
3. Add required libs to linker
4. Document any remaining warnings

**Files:**
- Various server source files (minimal changes)
- `Sources/Server/HGserver.vcxproj` (if needed)

**Verification:**
- [ ] Server builds in Debug
- [ ] Server builds in Release
- [ ] Document warning count: ___

**Commit Message:** `fix(server): resolve build errors for C++20`

---

## Micro-Phase 0.6: Fix Client Build Errors

**Goal:** Client compiles without errors

**Changes:**
1. Fix any include path issues
2. Fix DirectX SDK references
3. Add required libs to linker
4. Document any remaining warnings

**Files:**
- Various client source files (minimal changes)
- `Sources/Client/Client.vcxproj` (if needed)

**Verification:**
- [ ] Client builds in Debug
- [ ] Client builds in Release
- [ ] Document warning count: ___

**Commit Message:** `fix(client): resolve build errors for C++20`

---

## Micro-Phase 0.7: Create Log Header

**Goal:** Basic logging infrastructure (header only)

**Changes:**
1. Create `Dependencies/Shared/core/Log.h`
2. Define LogLevel enum
3. Define LOG_* macros
4. Header-only implementation initially

**Files:**
- `Dependencies/Shared/core/Log.h` (new)

**Verification:**
- [ ] Header compiles when included
- [ ] LOG_INFO("test") compiles

**Commit Message:** `feat(core): add logging header with level macros`

---

## Micro-Phase 0.8: Create Log Implementation

**Goal:** Logging writes to file and console

**Changes:**
1. Create `Dependencies/Shared/core/Log.cpp`
2. Implement file output
3. Implement console output (server only)
4. Add to Shared.vcxitems

**Files:**
- `Dependencies/Shared/core/Log.cpp` (new)
- `Dependencies/Shared/Shared.vcxitems` (modify)

**Verification:**
- [ ] LOG_INFO writes to log file
- [ ] Timestamps appear correctly
- [ ] Source location captured

**Commit Message:** `feat(core): implement logging to file and console`

---

## Micro-Phase 0.9: Create Assert Header

**Goal:** Debug assertion macros

**Changes:**
1. Create `Dependencies/Shared/core/Assert.h`
2. Define HB_ASSERT macro
3. Define HB_DEBUG_ASSERT macro
4. Trigger debugger break on failure

**Files:**
- `Dependencies/Shared/core/Assert.h` (new)

**Verification:**
- [ ] HB_ASSERT(false) triggers break in Debug
- [ ] HB_DEBUG_ASSERT is no-op in Release

**Commit Message:** `feat(core): add assertion macros`

---

## Micro-Phase 0.10: Integrate Logging in Server

**Goal:** Server uses new logging

**Changes:**
1. Include Log.h in server entry point
2. Replace one PutLogList call with LOG_INFO
3. Verify both old and new logging work

**Files:**
- `Sources/Server/Wmain.cpp` (modify)

**Verification:**
- [ ] Server starts with new logging
- [ ] Both log systems output
- [ ] No functionality change

**Commit Message:** `refactor(server): integrate new logging alongside legacy`

---

## Micro-Phase 0.11: Create BUILDING.md

**Goal:** Build instructions documented

**Changes:**
1. Create `BUILDING.md` at root
2. Document prerequisites
3. Document build steps
4. Document output locations
5. Document common issues

**Files:**
- `BUILDING.md` (new)

**Verification:**
- [ ] Fresh clone can build following instructions
- [ ] All prerequisites listed
- [ ] Troubleshooting section helpful

**Commit Message:** `docs: add BUILDING.md with build instructions`

---

## Micro-Phase 0.12: Create Baseline Stats Structure

**Goal:** Framework for capturing baseline metrics

**Changes:**
1. Create `Dependencies/Shared/debug/Baseline.h`
2. Define FrameStats struct
3. Define BaselineRecorder interface (no implementation yet)

**Files:**
- `Dependencies/Shared/debug/Baseline.h` (new)

**Verification:**
- [ ] Header compiles
- [ ] FrameStats can be instantiated
- [ ] No implementation needed yet

**Commit Message:** `feat(debug): add baseline stats structure`

---

## Phase 0 Completion Checklist

After all micro-phases:
- [ ] Solution builds both projects
- [ ] Server runs and accepts connections
- [ ] Client runs and shows graphics
- [ ] Logging infrastructure available
- [ ] Assertions available
- [ ] Build documented
- [ ] Ready for Phase 1
