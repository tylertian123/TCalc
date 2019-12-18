#include "sys.hpp"

namespace sys {
    // Initialization configuration for the RCC
    void initRCC() {
        // Reset RCC config
        RCC_DeInit();
        // Use the HSE (High-Speed External oscillator)
        RCC_HSEConfig(RCC_HSE_ON);
        // Wait for HSE to start
        ErrorStatus errStat = RCC_WaitForHSEStartUp();

        if (errStat == SUCCESS) {
            // Configure PLL
            // Use undivided HSE signal and multiply by 9 to get the full 72MHz
            RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_9);
            // Configure AHB clock
            RCC_HCLKConfig(RCC_SYSCLK_Div1);
            // Configure APB clock
            RCC_PCLK1Config(RCC_HCLK_Div2); // Low-Speed APB
            RCC_PCLK2Config(RCC_HCLK_Div1); // High-Speed APB
            // Configure Flash latency
            FLASH_SetLatency(FLASH_Latency_2);
            // Configure Flash Prefetch Buffer
            FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);
            // Enable PLL
            RCC_PLLCmd(ENABLE);
            // Wait until the RCC is ready
            while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) != SET)
                ;
            // Set SYSCLK source to PLL output
            RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
            // Wait until the SYSCLK source is correctly configured
            while (RCC_GetSYSCLKSource() != SYSCLK_Source_PLL)
                ;
        }
    }

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
