#include "stm32f10x.h"
#include "sys.hpp"
#include "delay.hpp"
#include "usart.hpp"
#include "gpiopin.hpp"
#include "lcd1602.hpp"

using namespace lcd;

int main() {
	
	sys::initRCC();
	sys::initNVIC();
	delay::init();
	usart::init(115200);
	
	GPIOPin led(GPIOC, GPIO_Pin_13);
	led.init(GPIO_Mode_Out_PP, GPIO_Speed_2MHz);
	led = 0;
	
	//Disable JTAG so we can use PB3 and 4
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_Disable, ENABLE);
	
	delay::sec(3);
	
	GPIOPin RS(GPIOB, GPIO_Pin_12), RW(GPIOB, GPIO_Pin_13), E(GPIOB, GPIO_Pin_14), 
			D0(GPIOA, GPIO_Pin_15), D1(GPIOB, GPIO_Pin_3), D2(GPIOB, GPIO_Pin_4), D3(GPIOB, GPIO_Pin_5),
			D4(GPIOB, GPIO_Pin_6), D5(GPIOB, GPIO_Pin_7), D6(GPIOB, GPIO_Pin_8), D7(GPIOB, GPIO_Pin_9);
	LCD1602 lcd(RS, RW, E, D0, D1, D2, D3, D4, D5, D6, D7);
	lcd.setDataPort(0xAA);
	
    while(true) {
    }
}
