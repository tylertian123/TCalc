#include <stdint.h>
#include <stdlib.h>
#include "tetris.hpp"
#include "util.hpp"
#include "keydef.h"
#include "ntoa.hpp"

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

    uint32_t score = 0;

    uint8_t currentBlock = 0;
    uint8_t currentRotation = 0;
    int8_t blockX = 0;
    int8_t blockY = 0;

    uint8_t nextBlock = 0;

    struct GameState {
        bool paused : 1;
        bool gameOver : 1;
    } gameState;

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

    void drawPhantomBlock(int16_t x, int16_t y, lcd::LCD12864 &display) {
        display.setPixel(x + 1, y + 1);
    }

    void drawTetromino(int16_t x, int16_t y, uint8_t tetromino, uint8_t rotation, lcd::LCD12864 &display, 
            void (*drawBlock)(int16_t, int16_t, lcd::LCD12864 &) = drawBlock) {
        for(uint8_t i = 0; i < 4; i ++) {
            for(uint8_t j = 0; j < 4; j ++) {
                if(TETROMINOES[tetromino].rotations[rotation][i] & (0x80 >> j)) {
                    drawBlock(x + j * BLOCK_SIZE, y + i * BLOCK_SIZE, display);
                }
            }
        }
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
                    else if(y + i >= 0) {
                        return true;
                    }
                }
            }
        }
        return false;
    }

    void newTetromino() {
        currentBlock = nextBlock;
        nextBlock = rand() % NUMBER_OF_TETROMINOES;

        currentRotation = 0;
        blockX = TETROMINOES[currentBlock].startX;
        blockY = TETROMINOES[currentBlock].startY - 1;

        // Check for game over
        if(collisionCheck(currentBlock, currentRotation, blockX, blockY)) {
            gameState.gameOver = true;
        }
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

        int8_t original = blockY;
        do {
            blockY ++;
        } while(!collisionCheck(currentBlock, currentRotation, blockX, blockY));
        blockY --;
        if(blockY != original) {
            drawTetromino(FIELD_X + blockX * BLOCK_SIZE, FIELD_Y + blockY * BLOCK_SIZE, currentBlock, currentRotation, display, drawPhantomBlock);
            blockY = original;
        }


        display.drawString(FIELD_X + FIELD_WIDTH_PIXELS + 3, 1, "Score:");
        char buf[20];
        util::ltoa(score, buf);
        display.drawString(FIELD_X + FIELD_WIDTH_PIXELS + 33, 11, buf, lcd::DrawBuf::FLAG_HALIGN_RIGHT);

        display.drawString(FIELD_X + FIELD_WIDTH_PIXELS + 3, 31, "Next:");
        drawTetromino(FIELD_X + FIELD_WIDTH_PIXELS + 13, 45, nextBlock, 0, display);
    }

    const uint16_t SCORE_ADDITIONS[] = {
        0, 40, 100, 300, 1200,
    };

    void lineClearCheck() {

        uint8_t clearCount = 0;
        for(uint8_t i = 0; i < FIELD_HEIGHT_BLOCKS; i ++) {
            if((gameField[i] & 0b1111'1111'1100'0000) == 0b1111'1111'1100'0000) {
                clearCount ++;
            }
        }

        score += SCORE_ADDITIONS[clearCount];

        for(uint8_t i = FIELD_HEIGHT_BLOCKS; i --> 0; ) {
            if((gameField[i] & 0b1111'1111'1100'0000) == 0b1111'1111'1100'0000) {
                for(uint8_t j = i; j --> 0; ) {
                    gameField[j + 1] = gameField[j];
                }
                ++i;
            }
        }
    }

    void startGame() {
        score = 0;
        memset(gameField, 0, sizeof(gameField));
        gameState.paused = false;
        gameState.gameOver = false;
        // Call this twice to fill both the current and the next one
        newTetromino();
        newTetromino();
    }

    void processGame(lcd::LCD12864 &display) {
        display.clearDrawingBuffer();
        
        if(!gameState.gameOver) {
            if(!gameState.paused) {
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
                    lineClearCheck();
                    newTetromino();
                }
            }
            
            drawGame(display);

            if(gameState.paused) {
                display.drawString(lcd::SIZE_WIDTH / 2, lcd::SIZE_HEIGHT / 2, "Paused", lcd::DrawBuf::FLAG_HALIGN_CENTER | lcd::DrawBuf::FLAG_INVERTED);
            }
        }
        else {
            display.drawString(lcd::SIZE_WIDTH / 2, 15, "Game Over", lcd::DrawBuf::FLAG_HALIGN_CENTER | lcd::DrawBuf::FLAG_INVERTED);
            display.drawString(lcd::SIZE_WIDTH / 2, 40, "Score:", lcd::DrawBuf::FLAG_HALIGN_CENTER);
            char buf[20];
            util::ltoa(score, buf);
            display.drawString(lcd::SIZE_WIDTH / 2, 50, buf, lcd::DrawBuf::FLAG_HALIGN_CENTER);
        }

        display.updateDrawing();
    }

    void handleKeyPress(uint16_t key, lcd::LCD12864 &display) {
        __NO_INTERRUPT_BEGIN
            if(gameState.gameOver && key == KEY_CENTER) {
                startGame();
            }
            else {
                switch(key) {
                case KEY_LCA:
                case KEY_LEFT:
                    if(!gameState.paused && !collisionCheck(currentBlock, currentRotation, blockX - 1, blockY)) {
                        blockX --;
                    }
                    break;
                case KEY_LCD:
                case KEY_RIGHT:
                    if(!gameState.paused && !collisionCheck(currentBlock, currentRotation, blockX + 1, blockY)) {
                        blockX ++;
                    }
                    break;
                case KEY_LCW:
                case KEY_UP:
                {
                    if(!gameState.paused) {
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
                    }
                    break;
                }
                case KEY_LCS:
                case KEY_DOWN:
                {
                    if(!gameState.paused) {
                        do {
                            blockY ++;
                        } while(!collisionCheck(currentBlock, currentRotation, blockX, blockY));
                        blockY --;
                        for(uint8_t i = 0; i < 4; i ++) {
                            // Just OR the tetromino data with the field
                            gameField[blockY + i] |= util::signedLeftShift<uint16_t, int>(TETROMINOES[currentBlock].rotations[currentRotation][i], 8 - blockX);
                        }
                        // TODO: Scoring
                        lineClearCheck();
                        newTetromino();
                        break;
                    }
                }
                case KEY_LCP:
                {
                    gameState.paused = !gameState.paused;
                }
                default:
                    break;
                }

                display.clearDrawingBuffer();
                drawGame(display);
                if(gameState.paused) {
                    display.drawString(lcd::SIZE_WIDTH / 2, lcd::SIZE_HEIGHT / 2, "Paused", lcd::DrawBuf::FLAG_HALIGN_CENTER | lcd::DrawBuf::FLAG_INVERTED);
                }
                display.updateDrawing();
            }
        __NO_INTERRUPT_END
    }
}
