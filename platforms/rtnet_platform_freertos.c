/**
 * @file rtnet_platform_freertos.c
 * @brief FreeRTOS-based platform hooks (critical sections, timing, TX)
 * @note Requires FreeRTOS headers and a board-specific Ethernet transmit hook.
 * @link https://github.com/seregonwar/rtnet-stack/blob/main/platforms/rtnet_platform_freertos.c
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
#include "FreeRTOS.h"
#include "task.h"

#if defined(_MSC_VER)
    #define RTNET_WEAK __declspec(selectany)
#else
    #define RTNET_WEAK __attribute__((weak))
#endif

/* Optional software loopback (for bring-up without NIC) */
static bool g_loopback_enabled = false;

void RTNET_Platform_EnableLoopback(bool enable)
{
    g_loopback_enabled = enable;
}

/* Board-specific transmit hook; override to use real NIC.
 * Default behavior: optional software loopback for testing. */
RTNET_WEAK void RTNET_Platform_EthTransmit(const uint8_t* data, uint16_t length)
{
    if (g_loopback_enabled && data != NULL && length > 0U) {
        /* Feed back into RX path to emulate a receive (safe in host/bring-up) */
        (void)RTNET_ProcessRxPacket(data, length);
    }
    /* Otherwise: drop silently; override this function in BSP for real TX */
}

void RTNET_CriticalSectionEnter(void)
{
    taskENTER_CRITICAL();
}

void RTNET_CriticalSectionExit(void)
{
    taskEXIT_CRITICAL();
}

uint32_t RTNET_GetTimeMs(void)
{
    return (uint32_t)(xTaskGetTickCount() * (TickType_t)portTICK_PERIOD_MS);
}

void RTNET_HardwareTransmit(const uint8_t* data, uint16_t length)
{
    RTNET_Platform_EthTransmit(data, length);
}
