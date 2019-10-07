#include <stdlib.h>
#include "keydef.h"
#include "snake.hpp"
#include "util.hpp"
#include "ntoa.hpp"

#define GAME_FIELD_X_MIN 16
#define GAME_FIELD_X_MAX 48
#define GAME_FIELD_Y_MIN 0
#define GAME_FIELD_Y_MAX 32

namespace snake {

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

	Coords getNextLocation(SnakeBody *body, SnakeDirection direction) {
		uint8_t nextX = body->x, nextY = body->y;
		switch(direction) {
		case SnakeDirection::UP:
			if(body->y > GAME_FIELD_Y_MIN) {
				nextY = body->y - 1;
			}
			else {
				return { 0xFF, 0xFF };
			}
			break;
		case SnakeDirection::DOWN:
			if(body->y < GAME_FIELD_Y_MAX - 1) {
				nextY = body->y + 1;
			}
			else {
				return { 0xFF, 0xFF };
			}
			break;
		case SnakeDirection::LEFT:
			if(body->x > GAME_FIELD_X_MIN) {
				nextX = body->x - 1;
			}
			else {
				return { 0xFF, 0xFF };
			}
			break;
		case SnakeDirection::RIGHT:
			if(body->x < GAME_FIELD_X_MAX - 1) {
				nextX = body->x + 1;
			}
			else {
				return { 0xFF, 0xFF };
			}
			break;
		}
		return { nextX, nextY };
	}

	void moveSnake(SnakeBody *head, SnakeBody *tail, SnakeDirection direction, bool eatsFood) {
		Coords next = getNextLocation(head, direction);

		if(!eatsFood) {
			// Disconnect the tail
			if(tail->prev) {
				tail->prev->next = nullptr;
				tail->prev = nullptr;
			}
			// Update position
			tail->x = next.x;
			tail->y = next.y;
			// Connect this to the head
			if(head != tail) {
				tail->next = head;
				head->prev = tail;
			}
		}
		else {
			SnakeBody *newHead = new SnakeBody;
			newHead->x = next.x;
			newHead->y = next.y;
			newHead->prev = nullptr;
			newHead->next = head;
			head->prev = newHead;
		}
	}

	bool inSnake(Coords c, SnakeBody *head) {
		do {
			if(head->x == c.x && head->y == c.y) {
				return true;
			}
		} while((head = head->next) != nullptr);
		return false;
	}

	void drawSnake(lcd::LCD12864 &display, SnakeBody *head) {
		do {

			display.setPixel(head->x * 2, head->y * 2, true);
			display.setPixel(head->x * 2 + 1, head->y * 2, true);
			display.setPixel(head->x * 2, head->y * 2 + 1, true);
			display.setPixel(head->x * 2 + 1, head->y * 2 + 1, true);
		} while((head = head->next) != nullptr);
	}

	SnakeBody *head = nullptr;
	SnakeBody *tail = nullptr;
	SnakeDirection direction = SnakeDirection::UP;
	Coords food;
	bool gamePaused = false;
	uint16_t gameScore = 0;

	void newFood() {
		do {
			food.x = rand() % (GAME_FIELD_X_MAX - GAME_FIELD_X_MIN) + GAME_FIELD_X_MIN;
			food.y = rand() % (GAME_FIELD_Y_MAX - GAME_FIELD_Y_MIN) + GAME_FIELD_Y_MIN;
		} while(inSnake(food, head));
	}

	void startGame() {
		head = new SnakeBody;
		tail = new SnakeBody;
		head->prev = nullptr;
		head->next = tail;
		tail->next = nullptr;
		tail->prev = head;

		head->x = (GAME_FIELD_X_MAX + GAME_FIELD_X_MIN) / 2;
		head->y = (GAME_FIELD_Y_MAX + GAME_FIELD_Y_MIN) / 2;
		tail->x = (GAME_FIELD_X_MAX + GAME_FIELD_X_MIN) / 2;
		tail->y = (GAME_FIELD_Y_MAX + GAME_FIELD_Y_MIN) / 2 + 1;

		gameScore = 0;

		newFood();
	}

	void processGame(lcd::LCD12864 &display) {
		if(!gamePaused) {
			Coords nextCoords = getNextLocation(head, direction);
			// See if the snake ran into itself or is out of bounds
			if((nextCoords.x == 0xFF && nextCoords.y == 0xFF) || inSnake(nextCoords, head)) {
				// Game over
				SnakeBody *tmp = head->next;
				while(head) {
					delete head;
					head = tmp;
					tmp = head->next;
				}
				startGame();
			}
			// Movement - eating food
			if(nextCoords.x == food.x && nextCoords.y == food.y) {
				moveSnake(head, tail, direction, true);
				head = head->prev;

				++gameScore;
				newFood();
			}
			// No eating food
			else {
				auto temp = tail->prev;
				moveSnake(head, tail, direction, false);
				head = head->prev;
				tail = temp;
			}
		}
		display.clearDrawingBuffer();
		drawSnake(display, head);
		display.setPixel(food.x * 2, food.y * 2, true);
		display.setPixel(food.x * 2 + 1, food.y * 2, true);
		display.setPixel(food.x * 2, food.y * 2 + 1, true);
		display.setPixel(food.x * 2 + 1, food.y * 2 + 1, true);
		display.drawLine(GAME_FIELD_X_MIN * 2 - 1, GAME_FIELD_Y_MIN * 2, GAME_FIELD_X_MIN * 2 - 1, GAME_FIELD_Y_MAX * 2 - 1);
		display.drawLine(GAME_FIELD_X_MAX * 2, GAME_FIELD_Y_MIN * 2, GAME_FIELD_X_MAX * 2, GAME_FIELD_Y_MAX * 2 - 1);
		
		display.drawString(GAME_FIELD_X_MAX * 2 + 2, 1, "Score");
		char buf[10];
		util::ltoa(gameScore, buf);
		display.drawString(GAME_FIELD_X_MAX * 2 + 2, 12, buf);

		if(gamePaused) {
			display.drawString(45, 25, "Paused", lcd::DrawBuf::FLAG_INVERTED);
		}

		display.updateDrawing();
	}

	void handleKeyPress(uint16_t key) {
		switch(key) {
		case KEY_LEFT:
			if(direction != SnakeDirection::RIGHT) {
				direction = SnakeDirection::LEFT;
			}
			break;
		case KEY_RIGHT:
			if(direction != SnakeDirection::LEFT) {
				direction = SnakeDirection::RIGHT;
			}
			break;
		case KEY_UP:
			if(direction != SnakeDirection::DOWN) {
				direction = SnakeDirection::UP;
			}
			break;
		case KEY_DOWN:
			if(direction != SnakeDirection::UP) {
				direction = SnakeDirection::DOWN;
			}
			break;
		case KEY_LCP:
			gamePaused = !gamePaused;
			break;
		default:
			break;
		}
	}
}
