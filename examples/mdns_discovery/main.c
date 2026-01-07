#include "rtnet_stack.h"
#include <string.h>
#include <stdio.h>

static const RTNET_IPv6Addr_t LOCAL_IP = { .addr = {0xFE,0x80,0,0,0,0,0,0,0,0,0,0,0,0,0,3} };
static const RTNET_MACAddr_t LOCAL_MAC = { .addr = {0x00,0x10,0x20,0x30,0x40,0x50} };

int main(void)
{
    RTNET_Initialize(&LOCAL_IP, &LOCAL_MAC);

    RTNET_mDNSRecord_t record;
    RTNET_Error_t err = RTNET_mDNS_Query("_http._tcp.local", &record);

    if (err == RTNET_OK) {
        printf("Found service at port %u\n", record.port);
    } else {
        printf("mDNS query returned %d (expected on host stub)\n", err);
    }

    /* Periodic upkeep */
    for (;;) {
        RTNET_PeriodicTask();
    }
}
