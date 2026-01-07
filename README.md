# RT Network Stack

Real-time IPv6/UDP/TCP-lite stack with deterministic timing and static memory, now runnable on desktop via host stubs.

## Why RTNS
- Deterministic paths (bounded loops, no heap)
- IPv6 + ICMPv6 (NDP), UDP, simplified TCP-lite, mDNS
- Static buffers and QoS-aware TX selection
- Host stubs for local testing (no hardware required)

## Quick Start (Host)
```ps1
cmake -S . -B build -DRTNS_USE_PLATFORM_STUBS=ON -DRTNS_BUILD_EXAMPLES=ON
cmake --build build --config Debug
.\build\Debug\rtns_desktop_demo.exe
```

## Examples
- `examples/desktop_demo` – UDP/TCP/mDNS demo with stats printout
- `examples/udp_echo_server` – echo skeleton
- `examples/tcp_http_client` – simple TCP GET flow
- `examples/mdns_discovery` – service query demo

## Documentation
- Integration guide: `docs/README_integration.md` (dettagli tecnico/embedded)
- API: `docs/api_reference.md`
- Architettura: `docs/architecture.md`
- Verifica: `docs/VERIFICATION_REPORT.md`

## Build & Test
```ps1
cmake -S . -B build -DRTNS_USE_PLATFORM_STUBS=ON
cmake --build build --config Debug
.\build\Debug\rtns_tests.exe
```

## Issue Tracker
[GitHub Issues](https://github.com/seregonwar/rtnet-stack/issues)