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

bool cursorOn = false;
neda::Cursor *cursor;
extern bool editExpr;
extern "C" void TIM3_IRQHandler() {
	if(TIM_GetITStatus(TIM3, TIM_IT_Update)) {
		TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
        if(dispMode == DispMode::EXPR_ENTRY && editExpr) {
            cursorOn = !cursorOn;
            display.clearDrawingBuffer();
            // Redraw the entire expr
            cursor->expr->Expr::drawConnected(display);
            if(cursorOn) {
                cursor->draw(display);
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

#define CURSOR_HORIZ_SPACING 1
#define CURSOR_VERT_SPACING 1
//Moves ex so that the cursor is in the display
void adjustExpr(neda::Expr *ex, neda::Cursor *cursorRef) {
	neda::CursorInfo info;
	cursorRef->getInfo(info);
    // First try to directly position the expression in the top-left corner of the display
    uint16_t xd = CURSOR_HORIZ_SPACING - ex->x;
    uint16_t yd = CURSOR_VERT_SPACING - ex->y;
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
//Previous expressions and their results
neda::Container *calcResults[RESULT_STORE_COUNT] = { nullptr };
neda::Container *expressions[RESULT_STORE_COUNT] = { nullptr };
void drawResult(uint8_t id, bool asDecimal = false) {
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
        ftoa(decimalResult, buf, 16, LCD_CHAR_EE);
        result->addString(buf);
    }
    else {
        result = calcResults[id];
    }
    // Fill the area first
    display.fill(128 - CURSOR_HORIZ_SPACING - 1 - result->exprWidth, 64 - CURSOR_VERT_SPACING - result->exprHeight, result->exprWidth, result->exprHeight, true);
    result->draw(display, 128 - CURSOR_HORIZ_SPACING - 1 - result->exprWidth, 64 - CURSOR_VERT_SPACING - result->exprHeight);
    // Clean up
    if(result != calcResults[id]) {
        delete result;
    }
    display.updateDrawing();
}

//Variables
DynamicArray<const char*> varNames;
DynamicArray<eval::Token*> varVals;
//Functions
DynamicArray<eval::UserDefinedFunction> functions;
//Returns whether a new variable was created
void updateVar(const char *name, eval::Token *value) {
    uint8_t i;
    for(i = 0; i < varNames.length(); ++i) {
        // Update it if found
        if(strcmp(varNames[i], name) == 0) {
            delete varVals[i];
            varVals[i] = value;
            
            // Delete the name since there's no use for it anymore
            delete name;

            break;
        }
    }
    // If i is equal to varNames.length() it was not found
    if(i == varNames.length()) {
        // Add the var if not found
        varNames.add(name);
        varVals.add(value);
    }
}
//name and argn should be allocated on the heap
char* getFuncFullName(eval::UserDefinedFunction);
void updateFunc(const char *name, neda::Container *expr, uint8_t argc, const char **argn) {
    uint8_t i;
    for(i = 0; i < functions.length(); ++i) {
        if(strcmp(functions[i].name, name) == 0) {
            delete functions[i].expr;
            for(uint8_t j = 0; j < functions[i].argc; j ++) {
                delete functions[i].argn[j];
            }
            delete functions[i].argn;
            delete functions[i].fullname;
            
            functions[i].expr = expr;
            functions[i].argc = argc;
            functions[i].argn = argn;
            functions[i].fullname = getFuncFullName(functions[i]);
            
            // delete the name since it's not updated
            delete name;

            break;
        }
    }
    if(i == functions.length()) {
        eval::UserDefinedFunction func(expr, name, argc, argn);
        func.fullname = getFuncFullName(func);
        functions.add(func);
    }
}
void clearVarsAndFuncs() {
    for(uint8_t i = 0; i < varNames.length(); i ++) {
        delete varNames[i];
        delete varVals[i];
    }
    varNames.empty();
    varVals.empty();
    for(auto func : functions) {
        delete func.name;
        delete func.expr;
        for(uint8_t i = 0; i < func.argc; ++i) {
            delete func.argn[i];
        }
        delete func.argn;
        delete func.fullname;
    }
    functions.empty();
}

//Key press handlers
//Probably gonna make this name shorter, but couldn't bother.
extern uint16_t selectorIndex;
bool editExpr = true;
uint8_t currentExpr = 0;
void expressionEntryKeyPressHandler(neda::Cursor *cursor, uint16_t key) {
    if(!editExpr && key != KEY_UP && key != KEY_DOWN && key != KEY_APPROX) {
        // If the key is a left or right, make a copy of the expression on display
        // Otherwise just insert a new expression
        neda::Container *newExpr;
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
        currentExpr = 0;
    }

	switch(key) {
	case KEY_LEFT:
		cursor->left();
		break;
	case KEY_RIGHT:
		cursor->right();
		break;
	case KEY_UP:
    {   
        if(editExpr) {
		    cursor->up();
        }
        else {
            if(currentExpr < RESULT_STORE_COUNT - 1 && expressions[currentExpr + 1]) {
                ++currentExpr;
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
        else {
            if(currentExpr >= 1) {
                --currentExpr;
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
        if(cursor->index != 0 && (ch = eval::extractChar(cursor->expr->contents[cursor->index - 1]), eval::isDigit(ch) || eval::isNameChar(ch))) {
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
        neda::Matrix *mat = new neda::Matrix(1, 1);
        mat->setEntry(1, 1, new neda::Container());
        cursor->add(mat);
        mat->getCursor(*cursor, neda::CURSORLOCATION_START);
        cursor->expr->parent->parent->draw(display);
        mat->computeWidth();
        mat->computeHeight();
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
        clearVarsAndFuncs();
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
        clearVarsAndFuncs();
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
                delete vName;
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
                        delete vName;
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
                        updateFunc(vName, funcExpr, argc, const_cast<const char**>(argn));
                        // Update the result to 1 to signify the operation succeeded
                        result = new eval::Number(1);
                    }
                }
                else {
                    // Evaluate
                    DynamicArray<neda::NEDAObj*> val(expr->contents.begin() + equalsIndex + 1, expr->contents.end());
                    result = eval::evaluate(&val, static_cast<uint8_t>(varNames.length()), const_cast<const char**>(varNames.asArray()), varVals.asArray(),
                            static_cast<uint8_t>(functions.length()), functions.asArray());

                    // If result is valid, add the variable
                    if(result) {
                        // Create a copy to avoid crashes
                        eval::Token *value;
                        if(result->getType() == eval::TokenType::NUMBER) {
                            value = new eval::Number(((eval::Number*) result)->value);
                        }
                        else {
                            value = new eval::Fraction(((eval::Fraction*) result)->num, ((eval::Fraction*) result)->denom);
                        }
                        // Add it
                        updateVar(vName, value);
                    }
                    else {
                        // Delete the variable name to avoid a memory leak
                        delete vName;
                    }
                }
            }
        }
        else {
            result = eval::evaluate(expr, static_cast<uint8_t>(varNames.length()), const_cast<const char**>(varNames.asArray()), varVals.asArray(),
                    static_cast<uint8_t>(functions.length()), functions.asArray());
        }
        // Create the container that will hold the result
        calcResults[0] = new neda::Container();
        if(!result) {
            calcResults[0]->add(new neda::Character(LCD_CHAR_SERR));
        }
        else {
            if(result->getType() == eval::TokenType::NUMBER) {
                if(isnan(((eval::Number*) result)->value)) {
                    // No complex numbers allowed!!
                    calcResults[0]->add(new neda::Character('\xff'));
                }
                else {
                    char buf[64];
                    // Convert the result and store it
                    ftoa(((eval::Number*) result)->value, buf, 16, LCD_CHAR_EE);
                    calcResults[0]->addString(buf);
                }
            }
            else {
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

            // Now update the value of the Ans variable
            // Var names must be allocated on the heap
            char *name = new char[4];
            strcpy(name, "Ans");
            updateVar(name, result);
        }
        expressions[0] = (neda::Container*) cursor->expr->getTopLevel();

        currentExpr = 0;
        drawResult(0, key != KEY_ENTER);
        return;
	}
    case KEY_APPROX:
    {
        if(!editExpr) {
            drawResult(currentExpr, true);
            return;
        }
        else {
            goto evaluateExpression;
        }
    }
    case KEY_CONST:
    case KEY_TRIG:
	case KEY_CONFIG:
    case KEY_CAT:
    case KEY_RECALL:
        // Set the display mode and reset the index
        dispMode = key == KEY_TRIG ? DispMode::TRIG_MENU : key == KEY_CONST ? DispMode::CONST_MENU : key == KEY_CAT ? DispMode::FUNC_MENU : key == KEY_RECALL ? DispMode::RECALL_MENU : DispMode::CONFIG_MENU;
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
void trigFunctionsMenuKeyPressHandler(neda::Cursor *cursor, uint16_t key) {

    switch(key) {
    case KEY_CENTER:
    case KEY_ENTER:
        // Insert the chars
        addStr(cursor, trigFuncNames[selectorIndex]);
        cursor->add(new neda::LeftBracket());
        cursor->expr->Expr::draw(display);
        // Intentional fall-through
    case KEY_TRIG:
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
void constSelectionMenuKeyPressHandler(neda::Cursor *cursor, uint16_t key) {
    switch(key) {
    case KEY_CENTER:
    case KEY_ENTER:
        // Insert the chars
        addStr(cursor, constantNames[selectorIndex]);
        cursor->expr->Expr::draw(display);
        // Intentional fall-through
    case KEY_CONST:
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

char* getFuncFullName(eval::UserDefinedFunction func) {
    uint16_t len = strlen(func.name);
    // 2 for brackets, one for each comma except the last one
    uint16_t totalLen = len + 2 + func.argc - 1;
    for(uint8_t j = 0; j < func.argc; ++j) {
        totalLen += strlen(func.argn[j]);
    }
    char *fullname = new char[totalLen + 1];
    // Copy the name
    strcpy(fullname, func.name);
    fullname[len++] = '(';
    // Copy each one of the arguments
    for(uint8_t j = 0; j < func.argc; ++j) {
        strcpy(fullname + len, func.argn[j]);
        len += strlen(func.argn[j]);
        // If not the last, then add a comma
        if(j + 1 != func.argc) {
            fullname[len++] = ',';
        }
    }
    fullname[len++] = ')';
    fullname[len] = '\0';
    return fullname;
}

#define BUILTIN_FUNC_COUNT 18
uint16_t funcCount = BUILTIN_FUNC_COUNT;
#define FUNC_SCROLLBAR_WIDTH 4
const char * const allFuncDispNames[BUILTIN_FUNC_COUNT] = {
    "sin(angle)", "cos(angle)", "tan(angle)", "asin(x)", "acos(x)", "atan(x)", 
    "sinh(angle)", "cosh(angle)", "tanh(angle)", "asinh(x)", "acosh(x)", "atanh(x)",
    "ln(x)", "qdRtA(a,b,c)", "qdRtB(a,b,c)", "round(n,decimals)", "abs(x)", "fact(x)",
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
void allAvailableFunctionsCatalogueSelectionMenuKeyPressHandler(neda::Cursor *cursor, uint16_t key) {
    funcCount = BUILTIN_FUNC_COUNT + functions.length();
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
            addStr(cursor, functions[selectorIndex - BUILTIN_FUNC_COUNT].name);
        }
        cursor->add(new neda::LeftBracket);
    }
        // Intentional fall-through
    case KEY_CAT:
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
            display.drawString(1, y, functions[i - BUILTIN_FUNC_COUNT].fullname, selectorIndex == i);
        }
        y += 10;
    }
    uint16_t scrollbarLocation = static_cast<uint16_t>(scrollingIndex * 64 / funcCount);
    uint16_t scrollbarHeight = 6 * 64 / funcCount;
    display.fill(128 - FUNC_SCROLLBAR_WIDTH, scrollbarLocation, FUNC_SCROLLBAR_WIDTH, scrollbarHeight);
    display.updateDrawing();
}

void recallUserDefinedFunctionsDefinitionsMenuKeyPressHandler(neda::Cursor *cursor, uint16_t key) {
    switch(key) {
    case KEY_CENTER:
    case KEY_ENTER:
    {
        if(functions.length() > 0) {
            for(auto ex : functions[selectorIndex].expr->contents) {
                cursor->add(ex->copy());
            }
        }
    }
    // Intentional fall-through
    case KEY_RECALL:
        dispMode = DispMode::EXPR_ENTRY;
        selectorIndex = 0;
        scrollingIndex = 0;
        // We need to call the function once to get the interface drawn
        // To do this, we insert a dummy value into the key buffer
        putKey(KEY_DUMMY);
        return;
    case KEY_UP:
        scrollUp(functions.length());
        break;
    case KEY_DOWN:
        scrollDown(functions.length());
        break;
    default: break;
    }

    display.clearDrawingBuffer();
    if(functions.length() == 0) {
        display.drawString(1, 1, "No Functions to");
        display.drawString(1, 11, "Recall");
    }
    else {
        int16_t y = 1;
        for(uint8_t i = scrollingIndex; i < scrollingIndex + 6 && i < functions.length(); i ++) {
            display.drawString(1, y, functions[i].fullname, selectorIndex == i);
            y += 10;
        }

        uint16_t scrollbarLocation = static_cast<uint16_t>(scrollingIndex * 64 / functions.length());
        uint16_t scrollbarHeight = 6 * 64 / functions.length();
        display.fill(128 - FUNC_SCROLLBAR_WIDTH, scrollbarLocation, FUNC_SCROLLBAR_WIDTH, scrollbarHeight);
    }
    display.updateDrawing();
}

void calculatorSettingsAndConfigurationMenuKeyPressHandler(uint16_t key) {
	switch(key) {
	case KEY_ENTER:
	case KEY_CONFIG:
		dispMode = DispMode::EXPR_ENTRY;
		selectorIndex = 0;
		// We need to call the function once to get the interface drawn
		// To do this, we insert a dummy value into the key buffer
		putKey(KEY_DUMMY);
		return;
	case KEY_LEFT:
	case KEY_RIGHT:
		eval::useRadians = !eval::useRadians;
		break;
	default: break;
	}

	display.clearDrawingBuffer();
	display.drawString(1, 1, "Angles:");
	display.drawString(64, 1, eval::useRadians ? "Radians" : "Degrees", true);
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

	// Create cursor
	cursor = new neda::Cursor;

	// Set up basic expression
    display.clearDrawingBuffer();
	neda::Container *master = new neda::Container();
	master->getCursor(*cursor, neda::CURSORLOCATION_START);
	adjustExpr(master, cursor);
	master->Expr::draw(display);
	cursor->draw(display);
	display.updateDrawing();

	// Start blink
	initCursorTimer(dispMode == DispMode::GAME ? 1000 : 2000);

	uint16_t key = KEY_NULL;
	
	SBDI_EN.init(GPIO_Mode_IN_FLOATING, GPIO_Speed_10MHz);
    while(true) {
		if((key = fetchKey()) != KEY_NULL) {
            switch(dispMode) {
            case DispMode::EXPR_ENTRY:
			    expressionEntryKeyPressHandler(cursor, key);
                break;
            case DispMode::TRIG_MENU:
                trigFunctionsMenuKeyPressHandler(cursor, key);
                break;
            case DispMode::CONST_MENU:
                constSelectionMenuKeyPressHandler(cursor, key);
                break;
			case DispMode::CONFIG_MENU:
				calculatorSettingsAndConfigurationMenuKeyPressHandler(key);
				break;
            case DispMode::FUNC_MENU:
                allAvailableFunctionsCatalogueSelectionMenuKeyPressHandler(cursor, key);
                break;
            case DispMode::RECALL_MENU:
                recallUserDefinedFunctionsDefinitionsMenuKeyPressHandler(cursor, key);
                break;
            case DispMode::GAME:
                gameKeyPressHandler(key);
                break;
            default: break;
            }
		}
    }
}
