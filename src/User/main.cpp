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

GPIOPin RS(GPIOB, GPIO_Pin_12), RW(GPIOB, GPIO_Pin_13), E(GPIOB, GPIO_Pin_14),
			D7(GPIOA, GPIO_Pin_15), D6(GPIOB, GPIO_Pin_3), D5(GPIOB, GPIO_Pin_4), D4(GPIOB, GPIO_Pin_5),
			D3(GPIOB, GPIO_Pin_6), D2(GPIOB, GPIO_Pin_7), D1(GPIOB, GPIO_Pin_8), D0(GPIOB, GPIO_Pin_9);
lcd::LCD12864 display(RS, RW, E, D0, D1, D2, D3, D4, D5, D6, D7);

GPIOPin backlightPin(GPIOA, GPIO_Pin_1);
pwm::PWMOutput<2, 72000000, 100> backlight(TIM2, RCC_APB1Periph_TIM2, backlightPin);

GPIOPin SBDI_EN(GPIOA, GPIO_Pin_12);
GPIOPin SBDI_CLK(GPIOA, GPIO_Pin_11);
GPIOPin SBDI_DATA(GPIOA, GPIO_Pin_8);

GPIOPin led(GPIOC, GPIO_Pin_13);

uint32_t keyDataBuffer = 0;

#define KEY_NULL	0x0000
#define KEY_UP		0x0001
#define KEY_DOWN	0x0002
#define KEY_LEFT	0x0003
#define KEY_RIGHT	0x0004
#define KEY_CENTER	0x0005

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

int main() {
	
	sys::initRCC();
	sys::initNVIC();
	usart::init(115200);
	
	led.init(GPIO_Mode_Out_PP, GPIO_Speed_2MHz);
	led = 0;
	
	//Disable JTAG so we can use PB3 and 4
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_Disable, ENABLE);
	
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
	});
	
	display.useExtended();
	display.startDraw();
	display.clearDrawing();

	//neda::StringExpression strExp2("B");
//	neda::StringExpression strExp("A");
//	neda::ContainerExpression expr;
//	expr.addExpr(&strExp);
//	neda::StringExpression strExp2("B");
//	expr.addExpr(&strExp2);
	//expr.getHeight();
	
	neda::StringExpr *strExp1 = new neda::StringExpr("1");
	neda::StringExpr *strExp2 = new neda::StringExpr("2");
	neda::StringExpr *strExp3 = new neda::StringExpr("9");
	neda::StringExpr *strExp4 = new neda::StringExpr("1");
	neda::StringExpr *strExp5 = new neda::StringExpr("2");
	neda::StringExpr *strExp6 = new neda::StringExpr("=X");
	neda::ExponentExpr *expExp = new neda::ExponentExpr(strExp1, strExp2);
	neda::FractionExpr *fracExp = new neda::FractionExpr(strExp4, strExp5);
	neda::ExponentExpr *expExp2 = new neda::ExponentExpr(strExp3, fracExp);
	neda::ContainerExpr *cont = new neda::ContainerExpr;
	cont->addExpr(expExp);
	//cont->addExpr(expExp2);
	cont->addExpr(strExp6);
	
	cont->draw(display, 0, 0);
	display.updateDrawing();
	
	usart::printf("Spacing: %d\r\n", expExp->getTopSpacing());
	
	delete cont;
	
	//char ch = ' ';
	//uint16_t key = 0;
	
    while(true) {
//		if((key = fetchKey()) != KEY_NULL) {
//			if(key == KEY_LEFT) {
//				if(ch > ' ') {
//					ch --;
//				}
//				else {
//					ch = 0x7F;
//				}
//			}
//			else if(key == KEY_RIGHT) {
//				if(ch < 0x7F) {
//					ch ++;
//				}
//				else {
//					ch = ' ';
//				}
//			}
//			
//			display.clearDrawingBuffer();
//			display.drawImage(0, 0, lcd::getChar(ch));
//			display.updateDrawing();
//		}
		led = !led;
		delay::ms(100);
    }
}
