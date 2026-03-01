/**
 * Tests for DeltaCompression — server-side delta encoding of entity
 * snapshots for bandwidth optimisation.
 */

#include <cassert>
#include <cmath>
#include <vector>
#include "../engine/net/DeltaCompression.h"

using namespace atlas::net;

static bool approxEq(float a, float b, float eps = 0.02f) {
    return std::fabs(a - b) < eps;
}

// ══════════════════════════════════════════════════════════════════
// Construction & configuration
// ══════════════════════════════════════════════════════════════════

void test_dc_defaults() {
    DeltaCompression dc;
    assert(dc.KeyframeInterval() == 30);
    assert(dc.BaselineCount() == 0);
}

void test_dc_custom_interval() {
    DeltaCompression dc(10);
    assert(dc.KeyframeInterval() == 10);
}

void test_dc_zero_interval_clamped() {
    DeltaCompression dc(0);
    assert(dc.KeyframeInterval() == 1);
}

// ══════════════════════════════════════════════════════════════════
// Quantisation
// ══════════════════════════════════════════════════════════════════

void test_dc_quantize_position() {
    int32_t q = DeltaCompression::QuantizePosition(1.23f);
    float   d = DeltaCompression::DequantizePosition(q);
    assert(approxEq(d, 1.23f, 0.011f));
}

void test_dc_quantize_negative() {
    int32_t q = DeltaCompression::QuantizePosition(-5.67f);
    float   d = DeltaCompression::DequantizePosition(q);
    assert(approxEq(d, -5.67f, 0.011f));
}

void test_dc_quantize_rotation() {
    int32_t q = DeltaCompression::QuantizeRotation(45.3f);
    float   d = DeltaCompression::DequantizeRotation(q);
    assert(approxEq(d, 45.3f, 0.11f));
}

void test_dc_quantize_zero() {
    assert(DeltaCompression::QuantizePosition(0.0f) == 0);
    assert(DeltaCompression::QuantizeRotation(0.0f) == 0);
}

// ══════════════════════════════════════════════════════════════════
// Keyframe encode / decode roundtrip
// ══════════════════════════════════════════════════════════════════

void test_dc_keyframe_roundtrip() {
    DeltaCompression encoder;
    DeltaCompression decoder;

    EntitySnapshot snap{1, 100, 10.0f, 20.0f, 30.0f, 1.0f, 2.0f, 3.0f, 90.0f};
    auto frame = encoder.Encode({snap});

    assert(frame.entries.size() == 1);
    assert(frame.entries[0].frameType == FrameType::Keyframe);
    assert(frame.entries[0].entityId == 1);

    auto decoded = decoder.Decode(frame);
    assert(decoded.size() == 1);
    assert(approxEq(decoded[0].posX, 10.0f));
    assert(approxEq(decoded[0].posY, 20.0f));
    assert(approxEq(decoded[0].posZ, 30.0f));
    assert(approxEq(decoded[0].velX, 1.0f));
    assert(approxEq(decoded[0].velY, 2.0f));
    assert(approxEq(decoded[0].velZ, 3.0f));
    assert(approxEq(decoded[0].rotYaw, 90.0f, 0.11f));
}

void test_dc_first_encode_is_keyframe() {
    DeltaCompression dc;
    EntitySnapshot snap{1, 0, 5.0f, 0, 0, 0, 0, 0, 0};
    auto frame = dc.Encode({snap});
    assert(frame.entries[0].frameType == FrameType::Keyframe);
    assert(dc.BaselineCount() == 1);
}

// ══════════════════════════════════════════════════════════════════
// Delta encode / decode
// ══════════════════════════════════════════════════════════════════

void test_dc_delta_after_keyframe() {
    DeltaCompression encoder(100); // high interval so no forced keyframe
    DeltaCompression decoder;

    EntitySnapshot s1{1, 0, 10.0f, 0, 0, 0, 0, 0, 0};
    auto f1 = encoder.Encode({s1});
    decoder.Decode(f1);

    EntitySnapshot s2{1, 1, 11.0f, 0, 0, 0, 0, 0, 0};
    auto f2 = encoder.Encode({s2});
    assert(f2.entries[0].frameType == FrameType::Delta);

    auto decoded = decoder.Decode(f2);
    assert(decoded.size() == 1);
    assert(approxEq(decoded[0].posX, 11.0f));
}

void test_dc_delta_values_small() {
    DeltaCompression encoder(100);

    EntitySnapshot s1{1, 0, 100.0f, 200.0f, 300.0f, 0, 0, 0, 0};
    encoder.Encode({s1});

    EntitySnapshot s2{1, 1, 100.05f, 200.05f, 300.05f, 0, 0, 0, 0};
    auto frame = encoder.Encode({s2});

    // Delta values should be small (5 quantised units)
    const auto& d = frame.entries[0].delta;
    assert(std::abs(d.dPosX) <= 10);
    assert(std::abs(d.dPosY) <= 10);
    assert(std::abs(d.dPosZ) <= 10);
}

void test_dc_delta_stationary_entity() {
    DeltaCompression encoder(100);

    EntitySnapshot s{1, 0, 50.0f, 50.0f, 50.0f, 0, 0, 0, 45.0f};
    encoder.Encode({s});

    // Same position, next tick
    s.tick = 1;
    auto frame = encoder.Encode({s});
    assert(frame.entries[0].frameType == FrameType::Delta);
    assert(frame.entries[0].delta.dPosX == 0);
    assert(frame.entries[0].delta.dPosY == 0);
    assert(frame.entries[0].delta.dPosZ == 0);
    assert(frame.entries[0].delta.dRotYaw == 0);
}

// ══════════════════════════════════════════════════════════════════
// Multiple entities
// ══════════════════════════════════════════════════════════════════

void test_dc_multiple_entities() {
    DeltaCompression encoder;
    DeltaCompression decoder;

    std::vector<EntitySnapshot> snaps = {
        {1, 0, 10, 0, 0, 0, 0, 0, 0},
        {2, 0, 20, 0, 0, 0, 0, 0, 0},
        {3, 0, 30, 0, 0, 0, 0, 0, 0},
    };

    auto frame = encoder.Encode(snaps);
    assert(frame.entries.size() == 3);
    assert(encoder.BaselineCount() == 3);

    auto decoded = decoder.Decode(frame);
    assert(decoded.size() == 3);
    assert(approxEq(decoded[0].posX, 10.0f));
    assert(approxEq(decoded[1].posX, 20.0f));
    assert(approxEq(decoded[2].posX, 30.0f));
}

// ══════════════════════════════════════════════════════════════════
// Keyframe interval forcing
// ══════════════════════════════════════════════════════════════════

void test_dc_periodic_keyframe() {
    DeltaCompression encoder(5); // keyframe every 5 ticks

    EntitySnapshot s{1, 0, 0, 0, 0, 0, 0, 0, 0};
    encoder.Encode({s}); // tick 0 → keyframe

    s.tick = 1; auto f1 = encoder.Encode({s}); // delta
    s.tick = 2; auto f2 = encoder.Encode({s}); // delta
    s.tick = 3; auto f3 = encoder.Encode({s}); // delta
    s.tick = 4; auto f4 = encoder.Encode({s}); // delta
    s.tick = 5; auto f5 = encoder.Encode({s}); // keyframe (interval=5)

    assert(f1.entries[0].frameType == FrameType::Delta);
    assert(f4.entries[0].frameType == FrameType::Delta);
    assert(f5.entries[0].frameType == FrameType::Keyframe);
}

void test_dc_force_keyframe() {
    DeltaCompression encoder(100);

    EntitySnapshot s{1, 0, 10, 0, 0, 0, 0, 0, 0};
    encoder.Encode({s});

    encoder.ForceKeyframe(1);

    s.tick = 1;
    auto frame = encoder.Encode({s});
    assert(frame.entries[0].frameType == FrameType::Keyframe);
}

void test_dc_force_all_keyframes() {
    DeltaCompression encoder(100);

    std::vector<EntitySnapshot> snaps = {
        {1, 0, 10, 0, 0, 0, 0, 0, 0},
        {2, 0, 20, 0, 0, 0, 0, 0, 0},
    };
    encoder.Encode(snaps);

    encoder.ForceAllKeyframes();

    snaps[0].tick = 1;
    snaps[1].tick = 1;
    auto frame = encoder.Encode(snaps);
    assert(frame.entries[0].frameType == FrameType::Keyframe);
    assert(frame.entries[1].frameType == FrameType::Keyframe);
}

// ══════════════════════════════════════════════════════════════════
// Entity removal & clear
// ══════════════════════════════════════════════════════════════════

void test_dc_remove_entity() {
    DeltaCompression dc;
    dc.Encode({{1, 0, 10, 0, 0, 0, 0, 0, 0}});
    assert(dc.BaselineCount() == 1);

    dc.RemoveEntity(1);
    assert(dc.BaselineCount() == 0);
}

void test_dc_clear() {
    DeltaCompression dc;
    dc.Encode({{1, 0, 10, 0, 0, 0, 0, 0, 0}, {2, 0, 20, 0, 0, 0, 0, 0, 0}});
    assert(dc.BaselineCount() == 2);

    dc.Clear();
    assert(dc.BaselineCount() == 0);
}

// ══════════════════════════════════════════════════════════════════
// Edge cases
// ══════════════════════════════════════════════════════════════════

void test_dc_decode_delta_without_baseline() {
    DeltaCompression decoder;

    // Craft a delta frame without having set a baseline
    CompressedFrame frame;
    frame.tick = 1;
    CompressedSnapshot cs;
    cs.entityId  = 99;
    cs.tick      = 1;
    cs.frameType = FrameType::Delta;
    frame.entries.push_back(cs);

    auto decoded = decoder.Decode(frame);
    // Delta without baseline → skipped
    assert(decoded.empty());
}

void test_dc_empty_encode() {
    DeltaCompression dc;
    auto frame = dc.Encode({});
    assert(frame.entries.empty());
}

void test_dc_encode_decode_full_sequence() {
    DeltaCompression encoder(100);
    DeltaCompression decoder;

    // Simulate 10 ticks of a moving entity
    for (uint32_t t = 0; t < 10; ++t) {
        EntitySnapshot s;
        s.entityId = 1;
        s.tick     = t;
        s.posX     = static_cast<float>(t) * 1.5f;
        s.posY     = static_cast<float>(t) * 0.5f;
        s.posZ     = 0;
        s.velX     = 1.5f;
        s.velY     = 0.5f;
        s.rotYaw   = static_cast<float>(t) * 10.0f;

        auto frame   = encoder.Encode({s});
        auto decoded = decoder.Decode(frame);

        assert(decoded.size() == 1);
        assert(approxEq(decoded[0].posX, s.posX));
        assert(approxEq(decoded[0].posY, s.posY));
        assert(approxEq(decoded[0].rotYaw, s.rotYaw, 0.11f));
    }
}
