# RTNet Stack – API Reference

## Initialization
- `RTNET_Error_t RTNET_Initialize(const RTNET_IPv6Addr_t* local_ipv6, const RTNET_MACAddr_t* local_mac);`  
  Initializes stack context and seeds timers/ports.

- `RTNET_Error_t RTNET_AddRoute(const RTNET_IPv6Addr_t* destination, uint8_t prefix_len, const RTNET_IPv6Addr_t* next_hop, uint16_t metric);`  
  Adds static route; `next_hop == NULL` means directly connected.

- `void RTNET_PeriodicTask(void);`  
  Maintenance (timeouts, cache aging). Call ~every 100 ms.

## RX/TX
- `RTNET_Error_t RTNET_ProcessRxPacket(const uint8_t* data, uint16_t length);`  
  Feed a received Ethernet frame (IPv6).

- `RTNET_Error_t RTNET_UDP_Send(const RTNET_IPv6Addr_t* dest_addr, uint16_t dest_port, uint16_t src_port, const uint8_t* payload, uint16_t payload_len, uint8_t qos_priority);`  
  Sends UDP datagram. `src_port=0` auto-assigns an ephemeral port. QoS values: `RTNET_QOS_{CRITICAL,HIGH,NORMAL,LOW}`.

## TCP-Lite
- `RTNET_Error_t RTNET_TCP_Connect(const RTNET_IPv6Addr_t* dest_addr, uint16_t dest_port, uint8_t* connection_id);`
- `RTNET_Error_t RTNET_TCP_Send(uint8_t connection_id, const uint8_t* data, uint16_t length);`
- `RTNET_Error_t RTNET_TCP_Close(uint8_t connection_id);`

Connections are limited by `RTNET_MAX_TCP_CONNECTIONS`. States are simplified for embedded timing.

## mDNS
- `RTNET_Error_t RTNET_mDNS_Query(const char* service_name, RTNET_mDNSRecord_t* result);`
- `RTNET_Error_t RTNET_mDNS_Announce(const char* service_name, uint16_t port, uint32_t ttl_sec);`

Cache size: `RTNET_MAX_MDNS_CACHE`. Names are limited to 63 chars (plus terminator).

## Statistics
- `RTNET_Error_t RTNET_GetStatistics(RTNET_Statistics_t* stats);`
  Returns RX/TX counters, drops, checksum and routing errors.

## Platform Hooks (BSP)
- `void RTNET_CriticalSectionEnter/Exit(void);`
- `uint32_t RTNET_GetTimeMs(void);`
- `void RTNET_HardwareTransmit(const uint8_t* data, uint16_t length);`

Provide these in production; host builds use stubs.
