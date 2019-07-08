#include "stm32f10x.h"
#include "sys.hpp"
#include "delay.hpp"
#include "usart.hpp"
#include "gpiopin.hpp"
#include "sbdi.hpp"
#include "lcd12864.hpp"
#include "lcd12864_charset.hpp"
#include "dynamarr.hpp"
#include "neda.hpp"
#include "eval.hpp"
#include "keydef.h"
#include "util.hpp"
#include "ntoa.hpp"
#include "snake.hpp"
#include "exprentry.hpp"
#include <stdlib.h>

#define VERSION_STR "V1.0"

/********** GPIO Pins and other pin defs **********/
GPIOPin RS(GPIOC, GPIO_Pin_10), RW(GPIOC, GPIO_Pin_11), E(GPIOC, GPIO_Pin_12),
			D7(GPIOC, GPIO_Pin_9), D6(GPIOC, GPIO_Pin_8), D5(GPIOC, GPIO_Pin_7), D4(GPIOC, GPIO_Pin_6),
			D3(GPIOB, GPIO_Pin_15), D2(GPIOB, GPIO_Pin_14), D1(GPIOB, GPIO_Pin_13), D0(GPIOB, GPIO_Pin_12);
lcd::LCD12864 display(RS, RW, E, D0, D1, D2, D3, D4, D5, D6, D7);

//GPIOPin backlightPin(GPIOA, GPIO_Pin_1);

GPIOPin SBDI_EN(GPIOB, GPIO_Pin_8);
GPIOPin SBDI_CLK(GPIOB, GPIO_Pin_7);
GPIOPin SBDI_DATA(GPIOB, GPIO_Pin_6);

GPIOPin statusLED(GPIOA, GPIO_Pin_1);
GPIOPin ctrlLED(GPIOA, GPIO_Pin_2);
GPIOPin shiftLED(GPIOA, GPIO_Pin_3);

/********** Keyboard stuff **********/
uint64_t keyDataBuffer = 0;
void putKey(uint16_t key) {
	// Store keystroke into buffer
	// If there is already data in the buffer then shift that data left to make room
	if(keyDataBuffer != 0) {
		keyDataBuffer <<= 16;
	}
	keyDataBuffer += (uint16_t) key;
}
uint16_t fetchKey() {
	uint16_t data;
	// Check for key in buffer
	if(keyDataBuffer != 0) {
		data = keyDataBuffer & 0xFFFF;
		keyDataBuffer >>= 16;
		return data;
	}
	// If buffer empty then no new keys have been entered
	else {
		return KEY_NULL;
	}
}

/********** Mode **********/
enum class DispMode {
	EXPR_ENTRY,
	TRIG_MENU,
	CONST_MENU,
	CONFIG_MENU,
	FUNC_MENU,
	RECALL_MENU,
	MATRIX_MENU,
	GAME,
};
DispMode dispMode = DispMode::EXPR_ENTRY;

/********** Cursor processing **********/
void initCursorTimer(uint16_t period = 2000) {
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
	TIM_TimeBaseInitTypeDef initStruct;
	TIM_TimeBaseStructInit(&initStruct);
	initStruct.TIM_CounterMode = TIM_CounterMode_Up;
	initStruct.TIM_ClockDivision = TIM_CKD_DIV1;
	initStruct.TIM_Prescaler = 17999;
	initStruct.TIM_Period = period;
	initStruct.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(TIM3, &initStruct);
	// Set up interrupts
	NVIC_InitTypeDef nvicInit;
	nvicInit.NVIC_IRQChannel = TIM3_IRQn;
	nvicInit.NVIC_IRQChannelCmd = ENABLE;
	nvicInit.NVIC_IRQChannelPreemptionPriority = 0x04;
	nvicInit.NVIC_IRQChannelSubPriority = 0x01;
	NVIC_Init(&nvicInit);
	// Enable timer and interrupts
	TIM_Cmd(TIM3, ENABLE);
	TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);
}

/********** Snake Game **********/

game::SnakeBody *head;
game::SnakeBody *tail;
game::SnakeDirection direction = game::SnakeDirection::UP;
game::Coords food;
bool gamePaused = false;
uint16_t gameScore = 0;

void newFood() {
	do {
		food.x = rand() % (GAME_FIELD_X_MAX - GAME_FIELD_X_MIN) + GAME_FIELD_X_MIN;
		food.y = rand() % (GAME_FIELD_Y_MAX - GAME_FIELD_Y_MIN) + GAME_FIELD_Y_MIN;
	} while(game::inSnake(food, head));
}
void respawn() {
	head = new game::SnakeBody;
	tail = new game::SnakeBody;
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

/********** Cursor flashing and game processing **********/

#define CURSOR_HORIZ_SPACING 1
#define CURSOR_VERT_SPACING 1
expr::ExprEntry mainExprEntry(display, CURSOR_HORIZ_SPACING, CURSOR_VERT_SPACING);

enum class ExprEditMode : uint8_t {
    NORMAL,
    RESULT,
    RESULT_SCROLL,
};
ExprEditMode exprEditMode = ExprEditMode::NORMAL;

bool cursorOn = false;

// Marked for removal
neda::Cursor *cursor;
extern bool editExpr;

extern "C" void TIM3_IRQHandler() {
	if(TIM_GetITStatus(TIM3, TIM_IT_Update)) {
		TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
        // Flash cursor
		if(dispMode == DispMode::EXPR_ENTRY && exprEditMode == ExprEditMode::NORMAL && mainExprEntry.mode == expr::ExprEntry::DisplayMode::NORMAL) {
			cursorOn = !cursorOn;
			display.clearDrawingBuffer();
			// Redraw the entire expr
            mainExprEntry.cursor->expr->drawConnected(display);
			if(cursorOn) {
				mainExprEntry.cursor->draw(display);
			}
			display.updateDrawing();
		}
		else if(dispMode == DispMode::GAME) {
			if(!gamePaused) {
				game::Coords nextCoords = game::getNextLocation(head, direction);
				// See if the snake ran into itself or is out of bounds
				if((nextCoords.x == 0xFF && nextCoords.y == 0xFF) || game::inSnake(nextCoords, head)) {
					// Game over
					do {
						delete head;
					} while((head = head->next) != nullptr);
					respawn();
				}
				// Movement - eating food
				if(nextCoords.x == food.x && nextCoords.y == food.y) {
					game::moveSnake(head, tail, direction, true);
					head = head->prev;

					++gameScore;
					newFood();
				}
				// No eating food
				else {
					auto temp = tail->prev;
					game::moveSnake(head, tail, direction);
					head = head->prev;
					tail = temp;
				}
			}
			display.clearDrawingBuffer();
			game::drawSnake(display, head);
			display.setPixel(food.x * 2, food.y * 2, true);
			display.setPixel(food.x * 2 + 1, food.y * 2, true);
			display.setPixel(food.x * 2, food.y * 2 + 1, true);
			display.setPixel(food.x * 2 + 1, food.y * 2 + 1, true);
			display.drawLine(GAME_FIELD_X_MIN * 2 - 1, GAME_FIELD_Y_MIN * 2, GAME_FIELD_X_MIN * 2 - 1, GAME_FIELD_Y_MAX * 2 - 1);
			display.drawLine(GAME_FIELD_X_MAX * 2, GAME_FIELD_Y_MIN * 2, GAME_FIELD_X_MAX * 2, GAME_FIELD_Y_MAX * 2 - 1);
			
			display.drawString(GAME_FIELD_X_MAX * 2 + 2, 1, "Score");
			char buf[10];
			ltoa(gameScore, buf);
			display.drawString(GAME_FIELD_X_MAX * 2 + 2, 12, buf);

			if(gamePaused) {
				display.drawString(45, 25, "Paused", true);
			}

			display.updateDrawing();
		}
	}
}


// Moves ex so that the cursor is in the display
void adjustExpr(neda::Expr *ex, neda::Cursor *cursorRef) {
	neda::CursorInfo info;
	cursorRef->getInfo(info);
	// First try to directly position the expression in the top-left corner of the display
	int16_t xd = CURSOR_HORIZ_SPACING + 1 - ex->x;
	int16_t yd = CURSOR_VERT_SPACING - ex->y;
	// Make sure it fits
	if(info.x + xd >= CURSOR_HORIZ_SPACING && info.y + yd >= CURSOR_VERT_SPACING
			&& info.x + info.width + xd + CURSOR_HORIZ_SPACING < 128 && info.y + info.height + yd + CURSOR_VERT_SPACING < 64) {
		ex->updatePosition(xd, yd);
	}
	else {
		// Fit the cursor normally
		if(info.x >= CURSOR_HORIZ_SPACING && info.y >= CURSOR_VERT_SPACING
				&& info.x + info.width + CURSOR_HORIZ_SPACING < 128 && info.y + info.height + CURSOR_VERT_SPACING < 64) {
			return;
		}

		int16_t xdiff = 0, ydiff = 0;
		if(info.x < CURSOR_HORIZ_SPACING) {
			xdiff = CURSOR_HORIZ_SPACING - info.x;
		}
		else if(info.x + info.width + CURSOR_HORIZ_SPACING >= 128) {
			xdiff = 127 - (info.x + info.width + CURSOR_HORIZ_SPACING);
		}
		if(info.y < CURSOR_VERT_SPACING) {
			ydiff = CURSOR_VERT_SPACING - info.y;
		}
		else if(info.y + info.height + CURSOR_VERT_SPACING >= 64) {
			ydiff = 63 - (info.y + info.height + CURSOR_VERT_SPACING);
		}
		ex->updatePosition(xdiff, ydiff);
	}
}

//Adds a char at the cursor
void addChar(neda::Cursor *cursor, char ch) {
	cursor->add(new neda::Character(ch));
}
void addStr(neda::Cursor *cursor, const char *str) {
	while(*str != '\0') {
		cursor->add(new neda::Character(*str++));
	}
}

#define RESULT_STORE_COUNT 4
// Previous expressions and their results
neda::Container *calcResults[RESULT_STORE_COUNT] = { nullptr };
neda::Container *expressions[RESULT_STORE_COUNT] = { nullptr };
int16_t resultX, resultY;
uint16_t resultWidth = 0, resultHeight = 0;
extern bool scrollExpr;
extern bool asDecimal;
void drawResult(uint8_t id, bool resetLocation = true) {
	// Display the result
	display.clearDrawingBuffer();
	expressions[id]->Expr::draw(display);
	// Convert to decimal if necessary
	neda::Container *result;
	if(asDecimal && calcResults[id]->contents[0]->getType() == neda::ObjType::FRACTION) {
		result = new neda::Container();
		// Evaluate the fraction lazily by calling evaluate
		eval::Token *evalResult = eval::evaluate(calcResults[id]);
		// Guaranteed to be a fraction
		double decimalResult = ((eval::Fraction*) evalResult)->doubleVal();
		delete evalResult;
		char buf[64];
		ftoa(decimalResult, buf, mainExprEntry.resultSignificantDigits, LCD_CHAR_EE);
		result->addString(buf);
	}
	else {
		result = calcResults[id];
	}
	// Set the location of the result
	if(resetLocation) {
		resultX = 128 - CURSOR_HORIZ_SPACING - 1 - result->exprWidth;
		resultY = 64 - CURSOR_VERT_SPACING - result->exprHeight;
		scrollExpr = false;
	}
	// Fill the area first
	display.fill(resultX - 1, resultY - 1, result->exprWidth + 2, result->exprHeight + 2, true);
	result->draw(display, resultX, resultY);
	resultWidth = result->exprWidth;
	resultHeight = result->exprHeight;
	// Clean up
	if(result != calcResults[id]) {
		delete result;
	}
	display.updateDrawing();
}

// Key press handlers
extern uint16_t selectorIndex;
bool editExpr = true;
bool scrollExpr = false;
bool asDecimal = false;
uint8_t currentExpr = 0;
uint8_t matRows = 0, matCols = 0;
constexpr uint16_t EXPR_SCROLL_SPEED = 4;
void exprKeyHandler(neda::Cursor *cursor, uint16_t key) {
    // If not editing or scrolling, ie normal result view
	if(!editExpr && !scrollExpr) {
        // Make sure the key pressed isn't one of these
        switch(key) {
        case KEY_CENTER:
        case KEY_UP:
        case KEY_DOWN:
        case KEY_APPROX:
        case KEY_CONFIG:
        case KEY_DUMMY:
            break;
        
        default:
        {
            neda::Container *newExpr;
            // If the key is a left or right, make a copy of the expression on display
            // Otherwise just insert a new expression
            if(key == KEY_LEFT || key == KEY_RIGHT) {
                newExpr = expressions[currentExpr]->copy();
            }
            else {
                newExpr = new neda::Container();
            }

            // Shift everything back
            if(calcResults[RESULT_STORE_COUNT - 1]) {
                delete calcResults[RESULT_STORE_COUNT - 1];
                delete expressions[RESULT_STORE_COUNT - 1];
                // Set to null pointers
                calcResults[RESULT_STORE_COUNT - 1] = expressions[RESULT_STORE_COUNT - 1] = nullptr;
            }
            for(uint8_t i = RESULT_STORE_COUNT - 1; i > 0; --i) {
                calcResults[i] = calcResults[i - 1];
                expressions[i] = expressions[i - 1];
            }
            calcResults[0] = expressions[0] = nullptr;

            newExpr->getCursor(*cursor, neda::CURSORLOCATION_END);
            newExpr->x = CURSOR_HORIZ_SPACING + 1;
            newExpr->y = CURSOR_VERT_SPACING;
            editExpr = true;
            scrollExpr = false;
            currentExpr = 0;
            
            display.clearDrawingBuffer();
            adjustExpr(cursor->expr->getTopLevel(), cursor);
            cursor->expr->drawConnected(display);
            cursor->draw(display);
            display.updateDrawing();
            return;
        }
        }
	}
/////////////////////////////////////////
	switch(key) {
	case KEY_CENTER:
		if(!editExpr) {
			scrollExpr = !scrollExpr;
		}
		return;
	case KEY_LEFT:
		if(!scrollExpr) {
			cursor->left();
		}
		else {
			// Verify that the expression is cut off
			if(resultX < 1) {
				resultX += EXPR_SCROLL_SPEED;
				drawResult(currentExpr, false);
			}
			return;
		}
		break;
	case KEY_RIGHT:
		if(!scrollExpr) {
			cursor->right();
		}
		else {
			// Verify that the expression is cut off
			if(resultX + resultWidth > 127) {
				resultX -= EXPR_SCROLL_SPEED;
			}
			drawResult(currentExpr, false);
			return;
		}
		break;
	case KEY_UP:
	{   
		if(editExpr) {
			cursor->up();
		}
		else if(scrollExpr) {
			// Verify that the result is cut off
			if(resultY < 1) {
				resultY += EXPR_SCROLL_SPEED;
				drawResult(currentExpr, false);
			}
			return;
		}
		else {
			if(currentExpr < RESULT_STORE_COUNT - 1 && expressions[currentExpr + 1]) {
				++currentExpr;
				asDecimal = false;
				drawResult(currentExpr);
			}
			return;
		}
		break;
	}
	case KEY_DOWN:
	{
		if(editExpr) {
			cursor->down();
		}
		else if(scrollExpr) {
			// Verify that the expression is cut off
			if(resultY + resultHeight > 63) {
				resultY -= EXPR_SCROLL_SPEED;
			}
			drawResult(currentExpr, false);
			return;
		}
		else {
			if(currentExpr >= 1) {
				--currentExpr;
				asDecimal = false;
				drawResult(currentExpr);
			}
			return;
		}
		break;
	}
	/* LETTER KEYS */
	case KEY_A:
		addChar(cursor, 'A');
		break;
	case KEY_B:
		addChar(cursor, 'B');
		break;
	case KEY_C:
		addChar(cursor, 'C');
		break;
	case KEY_D:
		addChar(cursor, 'D');
		break;
	case KEY_E:
		addChar(cursor, 'E');
		break;
	case KEY_F:
		addChar(cursor, 'F');
		break;
	case KEY_G:
		addChar(cursor, 'G');
		break;
	case KEY_H:
		addChar(cursor, 'H');
		break;
	case KEY_I:
		addChar(cursor, 'I');
		break;
	case KEY_J:
		addChar(cursor, 'J');
		break;
	case KEY_K:
		addChar(cursor, 'K');
		break;
	case KEY_L:
		addChar(cursor, 'L');
		break;
	case KEY_M:
		addChar(cursor, 'M');
		break;
	case KEY_N:
		addChar(cursor, 'N');
		break;
	case KEY_O:
		addChar(cursor, 'O');
		break;
	case KEY_P:
		addChar(cursor, 'P');
		break;
	case KEY_Q:
		addChar(cursor, 'Q');
		break;
	case KEY_R:
		addChar(cursor, 'R');
		break;
	case KEY_S:
		addChar(cursor, 'S');
		break;
	case KEY_T:
		addChar(cursor, 'T');
		break;
	case KEY_U:
		addChar(cursor, 'U');
		break;
	case KEY_V:
		addChar(cursor, 'V');
		break;
	case KEY_W:
		addChar(cursor, 'W');
		break;
	case KEY_X:
		addChar(cursor, 'X');
		break;
	case KEY_Y:
		addChar(cursor, 'Y');
		break;
	case KEY_Z:
		addChar(cursor, 'Z');
		break;
	case KEY_LCA:
		addChar(cursor, 'a');
		break;
	case KEY_LCB:
		addChar(cursor, 'b');
		break;
	case KEY_LCC:
		addChar(cursor, 'c');
		break;
	case KEY_LCD:
		addChar(cursor, 'd');
		break;
	case KEY_LCE:
		addChar(cursor, 'e');
		break;
	case KEY_LCF:
		addChar(cursor, 'f');
		break;
	case KEY_LCG:
		addChar(cursor, 'g');
		break;
	case KEY_LCH:
		addChar(cursor, 'h');
		break;
	case KEY_LCI:
		addChar(cursor, 'i');
		break;
	case KEY_LCJ:
		addChar(cursor, 'j');
		break;
	case KEY_LCK:
		addChar(cursor, 'k');
		break;
	case KEY_LCL:
		addChar(cursor, 'l');
		break;
	case KEY_LCM:
		addChar(cursor, 'm');
		break;
	case KEY_LCN:
		addChar(cursor, 'n');
		break;
	case KEY_LCO:
		addChar(cursor, 'o');
		break;
	case KEY_LCP:
		addChar(cursor, 'p');
		break;
	case KEY_LCQ:
		addChar(cursor, 'q');
		break;
	case KEY_LCR:
		addChar(cursor, 'r');
		break;
	case KEY_LCS:
		addChar(cursor, 's');
		break;
	case KEY_LCT:
		addChar(cursor, 't');
		break;
	case KEY_LCU:
		addChar(cursor, 'u');
		break;
	case KEY_LCV:
		addChar(cursor, 'v');
		break;
	case KEY_LCW:
		addChar(cursor, 'w');
		break;
	case KEY_LCX:
		addChar(cursor, 'x');
		break;
	case KEY_LCY:
		addChar(cursor, 'y');
		break;
	case KEY_LCZ:
		addChar(cursor, 'z');
		break;
	/* OTHER KEYS WITH NO SPECIAL HANDLING */
	case KEY_0:
		addChar(cursor, '0');
		break;
	case KEY_1:
		addChar(cursor, '1');
		break;
	case KEY_2:
		addChar(cursor, '2');
		break;
	case KEY_3:
		addChar(cursor, '3');
		break;
	case KEY_4:
		addChar(cursor, '4');
		break;
	case KEY_5:
		addChar(cursor, '5');
		break;
	case KEY_6:
		addChar(cursor, '6');
		break;
	case KEY_7:
		addChar(cursor, '7');
		break;
	case KEY_8:
		addChar(cursor, '8');
		break;
	case KEY_9:
		addChar(cursor, '9');
		break;
	case KEY_SPACE:
		addChar(cursor, ' ');
		break;
	case KEY_COMMA:
		addChar(cursor, ',');
		break;
	case KEY_DOT:
		addChar(cursor, '.');
		break;
	case KEY_PLUS:
		addChar(cursor, '+');
		break;
	case KEY_MINUS:
		addChar(cursor, '-');
		break;
	case KEY_MUL:
		addChar(cursor, LCD_CHAR_MUL);
		break;
	case KEY_DIV:
		addChar(cursor, LCD_CHAR_DIV);
		break;
	case KEY_PI:
		addChar(cursor, LCD_CHAR_PI);
		break;
	case KEY_EULER:
		addChar(cursor, LCD_CHAR_EULR);
		break;
	case KEY_EQUAL:
		addChar(cursor, '=');
		break;
	case KEY_EE:
		addChar(cursor, LCD_CHAR_EE);
		break;
	case KEY_CROSS:
		addChar(cursor, LCD_CHAR_CRS);
		break;
	/* EXPRESSIONS */
	case KEY_LBRACKET:
	{
		cursor->add(new neda::LeftBracket());
		cursor->expr->Expr::draw(display);
		break;
	}
	case KEY_RBRACKET:
	{
		cursor->add(new neda::RightBracket());
		cursor->expr->Expr::draw(display);
		break;
	}
	case KEY_ROOT:
	{
		neda::Radical *radical = new neda::Radical(new neda::Container, nullptr);
		cursor->add(radical);
		radical->getCursor(*cursor, neda::CURSORLOCATION_START);
		// Make sure the position is updated so adjustExpr will not mess up the display
		cursor->expr->parent->parent->draw(display);
		break;
	}
	case KEY_NTHROOT:
	{
		neda::Radical *radical = new neda::Radical(new neda::Container, new neda::Container);
		cursor->add(radical);
		radical->getCursor(*cursor, neda::CURSORLOCATION_START);
		// Make sure the position is updated so adjustExpr will not mess up the display
		cursor->expr->parent->parent->draw(display);
		break;
	}
	case KEY_SUM:
	{
		neda::SigmaPi *sigma = new neda::SigmaPi(lcd::CHAR_SUMMATION, new neda::Container(), new neda::Container(), new neda::Container());
		cursor->add(sigma);
		sigma->getCursor(*cursor, neda::CURSORLOCATION_START);
		// Make sure the position is updated so adjustExpr will not mess up the display
		cursor->expr->parent->parent->draw(display);
		break;
	}
	case KEY_PRODUCT:
	{
		neda::SigmaPi *product = new neda::SigmaPi(lcd::CHAR_PRODUCT, new neda::Container(), new neda::Container(), new neda::Container());
		cursor->add(product);
		product->getCursor(*cursor, neda::CURSORLOCATION_START);
		// Make sure the position is updated so adjustExpr will not mess up the display
		cursor->expr->parent->parent->draw(display);
		break;
	}
	case KEY_FRAC:
	{
		neda::Fraction *frac;
		// If there's a token in front of the cursor, enclose that in the fraction
		char ch;
		if(cursor->index != 0 && (cursor->expr->contents[cursor->index - 1]->getType() == neda::ObjType::MATRIX || (ch = eval::extractChar(cursor->expr->contents[cursor->index - 1]), eval::isDigit(ch) || eval::isNameChar(ch)))) {
			bool isNum;
			uint16_t end = eval::findTokenEnd(&cursor->expr->contents, cursor->index - 1, -1, isNum) + 1;
			uint16_t len = cursor->index - end;
			
			// Create a new array with the objects
			DynamicArray<neda::NEDAObj*> arr(cursor->expr->contents.begin() + end, cursor->expr->contents.begin() + cursor->index);
			cursor->index = end;
			// Remove the objects from the original array
			cursor->expr->contents.removeAt(end, len);
			frac = new neda::Fraction(new neda::Container(arr), new neda::Container());
			cursor->add(frac);
			frac->getCursor(*cursor, neda::CURSORLOCATION_END);
		}
		else {
			frac = new neda::Fraction(new neda::Container(), new neda::Container());
			cursor->add(frac);
			frac->getCursor(*cursor, neda::CURSORLOCATION_START);
		}
		// Make sure the position is updated so adjustExpr will not mess up the display
		cursor->expr->parent->parent->draw(display);
		break;
	}
	case KEY_SQUARE:
	case KEY_CUBE:
	case KEY_EXPONENT:
	{
		neda::Superscript *super = new neda::Superscript(neda::makeString(key == KEY_SQUARE ? "2" : (key == KEY_CUBE ? "3" : "")));
		cursor->add(super);
		// Only move the cursor if the exponent box is empty
		if(key == KEY_EXPONENT) {
			super->getCursor(*cursor, neda::CURSORLOCATION_START);
			// Make sure the position is updated so adjustExpr will not mess up the display
			cursor->expr->parent->parent->draw(display);
		}
		break;
	}
	case KEY_EXP:
	{
		cursor->add(new neda::Character(LCD_CHAR_EULR));
		neda::Superscript *super = new neda::Superscript(new neda::Container());
		cursor->add(super);
		super->getCursor(*cursor, neda::CURSORLOCATION_START);
		cursor->expr->parent->parent->draw(display);
		break;
	}
	case KEY_LN:
	{
		addStr(cursor, "ln");
		cursor->add(new neda::LeftBracket());
		break;
	}
	case KEY_LOG10:
	{
		addStr(cursor, "log");
		// cursor->add(new neda::Subscript(neda::makeString("10")));
		cursor->add(new neda::LeftBracket());
		break;
	}
	case KEY_LOGN:
	{
		addStr(cursor, "log");
		neda::Subscript *sub = new neda::Subscript(new neda::Container());
		cursor->add(sub);
		cursor->add(new neda::LeftBracket());
		sub->getCursor(*cursor, neda::CURSORLOCATION_START);
		cursor->expr->parent->parent->draw(display);
		break;
	}
	case KEY_ANS:
	{
		addStr(cursor, "Ans");
		break;
	}
	case KEY_MATRIX:
	{   
		// matRows == 0 means it was not set
		// So change mode to set matrix size
		if(matRows == 0) {
			matRows = matCols = 1;
			dispMode = DispMode::MATRIX_MENU;
			goto dispModeChange;
		}
		else {
			neda::Matrix *mat = new neda::Matrix(matRows, matCols);
			for(uint8_t i = 0; i < matRows; i ++) {
				for(uint8_t j = 0; j < matCols; j ++) {
					mat->setEntry(i, j, new neda::Container());
				}
			}
			// These have to be called manually since setting the entries do not trigger size recalculations
			mat->computeWidth();
			mat->computeHeight();

			cursor->add(mat);
			mat->getCursor(*cursor, neda::CURSORLOCATION_START);
			cursor->expr->parent->parent->draw(display);

			// Reset matRows
			matRows = 0;
		}
		break;
	}
	/* OTHER */
	case KEY_DELETE:
	{
		// Simple case: There is still stuff left before the cursor
		if(cursor->index != 0) {
			neda::NEDAObj *obj = cursor->expr->removeAtCursor(*cursor);
			delete obj;
		break;
		}
		// If there are no more characters to delete:
		// Confirm that the cursor is not in the top-level expression
		if(cursor->expr != cursor->expr->getTopLevel()) {
			// First put the cursor in the position before
			// The container the cursor is in must be inside some other expression, and then inside another container
			neda::Container *cont = (neda::Container*) cursor->expr->parent->parent;
			uint16_t index = cont->indexOf(cursor->expr->parent);
			// Manually change the position of the cursor
			cursor->expr = cont;
			cursor->index = index;
			// Remove and delete the elem
			neda::NEDAObj *obj = cont->remove(index);
			delete obj;
		}
		break;
	}
	case KEY_CLEARVAR:
	{
		expr::clearAll();
		break;
	}
	// AC does the same as regular clear except it deletes the stored expressions and variables as well
	case KEY_ALLCLEAR:
	{
		// Delete stored expressions
		for(uint8_t i = 0; i < RESULT_STORE_COUNT; i ++) {
			if(expressions[i]) {
				delete expressions[i];
				delete calcResults[i];
				expressions[i] = calcResults[i] = nullptr;
			}
		}
		expr::clearAll();
		// Intentional fall-through
	}
	case KEY_CLEAR:
	{
		display.clearDrawing();
		// Keep pointer of original
		neda::Expr *original = cursor->expr->getTopLevel();
		// Create new expression and change cursor location
		neda::Container *container = new neda::Container;
		container->getCursor(*cursor, neda::CURSORLOCATION_START);
		// Make sure the cursor's location is updated
		container->draw(display, 0, 0);
		// Delete old
		delete original;
		break;
	}
	case KEY_ENTER:
	{
evaluateExpression:
		editExpr = false;
		eval::Token *result = nullptr;
		neda::Container *expr = (neda::Container*) cursor->expr->getTopLevel();

		// First see if this is an assignment operation
		uint16_t equalsIndex = eval::findEquals(&expr->contents, false);
		if(equalsIndex != 0xFFFF) {
			// Isolate the variable name
			char *vName = new char[equalsIndex + 1];
			bool isFunc = false;
			bool isValid = true;
			uint16_t i = 0;
			for(; i < equalsIndex; i ++) {
				// Break when there's a left bracket since we found a function definition
				if(expr->contents[i]->getType() == neda::ObjType::L_BRACKET) {
					isFunc = true;
					break;
				}
				vName[i] = eval::extractChar(expr->contents[i]);
				if(!eval::isNameChar(vName[i])) {
					isValid = false;
					break;
				}
			}
			vName[i] = '\0';
			// If not valid or if the length is zero, cleanup and exit
			if(!isValid || i == 0) {
				delete[] vName;
			}
			else {
				if(isFunc) {
					// Now that the name has been isolated, do the same with each of the arguments
					uint16_t argStart = i + 1;
					uint16_t argEnd = i + 1;
					uint8_t argc = 0;
					DynamicArray<char*> argNames;

					// Find the closing bracket
					uint16_t end = argStart;
					for(; end < equalsIndex && expr->contents[end]->getType() != neda::ObjType::R_BRACKET; ++end);
					// Missing right bracket
					if(end == equalsIndex) {
						delete[] vName;
					}
					else {
						while(argStart < end) {
							// Find the end of each argument
							for(; argEnd < end; ++argEnd) {
								if(eval::extractChar(expr->contents[argEnd]) == ',') {
									break;
								}
							}
							// Extract argument name
							char *argName = new char[argEnd - argStart + 1];
							for(uint16_t i = 0; i < argEnd - argStart; ++i) {
								argName[i] = eval::extractChar(expr->contents[argStart + i]);
							}
							// Null termination
							argName[argEnd - argStart] = '\0';
							argNames.add(argName);
							++argc;

							++argEnd;
							// If there's a space after the comma, skip that too
							if(eval::extractChar(expr->contents[argEnd]) == ' ') {
								++argEnd;
							}
							argStart = argEnd;
						}

						// Now we should have all the arguments
						// Make a new array with the argument names since the DynamicArray's contents will be automatically freed
						char **argn = new char*[argc];
						// memcpy it in
						// Make sure the size is multiplied by the sizeof a char*
						memcpy(argn, argNames.asArray(), argc * sizeof(char*));

						// Make a new container that will hold the expression
						neda::Container *funcExpr = new neda::Container();
						for(uint16_t i = equalsIndex + 1; i < expr->contents.length(); ++i) {
							// Add each expression in
							// Make sure they're copied; otherwise segfaults will occur when this expr is deleted
							funcExpr->add(expr->contents[i]->copy());
						}

						// Finally add the damn thing
						expr::updateFunc(vName, funcExpr, argc, const_cast<const char**>(argn));
						// Update the result to 1 to signify the operation succeeded
						result = new eval::Number(1);
					}
				}
				else {
					// Evaluate
					DynamicArray<neda::NEDAObj*> val(expr->contents.begin() + equalsIndex + 1, expr->contents.end());
					result = eval::evaluate(&val, static_cast<uint8_t>(expr::varNames.length()), const_cast<const char**>(expr::varNames.asArray()), 
                            expr::varVals.asArray(), static_cast<uint8_t>(expr::functions.length()), expr::functions.asArray());

					// If result is valid, add the variable
					if(result) {
						// Create a copy since the result is shared between the history and the variable values
						eval::Token *value;
						if(result->getType() == eval::TokenType::NUMBER) {
							value = new eval::Number(((eval::Number*) result)->value);
						}
						else if(result->getType() == eval::TokenType::FRACTION) {
							value = new eval::Fraction(((eval::Fraction*) result)->num, ((eval::Fraction*) result)->denom);
						}
						else {
							value = new eval::Matrix(*((eval::Matrix*) result));
						}
						// Add it
						expr::updateVar(vName, value);
					}
					else {
						// Delete the variable name to avoid a memory leak
						delete[] vName;
					}
				}
			}
		}
		else {
			result = eval::evaluate(expr, static_cast<uint8_t>(expr::varNames.length()), const_cast<const char**>(expr::varNames.asArray()), 
                    expr::varVals.asArray(), static_cast<uint8_t>(expr::functions.length()), expr::functions.asArray());
		}
		// Create the container that will hold the result
		calcResults[0] = new neda::Container();
		if(!result) {
			calcResults[0]->add(new neda::Character(LCD_CHAR_SERR));
		}
		else {
			// Note: result is not deleted since updateVar needs it
			if(result->getType() == eval::TokenType::NUMBER) {
				if(isnan(((eval::Number*) result)->value)) {
					// No complex numbers allowed!!
					calcResults[0]->add(new neda::Character('\xff'));
				}
				else {
					char buf[64];
					// Convert the result and store it
					ftoa(((eval::Number*) result)->value, buf, mainExprEntry.resultSignificantDigits, LCD_CHAR_EE);
					calcResults[0]->addString(buf);
				}
			}
			else if(result->getType() == eval::TokenType::FRACTION) {
				char buf[64];
				neda::Container *num = new neda::Container();
				neda::Container *denom = new neda::Container();

				// Display negative fractions with the minus sign in front
				if(((eval::Fraction*) result)->num < 0) {
					calcResults[0]->add(new neda::Character('-'));
				}

				ltoa(labs(((eval::Fraction*) result)->num), buf);
				num->addString(buf);
				ltoa(((eval::Fraction*) result)->denom, buf);
				denom->addString(buf);

				calcResults[0]->add(new neda::Fraction(num, denom));
			}
			// Matrix
			else {
				char buf[64];
				eval::Matrix *mat = (eval::Matrix*) result;
				// Create NEDA matrix
				neda::Matrix *nMat = new neda::Matrix(mat->m, mat->n);
				// Go through every entry
				for(uint16_t i = 0; i < mat->m * mat->n; i ++) {
					// Create a container for the entry
					neda::Container *cont = new neda::Container();
					nMat->contents[i] = cont;
					// Convert the number
					ftoa(mat->contents[i], buf, mainExprEntry.resultSignificantDigits, LCD_CHAR_EE);
					cont->addString(buf);
				}
				nMat->computeWidth();
				nMat->computeHeight();

				calcResults[0]->add(nMat);
			}

			// Now update the value of the Ans variable
			// Var names must be allocated on the heap
			char *name = new char[4];
			strcpy(name, "Ans");
			expr::updateVar(name, result);
		}
		expressions[0] = (neda::Container*) cursor->expr->getTopLevel();

		currentExpr = 0;
		asDecimal = key == KEY_APPROX;
		drawResult(0, true);
		return;
	}
	case KEY_APPROX:
	{
		if(!editExpr) {
			asDecimal = true;
			drawResult(currentExpr);
			return;
		}
		else {
			goto evaluateExpression;
		}
	}
	case KEY_CONST:
		dispMode = DispMode::CONST_MENU;
		goto dispModeChange;
	case KEY_TRIG:
		dispMode = DispMode::TRIG_MENU;
		goto dispModeChange;
	case KEY_CONFIG:
		dispMode = DispMode::CONFIG_MENU;
		goto dispModeChange;
	case KEY_CAT:
		dispMode = DispMode::FUNC_MENU;
		goto dispModeChange;
	case KEY_RECALL:
		dispMode = DispMode::RECALL_MENU;
// Set the display mode and reset the index
dispModeChange:
		selectorIndex = 0;
		// We need to call the function once to get the interface drawn
		// To do this, we insert a dummy value into the key buffer
		putKey(KEY_DUMMY);
		break;
	default: break;
	}

	display.clearDrawingBuffer();
	adjustExpr(cursor->expr->getTopLevel(), cursor);
	cursor->expr->drawConnected(display);
	cursor->draw(display);
	display.updateDrawing();
}

uint16_t selectorIndex = 0;

const char * const trigFuncs[] = {
	"sin", "cos", "tan", "arcsin", "arccos", "arctan",
	"sinh", "cosh", "tanh", "arcsinh", "arccosh", "arctanh",
};
const char * const trigFuncNames[] = {
	"sin", "cos", "tan", "asin", "acos", "atan",
	"sinh", "cosh", "tanh", "asinh", "acosh", "atanh",
};
void trigKeyHandler(neda::Cursor *cursor, uint16_t key) {

	switch(key) {
	case KEY_CENTER:
	case KEY_ENTER:
		// Insert the chars
		addStr(cursor, trigFuncNames[selectorIndex]);
		cursor->add(new neda::LeftBracket());
		cursor->expr->Expr::draw(display);
		// Intentional fall-through
	case KEY_TRIG:
    case KEY_DELETE:
		dispMode = DispMode::EXPR_ENTRY;
		selectorIndex = 0;
		// We need to call the function once to get the interface drawn
		// To do this, we insert a dummy value into the key buffer
		putKey(KEY_DUMMY);
		return;
	case KEY_UP:
		if(selectorIndex > 0) {
			--selectorIndex;
		}
		else {
			selectorIndex = 11;
		}
		break;
	case KEY_DOWN:
		++selectorIndex;
		if(selectorIndex >= 12) {
			selectorIndex = 0;
		}
		break;
	case KEY_LEFT:
		if(selectorIndex >= 6) {
			selectorIndex -= 6;
		}
		break;
	case KEY_RIGHT:
		if(selectorIndex < 6) {
			selectorIndex += 6;
		}
		break;
	default: break;
	}

	display.clearDrawingBuffer();
	int16_t y = 1;
	for(uint8_t i = 0; i < 12; i ++) {
		// Reset y if we are in the second column
		if(i == 6) {
			y = 1;
		}

		if(i < 6) {
			display.drawString(1, y, trigFuncs[i], selectorIndex == i);
		}
		else {
			display.drawString(64, y, trigFuncs[i], selectorIndex == i);
		}
		y += 10;
	}

	display.updateDrawing();
}

const char *constantNames[] = {
	LCD_STR_PI, LCD_STR_EULR, LCD_STR_AVGO, LCD_STR_ECHG, LCD_STR_VLIG, LCD_STR_AGV,
};
void constKeyHandler(neda::Cursor *cursor, uint16_t key) {
	switch(key) {
	case KEY_CENTER:
	case KEY_ENTER:
		// Insert the chars
		addStr(cursor, constantNames[selectorIndex]);
		cursor->expr->Expr::draw(display);
		// Intentional fall-through
	case KEY_CONST:
    case KEY_DELETE:
		dispMode = DispMode::EXPR_ENTRY;
		selectorIndex = 0;
		// We need to call the function once to get the interface drawn
		// To do this, we insert a dummy value into the key buffer
		putKey(KEY_DUMMY);
		return;
	case KEY_UP:
		if(selectorIndex > 0) {
			--selectorIndex;
		}
		else {
			selectorIndex = 5;
		}
		break;
	case KEY_DOWN:
		++selectorIndex;
		if(selectorIndex >= 6) {
			selectorIndex = 0;
		}
		break;
	default: break;
	}

	display.clearDrawingBuffer();
	int16_t y = 1;
	for(uint8_t i = 0; i < 6; i ++) {
		display.drawString(1, y, constantNames[i], selectorIndex == i);
		y += 10;
	}

	display.updateDrawing();
}

#define BUILTIN_FUNC_COUNT 24
uint16_t funcCount = BUILTIN_FUNC_COUNT;
#define FUNC_SCROLLBAR_WIDTH 4
const char * const allFuncDispNames[BUILTIN_FUNC_COUNT] = {
	"sin(angle)", "cos(angle)", "tan(angle)", "asin(x)", "acos(x)", "atan(x)", 
	"sinh(angle)", "cosh(angle)", "tanh(angle)", "asinh(x)", "acosh(x)", "atanh(x)",
	"ln(x)", "qdRtA(a,b,c)", "qdRtB(a,b,c)", "round(n,decimals)", "abs(x)", "fact(x)",
	"det(A)", "len(v)", "transpose(A)", "inv(A)", "I(n)", "linSolve(A)"
};

uint16_t scrollingIndex = 0;
void scrollUp(uint16_t len) {
	if(selectorIndex > 0) {
		--selectorIndex;
		// Scrolling
		if(selectorIndex < scrollingIndex) {
			--scrollingIndex;
		}
	}
	else {
		selectorIndex = len - 1;
		scrollingIndex = max(len - 6, 0);
	}
}
void scrollDown(uint16_t len) {
	if(selectorIndex < len - 1) {
		++selectorIndex;
		// Scrolling
		if(scrollingIndex + 6 <= selectorIndex) {
			++scrollingIndex;
		}
	}
	else {
		selectorIndex = 0;
		scrollingIndex = 0;
	}
}
void funcKeyHandler(neda::Cursor *cursor, uint16_t key) {
	funcCount = BUILTIN_FUNC_COUNT + expr::functions.length();
	switch(key) {
	case KEY_CENTER:
	case KEY_ENTER:
	{
		if(selectorIndex < BUILTIN_FUNC_COUNT) {
			const char *s = allFuncDispNames[selectorIndex];
			// Add until we see the null terminator or the left bracket
			while(*s != '\0' && *s != '(') {
				cursor->add(new neda::Character(*s++));
			}
		}
		else {
			addStr(cursor, expr::functions[selectorIndex - BUILTIN_FUNC_COUNT].name);
		}
		cursor->add(new neda::LeftBracket);
	}
	// Intentional fall-through
	case KEY_CAT:
    case KEY_DELETE:
		dispMode = DispMode::EXPR_ENTRY;
		selectorIndex = 0;
		scrollingIndex = 0;
		// We need to call the function once to get the interface drawn
		// To do this, we insert a dummy value into the key buffer
		putKey(KEY_DUMMY);
		return;
	case KEY_UP:
		scrollUp(funcCount);
		break;
	case KEY_DOWN:
		scrollDown(funcCount);
		break;
	default: break;
	}

	display.clearDrawingBuffer();
	int16_t y = 1;
	for(uint8_t i = scrollingIndex; i < scrollingIndex + 6; i ++) {
		if(i < BUILTIN_FUNC_COUNT) {
			display.drawString(1, y, allFuncDispNames[i], selectorIndex == i);
		}
		else {
			display.drawString(1, y, expr::functions[i - BUILTIN_FUNC_COUNT].fullname, selectorIndex == i);
		}
		y += 10;
	}
	uint16_t scrollbarLocation = static_cast<uint16_t>(scrollingIndex * 64 / funcCount);
	uint16_t scrollbarHeight = 6 * 64 / funcCount;
	display.fill(128 - FUNC_SCROLLBAR_WIDTH, scrollbarLocation, FUNC_SCROLLBAR_WIDTH, scrollbarHeight);
	display.updateDrawing();
}

void recallKeyHandler(neda::Cursor *cursor, uint16_t key) {
	switch(key) {
	case KEY_CENTER:
	case KEY_ENTER:
	{
		if(expr::functions.length() > 0) {
			for(auto ex : expr::functions[selectorIndex].expr->contents) {
				cursor->add(ex->copy());
			}
		}
	}
	// Intentional fall-through
	case KEY_RECALL:
    case KEY_CAT:
    case KEY_DELETE:
		dispMode = DispMode::EXPR_ENTRY;
		selectorIndex = 0;
		scrollingIndex = 0;
		// We need to call the function once to get the interface drawn
		// To do this, we insert a dummy value into the key buffer
		putKey(KEY_DUMMY);
		return;
	case KEY_UP:
		scrollUp(expr::functions.length());
		break;
	case KEY_DOWN:
		scrollDown(expr::functions.length());
		break;
	default: break;
	}

	display.clearDrawingBuffer();
	if(expr::functions.length() == 0) {
		display.drawString(1, 1, "No Functions to");
		display.drawString(1, 11, "Recall");
	}
	else {
		int16_t y = 1;
		for(uint8_t i = scrollingIndex; i < scrollingIndex + 6 && i < expr::functions.length(); i ++) {
			display.drawString(1, y, expr::functions[i].fullname, selectorIndex == i);
			y += 10;
		}

		uint16_t scrollbarLocation = static_cast<uint16_t>(scrollingIndex * 64 / expr::functions.length());
		uint16_t scrollbarHeight = 6 * 64 / expr::functions.length();
		display.fill(128 - FUNC_SCROLLBAR_WIDTH, scrollbarLocation, FUNC_SCROLLBAR_WIDTH, scrollbarHeight);
	}
	display.updateDrawing();
}

void configKeyHandler(uint16_t key) {
	switch(key) {
	case KEY_CENTER:
	case KEY_ENTER:
	case KEY_CONFIG:
    case KEY_DELETE:
		dispMode = DispMode::EXPR_ENTRY;
		selectorIndex = 0;
		// We need to call the function once to get the interface drawn
		// To do this, we insert a dummy value into the key buffer
		putKey(KEY_DUMMY);
		return;
	case KEY_LEFT:
        if(selectorIndex == 0) {
            eval::useRadians = !eval::useRadians;
        }
        else if(selectorIndex == 1 && mainExprEntry.resultSignificantDigits > 1) {
            mainExprEntry.resultSignificantDigits --;
        }
		break;
	case KEY_RIGHT:
        if(selectorIndex == 0) {
		    eval::useRadians = !eval::useRadians;
        }
        else if(selectorIndex == 1 && mainExprEntry.resultSignificantDigits < 20) {
            mainExprEntry.resultSignificantDigits ++;
        }
		break;
    // Currently there are only two options, so this is good enough
    case KEY_UP:
    case KEY_DOWN:
        selectorIndex = !selectorIndex;
        break;
	default: break;
	}

	display.clearDrawingBuffer();
	display.drawString(1, 1, "Angles:");
	display.drawString(80, 1, eval::useRadians ? "Radians" : "Degrees", selectorIndex == 0);
    display.drawString(1, 11, "Result S.D.:");
    char buf[3];
    ltoa(mainExprEntry.resultSignificantDigits, buf);
    display.drawString(80, 11, buf, selectorIndex == 1);
	display.updateDrawing();
}

void matrixKeyHandler(uint16_t key) {
	switch(key) {
	case KEY_CENTER:
	case KEY_ENTER:
		dispMode = DispMode::EXPR_ENTRY;
		selectorIndex = 0;
		// Insert a KEY_MATRIX so we go back to the matrix entry processing
		putKey(KEY_MATRIX);
		return;
	case KEY_LEFT:
	case KEY_RIGHT:
		selectorIndex = !selectorIndex;
		break;
	case KEY_UP:
		if(selectorIndex == 0) {
			if(matRows < 255) {
				matRows ++;
			}
		}
		else {
			if(matCols < 255) {
				matCols ++;
			}
		}
		break;
	case KEY_DOWN:
		if(selectorIndex == 0) {
			if(matRows > 1) {
				matRows --;
			}
		}
		else {
			if(matCols > 1) {
				matCols --;
			}
		}
		break;
	}

	display.clearDrawingBuffer();
	display.drawString(1, 1, "Matrix Size:");

	char sizeBuf[8];
	uint8_t len = ltoa(matRows, sizeBuf);
	display.drawString(48, 13, sizeBuf, selectorIndex == 0);
	display.drawString(48 + len * 6, 13, "x");
	ltoa(matCols, sizeBuf);
	display.drawString(54 + len * 6, 13, sizeBuf, selectorIndex == 1);

	display.updateDrawing();
}

void gameKeyPressHandler(uint16_t key) {
	switch(key) {
	case KEY_LEFT:
		if(direction != game::SnakeDirection::RIGHT) {
			direction = game::SnakeDirection::LEFT;
		}
		break;
	case KEY_RIGHT:
		if(direction != game::SnakeDirection::LEFT) {
			direction = game::SnakeDirection::RIGHT;
		}
		break;
	case KEY_UP:
		if(direction != game::SnakeDirection::DOWN) {
			direction = game::SnakeDirection::UP;
		}
		break;
	case KEY_DOWN:
		if(direction != game::SnakeDirection::UP) {
			direction = game::SnakeDirection::DOWN;
		}
		break;
	case KEY_LCP:
		gamePaused = !gamePaused;
		break;
	default:
		break;
	}
}

void evaluateExpr(neda::Container*);
void normalKeyPressHandler(uint16_t key) {
	if(exprEditMode == ExprEditMode::RESULT) {
        // Special processing for these keys
        switch(key) {
        // Go into result scroll mode
        case KEY_CENTER:
            exprEditMode = ExprEditMode::RESULT_SCROLL;
            break;
        // Show the previous result
        case KEY_UP:
            if(currentExpr < RESULT_STORE_COUNT - 1 && expressions[currentExpr + 1]) {
				++currentExpr;
				asDecimal = false;
				drawResult(currentExpr);
			}
            break;
        // Show the next result
        case KEY_DOWN:
            if(currentExpr >= 1) {
				--currentExpr;
				asDecimal = false;
				drawResult(currentExpr);
			}
            break;
        // Display the current result with a different method
        case KEY_APPROX:
            asDecimal = !asDecimal;
            drawResult(currentExpr);
            break;
        case KEY_DUMMY:
            break;
        // All other keys create a new expression
        default:
        {
            neda::Container *newExpr;
            // If the key is a left or right, make a copy of the expression on display
            // Otherwise just insert a new expression
            if(key == KEY_LEFT || key == KEY_RIGHT) {
                newExpr = expressions[currentExpr]->copy();
            }
            else {
                newExpr = new neda::Container();
            }

            // Shift everything back
            if(calcResults[RESULT_STORE_COUNT - 1]) {
                delete calcResults[RESULT_STORE_COUNT - 1];
                delete expressions[RESULT_STORE_COUNT - 1];
                // Set to null pointers
                calcResults[RESULT_STORE_COUNT - 1] = expressions[RESULT_STORE_COUNT - 1] = nullptr;
            }
            for(uint8_t i = RESULT_STORE_COUNT - 1; i > 0; --i) {
                calcResults[i] = calcResults[i - 1];
                expressions[i] = expressions[i - 1];
            }
            calcResults[0] = expressions[0] = nullptr;

            newExpr->getCursor(*mainExprEntry.cursor, neda::CURSORLOCATION_END);
            newExpr->x = CURSOR_HORIZ_SPACING + 1;
            newExpr->y = CURSOR_VERT_SPACING + 1;
            exprEditMode = ExprEditMode::NORMAL;
            currentExpr = 0;
            
            display.clearDrawingBuffer();
            mainExprEntry.adjustExpr();
            mainExprEntry.cursor->expr->drawConnected(display);
            mainExprEntry.cursor->draw(display);
            display.updateDrawing();
        }
        }
	}
    // Otherwise if in scrolling mode
    else if(exprEditMode == ExprEditMode::RESULT_SCROLL) {
        switch(key) {
        case KEY_CENTER:
            exprEditMode = ExprEditMode::RESULT;
            break;
        case KEY_LEFT:
            // Verify that the expression is cut off
            if(resultX < 1) {
                resultX += EXPR_SCROLL_SPEED;
                drawResult(currentExpr, false);
            }
            break;
        case KEY_RIGHT:
            // Verify that the expression is cut off
            if(resultX + resultWidth > 127) {
                resultX -= EXPR_SCROLL_SPEED;
            }
            drawResult(currentExpr, false);
            break;
        case KEY_UP:
            // Verify that the result is cut off
            if(resultY < 1) {
                resultY += EXPR_SCROLL_SPEED;
                drawResult(currentExpr, false);
            }
            break;
        case KEY_DOWN:
            // Verify that the expression is cut off
            if(resultY + resultHeight > 63) {
                resultY -= EXPR_SCROLL_SPEED;
            }
            drawResult(currentExpr, false);
            break;
        // Ignore all other keys
        default: 
            break;
        }
    }
    // Normal mode
    else {
        // Special processing for some keys when in normal mode
        if(mainExprEntry.mode == expr::ExprEntry::DisplayMode::NORMAL) {
            switch(key) {
            case KEY_ENTER:
                evaluateExpr(static_cast<neda::Container*>(mainExprEntry.cursor->expr->getTopLevel()));
                asDecimal = false;
                drawResult(0, true);
                break;
            case KEY_APPROX:
                evaluateExpr(static_cast<neda::Container*>(mainExprEntry.cursor->expr->getTopLevel()));
                asDecimal = true;
                drawResult(0, true);
                break;
            // Forward others to handleKeyPress()
            default:
                mainExprEntry.handleKeyPress(key);
                break;
            }
        }
        // For other keys, forward to handleKeyPress()
        else {
            mainExprEntry.handleKeyPress(key);
        }
    }
}

// Evaluates the expression
// Handles evaluation and things like variable definitions
void evaluateExpr(neda::Container *expr) {
    exprEditMode = ExprEditMode::RESULT;
    eval::Token *result = nullptr;

    // First see if this is an assignment operation
    uint16_t equalsIndex = eval::findEquals(&expr->contents, false);
    if(equalsIndex != 0xFFFF) {
        // Isolate the variable name
        char *vName = new char[equalsIndex + 1];
        bool isFunc = false;
        bool isValid = true;
        uint16_t i = 0;
        for(; i < equalsIndex; i ++) {
            // Break when there's a left bracket since we found a function definition
            if(expr->contents[i]->getType() == neda::ObjType::L_BRACKET) {
                isFunc = true;
                break;
            }
            vName[i] = eval::extractChar(expr->contents[i]);
            if(!eval::isNameChar(vName[i])) {
                isValid = false;
                break;
            }
        }
        vName[i] = '\0';
        // If not valid or if the length is zero, cleanup and exit
        if(!isValid || i == 0) {
            delete[] vName;
        }
        else {
            if(isFunc) {
                // Now that the name has been isolated, do the same with each of the arguments
                uint16_t argStart = i + 1;
                uint16_t argEnd = i + 1;
                uint8_t argc = 0;
                DynamicArray<char*> argNames;

                // Find the closing bracket
                uint16_t end = argStart;
                for(; end < equalsIndex && expr->contents[end]->getType() != neda::ObjType::R_BRACKET; ++end);
                // Missing right bracket
                if(end == equalsIndex) {
                    delete[] vName;
                }
                else {
                    while(argStart < end) {
                        // Find the end of each argument
                        for(; argEnd < end; ++argEnd) {
                            if(eval::extractChar(expr->contents[argEnd]) == ',') {
                                break;
                            }
                        }
                        // Extract argument name
                        char *argName = new char[argEnd - argStart + 1];
                        for(uint16_t i = 0; i < argEnd - argStart; ++i) {
                            argName[i] = eval::extractChar(expr->contents[argStart + i]);
                        }
                        // Null termination
                        argName[argEnd - argStart] = '\0';
                        argNames.add(argName);
                        ++argc;

                        ++argEnd;
                        // If there's a space after the comma, skip that too
                        if(eval::extractChar(expr->contents[argEnd]) == ' ') {
                            ++argEnd;
                        }
                        argStart = argEnd;
                    }

                    // Now we should have all the arguments
                    // Make a new array with the argument names since the DynamicArray's contents will be automatically freed
                    char **argn = new char*[argc];
                    // memcpy it in
                    // Make sure the size is multiplied by the sizeof a char*
                    memcpy(argn, argNames.asArray(), argc * sizeof(char*));

                    // Make a new container that will hold the expression
                    neda::Container *funcExpr = new neda::Container();
                    for(uint16_t i = equalsIndex + 1; i < expr->contents.length(); ++i) {
                        // Add each expression in
                        // Make sure they're copied; otherwise segfaults will occur when this expr is deleted
                        funcExpr->add(expr->contents[i]->copy());
                    }

                    // Finally add the damn thing
                    expr::updateFunc(vName, funcExpr, argc, const_cast<const char**>(argn));
                    // Update the result to 1 to signify the operation succeeded
                    result = new eval::Number(1);
                }
            }
            else {
                // Evaluate
                DynamicArray<neda::NEDAObj*> val(expr->contents.begin() + equalsIndex + 1, expr->contents.end());
                result = eval::evaluate(&val, static_cast<uint8_t>(expr::varNames.length()), const_cast<const char**>(expr::varNames.asArray()), 
                        expr::varVals.asArray(), static_cast<uint8_t>(expr::functions.length()), expr::functions.asArray());

                // If result is valid, add the variable
                if(result) {
                    // Create a copy since the result is shared between the history and the variable values
                    eval::Token *value;
                    if(result->getType() == eval::TokenType::NUMBER) {
                        value = new eval::Number(((eval::Number*) result)->value);
                    }
                    else if(result->getType() == eval::TokenType::FRACTION) {
                        value = new eval::Fraction(((eval::Fraction*) result)->num, ((eval::Fraction*) result)->denom);
                    }
                    else {
                        value = new eval::Matrix(*((eval::Matrix*) result));
                    }
                    // Add it
                    expr::updateVar(vName, value);
                }
                else {
                    // Delete the variable name to avoid a memory leak
                    delete[] vName;
                }
            }
        }
    }
    else {
        result = eval::evaluate(expr, static_cast<uint8_t>(expr::varNames.length()), const_cast<const char**>(expr::varNames.asArray()), 
                expr::varVals.asArray(), static_cast<uint8_t>(expr::functions.length()), expr::functions.asArray());
    }
    // Create the container that will hold the result
    calcResults[0] = new neda::Container();
    if(!result) {
        calcResults[0]->add(new neda::Character(LCD_CHAR_SERR));
    }
    else {
        // Note: result is not deleted since updateVar needs it
        if(result->getType() == eval::TokenType::NUMBER) {
            if(isnan(((eval::Number*) result)->value)) {
                // No complex numbers allowed!!
                calcResults[0]->add(new neda::Character('\xff'));
            }
            else {
                char buf[64];
                // Convert the result and store it
                ftoa(((eval::Number*) result)->value, buf, mainExprEntry.resultSignificantDigits, LCD_CHAR_EE);
                calcResults[0]->addString(buf);
            }
        }
        else if(result->getType() == eval::TokenType::FRACTION) {
            char buf[64];
            neda::Container *num = new neda::Container();
            neda::Container *denom = new neda::Container();

            // Display negative fractions with the minus sign in front
            if(((eval::Fraction*) result)->num < 0) {
                calcResults[0]->add(new neda::Character('-'));
            }

            ltoa(labs(((eval::Fraction*) result)->num), buf);
            num->addString(buf);
            ltoa(((eval::Fraction*) result)->denom, buf);
            denom->addString(buf);

            calcResults[0]->add(new neda::Fraction(num, denom));
        }
        // Matrix
        else {
            char buf[64];
            eval::Matrix *mat = (eval::Matrix*) result;
            // Create NEDA matrix
            neda::Matrix *nMat = new neda::Matrix(mat->m, mat->n);
            // Go through every entry
            for(uint16_t i = 0; i < mat->m * mat->n; i ++) {
                // Create a container for the entry
                neda::Container *cont = new neda::Container();
                nMat->contents[i] = cont;
                // Convert the number
                ftoa(mat->contents[i], buf, mainExprEntry.resultSignificantDigits, LCD_CHAR_EE);
                cont->addString(buf);
            }
            nMat->computeWidth();
            nMat->computeHeight();

            calcResults[0]->add(nMat);
        }

        // Now update the value of the Ans variable
        // Var names must be allocated on the heap
        char *name = new char[4];
        strcpy(name, "Ans");
        expr::updateVar(name, result);
    }
    expressions[0] = expr;

    currentExpr = 0;
}

int main() {
	// Init system
	sys::initRCC();
	sys::initNVIC();
	usart::init(115200);
	// Init LEDs
	statusLED.init(GPIO_Mode_Out_PP, GPIO_Speed_2MHz);
	shiftLED.init(GPIO_Mode_Out_PP, GPIO_Speed_2MHz);
	ctrlLED.init(GPIO_Mode_Out_PP, GPIO_Speed_2MHz);
	statusLED = false;
	shiftLED = false;
	ctrlLED = false;

	// Disable JTAG so we can use PB3 and 4
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);

	// Set up SBDI receiver
	sbdi::Receiver receiver(SBDI_EN, SBDI_DATA, SBDI_CLK);
	receiver.init();
	
	receiver.onReceive([](uint32_t data) {
        usart::printf("%#010X\n", data);
		// Store keystroke into buffer
		// If there is already data in the buffer then shift that data left to make room
		if(data == KEY_SHIFTON) {
			shiftLED = true;
		}
		else if(data == KEY_SHIFTOFF) {
			shiftLED = false;
		}
		else if(data == KEY_CTRLON) {
			ctrlLED = true;
		}
		else if(data == KEY_CTRLOFF) {
			ctrlLED = false;
		}
		else {
			putKey(data);
		}
		statusLED = !statusLED;
	});

	// Startup delay
	delay::ms(100);

	// Initialize display
	display.init();
	display.useExtended();
	display.startDraw();
	display.clearDrawing();

	display.drawString(32, 25, "TCalc " VERSION_STR, true);
	display.updateDrawing();

	// Title screen delay
	delay::ms(1500);

	if(fetchKey() == KEY_LCT) {
		dispMode = DispMode::GAME;
		putKey(KEY_DUMMY);

		respawn();
	}

	// Start blink
	initCursorTimer(dispMode == DispMode::GAME ? 1000 : 2000);

    display.clearDrawing();
    if(dispMode == DispMode::EXPR_ENTRY) {
        mainExprEntry.adjustExpr();
        mainExprEntry.cursor->expr->drawConnected(display);
        mainExprEntry.cursor->draw(display);
        display.updateDrawing();
    }


	uint16_t key = KEY_NULL;
	
	SBDI_EN.init(GPIO_Mode_IN_FLOATING, GPIO_Speed_10MHz);
	while(true) {
		if((key = fetchKey()) != KEY_NULL) {
			switch(dispMode) {
            case DispMode::EXPR_ENTRY:
                normalKeyPressHandler(key);
                break;
            case DispMode::GAME:
                gameKeyPressHandler(key);
                break;
            }
		}
	}
}
