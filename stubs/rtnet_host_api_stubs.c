/* Host stub implementations for public APIs to enable desktop/test builds.
 * Not included in production builds unless RTNS_USE_PLATFORM_STUBS=ON. */

#include "rtnet_stack.h"
#include <string.h>

RTNET_Error_t RTNET_ProcessRxPacket(const uint8_t* data, uint16_t length)
{
    if ((data == NULL) || (length == 0U)) {
        return RTNET_ERR_INVALID_PARAM;
    }

    /* Basic length check (Ethernet + IPv6 header) */
    if (length < (14U + 40U)) {
        return RTNET_ERR_INVALID_PARAM;
    }

    /* Host stub: increment RX count, signal checksum validation path */
    return RTNET_ERR_CHECKSUM;
}

RTNET_Error_t RTNET_UDP_Send(const RTNET_IPv6Addr_t* dest_addr,
                              uint16_t dest_port,
                              uint16_t src_port,
                              const uint8_t* payload,
                              uint16_t payload_len,
                              uint8_t qos_priority)
{
    (void)qos_priority;

    if (!dest_addr || !payload || (dest_port == 0U) || (payload_len == 0U)) {
        return RTNET_ERR_INVALID_PARAM;
    }

    if (payload_len > RTNET_MTU_SIZE) {
        return RTNET_ERR_INVALID_PARAM;
    }

    if (src_port == 0U) {
        /* In host mode we don't manage ephemeral ports; accept as-is */
    }

    return RTNET_OK;
}

RTNET_Error_t RTNET_TCP_Connect(const RTNET_IPv6Addr_t* dest_addr,
                                 uint16_t dest_port,
                                 uint8_t* connection_id)
{
    if (!dest_addr || (dest_port == 0U) || (connection_id == NULL)) {
        return RTNET_ERR_INVALID_PARAM;
    }

    *connection_id = 0U;
    return RTNET_OK;
}

RTNET_Error_t RTNET_TCP_Send(uint8_t connection_id,
                              const uint8_t* data,
                              uint16_t length)
{
    (void)connection_id;
    if (!data || (length == 0U)) {
        return RTNET_ERR_INVALID_PARAM;
    }
    return RTNET_OK;
}

RTNET_Error_t RTNET_TCP_Close(uint8_t connection_id)
{
    (void)connection_id;
    return RTNET_OK;
}

RTNET_Error_t RTNET_mDNS_Query(const char* service_name,
                                RTNET_mDNSRecord_t* result)
{
    if (!service_name || !result) {
        return RTNET_ERR_INVALID_PARAM;
    }
    memset(result, 0, sizeof(RTNET_mDNSRecord_t));
    return RTNET_ERR_TIMEOUT; /* Host stub: no responder */
}

RTNET_Error_t RTNET_mDNS_Announce(const char* service_name,
                                   uint16_t port,
                                   uint32_t ttl_sec)
{
    if (!service_name || (port == 0U) || (ttl_sec == 0U)) {
        return RTNET_ERR_INVALID_PARAM;
    }
    return RTNET_OK;
}
