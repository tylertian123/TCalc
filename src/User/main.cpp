#include "stm32f10x.h"
#include "sys.hpp"
#include "delay.hpp"
#include "usart.hpp"
#include "gpiopin.hpp"
#include "lcd1602.hpp"
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
	
	GPIOPin RS(GPIOB, GPIO_Pin_12), RW(GPIOB, GPIO_Pin_13), E(GPIOB, GPIO_Pin_14),
			D4(GPIOB, GPIO_Pin_6), D5(GPIOB, GPIO_Pin_7), D6(GPIOB, GPIO_Pin_8), D7(GPIOB, GPIO_Pin_9);
	LCD1602 lcd(RS, RW, E, D4, D5, D6, D7);
	led = !lcd.init();
	lcd.writeString("Hello, World");
	
	GPIOPin PWMPin(GPIOA, GPIO_Pin_1);
	pwm::PWMOutput<2, 72000000, 100> PWMOut(TIM2, RCC_APB1Periph_TIM2, PWMPin);
	PWMOut.startTimer();
	
	uint8_t level = 0;
	
    while(true) {
		//PWMOut.set(0);
		PWMOut.set(level ++);
		delay::ms(10);
    }
}
