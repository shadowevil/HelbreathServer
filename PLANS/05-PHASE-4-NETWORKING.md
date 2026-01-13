# Phase 4: Networking Upgrade

## Overview

This phase replaces the legacy XSocket (WinSock 1.1 + Window Message Pump) with a modern WinSock2-based networking layer. The server becomes console-only, and the client uses non-blocking sockets without relying on window messages.

**Prerequisite:** Phase 3 complete (Systems separated)

---

## Goals

1. Replace XSocket with modern WinSock2 wrapper
2. Remove networking dependency on Window Message Pump
3. Create console-only server (no GUI)
4. Implement proper message serialization with validation
5. Add rate limiting and traffic optimization
6. Add network metrics and debugging

---

## Current Problems

### Problem 1: Window Message Dependency
```cpp
// Legacy XSocket uses WSAAsyncSelect which sends WM_USER messages
WSAAsyncSelect(socket, hwnd, WM_ONCLIENTSOCKETEVENT + clientId, FD_READ | FD_CLOSE);

// Requires a window handle and message pump
case WM_ONCLIENTSOCKETEVENT:
    G_pGame->OnClientSocketEvent(message, wParam, lParam);
```

### Problem 2: No Validation
```cpp
// Messages read directly without bounds checking
memcpy(&data, buffer + offset, sizeof(data));  // Buffer overflow risk
```

### Problem 3: Blocking Patterns
```cpp
// Some operations block the main thread
while (recv(socket, buffer, size, 0) > 0) { }
```

### Problem 4: Server has GUI Window
```cpp
// Server creates unnecessary window for message pump
G_hWnd = CreateWindowEx(...);  // Not needed for game logic
```

---

## Architecture

```
┌─────────────────────────────────────────────────────────┐
│                    INetSocket (Interface)                │
│  connect(), disconnect(), send(), receive(), poll()     │
└─────────────────────────────────────────────────────────┘
                              │
              ┌───────────────┴───────────────┐
              │                               │
              ▼                               ▼
┌─────────────────────────┐     ┌─────────────────────────┐
│   Win32Socket (Client)   │     │  Win32Socket (Server)   │
│  - Non-blocking select   │     │  - IOCP or select       │
│  - Event-driven          │     │  - Multiple connections │
│  - No window messages    │     │  - Console mode         │
└─────────────────────────┘     └─────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────┐
│               MessageDispatcher                          │
│  - Deserialize messages                                  │
│  - Validate message format                               │
│  - Route to handlers                                     │
│  - Rate limiting                                         │
└─────────────────────────────────────────────────────────┘
```

---

## Directory Structure

```
/Dependencies/Shared/
├── net/
│   ├── INetSocket.h           # Socket interface
│   ├── MessageDispatcher.h    # Message routing
│   ├── MessageQueue.h         # Thread-safe queue
│   ├── RateLimiter.h          # Traffic control
│   └── NetMetrics.h           # Statistics

/Sources/Client/
├── engine/
│   └── net/
│       ├── ClientSocket.h
│       └── ClientSocket.cpp

/Sources/Server/
├── engine/
│   └── net/
│       ├── ServerSocket.h
│       ├── ServerSocket.cpp
│       ├── ClientConnection.h
│       └── ConnectionManager.h
```

---

## Tasks

### Task 4.1: Create Socket Interface

**File to Create:**
```
/Dependencies/Shared/net/INetSocket.h
```

**INetSocket.h:**
```cpp
#pragma once

#include "../Types.h"
#include <span>
#include <functional>
#include <string>

namespace Helbreath::Net {

enum class SocketState : u8 {
    Disconnected,
    Connecting,
    Connected,
    Error
};

enum class SocketError : u8 {
    None,
    ConnectionRefused,
    ConnectionReset,
    Timeout,
    NetworkUnreachable,
    Unknown
};

struct SocketEvent {
    enum class Type : u8 {
        Connected,
        Disconnected,
        DataReceived,
        Error
    };

    Type type;
    SocketError error = SocketError::None;
};

using SocketEventCallback = std::function<void(const SocketEvent&)>;

/**
 * Abstract socket interface.
 * Platform-specific implementations provide the actual networking.
 */
class INetSocket {
public:
    virtual ~INetSocket() = default;

    // Connection management
    [[nodiscard]] virtual bool connect(std::string_view host, u16 port) = 0;
    virtual void disconnect() = 0;
    [[nodiscard]] virtual SocketState state() const = 0;

    // Data transfer
    [[nodiscard]] virtual bool send(ByteSpan data) = 0;
    [[nodiscard]] virtual usize receive(MutableByteSpan buffer) = 0;

    // Non-blocking poll (call each frame)
    virtual void poll() = 0;

    // Event handling
    virtual void setEventCallback(SocketEventCallback callback) = 0;

    // Statistics
    [[nodiscard]] virtual u64 bytesSent() const = 0;
    [[nodiscard]] virtual u64 bytesReceived() const = 0;
};

} // namespace Helbreath::Net
```

---

### Task 4.2: Implement WinSock2 Client Socket

**Files to Create:**
```
/Sources/Client/engine/net/ClientSocket.h
/Sources/Client/engine/net/ClientSocket.cpp
```

**ClientSocket.h:**
```cpp
#pragma once

#include <Dependencies/Shared/net/INetSocket.h>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#endif

namespace Helbreath::Net {

/**
 * WinSock2 client socket implementation.
 * Uses non-blocking sockets with select() for polling.
 * Does NOT use window messages.
 */
class ClientSocket : public INetSocket {
public:
    ClientSocket();
    ~ClientSocket() override;

    // INetSocket implementation
    [[nodiscard]] bool connect(std::string_view host, u16 port) override;
    void disconnect() override;
    [[nodiscard]] SocketState state() const override { return m_state; }

    [[nodiscard]] bool send(ByteSpan data) override;
    [[nodiscard]] usize receive(MutableByteSpan buffer) override;

    void poll() override;

    void setEventCallback(SocketEventCallback callback) override {
        m_callback = std::move(callback);
    }

    [[nodiscard]] u64 bytesSent() const override { return m_bytesSent; }
    [[nodiscard]] u64 bytesReceived() const override { return m_bytesReceived; }

private:
    void checkConnectResult();
    void processIncoming();
    void fireEvent(SocketEvent::Type type, SocketError error = SocketError::None);

#ifdef _WIN32
    SOCKET m_socket = INVALID_SOCKET;
#endif

    SocketState m_state = SocketState::Disconnected;
    SocketEventCallback m_callback;

    // Send buffer for coalescing small writes
    ByteBuffer m_sendBuffer;
    usize m_sendBufferPos = 0;

    // Statistics
    u64 m_bytesSent = 0;
    u64 m_bytesReceived = 0;
};

} // namespace Helbreath::Net
```

**ClientSocket.cpp (Key Parts):**
```cpp
#include "ClientSocket.h"
#include <Dependencies/Shared/core/Log.h>

#ifdef _WIN32
#pragma comment(lib, "ws2_32.lib")
#endif

namespace Helbreath::Net {

ClientSocket::ClientSocket() {
#ifdef _WIN32
    // Initialize WinSock if needed
    static bool wsaInitialized = false;
    if (!wsaInitialized) {
        WSADATA wsaData;
        WSAStartup(MAKEWORD(2, 2), &wsaData);
        wsaInitialized = true;
    }
#endif
}

bool ClientSocket::connect(std::string_view host, u16 port) {
#ifdef _WIN32
    // Create socket
    m_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (m_socket == INVALID_SOCKET) {
        LOG_ERROR("Failed to create socket");
        return false;
    }

    // Set non-blocking mode
    u_long mode = 1;
    ioctlsocket(m_socket, FIONBIO, &mode);

    // Resolve address
    addrinfo hints = {};
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    addrinfo* result = nullptr;
    std::string portStr = std::to_string(port);
    if (getaddrinfo(std::string(host).c_str(), portStr.c_str(), &hints, &result) != 0) {
        LOG_ERROR("Failed to resolve host: {}", host);
        closesocket(m_socket);
        m_socket = INVALID_SOCKET;
        return false;
    }

    // Initiate connection (non-blocking)
    int connectResult = ::connect(m_socket, result->ai_addr,
                                  static_cast<int>(result->ai_addrlen));
    freeaddrinfo(result);

    if (connectResult == SOCKET_ERROR) {
        int error = WSAGetLastError();
        if (error != WSAEWOULDBLOCK) {
            LOG_ERROR("Connect failed with error: {}", error);
            closesocket(m_socket);
            m_socket = INVALID_SOCKET;
            return false;
        }
    }

    m_state = SocketState::Connecting;
    LOG_INFO("Connecting to {}:{}", host, port);
    return true;
#else
    return false;
#endif
}

void ClientSocket::poll() {
    if (m_socket == INVALID_SOCKET) return;

#ifdef _WIN32
    fd_set readSet, writeSet, exceptSet;
    FD_ZERO(&readSet);
    FD_ZERO(&writeSet);
    FD_ZERO(&exceptSet);
    FD_SET(m_socket, &readSet);
    FD_SET(m_socket, &writeSet);
    FD_SET(m_socket, &exceptSet);

    timeval timeout = {0, 0};  // Non-blocking

    int result = select(0, &readSet, &writeSet, &exceptSet, &timeout);

    if (result > 0) {
        // Check for connection completion
        if (m_state == SocketState::Connecting) {
            if (FD_ISSET(m_socket, &writeSet)) {
                checkConnectResult();
            }
            if (FD_ISSET(m_socket, &exceptSet)) {
                m_state = SocketState::Error;
                fireEvent(SocketEvent::Type::Error, SocketError::ConnectionRefused);
            }
        }

        // Check for incoming data
        if (m_state == SocketState::Connected && FD_ISSET(m_socket, &readSet)) {
            fireEvent(SocketEvent::Type::DataReceived);
        }
    }
#endif
}

} // namespace Helbreath::Net
```

---

### Task 4.3: Create Message Dispatcher

**Files to Create:**
```
/Dependencies/Shared/net/MessageDispatcher.h
/Dependencies/Shared/net/MessageDispatcher.cpp
```

**MessageDispatcher.h:**
```cpp
#pragma once

#include "MessageTypes.h"
#include "Serialization.h"
#include <functional>
#include <unordered_map>

namespace Helbreath::Net {

using MessageHandler = std::function<void(BinaryReader&)>;

/**
 * Routes incoming messages to registered handlers.
 * Validates message format and provides rate limiting.
 */
class MessageDispatcher {
public:
    // Register a handler for a specific message ID
    void registerHandler(u32 messageId, MessageHandler handler);

    // Process incoming data buffer
    void processBuffer(ByteSpan data);

    // Statistics
    [[nodiscard]] u32 messagesProcessed() const { return m_messagesProcessed; }
    [[nodiscard]] u32 messagesDropped() const { return m_messagesDropped; }
    [[nodiscard]] u32 invalidMessages() const { return m_invalidMessages; }

    void resetStats() {
        m_messagesProcessed = 0;
        m_messagesDropped = 0;
        m_invalidMessages = 0;
    }

private:
    std::unordered_map<u32, MessageHandler> m_handlers;

    u32 m_messagesProcessed = 0;
    u32 m_messagesDropped = 0;
    u32 m_invalidMessages = 0;

    // Partial message buffer (for messages split across packets)
    ByteBuffer m_partialBuffer;
};

} // namespace Helbreath::Net
```

---

### Task 4.4: Create Rate Limiter

**File to Create:**
```
/Dependencies/Shared/net/RateLimiter.h
```

**RateLimiter.h:**
```cpp
#pragma once

#include "../Types.h"
#include <chrono>
#include <unordered_map>

namespace Helbreath::Net {

/**
 * Rate limiter for network messages.
 * Prevents spam and detects suspicious activity.
 */
class RateLimiter {
public:
    struct Config {
        u32 maxMessagesPerSecond = 100;
        u32 maxBytesPerSecond = 10000;
        u32 burstAllowance = 20;  // Extra messages allowed in bursts
    };

    explicit RateLimiter(const Config& config = {});

    // Check if a message should be allowed
    [[nodiscard]] bool allowMessage(u32 messageId, usize messageSize);

    // Reset limits (call periodically, e.g., every second)
    void resetPeriod();

    // Check if client is being rate limited
    [[nodiscard]] bool isLimited() const { return m_isLimited; }

    // Statistics
    [[nodiscard]] u32 messagesThisSecond() const { return m_messagesThisSecond; }
    [[nodiscard]] u32 bytesThisSecond() const { return m_bytesThisSecond; }

private:
    Config m_config;

    u32 m_messagesThisSecond = 0;
    u32 m_bytesThisSecond = 0;
    bool m_isLimited = false;

    // Per-message-type rate limiting
    std::unordered_map<u32, u32> m_messageTypeCounts;
};

} // namespace Helbreath::Net
```

---

### Task 4.5: Create Console Server

**Files to Create:**
```
/Sources/Server/engine/platform/win32/ConsoleMain.cpp
```

**ConsoleMain.cpp:**
```cpp
#include "../../../game/app/LegacyServerAdapter.h"
#include "../../core/ServerEngine.h"
#include <Dependencies/Shared/core/Log.h>

#ifdef _WIN32
#include <windows.h>

// Console control handler
BOOL WINAPI ConsoleHandler(DWORD signal) {
    if (signal == CTRL_C_EVENT || signal == CTRL_CLOSE_EVENT) {
        LOG_INFO("Shutdown signal received");
        Helbreath::Engine::ServerEngine::instance().requestShutdown();
        return TRUE;
    }
    return FALSE;
}

int main(int argc, char* argv[]) {
    // Set console control handler
    SetConsoleCtrlHandler(ConsoleHandler, TRUE);

    // Set console title
    SetConsoleTitle("Helbreath Game Server");

    // Initialize logging
    Helbreath::Core::Logger::instance().setOutputFile("server.log");
    Helbreath::Core::Logger::instance().enableConsole(true);
    Helbreath::Core::Logger::instance().setLevel(Helbreath::Core::LogLevel::Info);

    LOG_INFO("========================================");
    LOG_INFO("   Helbreath Game Server Starting");
    LOG_INFO("========================================");

    // Configure server
    Helbreath::Engine::ServerConfig config;
    config.gamePort = 9907;
    config.loginPort = 2500;
    config.tickRate = 30;  // 30 ticks per second (33ms)

    // Initialize server engine
    auto& engine = Helbreath::Engine::ServerEngine::instance();
    if (!engine.initialize(config)) {
        LOG_FATAL("Server initialization failed");
        return 1;
    }

    // Create and run server
    Helbreath::Game::LegacyServerAdapter server;
    engine.run(server);

    // Cleanup
    engine.shutdown();

    LOG_INFO("Server shutdown complete");
    return 0;
}

#endif // _WIN32
```

---

### Task 4.6: Create Server Engine (No GUI)

**Files to Create:**
```
/Sources/Server/engine/core/ServerEngine.h
/Sources/Server/engine/core/ServerEngine.cpp
```

**ServerEngine.h:**
```cpp
#pragma once

#include "../../game/app/IServerApp.h"
#include <Dependencies/Shared/core/Clock.h>

namespace Helbreath::Engine {

struct ServerConfig {
    u16 gamePort = 9907;
    u16 loginPort = 2500;
    u32 tickRate = 30;  // Ticks per second
    u32 maxClients = 2000;
};

/**
 * Console-only server engine.
 * No window, no GUI - pure networking and game logic.
 */
class ServerEngine {
public:
    static ServerEngine& instance();

    [[nodiscard]] bool initialize(const ServerConfig& config);
    void shutdown();

    void run(IServerApp& app);
    void requestShutdown();

    [[nodiscard]] bool isRunning() const noexcept { return m_running; }
    [[nodiscard]] const Core::Clock& clock() const noexcept { return m_clock; }
    [[nodiscard]] const ServerConfig& config() const noexcept { return m_config; }

private:
    ServerEngine() = default;

    void serverLoop(IServerApp& app);

    ServerConfig m_config;
    Core::Clock m_clock;
    bool m_running = false;

    // Fixed timestep accumulator
    f64 m_accumulator = 0.0;
    f64 m_tickInterval = 1.0 / 30.0;  // 33ms default
};

} // namespace Helbreath::Engine
```

**ServerEngine.cpp (Key Parts):**
```cpp
void ServerEngine::run(IServerApp& app) {
    m_running = true;

    if (!app.onInit(*this)) {
        LOG_ERROR("Server application initialization failed");
        m_running = false;
        return;
    }

    LOG_INFO("Server entering main loop (tick rate: {} Hz)", m_config.tickRate);

    m_tickInterval = 1.0 / m_config.tickRate;

    while (m_running) {
        serverLoop(app);
    }

    LOG_INFO("Server exiting main loop");
    app.onShutdown();
}

void ServerEngine::serverLoop(IServerApp& app) {
    m_clock.tick();
    m_accumulator += m_clock.deltaTimeSeconds();

    // Fixed timestep updates
    while (m_accumulator >= m_tickInterval) {
        app.onTick(m_tickInterval);
        m_accumulator -= m_tickInterval;
    }

    // Small sleep to prevent CPU spinning
    // Could use more sophisticated waiting
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
}
```

---

### Task 4.7: Create Network Metrics

**File to Create:**
```
/Dependencies/Shared/net/NetMetrics.h
```

**NetMetrics.h:**
```cpp
#pragma once

#include "../Types.h"
#include <atomic>
#include <array>

namespace Helbreath::Net {

/**
 * Network performance metrics for monitoring and debugging.
 */
class NetMetrics {
public:
    static NetMetrics& instance();

    // Message statistics
    void recordMessageSent(u32 messageId, usize size);
    void recordMessageReceived(u32 messageId, usize size);
    void recordMessageDropped(u32 messageId);
    void recordInvalidMessage();

    // Getters
    [[nodiscard]] u64 totalMessagesSent() const { return m_messagesSent.load(); }
    [[nodiscard]] u64 totalMessagesReceived() const { return m_messagesReceived.load(); }
    [[nodiscard]] u64 totalBytesSent() const { return m_bytesSent.load(); }
    [[nodiscard]] u64 totalBytesReceived() const { return m_bytesReceived.load(); }
    [[nodiscard]] u64 messagesDropped() const { return m_messagesDropped.load(); }
    [[nodiscard]] u64 invalidMessages() const { return m_invalidMessages.load(); }

    // Per-second rates (call updateRates() each second)
    void updateRates();
    [[nodiscard]] f64 messagesPerSecond() const { return m_messagesPerSecond; }
    [[nodiscard]] f64 bytesPerSecond() const { return m_bytesPerSecond; }

    // Reset all metrics
    void reset();

    // Format as string for display
    [[nodiscard]] std::string toString() const;

private:
    NetMetrics() = default;

    std::atomic<u64> m_messagesSent{0};
    std::atomic<u64> m_messagesReceived{0};
    std::atomic<u64> m_bytesSent{0};
    std::atomic<u64> m_bytesReceived{0};
    std::atomic<u64> m_messagesDropped{0};
    std::atomic<u64> m_invalidMessages{0};

    // Rate calculation
    u64 m_lastMessageCount = 0;
    u64 m_lastByteCount = 0;
    f64 m_messagesPerSecond = 0.0;
    f64 m_bytesPerSecond = 0.0;
};

} // namespace Helbreath::Net
```

---

### Task 4.8: Traffic Optimization

**Strategies to Implement:**

1. **Message Coalescing**
   - Buffer small messages and send together
   - Reduces TCP overhead

2. **Delta Compression**
   - Only send changed values
   - Track last sent state per client

3. **Priority Queue**
   - Critical messages (combat) sent immediately
   - Low priority (chat) can be delayed

4. **Tick Rate Control**
   - Server sends updates at fixed rate
   - Don't flood with every change

---

## Migration Strategy

### Phase 4a: Create New Socket Layer
1. Implement ClientSocket and ServerSocket
2. Test with simple echo server
3. Verify non-blocking operation

### Phase 4b: Parallel Operation
1. Add new socket alongside XSocket
2. Route new message types through new socket
3. Keep legacy messages on XSocket

### Phase 4c: Full Migration
1. Move all messages to new socket
2. Remove XSocket
3. Remove window message dependencies

### Phase 4d: Server Console Migration
1. Create console server entry point
2. Remove GUI code from server
3. Test full server functionality

---

## Verification

### Functional Tests
- [ ] Client connects to server
- [ ] Login process works
- [ ] Character selection works
- [ ] Game session establishes
- [ ] All message types work

### Performance Tests
- [ ] Message throughput meets requirements
- [ ] Latency acceptable
- [ ] No message loss
- [ ] Rate limiting works

### Stability Tests
- [ ] Handle reconnection
- [ ] Handle network errors gracefully
- [ ] No memory leaks
- [ ] Long-running stability

---

## Success Criteria

Phase 4 is complete when:

1. **XSocket removed** from both client and server
2. **Server runs console-only** without any window
3. **All network features work** identically to before
4. **Rate limiting** prevents message spam
5. **Metrics available** for monitoring

---

## Next Phase

Once Phase 4 is complete, proceed to:
- **Phase 5: UI + ECS Migration**
