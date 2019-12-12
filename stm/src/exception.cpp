#include <gunzip.h>
#include "exception.hpp"
#include <unwind.h>

// Definitions to match with external data

extern "C" {
	// Defined in the linker
	extern const uint32_t dbg_symtable_size;
	extern const uint8_t dbg_data_begin;

	// Derived in nicer types
	extern const uint8_t * const dbg_symtable_begin = &dbg_data_begin;
	extern const uint8_t * const dbg_backtable_begin = (&dbg_data_begin) + dbg_symtable_size;

	// Top of stack
	extern const uint32_t _estack;
};

namespace exception {

	void _applyUnwindTo(uint32_t &PC, uint32_t &LR, uint32_t &SP, int16_t unwind_amt) {
		// If unwind_amt is positive, then we add that much to SP, deref to get PC, and add 4 to SP.
		if (unwind_amt >= 0) {
			SP += unwind_amt;
			PC = *reinterpret_cast<uint32_t *>(SP);
			SP += 4;
		}
		// If unwind_amt is negative max, there was no stack allocated.
		else if (unwind_amt == -32768) {
			PC = LR;
		}
		else {
			// Otherwise, add -unwind_amt to SP, and use LR
			SP += -unwind_amt;
			PC = LR;
		}
	}

	// Sliding window backtrace parser
	
	struct BackTraceParseState {
		uint8_t parse_state = 0;  // Upper 4 bits = state, lower 4 bits = index
								  // where state = (
								  // 	0 - nothing
								  // 	1 - getting addr 1
								  // 	2 - getting addr 2
								  // 	3 - getting unwind amt
								  // )
								  // where index is the byte into the value
		uint32_t lower_PC, upper_PC;
		int16_t  unwind_amt;
		
		uint32_t PC;
		uint32_t LR;
		uint32_t SP;

		uint32_t * backtrace;
		uint16_t   bt_len;
		uint16_t   bt_maxlen;

		bool signalDone;
	};

	BackTraceParseState *btps;

	void fillBacktrace(uint32_t backtrace[], uint16_t &backtrace_length, uint32_t PC, uint32_t LR, uint32_t SP) {
		// Fill the backtrace with the PC over time
		// To do this, we need to scan through the dbg data, starting at dbg_data_begin + dbg_symtable_Size
		
		BackTraceParseState local_btps;
		btps = &local_btps; // to get around extraneous lambda types
		local_btps.bt_maxlen = backtrace_length;
		local_btps.bt_len = 1;

		// Scanning for is always the current PC. Start by laoding it into backtrace
		backtrace[0] = PC;
		local_btps.backtrace = backtrace;

		local_btps.PC = PC; local_btps.LR = LR; local_btps.SP = SP;
		local_btps.signalDone = false;

		while (!local_btps.signalDone && local_btps.bt_len < local_btps.bt_maxlen) {
			// Set the initial state of the BTP
			local_btps.parse_state = 0x10;

			// Deflate the entire buffer
			tinydeflate::Deflate(dbg_backtable_begin+0, +[](uint8_t data){ // this lambda is positive. (it forces a cast to pointer to avoid flash space)
				// Our callback; should be called whenever
				
				switch (btps->parse_state & 0xF0) {
					case 0x10:
						btps->lower_PC <<= 8; btps->lower_PC |= data;
						++btps->parse_state;
						if ((btps->parse_state & 0x0F) == 4) btps->parse_state = 0x20;
						break;
					case 0x20:
						btps->upper_PC <<= 8; btps->upper_PC |= data;
						++btps->parse_state;
						if ((btps->parse_state & 0x0F) == 4) btps->parse_state = 0x30;
						break;
					case 0x30:
						btps->unwind_amt <<= 8; btps->upper_PC |= data;
						++btps->parse_state;
						break;
					default:
						break;
				}

				if (btps->parse_state == 0x32) {
					// Check if this is an acceptable unwind value.
					while (btps->lower_PC <= btps->PC && btps->upper_PC < btps->PC) {
						// It is! apply the unwind amount
						_applyUnwindTo(btps->PC, btps->LR, btps->SP, btps->unwind_amt);
						// Push the new PC to the backtrace
						btps->backtrace[btps->bt_len++] = btps->PC;

						// Check if we have reached the end of the backtrace
						if (btps->SP >= reinterpret_cast<uint32_t>(&_estack)) {
							btps->signalDone = true;
							return true;
						}

						// Check if we have overshot our next PC.
						if (btps->PC < btps->lower_PC) {return true;} // If so, return early and re-decompress.

						// If we are still in this unwind-block, this inner loop keeps running.
						// If the value is later, this function returns and decompression continues.
					}

					btps->parse_state = 0x10; // Get ready for the next value
				}

				return false;
			});
		}

		backtrace_length = local_btps.bt_len;
		btps = nullptr;
	}

	void fillSymbols(char resolved[][64], const uint32_t backtrace[], uint16_t length) {
	}

	void loadRegsFromFaultTrace(uint32_t *faultSP, uint32_t &PC, uint32_t &LR, uint32_t &SP) {
		PC = faultSP[6];
		LR = faultSP[7];
		SP = reinterpret_cast<uint32_t>(SP);
	}

};
