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

GPIOPin RS(GPIOB, GPIO_Pin_12), RW(GPIOB, GPIO_Pin_13), E(GPIOB, GPIO_Pin_14),
			D7(GPIOA, GPIO_Pin_15), D6(GPIOB, GPIO_Pin_3), D5(GPIOB, GPIO_Pin_4), D4(GPIOB, GPIO_Pin_5),
			D3(GPIOB, GPIO_Pin_6), D2(GPIOB, GPIO_Pin_7), D1(GPIOB, GPIO_Pin_8), D0(GPIOB, GPIO_Pin_9);
lcd::LCD12864 display(RS, RW, E, D0, D1, D2, D3, D4, D5, D6, D7);

GPIOPin SBDI_EN(GPIOA, GPIO_Pin_12);
GPIOPin SBDI_CLK(GPIOA, GPIO_Pin_11);
GPIOPin SBDI_DATA(GPIOA, GPIO_Pin_8);

volatile uint8_t level = 0;

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
	
	sbdi::Receiver receiver(SBDI_EN, SBDI_DATA, SBDI_CLK);
	receiver.init();
	receiver.onReceive([](uint32_t data) {
		level = (uint8_t) data;
	});

//	SBDI_EN.init(GPIO_Mode_IN_FLOATING, GPIO_Speed_10MHz);
//	SBDI_DATA.init(GPIO_Mode_IN_FLOATING, GPIO_Speed_10MHz);
//	SBDI_CLK.init(GPIO_Mode_IN_FLOATING, GPIO_Speed_10MHz);
//	
//	DynamicArray<uint8_t> ioData(1000);
	
	
	
    while(true) {
		display.writeData(level / 100 + 0x30);
		display.writeData((level % 100) / 10 + 0x30);
		display.writeData(level % 10 + 0x30);
		delay::ms(100);
		display.home();
		display.clear();
		led = !led;
//		while(SBDI_EN);
//		do {
//			uint8_t data = SBDI_EN << 2 | SBDI_DATA << 1 | SBDI_CLK;
//			
//			if(ioData.length() == 0) {
//				ioData.add(data);
//			}
//			else {
//				if(data != ioData[ioData.length() - 1]) {
//					ioData.add(data);
//				}
//			}
//		} while(!SBDI_EN);
//		
//		for(uint32_t i = 0; i < ioData.length(); i ++) {
//			usart::printf("EN: %d  DATA: %d  CLK: %d\r\n", (ioData[i] & 1 << 2) >> 2, (ioData[i] & 1 << 1) >> 1, ioData[i] & 1);
//		}
//		ioData.empty();
    }
}
