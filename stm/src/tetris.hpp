#ifndef __TETRIS_H__
#define __TETRIS_H__

#include "lcd12864.hpp"

namespace tetris {
    
    void startGame();
    void processGame(lcd::LCD12864 &display);
    void handleKeyPress(uint16_t key, lcd::LCD12864 &display);
}

#endif
