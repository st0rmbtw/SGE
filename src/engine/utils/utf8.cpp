#include <SGE/utils/utf8.hpp>

uint32_t sge::next_utf8_codepoint(const char* text, size_t& index) {
    uint32_t c = (uint8_t) text[index];

    int cplen = 1;
    if ((text[0] & 0xf8) == 0xf0) {
        cplen = 4;

        const uint8_t b1 = (uint8_t) text[cplen - 1];
        const uint8_t b2 = (uint8_t) text[cplen - 2];
        const uint8_t b3 = (uint8_t) text[cplen - 3];
        const uint8_t b4 = (uint8_t) text[cplen - 4];

        const uint8_t x = (b2 & 0b00111100) >> 2;
        const uint8_t y = ((b1 & 0b00110000) >> 4) | (b2 & 0b00000011) << 2;
        const uint8_t z = b1 & 0b00001111;
        const uint8_t w = b3 & 0b00001111;
        const uint8_t v = ((b3 & 0b00110000) >> 4) | (b4 & 0b00000011);
        const uint8_t u = b4 & 0b00000100;

        c = (u << 18) | (v << 16) | (w << 12) | (x << 8) | (y << 4) | z;
    } else if ((text[0] & 0xf0) == 0xe0) {
        cplen = 3;

        const uint8_t b1 = (uint8_t) text[cplen - 1];
        const uint8_t b2 = (uint8_t) text[cplen - 2];
        const uint8_t b3 = (uint8_t) text[cplen - 3];

        const uint8_t x = (b2 & 0b00111100) >> 2;
        const uint8_t y = ((b1 & 0b00110000) >> 4) | (b2 & 0b00000011) << 2;
        const uint8_t z = b1 & 0b00001111;
        const uint8_t w = b3 & 0b00001111;

        c = (w << 12) | (x << 8) | (y << 4) | z;
    } else if ((text[0] & 0xe0) == 0xc0) {
        cplen = 2;

        const uint8_t b1 = (uint8_t) text[cplen - 1];
        const uint8_t b2 = (uint8_t) text[cplen - 2];

        const uint8_t x = (b2 & 0b00011100) >> 2;
        const uint8_t y = ((b1 & 0b00110000) >> 4) | (b2 & 0b00000011) << 2;
        const uint8_t z = b1 & 0b00001111;

        c = (x << 8) | (y << 4) | z;
    }

    index += cplen;

    return c;
}