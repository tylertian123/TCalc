#ifndef __LCD12864_CHARSET_H__
#define __LCD12864_CHARSET_H__
#include "lcd12864.hpp"

namespace lcd {
	
	const uint8_t TEST_CHAR_DATA[] = {
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
	const LCD12864Image TEST_CHAR(TEST_CHAR_DATA, 1, 9);

}

#endif
