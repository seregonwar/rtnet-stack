# RTNet Stack – Architecture Guide

## Overview
RTNet is a deterministic IPv6 stack for MCUs with bounded WCET. Layers are pared down for real-time control: IPv6 + ICMPv6 (NDP), UDP, and a simplified TCP-Lite. Memory is static, no heap.

## Core Components
- **Context (`RTNET_Context_t`)**: single instance holding buffers, TCP control blocks, routes, neighbor cache, mDNS cache, statistics, and local addressing.
- **Buffers**: fixed pools `RTNET_MAX_RX_BUFFERS` and `RTNET_MAX_TX_BUFFERS`, sized to `RTNET_BUFFER_SIZE` (MTU + headroom). Zero-copy offsets keep processing deterministic.
- **Routing**: longest-prefix match over `RTNET_MAX_ROUTING_ENTRIES` with metric tie-break. Link-local route is auto-added at init.
- **Neighbor Discovery**: small cache (`RTNET_MAX_NEIGHBOR_CACHE`) keyed by IPv6/MAC, refreshed via timestamps.
- **TCP-Lite**: minimal states, bounded retries (`RTNET_TCP_MAX_RETRIES`), timeout `RTNET_TCP_TIMEOUT_MS`.
- **mDNS**: cached service records (`RTNET_MAX_MDNS_CACHE`) with TTL management.
- **Platform hooks**: critical section, millisecond timer, and hardware TX provided by BSP.

## Data Flow
1. **RX path** (`RTNET_ProcessRxPacket`): validate Ethernet + IPv6 header, update stats, dispatch by Next Header (ICMPv6/UDP/TCP). Checksums validated; routing errors increment counters.
2. **TX path** (`RTNET_UDP_Send`/`RTNET_TCP_Send`): choose route, allocate TX buffer, build headers, call `RTNET_HardwareTransmit`. QoS selects preferred buffer first.
3. **Periodic task**: ages neighbor and routing entries, times out TCP connections, maintains mDNS TTLs.

## Timing & Determinism
- Bounded loops over fixed-size tables.
- No dynamic allocation or recursion.
- WCET targets: RX < 450 µs, TX < 320 µs, checksum < 80 µs for 1500B (Cortex-M4 168 MHz).

## Build Targets
- **Firmware**: compile with BSP-provided hooks.
- **Host tests**: enable `RTNS_USE_PLATFORM_STUBS` to use no-op hardware and timing stubs plus public API shims.

## Extending
- Increase table sizes cautiously; verify timing.
- Add protocols by keeping header parsing branchless and bounded.
- When adding stats, guard updates with critical sections if ISR concurrency exists.
