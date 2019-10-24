#ifndef __TETRIS_H__
#define __TETRIS_H__

#include "lcd12864.hpp"

namespace tetris {

    struct GameState {
        bool paused : 1;
        bool gameOver : 1;
        bool holdEnabled : 1;
        bool holdUsed : 1;
        unsigned int holdBlock : 3;
    };
    
    extern GameState gameState;
    
    void startGame();
    void processGame(lcd::LCD12864 &display);
    void handleKeyPress(uint16_t key, lcd::LCD12864 &display);
}

#endif
