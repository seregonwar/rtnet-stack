# RT Network Stack - Integration Guide

**Version:** 1.0.0  
**Target Audience:** Embedded Systems Engineers  
**Prerequisites:** ARM Cortex-M4 knowledge, basic TCP/IP understanding

---

## 📋 TABLE OF CONTENTS

1. [Quick Start](#1-quick-start)
2. [Platform Integration](#2-platform-integration)
3. [Configuration](#3-configuration)
4. [Usage Examples](#4-usage-examples)
5. [Best Practices](#5-best-practices)
6. [Troubleshooting](#6-troubleshooting)
7. [Performance Tuning](#7-performance-tuning)

---

## 1. QUICK START

### 1.1 Minimal Example

```c
#include "rtnet_stack.h"

/* Define your addresses */
static const RTNET_IPv6Addr_t my_ipv6 = {
    .addr = {0xFE, 0x80, 0, 0, 0, 0, 0, 0, 
             0x02, 0x00, 0x5E, 0xFF, 0xFE, 0x00, 0x53, 0x00}
};

static const RTNET_MACAddr_t my_mac = {
    .addr = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55}
};

int main(void)
{
    /* 1. Initialize hardware */
    HAL_Init();
    SystemClock_Config();
    Ethernet_Init();
    
    /* 2. Initialize network stack */
    if (RTNET_Initialize(&my_ipv6, &my_mac) != RTNET_OK) {
        Error_Handler();
    }
    
    /* 3. Add default route */
    RTNET_IPv6Addr_t gateway = {
        .addr = {0xFE, 0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x01}
    };
    RTNET_AddRoute(NULL, 0, &gateway, 10);
    
    /* 4. Main loop */
    while (1) {
        RTNET_PeriodicTask(); /* Call every 100ms */
        HAL_Delay(100);
    }
}

/* Ethernet RX interrupt handler */
void ETH_IRQHandler(void)
{
    uint8_t packet[RTNET_MTU_SIZE];
    uint16_t length;
    
    if (Ethernet_ReceivePacket(packet, &length)) {
        RTNET_ProcessRxPacket(packet, length);
    }
}
```

---

## 2. PLATFORM INTEGRATION

### 2.1 Required Platform Hooks

You must implement these functions in your BSP:

```c
/**
 * @brief Enter critical section (disable interrupts)
 * @note Must be reentrant (count nesting depth)
 */
void RTNET_CriticalSectionEnter(void)
{
    __disable_irq(); /* Or use FreeRTOS taskENTER_CRITICAL() */
}

/**
 * @brief Exit critical section (enable interrupts)
 */
void RTNET_CriticalSectionExit(void)
{
    __enable_irq(); /* Or use FreeRTOS taskEXIT_CRITICAL() */
}

/**
 * @brief Get monotonic timestamp in milliseconds
 * @return Current time (wraps at UINT32_MAX)
 */
uint32_t RTNET_GetTimeMs(void)
{
    return HAL_GetTick(); /* Or FreeRTOS xTaskGetTickCount() */
}

/**
 * @brief Transmit packet via Ethernet hardware
 * @param data Packet data (includes Ethernet header)
 * @param length Total packet length
 */
void RTNET_HardwareTransmit(const uint8_t* data, uint16_t length)
{
    Ethernet_Transmit(data, length);
    
    /* With DMA (preferred): */
    // DMA_Transfer(&ETH_DMA_TxDesc, data, length);
    // ETH->DMATPDR = 0; /* Trigger transmission */
}
```

---

### 2.2 Interrupt Configuration

**Priority Recommendations:**

| Interrupt | Priority | Notes |
|-----------|----------|-------|
| Ethernet RX | 5 | Medium priority |
| SysTick (timer) | 15 | Lowest priority |
| Critical control | 0-4 | Higher than network |

**Example (STM32):**

```c
void NVIC_Config(void)
{
    HAL_NVIC_SetPriority(ETH_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(ETH_IRQn);
}
```

---

### 2.3 Memory Layout

**Linker Script Recommendations:**

```ld
/* Place network buffers in fast RAM (CCM on STM32F4) */
.network_buffers (NOLOAD) :
{
    . = ALIGN(4);
    _network_buffers_start = .;
    *(.network_buffers)
    . = ALIGN(4);
    _network_buffers_end = .;
} >CCMRAM

/* Or use DMA-accessible RAM */
.dma_buffers (NOLOAD) :
{
    . = ALIGN(4);
    *(.dma_buffers)
} >SRAM1
```

**In code:**

```c
/* For zero-copy DMA */
__attribute__((section(".dma_buffers")))
static RTNET_Buffer_t g_dma_rx_buffers[RTNET_MAX_RX_BUFFERS];
```

---

## 3. CONFIGURATION

### 3.1 Compile-Time Options

Edit `rtnet_stack.h`:

```c
/* Reduce memory footprint for small MCUs */
#define RTNET_MAX_RX_BUFFERS        4U   /* Was: 8 */
#define RTNET_MAX_TCP_CONNECTIONS   2U   /* Was: 4 */
#define RTNET_MAX_ROUTING_ENTRIES   16U  /* Was: 32 */

/* Increase for high-throughput applications */
#define RTNET_TCP_WINDOW_SIZE       8192U  /* Was: 4096 */
#define RTNET_MAX_TX_BUFFERS        16U    /* Was: 8 */
```

---

### 3.2 Runtime Configuration

```c
/* Adjust QoS priorities based on application */
typedef enum {
    APP_QOS_CONTROL     = RTNET_QOS_CRITICAL,  /* 0 */
    APP_QOS_TELEMETRY   = RTNET_QOS_HIGH,      /* 1 */
    APP_QOS_BULK_DATA   = RTNET_QOS_NORMAL,    /* 2 */
    APP_QOS_LOGGING     = RTNET_QOS_LOW        /* 3 */
} App_QoS_t;
```

---

## 4. USAGE EXAMPLES

### 4.1 UDP Client (Sending Data)

```c
void send_sensor_reading(float temperature, float humidity)
{
    RTNET_IPv6Addr_t server = {
        .addr = {0x20, 0x01, 0x0D, 0xB8, /* ... */ }
    };
    
    /* Format payload */
    char payload[64];
    snprintf(payload, sizeof(payload), 
             "{\"temp\":%.2f,\"hum\":%.2f}", temperature, humidity);
    
    /* Send UDP packet */
    RTNET_Error_t err = RTNET_UDP_Send(
        &server,
        5000,                          /* dest port */
        0,                             /* auto-assign src port */
        (const uint8_t*)payload,
        strlen(payload),
        RTNET_QOS_NORMAL
    );
    
    if (err != RTNET_OK) {
        printf("UDP send failed: %d\n", err);
    }
}
```

---

### 4.2 UDP Server (Receiving Data)

```c
/* Register callback for UDP port */
void udp_receive_callback(const uint8_t* payload, uint16_t length,
                          const RTNET_IPv6Addr_t* src_addr, uint16_t src_port)
{
    /* Process received data */
    printf("Received %u bytes from port %u\n", length, src_port);
    
    /* Echo back */
    RTNET_UDP_Send(src_addr, src_port, 5000, payload, length, RTNET_QOS_NORMAL);
}

/* In your initialization */
RTNET_UDP_RegisterCallback(5000, udp_receive_callback);
```

---

### 4.3 TCP Client

```c
void http_get_request(void)
{
    RTNET_IPv6Addr_t web_server = {
        .addr = {0x20, 0x01, 0x04, 0x70, /* ... */}
    };
    
    uint8_t conn_id;
    
    /* 1. Connect */
    RTNET_Error_t err = RTNET_TCP_Connect(&web_server, 80, &conn_id);
    if (err != RTNET_OK) {
        printf("TCP connect failed\n");
        return;
    }
    
    /* 2. Send HTTP request */
    const char* request = "GET / HTTP/1.1\r\nHost: example.com\r\n\r\n";
    err = RTNET_TCP_Send(conn_id, (const uint8_t*)request, strlen(request));
    if (err != RTNET_OK) {
        printf("TCP send failed\n");
        RTNET_TCP_Close(conn_id);
        return;
    }
    
    /* 3. Receive response (callback-based) */
    /* ... handled by registered TCP receive callback ... */
    
    /* 4. Close connection */
    RTNET_TCP_Close(conn_id);
}
```

---

### 4.4 mDNS Service Discovery

```c
void discover_http_servers(void)
{
    RTNET_mDNSRecord_t record;
    
    RTNET_Error_t err = RTNET_mDNS_Query("_http._tcp.local", &record);
    
    if (err == RTNET_OK) {
        printf("Found HTTP server: %s at port %u\n", 
               record.service_name, record.port);
        
        /* Connect to discovered service */
        uint8_t conn_id;
        RTNET_TCP_Connect(&record.ipv6_addr, record.port, &conn_id);
    } else {
        printf("No HTTP servers found\n");
    }
}

void announce_device_service(void)
{
    /* Announce our device on the network */
    RTNET_mDNS_Announce("_device-control._tcp.local", 8080, 3600);
}
```

---

### 4.5 Routing Table Management

```c
void configure_routing(void)
{
    /* Link-local prefix (fe80::/10) - auto-added by stack */
    
    /* Add route to corporate network */
    RTNET_IPv6Addr_t corp_network = {
        .addr = {0x20, 0x01, 0x0D, 0xB8, 0x00, 0x01, /* ... */}
    };
    RTNET_IPv6Addr_t corp_gateway = {
        .addr = {0xFE, 0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x01}
    };
    RTNET_AddRoute(&corp_network, 64, &corp_gateway, 10);
    
    /* Add route to IoT subnet */
    RTNET_IPv6Addr_t iot_network = {
        .addr = {0xFD, 0x00, 0x00, 0x01, /* ULA prefix */ }
    };
    RTNET_AddRoute(&iot_network, 48, NULL, 5); /* Directly connected */
}
```

---

## 5. BEST PRACTICES

### 5.1 Error Handling

**Always check return values:**

```c
RTNET_Error_t err = RTNET_UDP_Send(...);

switch (err) {
    case RTNET_OK:
        break;
        
    case RTNET_ERR_NO_BUFFER:
        /* Retry after delay or drop packet */
        printf("TX buffer full, retrying...\n");
        HAL_Delay(10);
        break;
        
    case RTNET_ERR_NO_ROUTE:
        /* Trigger route discovery or alert operator */
        printf("No route to destination\n");
        break;
        
    default:
        printf("Unexpected error: %d\n", err);
        break;
}
```

---

### 5.2 QoS Priority Guidelines

```c
/* Real-time control (< 10ms latency required) */
RTNET_UDP_Send(..., RTNET_QOS_CRITICAL);

/* Time-sensitive telemetry (< 100ms acceptable) */
RTNET_UDP_Send(..., RTNET_QOS_HIGH);

/* Bulk file transfer */
RTNET_TCP_Send(...); /* Defaults to RTNET_QOS_NORMAL */

/* Background logging */
RTNET_UDP_Send(..., RTNET_QOS_LOW);
```

---

### 5.3 Periodic Task Scheduling

```c
/* RTOS-based scheduling (recommended) */
void NetworkTaskFunction(void *pvParameters)
{
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(100);
    
    for (;;) {
        RTNET_PeriodicTask();
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}

/* Create task with appropriate priority */
xTaskCreate(NetworkTaskFunction, "NetTask", 512, NULL, 
            tskIDLE_PRIORITY + 2, NULL);
```

```c
/* Bare-metal scheduling */
void SysTick_Handler(void)
{
    static uint32_t tick_count = 0;
    
    tick_count++;
    if (tick_count >= 100) { /* Every 100ms */
        RTNET_PeriodicTask();
        tick_count = 0;
    }
}
```

---

### 5.4 Statistics Monitoring

```c
void monitor_network_health(void)
{
    RTNET_Statistics_t stats;
    RTNET_GetStatistics(&stats);
    
    /* Calculate error rates */
    float rx_error_rate = (float)stats.rx_errors / stats.rx_packets;
    float tx_error_rate = (float)stats.tx_errors / stats.tx_packets;
    
    /* Alert if error rate exceeds threshold */
    if (rx_error_rate > 0.01f) { /* 1% */
        printf("WARNING: High RX error rate: %.2f%%\n", rx_error_rate * 100);
    }
    
    /* Log statistics periodically */
    printf("Network Stats: RX=%lu TX=%lu Errors=%lu\n",
           stats.rx_packets, stats.tx_packets, 
           stats.rx_errors + stats.tx_errors);
}
```

---

## 6. TROUBLESHOOTING

### 6.1 Common Issues

#### ❌ No Packets Received

**Symptoms:** `stats.rx_packets` remains 0

**Checklist:**
1. Verify Ethernet PHY link is up: `ETH->MACMIIAR & ETH_MACMIIAR_MB`
2. Check interrupt configuration: `NVIC->ISER[ETH_IRQn >> 5]`
3. Verify `RTNET_ProcessRxPacket` is called from ISR
4. Check IPv6 address configuration (must be link-local or globally routable)

```c
/* Debug: Check if interrupts are firing */
void ETH_IRQHandler(void)
{
    static volatile uint32_t irq_count = 0;
    irq_count++; /* Set breakpoint here */
    
    /* ... rest of handler ... */
}
```

---

#### ❌ Packets Sent But Not Received by Peer

**Symptoms:** `stats.tx_packets` increases, but peer doesn't receive

**Checklist:**
1. Verify routing table has entry for destination
2. Check neighbor cache has MAC address mapping
3. Use Wireshark to capture packets on wire
4. Verify checksums (disable HW checksum offload for debugging)

```c
/* Debug: Dump TX packets */
void RTNET_HardwareTransmit(const uint8_t* data, uint16_t length)
{
    printf("TX packet (%u bytes): ", length);
    for (uint16_t i = 0; i < (length < 64 ? length : 64); i++) {
        printf("%02X ", data[i]);
    }
    printf("\n");
    
    Ethernet_Transmit(data, length);
}
```

---

#### ❌ Stack Overflow / Hard Fault

**Symptoms:** Intermittent crashes in `RTNET_ProcessRxPacket`

**Causes:**
1. Insufficient stack size for network task
2. Interrupt nesting exceeds stack limit
3. Recursive calls (should never happen in this stack)

**Solutions:**
```c
/* Increase task stack size */
xTaskCreate(NetworkTask, "Net", 1024, NULL, /* was 512 */
            tskIDLE_PRIORITY + 2, NULL);

/* Reduce interrupt nesting */
NVIC_SetPriority(ETH_IRQn, 6); /* Lower priority */

/* Monitor stack usage */
UBaseType_t stackHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
printf("Stack free: %lu words\n", stackHighWaterMark);
```

---

### 6.2 Debugging Tools

#### Enable Stack Assertions

```c
/* In rtnet_stack.h */
#define RTNET_ENABLE_ASSERTIONS 1

#if RTNET_ENABLE_ASSERTIONS
#define RTNET_ASSERT(cond) \
    do { \
        if (!(cond)) { \
            printf("Assertion failed: %s:%d\n", __FILE__, __LINE__); \
            while(1); \
        } \
    } while(0)
#else
#define RTNET_ASSERT(cond) ((void)0)
#endif
```

---

#### Packet Capture to SD Card

```c
void log_packet_to_sd(const uint8_t* data, uint16_t length, bool is_rx)
{
    FIL file;
    f_open(&file, "packets.pcap", FA_WRITE | FA_OPEN_APPEND);
    
    /* Write PCAP record header */
    /* ... */
    
    /* Write packet data */
    UINT bytes_written;
    f_write(&file, data, length, &bytes_written);
    
    f_close(&file);
}

/* In RX path */
RTNET_Error_t RTNET_ProcessRxPacket(const uint8_t* data, uint16_t length)
{
    #ifdef RTNET_DEBUG_PCAP
    log_packet_to_sd(data, length, true);
    #endif
    
    /* ... rest of processing ... */
}
```

---

## 7. PERFORMANCE TUNING

### 7.1 Optimize for Latency

```c
/* Minimize interrupt latency */
void SystemInit(void)
{
    /* Set all interrupts to lowest priority */
    for (int i = 0; i < 8; i++) {
        NVIC->IP[i] = 0xFF;
    }
    
    /* Ethernet RX highest priority (except critical control) */
    NVIC_SetPriority(ETH_IRQn, 1);
}

/* Use smallest buffer sizes */
#define RTNET_MAX_RX_BUFFERS 4
#define RTNET_MAX_TX_BUFFERS 4

/* Disable non-critical features */
#define RTNET_ENABLE_MDNS 0
```

---

### 7.2 Optimize for Throughput

```c
/* Increase buffer counts */
#define RTNET_MAX_RX_BUFFERS 16
#define RTNET_MAX_TX_BUFFERS 16

/* Increase TCP window */
#define RTNET_TCP_WINDOW_SIZE 16384

/* Enable hardware checksum offload */
ETH->MACCR |= ETH_MACCR_IPCO; /* IPv4 checksum offload */

/* Use DMA burst mode */
ETH->DMABMR |= ETH_DMABMR_USP | (32 << 8); /* 32-word burst */
```

---

### 7.3 Optimize for Power Consumption

```c
/* Minimize wake-ups */
#define RTNET_MAX_RX_BUFFERS 2
#define RTNET_MAX_TX_BUFFERS 2

/* Increase periodic task interval */
void LowPowerNetworkTask(void)
{
    for (;;) {
        RTNET_PeriodicTask();
        vTaskDelay(pdMS_TO_TICKS(500)); /* Was 100ms */
    }
}

/* Use Ethernet Wake-on-LAN */
ETH->MACPMTCSR |= ETH_MACPMTCSR_WFE; /* Wake on magic packet */
```

---

## 8. MIGRATION FROM OTHER STACKS

### 8.1 From lwIP

| lwIP | RT Network Stack |
|------|------------------|
| `tcp_new()` | `RTNET_TCP_Connect()` |
| `tcp_write()` | `RTNET_TCP_Send()` |
| `tcp_close()` | `RTNET_TCP_Close()` |
| `udp_new()` | N/A (stateless) |
| `udp_send()` | `RTNET_UDP_Send()` |
| `ip6addr_t` | `RTNET_IPv6Addr_t` |

---

### 8.2 From Zephyr

| Zephyr | RT Network Stack |
|--------|------------------|
| `net_context_connect()` | `RTNET_TCP_Connect()` |
| `net_context_send()` | `RTNET_UDP_Send()` |
| `net_if_ipv6_addr_add()` | `RTNET_Initialize()` |
| `net_route_add()` | `RTNET_AddRoute()` |

---

## 9. PRODUCTION CHECKLIST

Before deploying to production:

- [ ] All platform hooks implemented correctly
- [ ] Interrupt priorities configured properly
- [ ] Stack sizes verified (use high-water-mark monitoring)
- [ ] Error handling implemented for all network calls
- [ ] Statistics monitoring in place
- [ ] Watchdog timer configured
- [ ] WCET verified on target hardware
- [ ] Full system tested with production traffic
- [ ] Backup/failover networking strategy defined
- [ ] Security review completed (if applicable)
- [ ] Documentation updated with deployment specifics

---

## 10. SUPPORT & RESOURCES

### Documentation
- API Reference: `docs/api_reference.md`
- Architecture Guide: `docs/architecture.md`
- Verification Report: `VERIFICATION_REPORT.md`

### Examples
- `/examples/udp_echo_server`
- `/examples/tcp_http_client`
- `/examples/mdns_discovery`

### Community
- GitHub Issues: [https://github.com/yourorg/rtnet-stack/issues](https://github.com/yourorg/rtnet-stack/issues)
- Email Support: rtnet-support@yourorg.com

---

**Document Version:** 1.0.0  
**Last Updated:** 2025-01-07  
**Maintainer:** RT Network Stack Team