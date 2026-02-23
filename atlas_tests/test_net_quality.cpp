#include "../engine/net/NetworkQualityMonitor.h"
#include "../engine/net/NetContext.h"
#include <iostream>
#include <cassert>
#include <cmath>

using namespace atlas::net;

// ---- RTT tracking ----

void test_nqm_initial_rtt() {
    NetworkQualityMonitor mon;
    assert(mon.getSmoothedRTT() == 0.0f);
    assert(mon.getJitter() == 0.0f);

    mon.recordRTT(0.1f);
    // First sample is taken verbatim
    assert(std::abs(mon.getSmoothedRTT() - 0.1f) < 1e-6f);
    assert(mon.getJitter() == 0.0f);

    std::cout << "[PASS] test_nqm_initial_rtt" << std::endl;
}

void test_nqm_rtt_ewma() {
    NetworkQualityMonitor mon;
    // Seed with 100 ms
    mon.recordRTT(0.1f);

    // Second sample at 200 ms — smoothed should move toward 200 ms
    mon.recordRTT(0.2f);
    float expected = 0.1f + NetworkQualityMonitor::kRTTAlpha * (0.2f - 0.1f);
    assert(std::abs(mon.getSmoothedRTT() - expected) < 1e-5f);

    // Jitter should be non-zero now
    assert(mon.getJitter() > 0.0f);

    std::cout << "[PASS] test_nqm_rtt_ewma" << std::endl;
}

void test_nqm_stable_rtt_low_jitter() {
    NetworkQualityMonitor mon;
    // Feed many identical samples
    for (int i = 0; i < 100; ++i) mon.recordRTT(0.08f);

    assert(std::abs(mon.getSmoothedRTT() - 0.08f) < 1e-4f);
    // Jitter should converge to ~0
    assert(mon.getJitter() < 0.001f);

    std::cout << "[PASS] test_nqm_stable_rtt_low_jitter" << std::endl;
}

void test_nqm_negative_rtt_ignored() {
    NetworkQualityMonitor mon;
    mon.recordRTT(0.1f);
    mon.recordRTT(-0.05f);   // invalid
    assert(std::abs(mon.getSmoothedRTT() - 0.1f) < 1e-6f);

    std::cout << "[PASS] test_nqm_negative_rtt_ignored" << std::endl;
}

// ---- Packet loss tracking ----

void test_nqm_no_loss() {
    NetworkQualityMonitor mon;
    for (uint32_t i = 0; i < 20; ++i) mon.recordPacketArrival(i);
    assert(mon.getPacketLossRate() == 0.0f);

    std::cout << "[PASS] test_nqm_no_loss" << std::endl;
}

void test_nqm_50_percent_loss() {
    NetworkQualityMonitor mon;
    // Receive every other packet: 0, 2, 4, 6, 8
    for (uint32_t i = 0; i < 10; i += 2) mon.recordPacketArrival(i);

    float loss = mon.getPacketLossRate();
    // Window should contain received + gaps
    assert(loss > 0.4f && loss < 0.6f);

    std::cout << "[PASS] test_nqm_50_percent_loss" << std::endl;
}

void test_nqm_window_trims() {
    NetworkQualityMonitor mon;
    // Fill window with no-loss data beyond kLossWindowSize
    for (uint32_t i = 0; i < NetworkQualityMonitor::kLossWindowSize + 50; ++i)
        mon.recordPacketArrival(i);

    assert(mon.getPacketLossRate() == 0.0f);

    std::cout << "[PASS] test_nqm_window_trims" << std::endl;
}

void test_nqm_empty_loss_rate() {
    NetworkQualityMonitor mon;
    assert(mon.getPacketLossRate() == 0.0f);

    std::cout << "[PASS] test_nqm_empty_loss_rate" << std::endl;
}

// ---- Adaptive interpolation ----

void test_nqm_interp_floor() {
    NetworkQualityMonitor mon;
    // Very low RTT / no jitter → should hit the floor
    for (int i = 0; i < 50; ++i) mon.recordRTT(0.01f);

    float interp = mon.getAdaptiveInterpolationTime();
    assert(interp >= NetworkQualityMonitor::kMinInterpTime);
    assert(interp <= NetworkQualityMonitor::kMaxInterpTime);

    std::cout << "[PASS] test_nqm_interp_floor" << std::endl;
}

void test_nqm_interp_ceiling() {
    NetworkQualityMonitor mon;
    // Very high RTT + huge jitter → should hit the ceiling
    for (int i = 0; i < 50; ++i) {
        mon.recordRTT(static_cast<float>(i % 2 == 0 ? 0.5f : 0.01f));
    }

    float interp = mon.getAdaptiveInterpolationTime();
    assert(interp <= NetworkQualityMonitor::kMaxInterpTime);

    std::cout << "[PASS] test_nqm_interp_ceiling" << std::endl;
}

void test_nqm_interp_increases_with_jitter() {
    NetworkQualityMonitor mon;
    // Low-jitter baseline
    for (int i = 0; i < 50; ++i) mon.recordRTT(0.1f);
    float stableInterp = mon.getAdaptiveInterpolationTime();

    // Inject jitter
    NetworkQualityMonitor mon2;
    for (int i = 0; i < 50; ++i)
        mon2.recordRTT(i % 2 == 0 ? 0.05f : 0.15f);
    float jitteryInterp = mon2.getAdaptiveInterpolationTime();

    assert(jitteryInterp > stableInterp);

    std::cout << "[PASS] test_nqm_interp_increases_with_jitter" << std::endl;
}

void test_nqm_interp_increases_with_loss() {
    // No loss
    NetworkQualityMonitor m1;
    for (int i = 0; i < 50; ++i) m1.recordRTT(0.1f);
    for (uint32_t i = 0; i < 50; ++i) m1.recordPacketArrival(i);
    float noLoss = m1.getAdaptiveInterpolationTime();

    // 50% loss
    NetworkQualityMonitor m2;
    for (int i = 0; i < 50; ++i) m2.recordRTT(0.1f);
    for (uint32_t i = 0; i < 50; i += 2) m2.recordPacketArrival(i);
    float withLoss = m2.getAdaptiveInterpolationTime();

    assert(withLoss > noLoss);

    std::cout << "[PASS] test_nqm_interp_increases_with_loss" << std::endl;
}

// ---- Reset ----

void test_nqm_reset() {
    NetworkQualityMonitor mon;
    mon.recordRTT(0.15f);
    mon.recordPacketArrival(0);
    mon.recordPacketArrival(1);
    mon.reset();

    assert(mon.getSmoothedRTT() == 0.0f);
    assert(mon.getJitter() == 0.0f);
    assert(mon.getPacketLossRate() == 0.0f);
    assert(mon.getAdaptiveInterpolationTime() == NetworkQualityMonitor::kMinInterpTime);

    std::cout << "[PASS] test_nqm_reset" << std::endl;
}

// ---- NetContext sequence numbers ----

void test_net_send_assigns_sequence() {
    NetContext net;
    net.Init(NetMode::Server);
    uint32_t peer = net.AddPeer();

    Packet p1, p2;
    p1.type = 1;
    p2.type = 2;
    net.Send(peer, p1);
    net.Send(peer, p2);
    net.Poll();

    Packet r1, r2;
    assert(net.Receive(r1));
    assert(net.Receive(r2));
    assert(r1.sequence == 1);
    assert(r2.sequence == 2);

    std::cout << "[PASS] test_net_send_assigns_sequence" << std::endl;
}

void test_net_broadcast_assigns_sequence() {
    NetContext net;
    net.Init(NetMode::Server);
    net.AddPeer();

    Packet p;
    p.type = 10;
    net.Broadcast(p);
    net.Poll();

    Packet r;
    assert(net.Receive(r));
    assert(r.sequence == 1);

    std::cout << "[PASS] test_net_broadcast_assigns_sequence" << std::endl;
}

void test_net_sequence_resets_on_init() {
    NetContext net;
    net.Init(NetMode::Server);
    uint32_t peer = net.AddPeer();

    Packet p;
    p.type = 1;
    net.Send(peer, p);
    net.Send(peer, p);
    net.Poll();

    Packet r;
    net.Receive(r);
    net.Receive(r);

    // Re-init should reset sequence counter
    net.Init(NetMode::Server);
    uint32_t peer2 = net.AddPeer();
    net.Send(peer2, p);
    net.Poll();
    assert(net.Receive(r));
    assert(r.sequence == 1);

    std::cout << "[PASS] test_net_sequence_resets_on_init" << std::endl;
}
