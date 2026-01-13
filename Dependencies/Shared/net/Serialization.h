/**
 * @file Serialization.h
 * @brief Binary serialization/deserialization utilities for network protocol
 *
 * Provides type-safe, bounds-checked serialization for network messages.
 * All multi-byte integers use little-endian byte order (x86 native).
 */

#pragma once

#include "../Types.h"
#include <bit>
#include <stdexcept>
#include <algorithm>

namespace Helbreath::Net {

// =============================================================================
// Serialization Error
// =============================================================================

class SerializationError : public std::runtime_error {
public:
    explicit SerializationError(const std::string& msg)
        : std::runtime_error(msg) {}
};

// =============================================================================
// Binary Writer - Writes data to a buffer
// =============================================================================

class BinaryWriter {
public:
    explicit BinaryWriter(ByteBuffer& buffer) noexcept
        : m_buffer(buffer) {}

    // Reserve space for the message header (size prefix)
    void reserveHeader(usize headerSize) {
        m_buffer.resize(headerSize);
        m_position = headerSize;
    }

    // Write primitive types
    void writeU8(u8 value) {
        ensureCapacity(1);
        m_buffer[m_position++] = static_cast<std::byte>(value);
    }

    void writeI8(i8 value) {
        writeU8(static_cast<u8>(value));
    }

    void writeU16(u16 value) {
        ensureCapacity(2);
        m_buffer[m_position++] = static_cast<std::byte>(value & 0xFF);
        m_buffer[m_position++] = static_cast<std::byte>((value >> 8) & 0xFF);
    }

    void writeI16(i16 value) {
        writeU16(static_cast<u16>(value));
    }

    void writeU32(u32 value) {
        ensureCapacity(4);
        m_buffer[m_position++] = static_cast<std::byte>(value & 0xFF);
        m_buffer[m_position++] = static_cast<std::byte>((value >> 8) & 0xFF);
        m_buffer[m_position++] = static_cast<std::byte>((value >> 16) & 0xFF);
        m_buffer[m_position++] = static_cast<std::byte>((value >> 24) & 0xFF);
    }

    void writeI32(i32 value) {
        writeU32(static_cast<u32>(value));
    }

    void writeU64(u64 value) {
        ensureCapacity(8);
        for (int i = 0; i < 8; ++i) {
            m_buffer[m_position++] = static_cast<std::byte>((value >> (i * 8)) & 0xFF);
        }
    }

    void writeI64(i64 value) {
        writeU64(static_cast<u64>(value));
    }

    void writeF32(f32 value) {
        writeU32(std::bit_cast<u32>(value));
    }

    void writeF64(f64 value) {
        writeU64(std::bit_cast<u64>(value));
    }

    // Write fixed-size string (null-padded to size)
    template<usize N>
    void writeFixedString(const FixedString<N>& str) {
        ensureCapacity(N);
        const auto& data = str.data();
        for (usize i = 0; i < N; ++i) {
            m_buffer[m_position++] = static_cast<std::byte>(data[i]);
        }
    }

    // Write raw string with length prefix
    void writeString(std::string_view str) {
        if (str.length() > 0xFFFF) {
            throw SerializationError("String too long for serialization");
        }
        writeU16(static_cast<u16>(str.length()));
        ensureCapacity(str.length());
        for (char c : str) {
            m_buffer[m_position++] = static_cast<std::byte>(c);
        }
    }

    // Write raw bytes
    void writeBytes(ByteSpan data) {
        ensureCapacity(data.size());
        std::copy(data.begin(), data.end(), m_buffer.begin() + m_position);
        m_position += data.size();
    }

    // Write raw bytes with length prefix
    void writeBytesWithLength(ByteSpan data) {
        if (data.size() > 0xFFFF) {
            throw SerializationError("Data too long for serialization");
        }
        writeU16(static_cast<u16>(data.size()));
        writeBytes(data);
    }

    // Write point
    void writePoint(Point2D pt) {
        writeI16(pt.x);
        writeI16(pt.y);
    }

    // Get current position
    [[nodiscard]] usize position() const noexcept { return m_position; }

    // Get buffer reference
    [[nodiscard]] ByteBuffer& buffer() noexcept { return m_buffer; }
    [[nodiscard]] const ByteBuffer& buffer() const noexcept { return m_buffer; }

    // Finalize and write header with total size
    void finalizeWithSizeHeader() {
        u32 size = static_cast<u32>(m_buffer.size());
        // Write size at position 0 (assumes 4-byte header was reserved)
        m_buffer[0] = static_cast<std::byte>(size & 0xFF);
        m_buffer[1] = static_cast<std::byte>((size >> 8) & 0xFF);
        m_buffer[2] = static_cast<std::byte>((size >> 16) & 0xFF);
        m_buffer[3] = static_cast<std::byte>((size >> 24) & 0xFF);
    }

private:
    void ensureCapacity(usize additional) {
        if (m_position + additional > m_buffer.size()) {
            m_buffer.resize(m_position + additional);
        }
    }

    ByteBuffer& m_buffer;
    usize m_position = 0;
};

// =============================================================================
// Binary Reader - Reads data from a buffer
// =============================================================================

class BinaryReader {
public:
    explicit BinaryReader(ByteSpan data) noexcept
        : m_data(data) {}

    // Read primitive types
    [[nodiscard]] u8 readU8() {
        checkRemaining(1);
        return static_cast<u8>(m_data[m_position++]);
    }

    [[nodiscard]] i8 readI8() {
        return static_cast<i8>(readU8());
    }

    [[nodiscard]] u16 readU16() {
        checkRemaining(2);
        u16 value = static_cast<u8>(m_data[m_position++]);
        value |= static_cast<u16>(static_cast<u8>(m_data[m_position++])) << 8;
        return value;
    }

    [[nodiscard]] i16 readI16() {
        return static_cast<i16>(readU16());
    }

    [[nodiscard]] u32 readU32() {
        checkRemaining(4);
        u32 value = static_cast<u8>(m_data[m_position++]);
        value |= static_cast<u32>(static_cast<u8>(m_data[m_position++])) << 8;
        value |= static_cast<u32>(static_cast<u8>(m_data[m_position++])) << 16;
        value |= static_cast<u32>(static_cast<u8>(m_data[m_position++])) << 24;
        return value;
    }

    [[nodiscard]] i32 readI32() {
        return static_cast<i32>(readU32());
    }

    [[nodiscard]] u64 readU64() {
        checkRemaining(8);
        u64 value = 0;
        for (int i = 0; i < 8; ++i) {
            value |= static_cast<u64>(static_cast<u8>(m_data[m_position++])) << (i * 8);
        }
        return value;
    }

    [[nodiscard]] i64 readI64() {
        return static_cast<i64>(readU64());
    }

    [[nodiscard]] f32 readF32() {
        return std::bit_cast<f32>(readU32());
    }

    [[nodiscard]] f64 readF64() {
        return std::bit_cast<f64>(readU64());
    }

    // Read fixed-size string
    template<usize N>
    [[nodiscard]] FixedString<N> readFixedString() {
        checkRemaining(N);
        FixedString<N> result;
        auto& data = result.data();
        for (usize i = 0; i < N; ++i) {
            data[i] = static_cast<char>(m_data[m_position++]);
        }
        return result;
    }

    // Read length-prefixed string
    [[nodiscard]] std::string readString() {
        u16 length = readU16();
        checkRemaining(length);
        std::string result(length, '\0');
        for (u16 i = 0; i < length; ++i) {
            result[i] = static_cast<char>(m_data[m_position++]);
        }
        return result;
    }

    // Read raw bytes into vector
    [[nodiscard]] ByteBuffer readBytes(usize count) {
        checkRemaining(count);
        ByteBuffer result(count);
        std::copy(m_data.begin() + m_position,
                  m_data.begin() + m_position + count,
                  result.begin());
        m_position += count;
        return result;
    }

    // Read length-prefixed bytes
    [[nodiscard]] ByteBuffer readBytesWithLength() {
        u16 length = readU16();
        return readBytes(length);
    }

    // Read point
    [[nodiscard]] Point2D readPoint() {
        Point2D pt;
        pt.x = readI16();
        pt.y = readI16();
        return pt;
    }

    // Skip bytes
    void skip(usize count) {
        checkRemaining(count);
        m_position += count;
    }

    // Peek at next byte without consuming
    [[nodiscard]] u8 peekU8() const {
        if (m_position >= m_data.size()) {
            throw SerializationError("Buffer underflow on peek");
        }
        return static_cast<u8>(m_data[m_position]);
    }

    // Get current position
    [[nodiscard]] usize position() const noexcept { return m_position; }

    // Get remaining bytes
    [[nodiscard]] usize remaining() const noexcept {
        return m_data.size() - m_position;
    }

    // Check if at end
    [[nodiscard]] bool atEnd() const noexcept {
        return m_position >= m_data.size();
    }

    // Get remaining data as span
    [[nodiscard]] ByteSpan remainingData() const noexcept {
        return m_data.subspan(m_position);
    }

private:
    void checkRemaining(usize count) const {
        if (m_position + count > m_data.size()) {
            throw SerializationError("Buffer underflow: requested " +
                std::to_string(count) + " bytes, but only " +
                std::to_string(m_data.size() - m_position) + " available");
        }
    }

    ByteSpan m_data;
    usize m_position = 0;
};

// =============================================================================
// Message Header Helpers
// =============================================================================

struct MessageHeader {
    u32 messageId = 0;
    u16 messageType = 0;
    u32 totalSize = 0;

    static constexpr usize HeaderSize = sizeof(u32) + sizeof(u16);
};

inline MessageHeader readMessageHeader(BinaryReader& reader) {
    MessageHeader header;
    header.messageId = reader.readU32();
    header.messageType = reader.readU16();
    return header;
}

inline void writeMessageHeader(BinaryWriter& writer, u32 messageId, u16 messageType) {
    writer.writeU32(messageId);
    writer.writeU16(messageType);
}

// =============================================================================
// Validation Helpers
// =============================================================================

namespace Validation {

inline bool isValidClientId(u16 id) noexcept {
    return id > 0 && id <= Limits::MaxClients;
}

inline bool isValidNpcId(u16 id) noexcept {
    return id > 0 && id <= Limits::MaxNpcs;
}

inline bool isValidMapId(u8 id) noexcept {
    return id < Limits::MaxMaps;
}

inline bool isValidItemId(u16 id) noexcept {
    return id > 0 && id <= Limits::MaxItems;
}

inline bool isValidInventorySlot(u8 slot) noexcept {
    return slot < Limits::MaxInventorySlots;
}

inline bool isValidLevel(u16 level) noexcept {
    return level > 0 && level <= Limits::MaxLevel;
}

inline bool isValidStatValue(u16 value) noexcept {
    return value <= Limits::MaxStatPoints;
}

} // namespace Validation

} // namespace Helbreath::Net
