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
/********** GPIO Pins and other pin defs **********/
GPIOPin RS(GPIOB, GPIO_Pin_12), RW(GPIOB, GPIO_Pin_13), E(GPIOB, GPIO_Pin_14),
			D7(GPIOA, GPIO_Pin_15), D6(GPIOB, GPIO_Pin_3), D5(GPIOB, GPIO_Pin_4), D4(GPIOB, GPIO_Pin_5),
			D3(GPIOB, GPIO_Pin_6), D2(GPIOB, GPIO_Pin_7), D1(GPIOB, GPIO_Pin_8), D0(GPIOB, GPIO_Pin_9);
lcd::LCD12864 display(RS, RW, E, D0, D1, D2, D3, D4, D5, D6, D7);

GPIOPin backlightPin(GPIOA, GPIO_Pin_1);

GPIOPin SBDI_EN(GPIOA, GPIO_Pin_12);
GPIOPin SBDI_CLK(GPIOA, GPIO_Pin_11);
GPIOPin SBDI_DATA(GPIOA, GPIO_Pin_8);

GPIOPin statusLED(GPIOC, GPIO_Pin_13);
GPIOPin shiftLED(GPIOA, GPIO_Pin_3);
GPIOPin ctrlLED(GPIOA, GPIO_Pin_2);

/********** Keyboard stuff **********/
uint32_t keyDataBuffer = 0;
void putKey(uint16_t key) {
    //Store keystroke into buffer
    //If there is already data in the buffer then shift that data left to make room
    if(keyDataBuffer != 0) {
        keyDataBuffer <<= 16;
    }
    keyDataBuffer += (uint16_t) key;
}
uint16_t fetchKey() {
	uint16_t data;
	//Check for key in buffer
	if(keyDataBuffer != 0) {
		//If there are 2 keys in the buffer
		if(keyDataBuffer >> 16 != 0) {
			//Return the first key
			data = keyDataBuffer >> 16;
			//Remove the first key from the buffer
			keyDataBuffer &= 0x0000FFFF;
		}
		else {
			//Return the key and clear the buffer
			data = keyDataBuffer;
			keyDataBuffer = 0;
		}
		return data;
	}
	//If buffer empty then no new keys have been entered
	else {
		return KEY_NULL;
	}
}

/********** Mode **********/
enum class DispMode {
    EXPR_ENTRY,
    TRIG_MENU,
};
DispMode dispMode = DispMode::EXPR_ENTRY;

/********** Cursor processing **********/
void initCursorTimer() {
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
	TIM_TimeBaseInitTypeDef initStruct;
    TIM_TimeBaseStructInit(&initStruct);
	initStruct.TIM_CounterMode = TIM_CounterMode_Up;
	initStruct.TIM_ClockDivision = TIM_CKD_DIV1;
	initStruct.TIM_Prescaler = 17999;
	initStruct.TIM_Period = 2000;
	initStruct.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(TIM3, &initStruct);
	//Set up interrupts
	NVIC_InitTypeDef nvicInit;
	nvicInit.NVIC_IRQChannel = TIM3_IRQn;
	nvicInit.NVIC_IRQChannelCmd = ENABLE;
	nvicInit.NVIC_IRQChannelPreemptionPriority = 0x04;
	nvicInit.NVIC_IRQChannelSubPriority = 0x01;
	NVIC_Init(&nvicInit);
	//Enable timer and interrupts
	TIM_Cmd(TIM3, ENABLE);
	TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);
}

bool cursorOn = false;
neda::Cursor *cursor;
extern bool editExpr;
extern "C" void TIM3_IRQHandler() {
	if(TIM_GetITStatus(TIM3, TIM_IT_Update)) {
		TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
        if(dispMode != DispMode::EXPR_ENTRY || !editExpr) {
            return;
        }
		cursorOn = !cursorOn;
		display.clearDrawingBuffer();
		//Redraw the entire expr
		cursor->expr->Expr::drawConnected(display);
		if(cursorOn) {
			cursor->draw(display);
		}
		display.updateDrawing();
	}
}

#define CURSOR_HORIZ_SPACING 1
#define CURSOR_VERT_SPACING 1
//Moves ex so that the cursor is in the display
void adjustExpr(neda::Expr *ex, neda::Cursor *cursorRef) {
	neda::CursorInfo info;
	cursorRef->getInfo(info);
	if(info.x > CURSOR_HORIZ_SPACING && info.y > CURSOR_VERT_SPACING
			&& info.x + info.width + CURSOR_HORIZ_SPACING < 128 && info.y + info.height + CURSOR_VERT_SPACING < 64) {
		return;
	}

	int16_t xdiff = 0, ydiff = 0;
	if(info.x <= CURSOR_HORIZ_SPACING) {
		xdiff = CURSOR_HORIZ_SPACING + 1 - info.x;
	}
	else if(info.x + info.width + CURSOR_HORIZ_SPACING >= 128) {
		xdiff = 127 - (info.x + info.width + CURSOR_HORIZ_SPACING);
	}
	if(info.y <= CURSOR_VERT_SPACING) {
		ydiff = CURSOR_VERT_SPACING + 1 - info.y;
	}
	else if(info.y + info.height + CURSOR_VERT_SPACING >= 64) {
		ydiff = 63 - (info.y + info.height + CURSOR_VERT_SPACING);
	}
	ex->updatePosition(xdiff, ydiff);
}

//Adds a char at the cursor
void addChar(neda::Cursor *cursor, char ch) {
    cursor->add(new neda::Character(ch));
}

#define RESULT_STORE_COUNT 3
//Previous expressions and their results
neda::Container *calcResults[RESULT_STORE_COUNT] = { nullptr };
neda::Container *expressions[RESULT_STORE_COUNT] = { nullptr };
void drawResult(uint8_t id) {
    //Display the result
    display.clearDrawingBuffer();
    expressions[id]->Expr::draw(display);
    //Fill the area first
    display.fill(128 - CURSOR_HORIZ_SPACING - calcResults[id]->exprWidth, 64 - CURSOR_VERT_SPACING - calcResults[id]->exprHeight, calcResults[id]->exprWidth, calcResults[id]->exprHeight, true);
    calcResults[id]->draw(display, 128 - CURSOR_HORIZ_SPACING - calcResults[id]->exprWidth, 64 - CURSOR_VERT_SPACING - calcResults[id]->exprHeight);
}

//Key press handlers
//Probably gonna make this name shorter, but couldn't bother.
extern uint16_t trigFuncIndex;
extern void trigFunctionsMenuKeyPressHandler(neda::Cursor*, uint16_t);
bool editExpr = true;
void expressionEntryKeyPressHandler(neda::Cursor *cursor, uint16_t key) {
	switch(key) {
	case KEY_SHIFT:
		shiftLED = !shiftLED;
		break;
	case KEY_CTRL:
		ctrlLED = !ctrlLED;
		break;
	case KEY_LEFT:
		cursor->left();
		break;
	case KEY_RIGHT:
		cursor->right();
		break;
	case KEY_UP:
		cursor->up();
		break;
	case KEY_DOWN:
		cursor->down();
		break;
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
        //Make sure the position is updated so adjustExpr will not mess up the display
        cursor->expr->parent->parent->draw(display);
		break;
	}
	case KEY_NTHROOT:
	{
		neda::Radical *radical = new neda::Radical(new neda::Container, new neda::Container);
        cursor->add(radical);
        radical->getCursor(*cursor, neda::CURSORLOCATION_START);
        //Make sure the position is updated so adjustExpr will not mess up the display
        cursor->expr->parent->parent->draw(display);
		break;
	}
	case KEY_SUM:
	{
        neda::SigmaPi *sigma = new neda::SigmaPi(lcd::CHAR_SUMMATION, new neda::Container(), new neda::Container(), new neda::Container());
		cursor->add(sigma);
        sigma->getCursor(*cursor, neda::CURSORLOCATION_START);
        //Make sure the position is updated so adjustExpr will not mess up the display
        cursor->expr->parent->parent->draw(display);
		break;
	}
	case KEY_PRODUCT:
	{
        neda::SigmaPi *product = new neda::SigmaPi(lcd::CHAR_PRODUCT, new neda::Container(), new neda::Container(), new neda::Container());
        cursor->add(product);
        product->getCursor(*cursor, neda::CURSORLOCATION_START);
        //Make sure the position is updated so adjustExpr will not mess up the display
        cursor->expr->parent->parent->draw(display);
		break;
    }
	case KEY_FRAC:
	{
        neda::Fraction *frac;
        //If there's a number in front of the cursor, enclose that in the fraction
        if(cursor->index != 0 && eval::isDigit(eval::extractChar(cursor->expr->contents[cursor->index - 1]))) {
            bool isNum;
            uint16_t end = eval::findTokenEnd(&cursor->expr->contents, cursor->index - 1, -1, isNum) + 1;
            uint16_t len = cursor->index - end;
            
            //Create a new array with the objects
            DynamicArray<neda::NEDAObj*> arr(cursor->expr->contents.begin() + end, cursor->expr->contents.begin() + cursor->index);
            cursor->index = end;
            //Remove the objects from the original array
            cursor->expr->contents.removeAt(end, len);
            frac = new neda::Fraction(new neda::Container(arr), new neda::Container());
        }
        else {
		    frac = new neda::Fraction(new neda::Container(), new neda::Container());
        }
        cursor->add(frac);
        frac->getCursor(*cursor, neda::CURSORLOCATION_START);
        //Make sure the position is updated so adjustExpr will not mess up the display
        cursor->expr->parent->parent->draw(display);
		break;
	}
	case KEY_SQUARE:
	case KEY_CUBE:
	case KEY_EXPONENT:
	{
        neda::Superscript *super = new neda::Superscript(neda::makeString(key == KEY_SQUARE ? "2" : (key == KEY_CUBE ? "3" : "")));
        cursor->add(super);
        //Only move the cursor if the exponent box is empty
        if(key == KEY_EXPONENT) {
            super->getCursor(*cursor, neda::CURSORLOCATION_START);
            //Make sure the position is updated so adjustExpr will not mess up the display
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
        cursor->add(new neda::Character('l'));
        cursor->add(new neda::Character('n'));
        cursor->add(new neda::LeftBracket());
		break;
    }
    case KEY_LOG10:
    {
        cursor->add(new neda::Character('l'));
        cursor->add(new neda::Character('o'));
        cursor->add(new neda::Character('g'));
        //cursor->add(new neda::Subscript(neda::makeString("10")));
        cursor->add(new neda::LeftBracket());
		break;
    }
    case KEY_LOGN:
    {
        cursor->add(new neda::Character('l'));
        cursor->add(new neda::Character('o'));
        cursor->add(new neda::Character('g'));
        neda::Subscript *sub = new neda::Subscript(new neda::Container());
        cursor->add(sub);
        cursor->add(new neda::LeftBracket());
        sub->getCursor(*cursor, neda::CURSORLOCATION_START);
        cursor->expr->parent->parent->draw(display);
		break;
    }
	/* OTHER */
	case KEY_DELETE:
	{
		//Simple case: There is still stuff left before the cursor
		if(cursor->index != 0) {
			neda::NEDAObj *obj = cursor->expr->removeAtCursor(*cursor);
            delete obj;
		break;
		}
		//If there are no more characters to delete:
		//Confirm that the cursor is not in the top-level expression
        if(cursor->expr != cursor->expr->getTopLevel()) {
            //First put the cursor in the position before
            //The container the cursor is in must be inside some other expression, and then inside another container
            neda::Container *cont = (neda::Container*) cursor->expr->parent->parent;
            uint16_t index = cont->indexOf(cursor->expr->parent);
            //Manually change the position of the cursor
            cursor->expr = cont;
            cursor->index = index;
            //Remove and delete the elem
            neda::NEDAObj *obj = cont->remove(index);
            delete obj;
        }
		break;
	}
	case KEY_CLEAR:
	{
		//Keep pointer of original
		neda::Expr *original = cursor->expr->getTopLevel();
		//Create new expression and change cursor location
		neda::Container *container = new neda::Container;
		container->getCursor(*cursor, neda::CURSORLOCATION_START);
		//Make sure the cursor's location is updated
		container->draw(display, 0, 0);
		//Delete old
		delete original;
		break;
	}
	case KEY_ENTER:
	{
        editExpr = false;
		eval::Token *result = eval::evaluate((neda::Container*) cursor->expr->getTopLevel());
        //Create the container that will hold the result
        calcResults[0] = new neda::Container();
        if(!result) {
            calcResults[0]->add(new neda::Character(LCD_CHAR_SERR));
        }
        else {
            if(result->getType() == eval::TokenType::NUMBER) {
                if(isnan(((eval::Number*) result)->value)) {
                    //No complex numbers allowed!!
                    calcResults[0]->add(new neda::Character('\xff'));
                }
                else {
                    char buf[64];
                    //Convert the result and store it
                    ftoa(((eval::Number*) result)->value, buf, 16, LCD_CHAR_EE);
                    neda::addString(calcResults[0], buf);
                }
            }
            else {
                char buf[64];
                neda::Container *num = new neda::Container();
                neda::Container *denom = new neda::Container();
                ltoa(((eval::Fraction*) result)->num, buf);
                neda::addString(num, buf);
                ltoa(((eval::Fraction*) result)->denom, buf);
                neda::addString(denom, buf);

                calcResults[0]->add(new neda::Fraction(num, denom));
            }
        }
        expressions[0] = (neda::Container*) cursor->expr->getTopLevel();

        drawResult(0);
        display.updateDrawing();
        return;
	}
    case KEY_TRIG:
        //Set the display mode and reset the index
        dispMode = DispMode::TRIG_MENU;
        trigFuncIndex = 0;
        //We need to call the function once to get the interface drawn
        //To do this, we insert a dummy value into the key buffer
        putKey(KEY_DUMMY);
		break;
	default: break;
	}

    if(!editExpr) {
        //When any key is pressed, shift everything right
        delete calcResult;
        calcResult = nullptr;
	    editExpr = true;
    }

	display.clearDrawingBuffer();
	adjustExpr(cursor->expr->getTopLevel(), cursor);
	cursor->expr->drawConnected(display);
	cursor->draw(display);
	display.updateDrawing();
}
uint16_t trigFuncIndex = 0;
const char *trigFuncs[] = {
    "sin", "cos", "tan", "arcsin", "arccos", "arctan",
    "sinh", "cosh", "tanh", "arcsinh", "arccosh", "arctanh",
};
const char *trigFuncNames[] = {
    "sin", "cos", "tan", "asin", "acos", "atan",
    "sinh", "cosh", "tanh", "asinh", "acosh", "atanh",
};
void trigFunctionsMenuKeyPressHandler(neda::Cursor *cursor, uint16_t key) {

    switch(key) {
    case KEY_CENTER:
    case KEY_ENTER:
        //Insert the chars
        for(uint8_t i = 0; trigFuncNames[trigFuncIndex][i] != '\0'; i ++) {
            cursor->add(new neda::Character(trigFuncNames[trigFuncIndex][i]));
        }
        cursor->add(new neda::LeftBracket());
        cursor->expr->Expr::draw(display);
        //Intentional fall-through
    case KEY_TRIG:
        dispMode = DispMode::EXPR_ENTRY;
        trigFuncIndex = 0;
        //We need to call the function once to get the interface drawn
        //To do this, we insert a dummy value into the key buffer
        putKey(KEY_DUMMY);
        return;
    case KEY_UP:
        if(trigFuncIndex > 0) {
            --trigFuncIndex;
        }
        else {
            trigFuncIndex = 11;
        }
        break;
    case KEY_DOWN:
        ++trigFuncIndex;
        if(trigFuncIndex >= 12) {
            trigFuncIndex = 0;
        }
        break;
    case KEY_LEFT:
        if(trigFuncIndex >= 6) {
            trigFuncIndex -= 6;
        }
        break;
    case KEY_RIGHT:
        if(trigFuncIndex < 6) {
            trigFuncIndex += 6;
        }
        break;
    default: break;
    }

    display.clearDrawingBuffer();
    int16_t y = 1;
    for(uint8_t i = 0; i < 12; i ++) {
        //Reset y if we are in the second column
        if(i == 6) {
            y = 1;
        }

        if(i < 6) {
            display.drawString(1, y, trigFuncs[i], trigFuncIndex == i);
        }
        else {
            display.drawString(64, y, trigFuncs[i], trigFuncIndex == i);
        }
        y += 10;
    }

    display.updateDrawing();
}

int main() {
	//Init system
	sys::initRCC();
	sys::initNVIC();
	usart::init(115200);
	//Init LEDs
	statusLED.init(GPIO_Mode_Out_PP, GPIO_Speed_2MHz);
	shiftLED.init(GPIO_Mode_Out_PP, GPIO_Speed_2MHz);
	ctrlLED.init(GPIO_Mode_Out_PP, GPIO_Speed_2MHz);
	statusLED = false;
	shiftLED = false;
	ctrlLED = false;

	//Disable JTAG so we can use PB3 and 4
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);

	//1s startup delay
	delay::sec(1);

	//Initialize display and backlight control
	display.init();
	display.useExtended();
	display.startDraw();
	display.clearDrawing();

	//Set up SBDI receiver
	sbdi::Receiver receiver(SBDI_EN, SBDI_DATA, SBDI_CLK);
	receiver.init();
	receiver.onReceive([](uint32_t data) {
		//Store keystroke into buffer
		//If there is already data in the buffer then shift that data left to make room
		putKey(data);
		statusLED = !statusLED;
	});

	//Create cursor
	cursor = new neda::Cursor;

	//Set up basic expression
	neda::Container *master = new neda::Container();
	master->getCursor(*cursor, neda::CURSORLOCATION_START);
	adjustExpr(master, cursor);
	master->Expr::draw(display);
	cursor->draw(display);
	display.updateDrawing();

	//Start blink
	initCursorTimer();

	uint16_t key = KEY_NULL;

    while(true) {
		if((key = fetchKey()) != KEY_NULL) {
            switch(dispMode) {
            case DispMode::EXPR_ENTRY:
			    expressionEntryKeyPressHandler(cursor, key);
                break;
            case DispMode::TRIG_MENU:
                trigFunctionsMenuKeyPressHandler(cursor, key);
                break;
            default: break;
            }
		}
    }
}
