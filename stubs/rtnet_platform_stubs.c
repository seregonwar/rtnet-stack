/* Host stub implementations for platform hooks */
#include "rtnet_stack.h"

void RTNET_CriticalSectionEnter(void) {}

void RTNET_CriticalSectionExit(void) {}

uint32_t RTNET_GetTimeMs(void)
{
    static uint32_t t = 0U;
    t += 10U;
    return t;
}

void RTNET_HardwareTransmit(const uint8_t* data, uint16_t length)
{
    (void)data;
    (void)length;
    /* Stub: no-op for host build */
}
