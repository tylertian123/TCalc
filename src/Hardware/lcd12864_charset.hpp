#ifndef __LCD12864_CHARSET_H__
#define __LCD12864_CHARSET_H__
#include "lcd12864.hpp"

namespace lcd {

	typedef LCD12864Image Img;

	const uint8_t CHAR_0_DATA[] = {
		0x70,
		0x88,
		0x88,
		0x98,
		0xa8,
		0xc8,
		0x88,
		0x88,
		0x70,
	};
	const uint8_t CHAR_1_DATA[] = {
		0x20,
		0x60,
		0x20,
		0x20,
		0x20,
		0x20,
		0x20,
		0x20,
		0x70,
	};
	const uint8_t CHAR_2_DATA[] = {
		0x20,
		0x60,
		0x20,
		0x20,
		0x20,
		0x20,
		0x20,
		0x20,
		0x70,
	};
    const uint8_t CHAR_3_DATA[] = {
        0x70,
        0x88,
        0x08,
        0x08,
        0x70,
        0x08,
        0x08,
        0x88,
        0x70,
    };
    const uint8_t CHAR_4_DATA[] = {
        0x10,
        0x30,
        0x50,
        0x50,
        0x90,
        0xf8,
        0x10,
        0x10,
        0x10,
    };
    const uint8_t CHAR_5_DATA[] = {
        0xf8,
        0x80,
        0x80,
        0xf0,
        0x88,
        0x08,
        0x08,
        0x88,
        0x70,
    };
    const uint8_t CHAR_6_DATA[] = {
        0x70,
        0x88,
        0x80,
        0x80,
        0xf0,
        0x88,
        0x88,
        0x88,
        0x70,
    };
    const uint8_t CHAR_7_DATA[] = {
        0xf8,
        0x08,
        0x08,
        0x10,
        0x10,
        0x20,
        0x20,
        0x20,
        0x20,
    };
    const uint8_t CHAR_8_DATA[] = {
        0x70,
        0x88,
        0x88,
        0x88,
        0x70,
        0x88,
        0x88,
        0x88,
        0x70,
    };
    const uint8_t CHAR_9_DATA[] = {
        0x70,
        0x88,
        0x88,
        0x88,
        0x78,
        0x08,
        0x08,
        0x88,
        0x70,
    };

    const LCD12864Image CHAR_NUMBERS[] = {
        Img(CHAR_0_DATA, 1, 5, 9),
        Img(CHAR_1_DATA, 1, 5, 9),
        Img(CHAR_2_DATA, 1, 5, 9),
        Img(CHAR_3_DATA, 1, 5, 9),
        Img(CHAR_4_DATA, 1, 5, 9),
        Img(CHAR_5_DATA, 1, 5, 9),
        Img(CHAR_6_DATA, 1, 5, 9),
        Img(CHAR_7_DATA, 1, 5, 9),
        Img(CHAR_8_DATA, 1, 5, 9),
        Img(CHAR_9_DATA, 1, 5, 9),
    };

    
}

#endif
