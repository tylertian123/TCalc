#include "stm32f10x.h"
#include "sys.hpp"
#include "delay.hpp"
#include "usart.hpp"
#include "gpiopin.hpp"
#include "extipin.hpp"
#include "lcd12864.hpp"
#include "lcd12864_charset.hpp"
#include "pwm.hpp"

GPIOPin RS(GPIOB, GPIO_Pin_12), RW(GPIOB, GPIO_Pin_13), E(GPIOB, GPIO_Pin_14),
			D7(GPIOA, GPIO_Pin_15), D6(GPIOB, GPIO_Pin_3), D5(GPIOB, GPIO_Pin_4), D4(GPIOB, GPIO_Pin_5),
			D3(GPIOB, GPIO_Pin_6), D2(GPIOB, GPIO_Pin_7), D1(GPIOB, GPIO_Pin_8), D0(GPIOB, GPIO_Pin_9);
lcd::LCD12864 display(RS, RW, E, D0, D1, D2, D3, D4, D5, D6, D7);

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
	
	display.init();
	
	
	GPIOPin PWMPin(GPIOA, GPIO_Pin_1);
	pwm::PWMOutput<2, 72000000, 100> PWMOut(TIM2, RCC_APB1Periph_TIM2, PWMPin);
	PWMOut.startTimer();
	PWMOut.set(0xA0);
	
	GPIOPin interruptPin(GPIOA, GPIO_Pin_11);
	
	EXTIPin interrupt(interruptPin);
	interrupt.init(GPIO_Speed_10MHz, EXTI_Trigger_Falling, 0x03, 0x03);
	interrupt.setCallback([]() {
		display.writeString("Falling Edge");
	});
	
    while(true) {
    }
}
