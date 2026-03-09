// Tests for: FleetPositionalAudioSystem Tests
#include "test_log.h"
#include "components/fleet_components.h"
#include "systems/fleet_positional_audio_system.h"

using namespace atlas;
using namespace atlas::systems;

// ==================== FleetPositionalAudioSystem Tests ====================

static void testFPAAudioPosition() {
    std::cout << "\n=== FPA Audio Position ===" << std::endl;

    float out_x = 0.0f, out_y = 0.0f, out_z = 0.0f;

    FleetPositionalAudioSystem::computeAudioPosition(
        100.0f, 200.0f, 300.0f,
        10.0f, 20.0f, 30.0f,
        out_x, out_y, out_z);
    assertTrue(approxEqual(out_x, 110.0f), "Position X = cmd_x + off_x");
    assertTrue(approxEqual(out_y, 220.0f), "Position Y = cmd_y + off_y");
    assertTrue(approxEqual(out_z, 330.0f), "Position Z = cmd_z + off_z");

    // Negative offsets
    FleetPositionalAudioSystem::computeAudioPosition(
        50.0f, 50.0f, 50.0f,
        -25.0f, -10.0f, -50.0f,
        out_x, out_y, out_z);
    assertTrue(approxEqual(out_x, 25.0f), "Negative offset X works");
    assertTrue(approxEqual(out_y, 40.0f), "Negative offset Y works");
    assertTrue(approxEqual(out_z, 0.0f), "Negative offset Z works");

    // Zero commander position
    FleetPositionalAudioSystem::computeAudioPosition(
        0.0f, 0.0f, 0.0f,
        5.0f, 10.0f, 15.0f,
        out_x, out_y, out_z);
    assertTrue(approxEqual(out_x, 5.0f), "Zero commander X");
    assertTrue(approxEqual(out_y, 10.0f), "Zero commander Y");
    assertTrue(approxEqual(out_z, 15.0f), "Zero commander Z");
}

static void testFPAAttenuationBasic() {
    std::cout << "\n=== FPA Attenuation Basic ===" << std::endl;

    // At min range → full volume
    float atten = FleetPositionalAudioSystem::computeAttenuation(5.0f, 5.0f, 200.0f);
    assertTrue(approxEqual(atten, 1.0f), "At min range, attenuation is 1.0");

    // At max range → silent
    atten = FleetPositionalAudioSystem::computeAttenuation(200.0f, 5.0f, 200.0f);
    assertTrue(approxEqual(atten, 0.0f), "At max range, attenuation is 0.0");

    // At midpoint
    float mid = (5.0f + 200.0f) / 2.0f;  // 102.5
    atten = FleetPositionalAudioSystem::computeAttenuation(mid, 5.0f, 200.0f);
    float expected = 1.0f - (mid - 5.0f) / (200.0f - 5.0f);
    assertTrue(approxEqual(atten, expected), "At midpoint, attenuation follows linear formula");

    // Below min range → full volume
    atten = FleetPositionalAudioSystem::computeAttenuation(0.0f, 5.0f, 200.0f);
    assertTrue(approxEqual(atten, 1.0f), "Below min range, attenuation is 1.0");

    // Beyond max range → silent
    atten = FleetPositionalAudioSystem::computeAttenuation(500.0f, 5.0f, 200.0f);
    assertTrue(approxEqual(atten, 0.0f), "Beyond max range, attenuation is 0.0");

    // Quarter distance
    float quarter = 5.0f + (200.0f - 5.0f) * 0.25f;  // 53.75
    atten = FleetPositionalAudioSystem::computeAttenuation(quarter, 5.0f, 200.0f);
    assertTrue(approxEqual(atten, 0.75f), "At quarter distance, attenuation is 0.75");
}

static void testFPAAttenuationEdgeCases() {
    std::cout << "\n=== FPA Attenuation Edge Cases ===" << std::endl;

    // Negative min_range treated as 0
    float atten = FleetPositionalAudioSystem::computeAttenuation(0.0f, -10.0f, 100.0f);
    assertTrue(approxEqual(atten, 1.0f), "Negative min_range: distance 0 gives 1.0");

    atten = FleetPositionalAudioSystem::computeAttenuation(50.0f, -10.0f, 100.0f);
    float expected = 1.0f - (50.0f - 0.0f) / (100.0f - 0.0f);
    assertTrue(approxEqual(atten, expected), "Negative min_range: treated as 0 in formula");

    // max_range <= min_range → return 0
    atten = FleetPositionalAudioSystem::computeAttenuation(5.0f, 100.0f, 100.0f);
    assertTrue(approxEqual(atten, 0.0f), "max == min returns 0.0");

    atten = FleetPositionalAudioSystem::computeAttenuation(5.0f, 200.0f, 100.0f);
    assertTrue(approxEqual(atten, 0.0f), "max < min returns 0.0");

    // Zero range (both 0)
    atten = FleetPositionalAudioSystem::computeAttenuation(0.0f, 0.0f, 0.0f);
    assertTrue(approxEqual(atten, 0.0f), "Both ranges 0 returns 0.0");

    // Exact boundary at min
    atten = FleetPositionalAudioSystem::computeAttenuation(10.0f, 10.0f, 50.0f);
    assertTrue(approxEqual(atten, 1.0f), "Exact min boundary returns 1.0");

    // Exact boundary at max
    atten = FleetPositionalAudioSystem::computeAttenuation(50.0f, 10.0f, 50.0f);
    assertTrue(approxEqual(atten, 0.0f), "Exact max boundary returns 0.0");
}

static void testFPAWarpReverbOff() {
    std::cout << "\n=== FPA Warp Reverb Off ===" << std::endl;

    float wet_mix = 999.0f, decay = 999.0f;

    FleetPositionalAudioSystem::computeWarpReverb(false, 0.0f, wet_mix, decay);
    assertTrue(approxEqual(wet_mix, 0.0f), "Not in warp: wet_mix is 0");
    assertTrue(approxEqual(decay, 0.0f), "Not in warp: decay is 0");

    // Even with high speed, not in warp → zero
    FleetPositionalAudioSystem::computeWarpReverb(false, 100.0f, wet_mix, decay);
    assertTrue(approxEqual(wet_mix, 0.0f), "Not in warp with speed: wet_mix is 0");
    assertTrue(approxEqual(decay, 0.0f), "Not in warp with speed: decay is 0");
}

static void testFPAWarpReverbOn() {
    std::cout << "\n=== FPA Warp Reverb On ===" << std::endl;

    float wet_mix = 0.0f, decay = 0.0f;

    // Speed 0: speed_factor=0 → wet=0.3, decay=1.5
    FleetPositionalAudioSystem::computeWarpReverb(true, 0.0f, wet_mix, decay);
    assertTrue(approxEqual(wet_mix, 0.3f), "Warp speed 0: wet_mix is 0.3");
    assertTrue(approxEqual(decay, 1.5f), "Warp speed 0: decay is 1.5");

    // Speed 5: speed_factor=0.5 → wet=0.3+0.15=0.45, decay=1.5+0.5=2.0
    FleetPositionalAudioSystem::computeWarpReverb(true, 5.0f, wet_mix, decay);
    assertTrue(approxEqual(wet_mix, 0.45f), "Warp speed 5: wet_mix is 0.45");
    assertTrue(approxEqual(decay, 2.0f), "Warp speed 5: decay is 2.0");

    // Speed 10: speed_factor=1.0 → wet=0.3+0.3=0.6, decay=1.5+1.0=2.5
    FleetPositionalAudioSystem::computeWarpReverb(true, 10.0f, wet_mix, decay);
    assertTrue(approxEqual(wet_mix, 0.6f), "Warp speed 10: wet_mix is 0.6");
    assertTrue(approxEqual(decay, 2.5f), "Warp speed 10: decay is 2.5");

    // Speed 7.5: speed_factor=0.75 → wet=0.3+0.225=0.525, decay=1.5+0.75=2.25
    FleetPositionalAudioSystem::computeWarpReverb(true, 7.5f, wet_mix, decay);
    assertTrue(approxEqual(wet_mix, 0.525f), "Warp speed 7.5: wet_mix is 0.525");
    assertTrue(approxEqual(decay, 2.25f), "Warp speed 7.5: decay is 2.25");
}

static void testFPAWarpReverbClamp() {
    std::cout << "\n=== FPA Warp Reverb Clamp ===" << std::endl;

    float wet_mix = 0.0f, decay = 0.0f;

    // Very high speed: speed_factor clamped to 1.0
    FleetPositionalAudioSystem::computeWarpReverb(true, 100.0f, wet_mix, decay);
    assertTrue(approxEqual(wet_mix, 0.6f), "Extreme speed: wet_mix clamped at 0.6");
    assertTrue(approxEqual(decay, 2.5f), "Extreme speed: decay clamped at 2.5");

    // Negative speed: speed_factor clamped to 0.0
    FleetPositionalAudioSystem::computeWarpReverb(true, -5.0f, wet_mix, decay);
    assertTrue(approxEqual(wet_mix, 0.3f), "Negative speed: wet_mix is base 0.3");
    assertTrue(approxEqual(decay, 1.5f), "Negative speed: decay is base 1.5");

    // Speed 20 → still factor=1.0 clamped
    FleetPositionalAudioSystem::computeWarpReverb(true, 20.0f, wet_mix, decay);
    assertTrue(approxEqual(wet_mix, 0.6f), "Speed 20: wet_mix clamped at 0.6");
    assertTrue(approxEqual(decay, 2.5f), "Speed 20: decay clamped at 2.5");

    // wet_mix should always be in [0,1]
    assertTrue(wet_mix >= 0.0f && wet_mix <= 1.0f, "wet_mix within [0,1] range");
    // decay should always be >= 0
    assertTrue(decay >= 0.0f, "decay is non-negative");
}

void run_fleet_positional_audio_system_tests() {
    testFPAAudioPosition();
    testFPAAttenuationBasic();
    testFPAAttenuationEdgeCases();
    testFPAWarpReverbOff();
    testFPAWarpReverbOn();
    testFPAWarpReverbClamp();
}
