#include "rtnet_stack.h"
#include <string.h>

static const RTNET_IPv6Addr_t LOCAL_IP = { .addr = {0xFE,0x80,0,0,0,0,0,0,0,0,0,0,0,0,0,2} };
static const RTNET_MACAddr_t LOCAL_MAC = { .addr = {0x00,0xAA,0xBB,0xCC,0xDD,0xEE} };
static const RTNET_IPv6Addr_t SERVER_IP = { .addr = {0x20,0x01,0x0D,0xB8,0,0,0,0,0,0,0,0,0,0,0,1} }; /* 2001:db8::1 */

int main(void)
{
    RTNET_Initialize(&LOCAL_IP, &LOCAL_MAC);
    RTNET_AddRoute(&SERVER_IP, 128U, NULL, 1U); /* direct for demo */

    uint8_t conn_id;
    if (RTNET_TCP_Connect(&SERVER_IP, 80U, &conn_id) == RTNET_OK) {
        const uint8_t http_get[] = "GET / HTTP/1.1\r\nHost: demo\r\n\r\n";
        RTNET_TCP_Send(conn_id, http_get, (uint16_t)sizeof(http_get));
        RTNET_TCP_Close(conn_id);
    }

    for (;;) {
        RTNET_PeriodicTask();
    }
}
