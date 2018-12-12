#ifndef __SNAKE_H__
#define __SNAKE_H__

//TCalc's hidden snake game!
#include "stm32f10x.h"
#include "lcd12864.hpp"

namespace game {

    enum SnakeDirection : uint8_t  {
        UP, DOWN, LEFT, RIGHT,
    };

    struct SnakeBody {
        uint8_t x;
        uint8_t y;
        
        SnakeBody *prev;
        SnakeBody *next;
    };

    struct Coords {
        uint8_t x;
        uint8_t y;
    };

    Coords getNextLocation(SnakeBody*, SnakeDirection);
    void moveSnake(SnakeBody*, SnakeBody*, SnakeDirection, bool eatsFood = false);
    bool inSnake(Coords, SnakeBody*);
    void drawSnake(lcd::LCD12864&, SnakeBody*);
}

#endif
