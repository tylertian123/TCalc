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

int main() {
	
	sys::initRCC();
	sys::initNVIC();
	usart::init(115200);
	
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
	
	//Init display
	display.useExtended();
	display.startDraw();
	display.clearDrawing();
	
	//Create cursor
	cursor = new neda::Cursor;
	
	//Set up basic expression
	neda::ContainerExpr *master = new neda::ContainerExpr;
	master->addExpr(new neda::StringExpr);
	
	master->getCursor(*cursor, neda::CURSORLOCATION_START);
	master->draw(display, 0, 0);
	cursor->draw(display);
	display.updateDrawing();
	
	//Start blink
	initCursorTimer();
	
	uint16_t key = KEY_NULL;
	
    while(true) {
		if((key = fetchKey()) != KEY_NULL) {
			
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
			case KEY_DELETE:
				cursor->removeChar();
				break;
			/* LETTER KEYS */
			case KEY_A:
				cursor->addChar('A');
				break;
			case KEY_B:
				cursor->addChar('B');
				break;
			case KEY_C:
				cursor->addChar('C');
				break;
			case KEY_D:
				cursor->addChar('D');
				break;
			case KEY_E:
				cursor->addChar('E');
				break;
			case KEY_F:
				cursor->addChar('F');
				break;
			case KEY_G:
				cursor->addChar('G');
				break;
			case KEY_H:
				cursor->addChar('H');
				break;
			case KEY_I:
				cursor->addChar('I');
				break;
			case KEY_J:
				cursor->addChar('J');
				break;
			case KEY_K:
				cursor->addChar('K');
				break;
			case KEY_L:
				cursor->addChar('L');
				break;
			case KEY_M:
				cursor->addChar('M');
				break;
			case KEY_N:
				cursor->addChar('N');
				break;
			case KEY_O:
				cursor->addChar('O');
				break;
			case KEY_P:
				cursor->addChar('P');
				break;
			case KEY_Q:
				cursor->addChar('Q');
				break;
			case KEY_R:
				cursor->addChar('R');
				break;
			case KEY_S:
				cursor->addChar('S');
				break;
			case KEY_T:
				cursor->addChar('T');
				break;
			case KEY_U:
				cursor->addChar('U');
				break;
			case KEY_V:
				cursor->addChar('V');
				break;
			case KEY_W:
				cursor->addChar('W');
				break;
			case KEY_X:
				cursor->addChar('X');
				break;
			case KEY_Y:
				cursor->addChar('Y');
				break;
			case KEY_Z:
				cursor->addChar('Z');
				break;
			case KEY_LCA:
				cursor->addChar('a');
				break;
			case KEY_LCB:
				cursor->addChar('b');
				break;
			case KEY_LCC:
				cursor->addChar('c');
				break;
			case KEY_LCD:
				cursor->addChar('d');
				break;
			case KEY_LCE:
				cursor->addChar('e');
				break;
			case KEY_LCF:
				cursor->addChar('f');
				break;
			case KEY_LCG:
				cursor->addChar('g');
				break;
			case KEY_LCH:
				cursor->addChar('h');
				break;
			case KEY_LCI:
				cursor->addChar('i');
				break;
			case KEY_LCJ:
				cursor->addChar('j');
				break;
			case KEY_LCK:
				cursor->addChar('k');
				break;
			case KEY_LCL:
				cursor->addChar('l');
				break;
			case KEY_LCM:
				cursor->addChar('m');
				break;
			case KEY_LCN:
				cursor->addChar('n');
				break;
			case KEY_LCO:
				cursor->addChar('o');
				break;
			case KEY_LCP:
				cursor->addChar('p');
				break;
			case KEY_LCQ:
				cursor->addChar('q');
				break;
			case KEY_LCR:
				cursor->addChar('r');
				break;
			case KEY_LCS:
				cursor->addChar('s');
				break;
			case KEY_LCT:
				cursor->addChar('t');
				break;
			case KEY_LCU:
				cursor->addChar('u');
				break;
			case KEY_LCV:
				cursor->addChar('v');
				break;
			case KEY_LCW:
				cursor->addChar('w');
				break;
			case KEY_LCX:
				cursor->addChar('x');
				break;
			case KEY_LCY:
				cursor->addChar('y');
				break;
			case KEY_LCZ:
				cursor->addChar('z');
				break;
			/* OTHER KEYS WITH NO SPECIAL HANDLING */
			case KEY_0:
				cursor->addChar('0');
				break;
			case KEY_1:
				cursor->addChar('1');
				break;
			case KEY_2:
				cursor->addChar('2');
				break;
			case KEY_3:
				cursor->addChar('3');
				break;
			case KEY_4:
				cursor->addChar('4');
				break;
			case KEY_5:
				cursor->addChar('5');
				break;
			case KEY_6:
				cursor->addChar('6');
				break;
			case KEY_7:
				cursor->addChar('7');
				break;
			case KEY_8:
				cursor->addChar('8');
				break;
			case KEY_9:
				cursor->addChar('9');
				break;
			case KEY_SPACE:
				cursor->addChar(' ');
				break;
			case KEY_COMMA:
				cursor->addChar(',');
				break;
			
			default: break;
			}
			
			display.clearDrawingBuffer();
			master->draw(display, 0, 0);
			cursor->draw(display);
			display.updateDrawing();
			
		}
    }
}
