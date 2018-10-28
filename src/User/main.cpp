#include "stm32f10x.h"
#include "sys.hpp"
#include "delay.hpp"
#include "usart.hpp"
#include "gpiopin.hpp"
#include "lcd1602.hpp"
#include "lcd12864.hpp"
#include "pwm.hpp"

using namespace lcd;

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
	
	GPIOPin RS(GPIOB, GPIO_Pin_12), RW(GPIOB, GPIO_Pin_13), pE(GPIOA, GPIO_Pin_12), sE(GPIOB, GPIO_Pin_14),
			pD4(GPIOB, GPIO_Pin_6), pD5(GPIOB, GPIO_Pin_7), pD6(GPIOB, GPIO_Pin_8), pD7(GPIOB, GPIO_Pin_9),
			sD4(GPIOA, GPIO_Pin_15), sD5(GPIOB, GPIO_Pin_3), sD6(GPIOB, GPIO_Pin_4), sD7(GPIOB, GPIO_Pin_5);
	LCD12864 primaryLCD(RS, RW, pE, pD4, pD5, pD6, pD7);
	primaryLCD.init();
	LCD1602 secondaryLCD(RS, RW, sE, sD4, sD5, sD6, sD7);
	secondaryLCD.init();
	
	primaryLCD.writeString("Primary LCD");
	secondaryLCD.writeString("Secondary LCD");
	
	
	GPIOPin PWMPin(GPIOA, GPIO_Pin_1);
	pwm::PWMOutput<2, 72000000, 100> PWMOut(TIM2, RCC_APB1Periph_TIM2, PWMPin);
	PWMOut.startTimer();
	PWMOut.set(0xA0);
	
	led = !primaryLCD.clear();
	led = !primaryLCD.useExtended();
	led = !primaryLCD.startDraw();
	led = !primaryLCD.clearDrawing();
	//primaryLCD.setPixel(1, 1, 1);
	//primaryLCD.setPixel(0, 1, 1);
	//primaryLCD.updateDrawing();
	//primaryLCD.endDraw();
	//primaryLCD.useBasic();
	
    while(true) {
    }
}
