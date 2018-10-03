#include "stm32f10x.h"
#include "sys.hpp"
#include "delay.hpp"
#include "lcd.hpp"

int main() {
	
	sys::initRCC();
	sys::initNVIC();
	delay::init();
	
    while(true) {

    }
}
