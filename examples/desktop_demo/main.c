#include "rtnet_stack.h"
#include <stdio.h>
#include <string.h>

static const RTNET_IPv6Addr_t LOCAL_IP = { .addr = {0xFE,0x80,0,0,0,0,0,0,0,0,0,0,0,0,0,0x10} };
static const RTNET_MACAddr_t  LOCAL_MAC = { .addr = {0x00,0xDE,0xAD,0xBE,0xEF,0x01} };
static const RTNET_IPv6Addr_t REMOTE_IP = { .addr = {0x20,0x01,0x0D,0xB8,0,0,0,0,0,0,0,0,0,0,0,0x01} }; /* 2001:db8::1 */

static bool setup_stack(void)
{
    if (RTNET_Initialize(&LOCAL_IP, &LOCAL_MAC) != RTNET_OK) {
        printf("[demo] Init failed\n");
        return false;
    }

    /* Add a host route to REMOTE_IP (direct for demo) */
    if (RTNET_AddRoute(&REMOTE_IP, 128U, NULL, 1U) != RTNET_OK) {
        printf("[demo] Route add failed\n");
        return false;
    }

    return true;
}

static void demo_udp(void)
{
    const uint8_t payload[] = "hello from host";
    RTNET_Error_t err = RTNET_UDP_Send(&REMOTE_IP, 12345U, 0U,
                                       payload, (uint16_t)sizeof(payload),
                                       RTNET_QOS_NORMAL);
    printf("[demo][udp] send -> %d\n", err);
}

static void demo_tcp(void)
{
    uint8_t conn_id = 0U;
    RTNET_Error_t err = RTNET_TCP_Connect(&REMOTE_IP, 80U, &conn_id);
    printf("[demo][tcp] connect -> %d (conn=%u)\n", err, conn_id);
    if (err == RTNET_OK) {
        const uint8_t http_get[] = "GET / HTTP/1.1\r\nHost: demo\r\n\r\n";
        err = RTNET_TCP_Send(conn_id, http_get, (uint16_t)sizeof(http_get));
        printf("[demo][tcp] send -> %d\n", err);
        err = RTNET_TCP_Close(conn_id);
        printf("[demo][tcp] close -> %d\n", err);
    }
}

static void demo_mdns(void)
{
    RTNET_mDNSRecord_t rec;
    RTNET_Error_t err = RTNET_mDNS_Query("_http._tcp.local", &rec);
    printf("[demo][mdns] query -> %d", err);
    if (err == RTNET_OK) {
        printf(" (port=%u)\n", rec.port);
    } else {
        printf("\n");
    }
}

int main(void)
{
    if (!setup_stack()) {
        return -1;
    }

    demo_udp();
    demo_tcp();
    demo_mdns();

    /* Run a few maintenance ticks to emulate periodic servicing */
    for (int i = 0; i < 3; i++) {
        RTNET_PeriodicTask();
    }

    RTNET_Statistics_t stats;
    if (RTNET_GetStatistics(&stats) == RTNET_OK) {
        printf("[demo][stats] tx=%lu rx=%lu dropped=%lu routing_err=%lu\n",
               (unsigned long)stats.tx_packets,
               (unsigned long)stats.rx_packets,
               (unsigned long)stats.tx_dropped,
               (unsigned long)stats.routing_errors);
    }

    return 0;
}
