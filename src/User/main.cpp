#include "stm32f10x.h"
#include "sys.hpp"
#include "delay.hpp"
#include "usart.hpp"
#include "gpiopin.hpp"
#include "sbdi.hpp"
#include "lcd12864.hpp"
#include "lcd12864_charset.hpp"
#include "pwm.hpp"
#include "dynamarr.hpp"
#include "neda.hpp"
#include "keydef.h"
#include "util.hpp"

/********** GPIO Pins and other pin defs **********/
GPIOPin RS(GPIOB, GPIO_Pin_12), RW(GPIOB, GPIO_Pin_13), E(GPIOB, GPIO_Pin_14),
			D7(GPIOA, GPIO_Pin_15), D6(GPIOB, GPIO_Pin_3), D5(GPIOB, GPIO_Pin_4), D4(GPIOB, GPIO_Pin_5),
			D3(GPIOB, GPIO_Pin_6), D2(GPIOB, GPIO_Pin_7), D1(GPIOB, GPIO_Pin_8), D0(GPIOB, GPIO_Pin_9);
lcd::LCD12864 display(RS, RW, E, D0, D1, D2, D3, D4, D5, D6, D7);

GPIOPin backlightPin(GPIOA, GPIO_Pin_1);
pwm::PWMOutput<2, 72000000, 100> backlight(TIM2, RCC_APB1Periph_TIM2, backlightPin);

GPIOPin SBDI_EN(GPIOA, GPIO_Pin_12);
GPIOPin SBDI_CLK(GPIOA, GPIO_Pin_11);
GPIOPin SBDI_DATA(GPIOA, GPIO_Pin_8);

GPIOPin statusLED(GPIOC, GPIO_Pin_13);
GPIOPin shiftLED(GPIOA, GPIO_Pin_3);
GPIOPin ctrlLED(GPIOA, GPIO_Pin_2);

/********** Keyboard stuff **********/
uint32_t keyDataBuffer = 0;

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

/********** Cursor processing **********/
void initCursorTimer() {
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
	TIM_TimeBaseInitTypeDef initStruct;
	initStruct.TIM_CounterMode = TIM_CounterMode_Up;
	initStruct.TIM_ClockDivision = TIM_CKD_DIV4;
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
extern "C" void TIM3_IRQHandler() {
	if(TIM_GetITStatus(TIM3, TIM_IT_Update)) {
		TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
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

#define CURSOR_HORIZ_SPACING 2
#define CURSOR_VERT_SPACING 2
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


/* void insertExprAtCursor(neda::Expr *expr, neda::Cursor *cursor) {
	//Split the original expression into 2 parts
	neda::String *first = cursor->expr->beforeCursor(*cursor);
	neda::String *second = cursor->expr->afterCursor(*cursor);
	//The parent of a String must always be a Container
	//If not, then, well, someone's getting fired.
	neda::Container *container = (neda::Container*) cursor->expr->parent;
	uint16_t index = container->indexOf(cursor->expr);
	//Insert the expressions back in
	container->replaceExpr(index ++, first);
	container->addAt(index ++, expr);
	container->addAt(index ++, second);
	//SUPER IMPORTANT: DELETE ORIGINAL STRING!!!
	//Keep a copy of original so we can get the new cursor before deleting the old one (so that interrupts don't cause errors)
	neda::String *original = cursor->expr;
	expr->getCursor(*cursor, neda::CURSORLOCATION_START);
	delete original;
	//Use draw to figure out the approx location of the new cursor so adjustExpr won't mess up the display
	container->Expr::draw(display);
} */
//Adds a char at the cursor
void addChar(neda::Cursor *cursor, char ch) {
	if(cursor->expr->getType() == neda::ExprType::STRING) {
		((neda::String*) cursor->expr)->addChar(ch);
		return;
	}
	else if(cursor->expr->getType() == neda::ExprType::CONTAINER) {
		neda::Container *container = (neda::Container*) cursor->expr;
		neda::String *str = new neda::String;
		str->addChar(ch);
		container->addAtCursor(str, *cursor);
		str->getCursor(*cursor, neda::CURSORLOCATION_START);
	}
}
//Key press handlers
//Probably gonna make this name shorter, but couldn't bother.
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

	/* EXPRESSIONS */
	case KEY_LBRACKET:
	{
		insertExprAtCursor(new neda::Bracket(createEmptyContainer()), cursor);
		break;
	}
	case KEY_ROOT:
	{
		insertExprAtCursor(new neda::Radical(createEmptyContainer(), nullptr), cursor);
		break;
	}
	case KEY_NTHROOT:
	{
		insertExprAtCursor(new neda::Radical(createEmptyContainer(), createEmptyContainer()), cursor);
		break;
	}
	case KEY_SUM:
	{
		insertExprAtCursor(new neda::SigmaPi(lcd::CHAR_SUMMATION, createEmptyContainer(), createEmptyContainer(), createEmptyContainer()), cursor);
		break;
	}
	case KEY_PRODUCT:
	{
		insertExprAtCursor(new neda::SigmaPi(lcd::CHAR_PRODUCT, createEmptyContainer(), createEmptyContainer(), createEmptyContainer()), cursor);
		break;
	}
	case KEY_FRAC:
	{
		insertExprAtCursor(new neda::Fraction(createEmptyContainer(), createEmptyContainer()), cursor);
		break;
	}
	case KEY_SQUARE:
	case KEY_CUBE:
	case KEY_EXPONENT:
	{
		if(cursor->index == 0) {
			insertExprAtCursor(new neda::Exponent(createEmptyContainer(), createEmptyContainer(key == KEY_SQUARE ? "2" : (key == KEY_CUBE ? "3" : ""))), cursor);
			break;
		}
		//Split the original expression into 2 parts
		neda::String *first = cursor->expr->beforeCursor(*cursor);
		neda::String *second = cursor->expr->afterCursor(*cursor);
		//The parent of a String must always be a Container
		//If not, then, well, someone's getting fired.
		neda::Container *container = (neda::Container*) cursor->expr->parent;
		uint16_t index = container->indexOf(cursor->expr);
		//Insert the expressions back in
		neda::Expr *temp = createEmptyContainer(key == KEY_SQUARE ? "2" : (key == KEY_CUBE ? "3" : ""));
		neda::Exponent *expr = new neda::Exponent(first, temp);
		container->replaceExpr(index ++, expr);
		container->addAt(index ++, second);
		//SUPER IMPORTANT: DELETE ORIGINAL STRING!!!
		//Keep a copy of original so we can get the new cursor before deleting the old one (so that interrupts don't cause errors)
		neda::String *original = cursor->expr;
		temp->getCursor(*cursor, neda::CURSORLOCATION_START);
		delete original;
		//Use draw to figure out the approx location of the new cursor so adjustExpr won't mess up the display
		container->Expr::draw(display);
		break;
	}
	/* OTHER */
	case KEY_DELETE:
	{
		//Simple case: There are still characters left
		if(cursor->index != 0) {
			cursor->removeChar();
			break;
		}
		//If there are no more characters to delete:
		neda::Container *container = (neda::Container*) cursor->expr->parent;
		uint16_t index = container->indexOf(cursor->expr);
		auto contents = container->getContents();
		//If there is an expr in front of the cursor and that expression is not an empty string
		if(index >= 1 && !neda::isEmptyString((*contents)[index - 1])) {
			//Delete that expr in front of the cursor
			neda::Expr *ex = (*contents)[index - 1];
			container->removeExpr(index - 1);
			delete ex;
			//If there's a String in front of what we just deleted, then merge them
			neda::String *frontStr = ((neda::String*) (*contents)[index - 2]);
			if(index >= 2 && frontStr->getType() == neda::ExprType::STRING) {
				frontStr->merge(cursor->expr);
				//Keep temp. copy of original expr
				neda::String *temp = cursor->expr;
				//Remove original
				//Subtract 1 from the index because of the expression previously removed
				container->removeExpr(index - 1);
				//Reposition cursor
				cursor->expr = frontStr;
				cursor->index = frontStr->length() - temp->length();
				//Delete original expr the cursor was in
				delete temp;
			}
		}
		//Otherwise either there's no more stuff to delete or there's only an empty String in front
		//Confirm that the cursor is not in the top-level expression
		//So see if the cursor's String has a great-grandparent
		//(String->Container->(Some Other Expr)->Container)
		else if(cursor->expr->parent->parent && cursor->expr->parent->parent->parent) {
			//First, to avoid errors, put the cursor in the String before (there should always be one)
			neda::Container *container = (neda::Container*) cursor->expr->parent->parent->parent;
			neda::Expr *exprToRemove = cursor->expr->parent->parent;
			uint16_t index = container->indexOf(exprToRemove);
			auto contents = container->getContents();
			neda::Expr *frontExpr = (*contents)[index - 1];
			frontExpr->getCursor(*cursor, neda::CURSORLOCATION_END);
			//Remove and delete the expression
			container->removeExpr(index);
			delete exprToRemove;
			//Merge if there are two Strings sandwiching it
			//Since the expression is already removed, no need to +1
			neda::Expr *backExpr = (*contents)[index];
			if(frontExpr->getType() == neda::ExprType::STRING && backExpr->getType() == neda::ExprType::STRING) {
				//Merge
				((neda::String*) frontExpr)->merge((neda::String*) backExpr);
				//Remove
				container->removeExpr(index);
				//Delete
				delete backExpr;
			}
		}
		break;
	}
	case KEY_ALLCLEAR:
	{
		//Keep pointer of original
		neda::Expr *original = cursor->expr->getTopLevel();
		//Create new expression and change cursor location
		neda::Container *container = new neda::Container;
		neda::String *str = new neda::String;
		container->addExpr(str);
		str->getCursor(*cursor, neda::CURSORLOCATION_START);
		//Make sure the cursor's location is updated
		container->draw(display, 0, 0);
		//Delete old
		delete original;
		break;
	}

	default: break;
	}

	display.clearDrawingBuffer();
	adjustExpr(cursor->expr->getTopLevel(), cursor);
	cursor->expr->drawConnected(display);
	cursor->draw(display);
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

	backlight.startTimer();
	backlight.set(0xA0);

	//Set up SBDI receiver
	sbdi::Receiver receiver(SBDI_EN, SBDI_DATA, SBDI_CLK);
	receiver.init();
	receiver.onReceive([](uint32_t data) {
		//Store keystroke into buffer
		//If there is already data in the buffer then shift that data left to make room
		if(keyDataBuffer != 0) {
			keyDataBuffer <<= 16;
		}
		keyDataBuffer += (uint16_t) data;
		statusLED = !statusLED;
	});

	//Create cursor
	cursor = new neda::Cursor;

	//Set up basic expression
	neda::Container *master = new neda::Container;
	master->addExpr(new neda::String());

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
			expressionEntryKeyPressHandler(cursor, key);
		}
    }
}
