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
			default: break;
			}
			
			display.clearDrawingBuffer();
			master->draw(display, 0, 0);
			cursor->draw(display);
			display.updateDrawing();
			
		}
    }
}
