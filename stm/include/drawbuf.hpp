#ifndef __DRAWBUF_H__
#define __DRAWBUF_H__

#include "lcd12864_charset.hpp"
#include <stdint.h>

namespace lcd {

    class Image {
    public:
        const uint8_t *data;
        uint16_t bytesWide;
        uint16_t width;
        uint16_t height;

        constexpr Image(const uint8_t *data, uint8_t bytesWide, uint8_t width, uint8_t height)
                : data(data), bytesWide(bytesWide), width(width), height(height) {
        }
    };

    // A 128*64 buffer for drawing.
    // Note: Each one of these is 1KB of RAM!
    class DrawBuf {
    public:
        DrawBuf() {
        }

        // Passed into drawString
        typedef const Image &(*Charset)(char);
        static constexpr Charset CHARSET_NORMAL = &getChar;
        static constexpr Charset CHARSET_SMALL = &getSmallChar;

        // Passed into drawString
        /*
         *   7         6         5           4          3           2           1         0
         * unused    unused    unused    v bottom    v center    h right    h center    invert
         */
        typedef int Flags;
        static constexpr Flags FLAG_NONE = 0b0000'0000;

        static constexpr Flags FLAG_NORMAL = 0b0000'0000;
        static constexpr Flags FLAG_INVERTED = 0b0000'0001;

        static constexpr Flags FLAG_HALIGN_LEFT = 0b0000'0000;
        static constexpr Flags FLAG_HALIGN_CENTER = 0b0000'0010;
        static constexpr Flags FLAG_HALIGN_RIGHT = 0b0000'0100;

        static constexpr Flags FLAG_VALIGN_TOP = 0b0000'0000;
        static constexpr Flags FLAG_VALIGN_CENTER = 0b0000'1000;
        static constexpr Flags FLAG_VALIGN_BOTTOM = 0b0001'0000;

        // Clears the contents; sets every pixel to 0.
        void clear();
        // Sets a single pixel.
        void setPixel(int16_t, int16_t, bool state = true);
        // Draws an image
        void drawImage(int16_t, int16_t, const Image &, bool invert = false);
        // Draws a line
        void drawLine(int16_t, int16_t, int16_t, int16_t, bool invert = false);
        // Draws a string
        void drawString(int16_t, int16_t, const char *, Flags flags = FLAG_NONE, Charset charset = CHARSET_NORMAL);
        // Fills an area
        void fill(int16_t, int16_t, uint16_t, uint16_t, bool invert = false);
        // Gets the width of a string when drawn
        static uint16_t getDrawnStringWidth(const char *, Charset charset = CHARSET_NORMAL);
        // Used internally by the LCD classes
        uint16_t getLCDWord(uint8_t row, uint8_t col);
        // Copies the content of another DrawBuf
        void copy(const DrawBuf &);

    protected:
        void drawLineHigh(int16_t, int16_t, int16_t, int16_t, bool invert = false);
        void drawLineLow(int16_t, int16_t, int16_t, int16_t, bool invert = false);
        void drawLineVertical(int16_t x, int16_t y1, int16_t y2, bool invert = false);
        void drawLineHorizontal(int16_t y, int16_t x1, int16_t x2, bool invert = false);

        uint8_t buf[64][16] = {0};
    };
} // namespace lcd

#endif
