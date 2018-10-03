#include "stm32f10x.h"
#include "sys.hpp"
#include "delay.hpp"
#include "gpiopin.hpp"
#include "lcd1602.hpp"

using namespace lcd;

int main() {
	
	sys::initRCC();
	sys::initNVIC();
	delay::init();
	
	GPIOPin RS(GPIOB, GPIO_Pin_3), RW(GPIOB, GPIO_Pin_4), E(GPIOB, GPIO_Pin_5), BUSY(GPIOA, GPIO_Pin_7);
	LCD1602 lcd(RS, RW, E, BUSY, GPIOA);
	
	lcd.init();
	lcd.writeString("Hello, World");
	
    while(true) {

    }
}
