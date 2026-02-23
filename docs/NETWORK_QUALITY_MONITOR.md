# Network Quality Monitoring & Adaptive Interpolation

**Date**: February 2026  
**Status**: Complete  

---

## Overview

The **NetworkQualityMonitor** tracks real-time network conditions and recommends an adaptive interpolation buffer so the client can render smooth entity movement regardless of latency variance or packet loss.

## Components

### `NetworkQualityMonitor` (`engine/net/NetworkQualityMonitor.h`)

A lightweight, stateless-friendly class that tracks three metrics:

| Metric | Method | Algorithm |
|--------|--------|-----------|
| **Smoothed RTT** | `recordRTT()` → `getSmoothedRTT()` | EWMA (α = 0.125, per RFC 6298) |
| **Jitter** | (computed alongside RTT) → `getJitter()` | Mean deviation EWMA (α = 0.0625, per RFC 3550) |
| **Packet loss rate** | `recordPacketArrival(seq)` → `getPacketLossRate()` | Sliding window (100 seq) with gap detection |

The monitor combines all three into a single recommendation:

```
adaptiveInterpTime = clamp(RTT/2 + jitter×2 + lossBonus, 50ms, 300ms)
```

### Packet Sequence Numbers (`engine/net/NetContext.h`)

Every `Packet` now carries a monotonic `sequence` field. `NetContext::Send()` and `Broadcast()` auto-stamp it. Receivers feed the sequence into `recordPacketArrival()` for loss detection.

### Entity Staleness Tracking (`cpp_client/include/core/entity.h`)

`Entity` now exposes:
- `getUpdateCount()` — number of server state updates received
- `getTimeSinceLastUpdate()` — seconds elapsed since last server snapshot

These help the client detect stale entities (e.g., server stopped sending updates).

## Usage

### Client-side integration (pseudocode)

```cpp
NetworkQualityMonitor m_netQuality;

// When a PONG arrives:
float rtt = now() - pingSentTime;
m_netQuality.recordRTT(rtt);

// When any state packet arrives:
m_netQuality.recordPacketArrival(packet.sequence);

// Every frame, interpolate with adaptive window:
float interpTime = m_netQuality.getAdaptiveInterpolationTime();
for (auto& entity : entities) {
    entity.interpolate(deltaTime, interpTime);
}
```

## Tuning Constants

| Constant | Default | Purpose |
|----------|---------|---------|
| `kRTTAlpha` | 0.125 | EWMA weight for RTT |
| `kJitterAlpha` | 0.0625 | EWMA weight for jitter |
| `kMinInterpTime` | 50 ms | Floor for adaptive buffer |
| `kMaxInterpTime` | 300 ms | Ceiling for adaptive buffer |
| `kJitterMultiplier` | 2.0 | Safety margin on jitter |
| `kLossWindowSize` | 100 | Sequence window for loss calc |

## Tests

16 new test assertions in `atlas_tests/test_net_quality.cpp`:

- RTT: initial value, EWMA convergence, stable-low-jitter, negative ignored
- Packet loss: no loss, 50% loss, window trimming, empty state
- Adaptive interpolation: floor, ceiling, jitter sensitivity, loss sensitivity
- Reset clears all state
- NetContext sequence numbering: Send, Broadcast, reset on Init
