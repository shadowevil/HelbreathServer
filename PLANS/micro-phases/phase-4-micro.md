# Phase 4: Networking Upgrade - Micro-Phases

## Overview
Phase 4 is broken into 18 micro-phases, replacing XSocket with modern WinSock2.

---

## Micro-Phase 4.1: Create INetSocket Interface

**Goal:** Abstract socket interface

**Changes:**
1. Create `Dependencies/Shared/net/INetSocket.h`
2. Define SocketState enum
3. Define SocketError enum
4. Define INetSocket abstract class

**Files:**
- `Dependencies/Shared/net/INetSocket.h` (new)

**Verification:**
- [ ] Interface compiles
- [ ] All methods defined

**Commit Message:** `feat(net): add INetSocket interface`

---

## Micro-Phase 4.2: Create ClientSocket Header

**Goal:** Define WinSock2 client

**Changes:**
1. Create `Sources/Client/engine/net/ClientSocket.h`
2. Inherit from INetSocket
3. Include WinSock2 headers
4. Declare all methods

**Files:**
- `Sources/Client/engine/net/ClientSocket.h` (new)

**Verification:**
- [ ] Header compiles with WinSock2
- [ ] No WinSock1 contamination

**Commit Message:** `feat(client/net): add ClientSocket header`

---

## Micro-Phase 4.3: Implement ClientSocket - Initialize

**Goal:** Socket creation and WinSock startup

**Changes:**
1. Create `Sources/Client/engine/net/ClientSocket.cpp`
2. Implement WSAStartup in constructor
3. Implement socket creation
4. Set non-blocking mode with ioctlsocket

**Files:**
- `Sources/Client/engine/net/ClientSocket.cpp` (new)

**Verification:**
- [ ] WinSock2 initialized
- [ ] Socket created successfully
- [ ] Non-blocking mode set

**Commit Message:** `feat(client/net): implement ClientSocket initialization`

---

## Micro-Phase 4.4: Implement ClientSocket - Connect

**Goal:** Non-blocking connect

**Changes:**
1. Implement connect() method
2. Use getaddrinfo for resolution
3. Initiate non-blocking connect
4. Track connecting state

**Files:**
- `Sources/Client/engine/net/ClientSocket.cpp` (modify)

**Verification:**
- [ ] Connect starts without blocking
- [ ] State set to Connecting
- [ ] Handles invalid host

**Commit Message:** `feat(client/net): implement non-blocking connect`

---

## Micro-Phase 4.5: Implement ClientSocket - Poll

**Goal:** Check socket state

**Changes:**
1. Implement poll() method
2. Use select() for readability/writability
3. Check connection completion
4. Fire events via callback

**Files:**
- `Sources/Client/engine/net/ClientSocket.cpp` (modify)

**Verification:**
- [ ] poll() detects connected
- [ ] poll() detects data available
- [ ] No blocking occurs

**Commit Message:** `feat(client/net): implement socket polling`

---

## Micro-Phase 4.6: Implement ClientSocket - Send/Receive

**Goal:** Data transfer

**Changes:**
1. Implement send(ByteSpan) method
2. Implement receive(MutableByteSpan) method
3. Handle WSAEWOULDBLOCK
4. Track bytes sent/received

**Files:**
- `Sources/Client/engine/net/ClientSocket.cpp` (modify)

**Verification:**
- [ ] Can send data
- [ ] Can receive data
- [ ] Statistics accurate

**Commit Message:** `feat(client/net): implement send/receive`

---

## Micro-Phase 4.7: Create Simple Echo Test

**Goal:** Verify socket works

**Changes:**
1. Create simple test server (separate exe or script)
2. Test connect, send, receive, disconnect
3. Document test procedure

**Files:**
- `Tools/socket_test.py` (new - simple echo server)
- `PLANS/TESTING/socket-test.md` (new)

**Verification:**
- [ ] Can connect to test server
- [ ] Data roundtrips correctly
- [ ] Disconnect clean

**Commit Message:** `test(net): add socket echo test`

---

## Micro-Phase 4.8: Create NetMetrics

**Goal:** Network statistics

**Changes:**
1. Create `Dependencies/Shared/net/NetMetrics.h`
2. Create `Dependencies/Shared/net/NetMetrics.cpp`
3. Track messages/bytes sent/received
4. Calculate rates per second

**Files:**
- `Dependencies/Shared/net/NetMetrics.h` (new)
- `Dependencies/Shared/net/NetMetrics.cpp` (new)

**Verification:**
- [ ] Metrics accumulate correctly
- [ ] Rates calculate correctly

**Commit Message:** `feat(net): add NetMetrics for statistics`

---

## Micro-Phase 4.9: Create RateLimiter

**Goal:** Traffic control

**Changes:**
1. Create `Dependencies/Shared/net/RateLimiter.h`
2. Create `Dependencies/Shared/net/RateLimiter.cpp`
3. Implement per-second limits
4. Track per-message-type counts

**Files:**
- `Dependencies/Shared/net/RateLimiter.h` (new)
- `Dependencies/Shared/net/RateLimiter.cpp` (new)

**Verification:**
- [ ] Allows normal traffic
- [ ] Blocks excessive traffic
- [ ] Resets each period

**Commit Message:** `feat(net): add RateLimiter for traffic control`

---

## Micro-Phase 4.10: Create MessageDispatcher

**Goal:** Message routing

**Changes:**
1. Create `Dependencies/Shared/net/MessageDispatcher.h`
2. Create `Dependencies/Shared/net/MessageDispatcher.cpp`
3. Register handlers by message ID
4. Process incoming buffer

**Files:**
- `Dependencies/Shared/net/MessageDispatcher.h` (new)
- `Dependencies/Shared/net/MessageDispatcher.cpp` (new)

**Verification:**
- [ ] Handlers registered
- [ ] Messages dispatched correctly
- [ ] Invalid messages counted

**Commit Message:** `feat(net): add MessageDispatcher for routing`

---

## Micro-Phase 4.11: Create IServerApp Interface

**Goal:** Define server application contract

**Changes:**
1. Create `Sources/Server/engine/core/IServerApp.h`
2. Define onInit, onShutdown, onTick
3. Different from client (no render)

**Files:**
- `Sources/Server/engine/core/IServerApp.h` (new)

**Verification:**
- [ ] Interface compiles

**Commit Message:** `feat(server/engine): add IServerApp interface`

---

## Micro-Phase 4.12: Create ServerEngine Header

**Goal:** Console server engine

**Changes:**
1. Create `Sources/Server/engine/core/ServerEngine.h`
2. Define ServerConfig struct
3. Define singleton ServerEngine
4. No window, no GUI

**Files:**
- `Sources/Server/engine/core/ServerEngine.h` (new)

**Verification:**
- [ ] Header compiles
- [ ] No GUI dependencies

**Commit Message:** `feat(server/engine): add ServerEngine header`

---

## Micro-Phase 4.13: Implement ServerEngine

**Goal:** Console-based main loop

**Changes:**
1. Create `Sources/Server/engine/core/ServerEngine.cpp`
2. Implement initialize() without window
3. Implement run() with fixed timestep
4. Implement requestShutdown()

**Files:**
- `Sources/Server/engine/core/ServerEngine.cpp` (new)

**Verification:**
- [ ] Runs without window
- [ ] Fixed timestep works
- [ ] Shutdown graceful

**Commit Message:** `feat(server/engine): implement ServerEngine`

---

## Micro-Phase 4.14: Create LegacyServerAdapter

**Goal:** Wrap legacy server

**Changes:**
1. Create `Sources/Server/game/app/LegacyServerAdapter.h`
2. Create `Sources/Server/game/app/LegacyServerAdapter.cpp`
3. Wrap CGame for server
4. Implement onTick

**Files:**
- `Sources/Server/game/app/LegacyServerAdapter.h` (new)
- `Sources/Server/game/app/LegacyServerAdapter.cpp` (new)

**Verification:**
- [ ] Server logic runs
- [ ] Timer equivalent behavior

**Commit Message:** `feat(server/game): add LegacyServerAdapter`

---

## Micro-Phase 4.15: Create Console Entry Point

**Goal:** Main function for server

**Changes:**
1. Create `Sources/Server/engine/platform/win32/ConsoleMain.cpp`
2. Set console title
3. Handle Ctrl+C
4. Use ServerEngine

**Files:**
- `Sources/Server/engine/platform/win32/ConsoleMain.cpp` (new)

**Verification:**
- [ ] Compiles as console app
- [ ] Ctrl+C handled
- [ ] No window created

**Commit Message:** `feat(server/platform): add console entry point`

---

## Micro-Phase 4.16: Switch Server to Console

**Goal:** Remove server GUI

**Changes:**
1. Change subsystem to Console in project
2. Switch to ConsoleMain entry point
3. Remove old window code from build
4. Keep old code for reference (don't delete)

**Files:**
- `Sources/Server/HGserver.vcxproj` (modify)

**Verification:**
- [ ] Server runs as console
- [ ] No window appears
- [ ] All functionality works

**Commit Message:** `refactor(server): switch to console mode`

---

## Micro-Phase 4.17: Parallel Socket Testing

**Goal:** Test new socket with legacy protocol

**Changes:**
1. Add flag to switch between XSocket and new socket
2. Test login flow with new socket
3. Test game connection
4. Compare behavior

**Files:**
- `Sources/Client/game/app/LegacyGameAdapter.cpp` (modify)

**Verification:**
- [ ] Login works with new socket
- [ ] Game connects properly
- [ ] No protocol changes

**Commit Message:** `test(client/net): parallel test new socket`

---

## Micro-Phase 4.18: Remove XSocket

**Goal:** Complete migration

**Changes:**
1. Remove XSocket from build
2. Use new socket everywhere
3. Remove window message handlers for network
4. Clean up unused code

**Files:**
- `Sources/Client/Client.vcxproj` (modify)
- `Sources/Server/HGserver.vcxproj` (modify)

**Verification:**
- [ ] No XSocket references
- [ ] All networking works
- [ ] Message pump not needed for network

**Commit Message:** `refactor(net): remove legacy XSocket`

---

## Phase 4 Completion Checklist

After all micro-phases:
- [ ] ClientSocket implements INetSocket
- [ ] Server runs console-only
- [ ] No window messages for networking
- [ ] XSocket removed
- [ ] Rate limiting functional
- [ ] Metrics available
- [ ] Ready for Phase 5
