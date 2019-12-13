#ifndef __TCALC_EXCEPTION_H__
#define __TCALC_EXCEPTION_H__

#include <stdint.h>

namespace exception {
	void fillBacktrace(uint32_t backtrace[], uint16_t &backtrace_length, uint32_t PC, uint32_t LR, uint32_t SP);

	void fillSymbols(char resolved[][64], const uint32_t backtrace[], uint16_t length);

	void loadRegsFromFaultTrace(uint32_t *faultSP, uint32_t &PC, uint32_t &LR, uint32_t &SP);

	__attribute__((always_inline)) static inline void loadRegsFromCurrentLocation(uint32_t &PC, uint32_t &LR, uint32_t &SP) {
		uint32_t pc, lr, sp;
		asm volatile (
			"mov %0, pc\n\t"
			"mov %1, lr\n\t"
			"mov %2, sp\n\t" :
			"=r" (pc), "=r" (lr), "=r" (sp)
		);
		PC = pc; LR = lr; SP = sp;
	}
}

#endif
