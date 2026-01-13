/**
 * @file Types.h
 * @brief Portable type definitions for cross-platform compatibility
 *
 * This file provides portable type definitions that must be used throughout
 * the codebase instead of Windows-specific types (DWORD, WORD, BYTE, etc.)
 *
 * Windows types should ONLY be used at the Windows API boundary layer.
 */

#pragma once

#include <cstdint>
#include <cstddef>
#include <string>
#include <string_view>
#include <vector>
#include <span>
#include <optional>
#include <array>

namespace Helbreath {

// =============================================================================
// Portable Integer Types
// =============================================================================

using u8  = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;
using u64 = std::uint64_t;

using i8  = std::int8_t;
using i16 = std::int16_t;
using i32 = std::int32_t;
using i64 = std::int64_t;

using f32 = float;
using f64 = double;

using usize = std::size_t;
using isize = std::ptrdiff_t;

// =============================================================================
// Type Aliases for Semantic Clarity
// =============================================================================

using ClientId   = u16;
using NpcId      = u16;
using ItemId     = u16;
using MapId      = u8;
using MagicId    = u8;
using SkillId    = u8;
using QuestId    = u16;
using GuildId    = u16;
using PartyId    = u16;
using MessageId  = u32;

// Entity handle (combines type and id)
struct EntityHandle {
    enum class Type : u8 {
        None = 0,
        Player = 1,
        Npc = 2,
        Item = 3,
        DynamicObject = 4
    };

    Type type = Type::None;
    u16 id = 0;

    [[nodiscard]] constexpr bool isValid() const noexcept {
        return type != Type::None && id != 0;
    }

    [[nodiscard]] constexpr bool operator==(const EntityHandle& other) const noexcept {
        return type == other.type && id == other.id;
    }
};

// =============================================================================
// Coordinate Types
// =============================================================================

struct Point2D {
    i16 x = 0;
    i16 y = 0;

    [[nodiscard]] constexpr bool operator==(const Point2D& other) const noexcept {
        return x == other.x && y == other.y;
    }
};

struct Rect {
    i16 left = 0;
    i16 top = 0;
    i16 right = 0;
    i16 bottom = 0;

    [[nodiscard]] constexpr i16 width() const noexcept { return right - left; }
    [[nodiscard]] constexpr i16 height() const noexcept { return bottom - top; }
    [[nodiscard]] constexpr bool contains(Point2D pt) const noexcept {
        return pt.x >= left && pt.x < right && pt.y >= top && pt.y < bottom;
    }
};

// =============================================================================
// String Types
// =============================================================================

// Fixed-size string buffer for network protocol (null-terminated)
template<usize N>
class FixedString {
public:
    static constexpr usize MaxLength = N - 1; // Reserve space for null terminator

    FixedString() noexcept { m_data[0] = '\0'; }

    explicit FixedString(std::string_view sv) noexcept {
        assign(sv);
    }

    void assign(std::string_view sv) noexcept {
        const usize len = (sv.length() < MaxLength) ? sv.length() : MaxLength;
        std::copy_n(sv.data(), len, m_data.data());
        m_data[len] = '\0';
    }

    [[nodiscard]] const char* c_str() const noexcept { return m_data.data(); }
    [[nodiscard]] std::string_view view() const noexcept { return {m_data.data()}; }
    [[nodiscard]] usize length() const noexcept { return view().length(); }
    [[nodiscard]] bool empty() const noexcept { return m_data[0] == '\0'; }

    [[nodiscard]] const std::array<char, N>& data() const noexcept { return m_data; }
    [[nodiscard]] std::array<char, N>& data() noexcept { return m_data; }

private:
    std::array<char, N> m_data{};
};

// Common fixed string sizes used in the protocol
using CharacterName = FixedString<11>;   // 10 chars + null
using AccountName   = FixedString<11>;   // 10 chars + null
using MapName       = FixedString<12>;   // 11 chars + null
using GuildName     = FixedString<21>;   // 20 chars + null
using ItemName      = FixedString<21>;   // 20 chars + null

// =============================================================================
// Buffer Types
// =============================================================================

using ByteBuffer = std::vector<std::byte>;
using ByteSpan = std::span<const std::byte>;
using MutableByteSpan = std::span<std::byte>;

// =============================================================================
// Result Types
// =============================================================================

template<typename T, typename E = std::string>
using Result = std::variant<T, E>;

// =============================================================================
// Constants
// =============================================================================

namespace Limits {
    constexpr u16 MaxClients = 2000;
    constexpr u16 MaxNpcs = 5000;
    constexpr u8  MaxMaps = 100;
    constexpr u16 MaxItems = 5000;
    constexpr u16 MaxGuilds = 1000;
    constexpr u16 MaxParties = 5000;
    constexpr u8  MaxInventorySlots = 50;
    constexpr u8  MaxBankSlots = 120;
    constexpr u8  MaxEquipmentSlots = 15;
    constexpr u8  MaxMagicTypes = 100;
    constexpr u8  MaxSkillTypes = 24;
    constexpr u16 MaxQuestTypes = 200;
    constexpr u32 MaxMessageQueueSize = 100000;
    constexpr u16 MaxDynamicObjects = 60000;
    constexpr u16 MaxLevel = 376;
    constexpr u16 MaxStatPoints = 200;
}

// =============================================================================
// Windows Type Conversion Helpers (Only use at boundary layer)
// =============================================================================

#ifdef _WIN32
#include <windows.h>

namespace Win32Convert {
    [[nodiscard]] inline u32 fromDWORD(DWORD val) noexcept { return static_cast<u32>(val); }
    [[nodiscard]] inline u16 fromWORD(WORD val) noexcept { return static_cast<u16>(val); }
    [[nodiscard]] inline u8  fromBYTE(BYTE val) noexcept { return static_cast<u8>(val); }

    [[nodiscard]] inline DWORD toDWORD(u32 val) noexcept { return static_cast<DWORD>(val); }
    [[nodiscard]] inline WORD  toWORD(u16 val) noexcept { return static_cast<WORD>(val); }
    [[nodiscard]] inline BYTE  toBYTE(u8 val) noexcept { return static_cast<BYTE>(val); }
}
#endif

} // namespace Helbreath
