#ifndef __DRAWBUF_H__
#define __DRAWBUF_H__

#include "stm32f10x.h"

namespace lcd {

    class Image {
	public:
		const uint8_t *data;
		uint16_t bytesWide;
		uint16_t width;
		uint16_t height;
		
		constexpr Image(const uint8_t *data, uint8_t bytesWide, uint8_t width, uint8_t height) : data(data), bytesWide(bytesWide), width(width), height(height) {}
	};
    
    // A 128*64 buffer for drawing.
    // Note: Each one of these is 1KB of RAM!
    class DrawBuf {
    public:
        DrawBuf() {}

        // Clears the contents; sets every pixel to 0.
        void clear();
        // Sets a single pixel.
        void setPixel(int16_t, int16_t, bool state = true);
        // Draws an image
        void drawImage(int16_t, int16_t, const Image&, bool invert = false);
        // Draws a line
		void drawLine(int16_t, int16_t, int16_t, int16_t, bool invert = false);
        // Draws a string
		void drawString(int16_t, int16_t, const char*, bool invert = false);
        // Fills an area
		void fill(int16_t, int16_t, uint16_t, uint16_t, bool invert = false);
        // Gets the width of a string when drawn
        static uint16_t getDrawnStringWidth(const char*);
        // Used internally by the LCD classes
        uint16_t getLCDWord(uint8_t row, uint8_t col);
        // Copies the content of another DrawBuf
        void copy(const DrawBuf&);
    
    protected:
        void drawLineHigh(int16_t, int16_t, int16_t, int16_t, bool invert = false);
        void drawLineLow(int16_t, int16_t, int16_t, int16_t, bool invert = false);

        uint8_t buf[64][16] = { 0 };
    };
}

#endif
