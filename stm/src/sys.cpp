#include "sys.hpp"

namespace sys {
    void initNVIC() {
        NVIC_SetPriorityGrouping(NVIC_PriorityGroup_2);

		// Enable Usage and Bus faults
		NVIC_InitTypeDef nvicInit;
		nvicInit.NVIC_IRQChannel = BusFault_IRQn;
		nvicInit.NVIC_IRQChannelCmd = ENABLE;
		nvicInit.NVIC_IRQChannelSubPriority = 1;
		nvicInit.NVIC_IRQChannelPreemptionPriority = 1;
		NVIC_Init(&nvicInit);
		nvicInit.NVIC_IRQChannel = UsageFault_IRQn;
		nvicInit.NVIC_IRQChannelSubPriority = 2;
		NVIC_Init(&nvicInit);
    }
} // namespace sys
