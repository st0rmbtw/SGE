#ifndef _SGE_UTILS_UTF8_HPP_
#define _SGE_UTILS_UTF8_HPP_

#include <cstdint>
#include <cstddef>

namespace sge {

/**
 * @brief Reads a UTF-8 codepoint from the buffer and converts it to a UTF-32 codepoint.
 * 
 * @param buffer The UTF-8 codepoint buffer.
 * @param codepoint The resulting UTF-32 codepoint.
 * @return The length of the read UTF-8 codepoint in bytes.
 */
inline uint8_t utf8_codepoint_to_utf32(const uint8_t* buffer, uint32_t& codepoint) noexcept {
    codepoint = (uint8_t) buffer[0];

    uint8_t cplen = 1;
    if ((buffer[0] & 0xf8) == 0xf0) {
        cplen = 4;

        const uint8_t b1 = (uint8_t) buffer[cplen - 1];
        const uint8_t b2 = (uint8_t) buffer[cplen - 2];
        const uint8_t b3 = (uint8_t) buffer[cplen - 3];
        const uint8_t b4 = (uint8_t) buffer[cplen - 4];

        const uint8_t x = (b2 & 0b00111100) >> 2;
        const uint8_t y = ((b1 & 0b00110000) >> 4) | (b2 & 0b00000011) << 2;
        const uint8_t z = b1 & 0b00001111;
        const uint8_t w = b3 & 0b00001111;
        const uint8_t v = ((b3 & 0b00110000) >> 4) | (b4 & 0b00000011);
        const uint8_t u = b4 & 0b00000100;

        codepoint = (u << 18) | (v << 16) | (w << 12) | (x << 8) | (y << 4) | z;
    } else if ((buffer[0] & 0xf0) == 0xe0) {
        cplen = 3;

        const uint8_t b1 = (uint8_t) buffer[cplen - 1];
        const uint8_t b2 = (uint8_t) buffer[cplen - 2];
        const uint8_t b3 = (uint8_t) buffer[cplen - 3];

        const uint8_t x = (b2 & 0b00111100) >> 2;
        const uint8_t y = ((b1 & 0b00110000) >> 4) | (b2 & 0b00000011) << 2;
        const uint8_t z = b1 & 0b00001111;
        const uint8_t w = b3 & 0b00001111;

        codepoint = (w << 12) | (x << 8) | (y << 4) | z;
    } else if ((buffer[0] & 0xe0) == 0xc0) {
        cplen = 2;

        const uint8_t b1 = (uint8_t) buffer[cplen - 1];
        const uint8_t b2 = (uint8_t) buffer[cplen - 2];

        const uint8_t x = (b2 & 0b00011100) >> 2;
        const uint8_t y = ((b1 & 0b00110000) >> 4) | (b2 & 0b00000011) << 2;
        const uint8_t z = b1 & 0b00001111;

        codepoint = (x << 8) | (y << 4) | z;
    }

    return cplen;
}

/**
 * @brief Converts a UTF-32 codepoint to a UTF-8 codepoint.
 * 
 * @param codepoint The UTF-32 codepoint.
 * @param buffer The buffer that UTF-8 codepoints are written to.
 * @return The total number of UTF-8 codepoints written into the buffer.
 */
inline uint8_t utf32_codepoint_to_ut8(const uint32_t codepoint, uint8_t* buffer) noexcept {
    if (codepoint <= 0x7F) {
        buffer[0] = codepoint & 0xFF;
        return 1;
    }
    if (codepoint <= 0x7FF) {
        buffer[0] = 0xC0 | (codepoint >> 6);            /* 110xxxxx */
        buffer[1] = 0x80 | (codepoint & 0x3F);          /* 10xxxxxx */
        return 2;
    }
    if (codepoint <= 0xFFFF) {
        buffer[0] = 0xE0 | (codepoint >> 12);           /* 1110xxxx */
        buffer[1] = 0x80 | ((codepoint >> 6) & 0x3F);   /* 10xxxxxx */
        buffer[2] = 0x80 | (codepoint & 0x3F);          /* 10xxxxxx */
        return 3;
    }
    if (codepoint <= 0x10FFFF) {
        buffer[0] = 0xF0 | (codepoint >> 18);           /* 11110xxx */
        buffer[1] = 0x80 | ((codepoint >> 12) & 0x3F);  /* 10xxxxxx */
        buffer[2] = 0x80 | ((codepoint >> 6) & 0x3F);   /* 10xxxxxx */
        buffer[3] = 0x80 | (codepoint & 0x3F);          /* 10xxxxxx */
        return 4;
    }
    return 0;
}

/**
 * @brief Counts how many bytes a UTF-8 codepoint takes starting from the end of the buffer.
 * 
 * @param buffer The UTF-8 codepoint buffer.
 * @param length The length of the buffer.
 * @return The length of the UTF-8 codepoint in bytes.
 */
inline uint8_t count_utf8_char_bytes_from_end(const void* buffer, size_t length) {
    uint8_t count = 0;
    const uint8_t* data = static_cast<const uint8_t*>(buffer);
    // Moving to the left until we encounter trailing character
    while(++count < length && (data[length - count] & 0xC0u) == 0x80u);
    return count;
}

/**
 * @brief Counts how many bytes a UTF-8 codepoint takes.
 * 
 * @param c The UTF-8 codepoint.
 * @return The length of the UTF-8 codepoint in bytes.
 */
constexpr inline uint8_t count_utf8_char_bytes(uint8_t c) {
   // if the most significant bit with a zero in it is in position
   // 8-N then there are N bytes in this UTF-8 sequence:
   uint8_t mask = 0x80u;
   uint8_t result = 0;
   while (c & mask) {
        ++result;
        mask >>= 1;
   }
   return (result == 0) ? 1 : ((result > 4) ? 4 : result);
}

/**
 * @brief Counts UTF-8 codepoints in the \param buffer.
 * 
 * @param buffer A buffer.
 * @param length Length of the buffer.
 * @return Number of UTF-8 codepoints in the buffer.
 */
inline size_t count_utf8_codepoints(const void* buffer, size_t length) {
    size_t count = 0;
    size_t i = 0;
    const uint8_t* data = static_cast<const uint8_t*>(buffer);
    while (i++ < length) count += count_utf8_char_bytes(data[i]);
    return count;
}

}

#endif
