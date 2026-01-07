#include "rtnet_stack.h"
#include <stdio.h>
#include <string.h>

static const RTNET_IPv6Addr_t LOCAL_IP = { .addr = {0xFE,0x80,0,0,0,0,0,0,0,0,0,0,0,0,0,1} };
static const RTNET_MACAddr_t  LOCAL_MAC = { .addr = {0x00,0x11,0x22,0x33,0x44,0x55} };
static const RTNET_IPv6Addr_t REMOTE_IP = { .addr = {0xFE,0x80,0,0,0,0,0,0,0,0,0,0,0,0,0,2} };

/* On target: invoke from Ethernet RX ISR with the received frame */
static void ethernet_rx_handler(const uint8_t* frame, uint16_t length)
{
    if ((frame != NULL) && (length > 0U)) {
        (void)RTNET_ProcessRxPacket(frame, length);
    }
}

static bool init_stack(void)
{
    if (RTNET_Initialize(&LOCAL_IP, &LOCAL_MAC) != RTNET_OK) {
        printf("[udp_echo] Init failed\n");
        return false;
    }

    /* Add link-local /64 for echo traffic */
    RTNET_IPv6Addr_t prefix = LOCAL_IP;
    prefix.addr[15] = 0;
    if (RTNET_AddRoute(&prefix, 64U, NULL, 1U) != RTNET_OK) {
        printf("[udp_echo] Route add failed\n");
        return false;
    }

    return true;
}

static void send_echo_demo(void)
{
    const uint8_t echo_payload[] = "echo";
    RTNET_Error_t err = RTNET_UDP_Send(&REMOTE_IP, 7U, 0U,
                                       echo_payload, (uint16_t)sizeof(echo_payload),
                                       RTNET_QOS_NORMAL);
    if (err != RTNET_OK) {
        printf("[udp_echo] UDP send error: %d\n", err);
    }
}

int main(void)
{
    if (!init_stack()) {
        return -1;
    }

    for (;;) {
        /* In real firmware, feed frames from ETH driver to the stack */
        ethernet_rx_handler(NULL, 0);

        /* Reply/demo send (would typically be triggered by received data) */
        send_echo_demo();

        /* Periodic maintenance for aging and timeouts */
        RTNET_PeriodicTask();
    }
}
