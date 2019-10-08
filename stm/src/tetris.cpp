#include <stdint.h>
#include <stdlib.h>
#include "tetris.hpp"
#include "util.hpp"
#include "keydef.h"

namespace tetris {

    constexpr uint8_t FIELD_WIDTH_BLOCKS = 10;
    constexpr uint8_t FIELD_HEIGHT_BLOCKS = 20;
    constexpr uint8_t BLOCK_SIZE = 3;
    constexpr int16_t FIELD_WIDTH_PIXELS = FIELD_WIDTH_BLOCKS * BLOCK_SIZE;
    constexpr int16_t FIELD_HEIGHT_PIXELS = FIELD_HEIGHT_BLOCKS * BLOCK_SIZE;

    constexpr int16_t FIELD_X = 49;
    constexpr int16_t FIELD_Y = 3;

    struct Tetromino {
        int8_t startX;
        int8_t startY;

        const uint8_t rotations[4][4];
    };

    const Tetromino TETROMINOES[] = {
        // I block
        {
            3, 0,
            {
                {
                    0b1111'0000
                },
                {
                    0b0010'0000,
                    0b0010'0000,
                    0b0010'0000,
                    0b0010'0000,
                },
                {
                    0b1111'0000
                },
                {
                    0b0010'0000,
                    0b0010'0000,
                    0b0010'0000,
                    0b0010'0000,
                },
            }
        },
        // O block
        {
            4, 0,
            {
                {
                    0b1100'0000,
                    0b1100'0000,
                },
                {
                    0b1100'0000,
                    0b1100'0000,
                },
                {
                    0b1100'0000,
                    0b1100'0000,
                },
                {
                    0b1100'0000,
                    0b1100'0000,
                },
            }
        },
        // T block
        {
            3, -1,
            {
                {
                    0b0000'0000,
                    0b1110'0000,
                    0b0100'0000,
                },
                {
                    0b0100'0000,
                    0b1100'0000,
                    0b0100'0000,
                },
                {
                    0b0100'0000,
                    0b1110'0000,
                    0b0000'0000,
                },
                {
                    0b0100'0000,
                    0b0110'0000,
                    0b0100'0000,
                }
            }
        },
        // J block
        {
            3, -1,
            {
                {
                    0b0000'0000,
                    0b1110'0000,
                    0b0010'0000,
                },
                {
                    0b0100'0000,
                    0b0100'0000,
                    0b1100'0000,
                },
                {
                    0b1000'0000,
                    0b1110'0000,
                },
                {
                    0b0110'0000,
                    0b0100'0000,
                    0b0100'0000,
                }
            }
        },
        // L block
        {
            3, -1,
            {
                {
                    0b0000'0000,
                    0b1110'0000,
                    0b1000'0000,
                },
                {
                    0b1100'0000,
                    0b0100'0000,
                    0b0100'0000,
                },
                {
                    0b0010'0000,
                    0b1110'0000,
                },
                {
                    0b0100'0000,
                    0b0100'0000,
                    0b0110'0000,
                },
            }
        },
        // S block
        {
            3, -1,
            {
                {
                    0b0000'0000,
                    0b0110'0000,
                    0b1100'0000,
                },
                {
                    0b0100'0000,
                    0b0110'0000,
                    0b0010'0000,
                },
                {
                    0b0110'0000,
                    0b1100'0000,
                },
                {
                    0b1000'0000,
                    0b1100'0000,
                    0b0100'0000,
                },
            }
        },
        // Z block
        {
            3, -1,
            {
                {
                    0b0000'0000,
                    0b1100'0000,
                    0b0110'0000,
                },
                {
                    0b0010'0000,
                    0b0110'0000,
                    0b0100'0000,
                },
                {
                    0b1100'0000,
                    0b0110'0000,
                },
                {
                    0b0100'0000,
                    0b1100'0000,
                    0b1000'0000,
                },
            }
        },
    };

    constexpr uint8_t NUMBER_OF_TETROMINOES = sizeof(TETROMINOES) / sizeof(Tetromino);

    uint16_t gameField[FIELD_HEIGHT_BLOCKS] = { 0 };

    uint16_t score = 0;

    uint8_t currentBlock = 0;
    uint8_t currentRotation = 0;
    int8_t blockX = 0;
    int8_t blockY = 0;

    uint8_t nextBlock = 0;

    void drawBlock(int16_t x, int16_t y, lcd::LCD12864 &display) {
        display.setPixel(x, y);
        display.setPixel(x + 1, y);
        display.setPixel(x + 2, y);

        display.setPixel(x, y + 1);
        display.setPixel(x + 2, y + 1);

        display.setPixel(x, y + 2);
        display.setPixel(x + 1, y + 2);
        display.setPixel(x + 2, y + 2);
    }

    void drawTetromino(int16_t x, int16_t y, uint8_t tetromino, uint8_t rotation, lcd::LCD12864 &display) {
        for(uint8_t i = 0; i < 4; i ++) {
            for(uint8_t j = 0; j < 4; j ++) {
                if(TETROMINOES[tetromino].rotations[rotation][i] & (0x80 >> j)) {
                    drawBlock(x + j * BLOCK_SIZE, y + i * BLOCK_SIZE, display);
                }
            }
        }
    }

    void newTetromino() {
        currentBlock = nextBlock;
        nextBlock = rand() % NUMBER_OF_TETROMINOES;

        currentRotation = 0;
        blockX = TETROMINOES[currentBlock].startX;
        blockY = TETROMINOES[currentBlock].startY - 1;
    }

    void drawGame(lcd::LCD12864 &display) {
        display.drawLine(FIELD_X - 1, 0, FIELD_X - 1, lcd::SIZE_HEIGHT - 1);
        display.drawLine(FIELD_X + FIELD_WIDTH_PIXELS, 0, FIELD_X + FIELD_WIDTH_PIXELS, lcd::SIZE_HEIGHT - 1);
        display.drawLine(FIELD_X - 1, lcd::SIZE_HEIGHT - 1, FIELD_X + FIELD_WIDTH_PIXELS, lcd::SIZE_HEIGHT - 1);
        display.fill(FIELD_X, 0, FIELD_WIDTH_PIXELS, FIELD_Y);

        for(uint8_t i = 0; i < FIELD_HEIGHT_BLOCKS; i ++) {
            for(uint8_t j = 0; j < FIELD_WIDTH_BLOCKS; j ++) {
                if(gameField[i] & (0x8000 >> j)) {
                    drawBlock(FIELD_X + j * BLOCK_SIZE, FIELD_Y + i * BLOCK_SIZE, display);
                }
            }
        }

        drawTetromino(FIELD_X + blockX * BLOCK_SIZE, FIELD_Y + blockY * BLOCK_SIZE, currentBlock, currentRotation, display);
    }

    bool collisionCheck(uint8_t tetromino, uint8_t rotation, int8_t x, int8_t y) {
        // Loop through every block of the tetromino
        for(uint8_t i = 0; i < 4; i ++) {
            for(uint8_t j = 0; j < 4; j ++) {
                // If the tetromino has a block here
                if(TETROMINOES[currentBlock].rotations[currentRotation][i] & (0x80 >> j)) {
                    // Check if the block is in bounds
                    if (y + i >= 0 && x + j >= 0 && blockY + i < FIELD_HEIGHT_BLOCKS && x + j < FIELD_WIDTH_BLOCKS) {
                        // Check if the game field has a block there
                        if(gameField[blockY + i] & (0x8000 >> (x + j))) {
                            return true;
                        }
                    }
                    // If out of bounds then collide
                    else {
                        return true;
                    }
                }
            }
        }
        return false;
    }

    void startGame() {
        score = 0;
        // Call this twice to fill both the current and the next one
        newTetromino();
        newTetromino();
    }

    void processGame(lcd::LCD12864 &display) {
        display.clearDrawingBuffer();

        // Check for collisions
        blockY ++;
        if(collisionCheck(currentBlock, currentRotation, blockX, blockY)) {
            // If collided then move the tetromino to the field
            blockY --;
            for(uint8_t i = 0; i < 4; i ++) {
                // Just OR the tetromino data with the field
                gameField[blockY + i] |= util::signedLeftShift<uint16_t, int>(TETROMINOES[currentBlock].rotations[currentRotation][i], 8 - blockX);
            }
            // TODO: Scoring
            newTetromino();
        }
        
        drawGame(display);

        display.updateDrawing();
    }

    void handleKeyPress(uint16_t key, lcd::LCD12864 &display) {
        __NO_INTERRUPT(

            switch(key) {
            case KEY_LEFT:
                if(!collisionCheck(currentBlock, currentRotation, blockX - 1, blockY)) {
                    blockX --;
                }
                break;
            case KEY_RIGHT:
                if(!collisionCheck(currentBlock, currentRotation, blockX + 1, blockY)) {
                    blockX ++;
                }
                break;
            case KEY_UP:
            {
                uint8_t original = currentRotation;
                currentRotation = (currentRotation + 1) % 4;
                if(collisionCheck(currentBlock, currentRotation, blockX, blockY)) {
                    // Try shifting to the left or right as necessary
                    if(!collisionCheck(currentBlock, currentRotation, blockX - 1, blockY)) {
                        blockX --;
                    }
                    else if(!collisionCheck(currentBlock, currentRotation, blockX + 1, blockY)) {
                        blockX ++;
                    }
                    // If not possible then give up
                    else {
                        currentRotation = original;
                    }
                }
                break;
            }
            default:
                break;
            }

            display.clearDrawingBuffer();
            drawGame(display);
            display.updateDrawing();
        )
    }
}
