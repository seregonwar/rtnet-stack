/**
 * @file rtnet_platform_baremetal.c
 * @brief Bare-metal platform hooks using IRQ disable/enable and a weak TX hook.
 * @link https://github.com/seregonwar/rtnet-stack/blob/main/platforms/rtnet_platform_baremetal.c
 * @version 1.0.0
 * @date 2026-01-07
 * @author Seregon
 * 
MIT License

Copyright (c) 2026 Seregon

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
 */

#include "rtnet_stack.h"
#include <stdint.h>

#if defined(_MSC_VER)
    #define RTNET_WEAK __declspec(selectany)
#else
    #define RTNET_WEAK __attribute__((weak))
#endif

/* Board-specific transmit hook to be provided by the BSP */
RTNET_WEAK void RTNET_Platform_EthTransmit(const uint8_t* data, uint16_t length)
{
    (void)data;
    (void)length;
    /* Implement MAC driver TX here */
}

/* Weak hooks for IRQ control; override with MCU-specific intrinsics */
RTNET_WEAK void RTNET_Platform_DisableIRQ(void)
{
    /* __disable_irq(); */
}

RTNET_WEAK void RTNET_Platform_EnableIRQ(void)
{
    /* __enable_irq(); */
}

static volatile uint32_t g_time_ms = 0U;

void RTNET_CriticalSectionEnter(void)
{
    RTNET_Platform_DisableIRQ();
}

void RTNET_CriticalSectionExit(void)
{
    RTNET_Platform_EnableIRQ();
}

uint32_t RTNET_GetTimeMs(void)
{
    /* In bare-metal mode, ensure a 1ms tick updates g_time_ms (e.g., SysTick) */
    return g_time_ms;
}

void RTNET_HardwareTransmit(const uint8_t* data, uint16_t length)
{
    RTNET_Platform_EthTransmit(data, length);
}

/* Call this from a 1ms ISR (e.g., SysTick) to advance time */
void RTNET_Platform_Tick1ms(void)
{
    g_time_ms++;
}
