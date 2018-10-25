#include "lcd12864.hpp"

namespace lcd {
	
	#define W_CMD(x) \
		if(!writeCommand(x)) \
			return false
	
	bool LCD12864::clear() {
		return writeCommand(Command::CLEAR);
	}
	bool LCD12864::home() {
		return writeCommand(Command::HOME);
	}
			
	bool LCD12864::init() {
		delay::ms(15);
		writeCommandNoWait(Command::NORMAL_CMD_8BIT);
		delay::ms(5);
		writeCommandNoWait(Command::NORMAL_CMD_8BIT);
		delay::ms(5);
		
		W_CMD(Command::ENTRY_CURSOR_SHIFT_RIGHT);
		W_CMD(Command::CLEAR);
		W_CMD(Command::DISPLAY_ON_CURSOR_OFF);
		return true;
	}
	
	bool LCD12864::setCursor(uint8_t row, uint8_t col) {
		//If using extended command set, first set to use basic, write the address and change back
		if(isUsingExtended()) {
			if(!useBasic()) 
				return false;
			switch(row){
			case 0: col += 0x80; break;
			case 1: col += 0x90; break;
			case 2: col += 0x88; break;
			case 3: col += 0x98; break;
			default: break;
			}
			//Make the first bit 1 and second bit 0 to match the command requirements
			col |= 0x80; //1000 0000
			col &= 0xBF; //1011 1111
			W_CMD(col);
			return useExtended();
		}
		else {
			switch(row){
			case 0: col += 0x80; break;
			case 1: col += 0x90; break;
			case 2: col += 0x88; break;
			case 3: col += 0x98; break;
			default: break;
			}
			//Make the first bit 1 and second bit 0 to match the command requirements
			col |= 0x80; //1000 0000
			col &= 0xBF; //1011 1111
			W_CMD(col);
			return true;
		}
	}
	
	bool LCD12864::isUsingExtended() {
		return extendedCmd;
	}
	bool LCD12864::useExtended() {
		if(extendedCmd) 
			return true;
		W_CMD(FOUR_WIRE_INTERFACE ? Command::EXT_CMD_4BIT : Command::EXT_CMD_8BIT);
		extendedCmd = true;
		return true;
	}
	bool LCD12864::useBasic() {
		if(!extendedCmd)
			return true;
		W_CMD(FOUR_WIRE_INTERFACE ? Command::NORMAL_CMD_4BIT : Command::NORMAL_CMD_8BIT);
		extendedCmd = false;
		return true;
	}
	
	#undef W_CMD
}
