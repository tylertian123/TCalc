#ifndef __SNAKE_H__
#define __SNAKE_H__

#include "lcd12864.hpp"

// TCalc's hidden Snake game!
namespace snake {
	
	void startGame();
	void processGame(lcd::LCD12864 &display);
	void handleKeyPress(uint16_t key);
}

#endif
