#include "snake.hpp"

namespace game {

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
}
