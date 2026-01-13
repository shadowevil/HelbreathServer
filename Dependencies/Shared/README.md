# Shared Dependencies

This folder contains code that **must be kept in sync** between the client and server.

## Directory Structure

```
Shared/
├── Types.h              # Portable type definitions
├── net/                 # Network protocol
│   ├── MessageTypes.h   # Message ID enums (organized)
│   ├── Serialization.h  # Binary serialization utilities
│   ├── NetMessages.h    # Legacy message IDs (compatibility)
│   └── Msg.h/cpp        # Legacy message class
├── game/                # Game definitions
│   ├── GlobalDef.h      # Game constants
│   ├── ActionID.h       # Action identifiers
│   └── DynamicObjectID.h # Dynamic object types
└── util/                # Utility code
    ├── StrTok.h/cpp     # String tokenization
    └── Misc.h/cpp       # Miscellaneous utilities
```

## Synchronization Rules

### 1. Protocol Version
When changing message formats or IDs, you MUST:
- Increment `PROTOCOL_VERSION` in `net/MessageTypes.h`
- Update both client and server simultaneously
- Document the change in the commit message

### 2. Building
Both client and server projects must include these shared files:
```
Include Path: ../../Dependencies/Shared
```

### 3. Type Safety
- Use types from `Types.h` instead of Windows types
- Use `enum class` instead of raw `#define` where possible
- Use `BinaryReader`/`BinaryWriter` for serialization

### 4. Compatibility
The legacy files (`NetMessages.h`, `Msg.h`) are provided for backward compatibility during migration. New code should use the modern equivalents:
- `NetMessages.h` → `MessageTypes.h`
- Raw serialization → `Serialization.h`

## Adding New Shared Code

1. Create the file in the appropriate subdirectory
2. Use `#pragma once` for header guards
3. Use the `Helbreath` namespace
4. Document any protocol changes
5. Update both client and server projects
