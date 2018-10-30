#include "stm32f10x.h"
#include "sys.hpp"
#include "delay.hpp"
#include "usart.hpp"
#include "gpiopin.hpp"
#include "lcd12864.hpp"
#include "lcd12864_charset.hpp"
#include "pwm.hpp"

int main() {
	
	sys::initRCC();
	sys::initNVIC();
	usart::init(115200);
	
	GPIOPin led(GPIOC, GPIO_Pin_13);
	led.init(GPIO_Mode_Out_PP, GPIO_Speed_2MHz);
	led = 0;
	
	//Disable JTAG so we can use PB3 and 4
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_Disable, ENABLE);
	
	delay::sec(3);
	
	GPIOPin RS(GPIOB, GPIO_Pin_12), RW(GPIOB, GPIO_Pin_13), E(GPIOB, GPIO_Pin_14),
			D7(GPIOA, GPIO_Pin_15), D6(GPIOB, GPIO_Pin_3), D5(GPIOB, GPIO_Pin_4), D4(GPIOB, GPIO_Pin_5),
			D3(GPIOB, GPIO_Pin_6), D2(GPIOB, GPIO_Pin_7), D1(GPIOB, GPIO_Pin_8), D0(GPIOB, GPIO_Pin_9);
	lcd::LCD12864 primaryLCD(RS, RW, E, D0, D1, D2, D3, D4, D5, D6, D7);
	primaryLCD.init();
	
	
	GPIOPin PWMPin(GPIOA, GPIO_Pin_1);
	pwm::PWMOutput<2, 72000000, 100> PWMOut(TIM2, RCC_APB1Periph_TIM2, PWMPin);
	PWMOut.startTimer();
	PWMOut.set(0xA0);
	
	led = !primaryLCD.clear();
	led = !primaryLCD.useExtended();
	led = !primaryLCD.startDraw();
	led = !primaryLCD.clearDrawing();
	primaryLCD.drawImage(0, 0, lcd::TEST_CHAR);
	for(uint8_t x = 0; x < 128; x += 4) {
		primaryLCD.drawImage(x, 10, lcd::TEST_CHAR);
	}
	//Looks like train tracks! Yay!
	for(uint8_t x = 0; x < 128; x += 2) {
		primaryLCD.drawImage(x, 30, lcd::TEST_CHAR);
	}
	primaryLCD.updateDrawing();
	
    while(true) {
    }
}
