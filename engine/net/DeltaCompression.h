#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
#include "NetworkInterpolationBuffer.h"

namespace atlas::net {

/**
 * Frame type for delta-compressed snapshots.
 *   Keyframe  — full snapshot (baseline), sent periodically
 *   Delta     — difference from the last acknowledged baseline
 */
enum class FrameType : uint8_t {
    Keyframe = 0,
    Delta    = 1
};

/**
 * A single field delta for a snapshot.
 * Uses fixed-point quantisation to reduce floating-point bandwidth.
 */
struct FieldDelta {
    int32_t dPosX = 0;
    int32_t dPosY = 0;
    int32_t dPosZ = 0;
    int32_t dVelX = 0;
    int32_t dVelY = 0;
    int32_t dVelZ = 0;
    int32_t dRotYaw = 0;
};

/**
 * Compressed representation of an entity's state in a single frame.
 */
struct CompressedSnapshot {
    uint32_t  entityId  = 0;
    uint32_t  tick      = 0;
    FrameType frameType = FrameType::Keyframe;
    FieldDelta delta;
};

/**
 * A compressed frame containing updates for many entities.
 */
struct CompressedFrame {
    uint32_t tick = 0;
    std::vector<CompressedSnapshot> entries;
};

/**
 * @brief Server-side delta compression for entity state snapshots.
 *
 * Converts full EntitySnapshot data into compact delta-encoded frames.
 * The encoder maintains a per-entity baseline; each new snapshot is
 * compared against the baseline and either:
 *   - emitted as a Keyframe (full state, resets the baseline), or
 *   - emitted as a Delta (difference from baseline).
 *
 * Keyframes are forced every `keyframeInterval` ticks or when no
 * baseline exists for the entity.
 *
 * Quantisation:
 *   Positions are encoded with 0.01 unit precision (×100 fixed-point).
 *   Velocities use the same scale.
 *   Rotation uses 0.1 degree precision (×10 fixed-point).
 *
 * Workflow:
 *   1. Each server tick, call Encode() with the full snapshots.
 *   2. Serialise the returned CompressedFrame into a Packet payload.
 *   3. On the client, call Decode() to reconstruct full snapshots.
 */
class DeltaCompression {
public:
    explicit DeltaCompression(uint32_t keyframeInterval = 30);

    /** Encode a set of full snapshots into a compressed frame. */
    CompressedFrame Encode(const std::vector<EntitySnapshot>& snapshots);

    /**
     * Decode a compressed frame back into full snapshots.
     * Updates internal baseline state on the decode side.
     */
    std::vector<EntitySnapshot> Decode(const CompressedFrame& frame);

    /** Force next encode for a specific entity to be a keyframe. */
    void ForceKeyframe(uint32_t entityId);

    /** Force all entities to keyframe on the next encode. */
    void ForceAllKeyframes();

    /** Remove baseline state for an entity (e.g. when it is destroyed). */
    void RemoveEntity(uint32_t entityId);

    /** Clear all baseline state. */
    void Clear();

    /** Number of entities with active baselines. */
    size_t BaselineCount() const { return m_baselines.size(); }

    /** Get the keyframe interval. */
    uint32_t KeyframeInterval() const { return m_keyframeInterval; }

    // Quantisation helpers (public for testability)
    static int32_t QuantizePosition(float v);
    static float   DequantizePosition(int32_t v);
    static int32_t QuantizeRotation(float deg);
    static float   DequantizeRotation(int32_t v);

    static constexpr float kPositionScale = 100.0f;   // 0.01 unit precision
    static constexpr float kRotationScale = 10.0f;    // 0.1 degree precision

private:
    struct Baseline {
        EntitySnapshot snapshot;
        uint32_t lastKeyframeTick = 0;
        bool     forceKeyframe    = false;
    };

    bool needsKeyframe(uint32_t entityId, uint32_t currentTick) const;

    std::unordered_map<uint32_t, Baseline> m_baselines;
    uint32_t m_keyframeInterval;
    bool m_forceAll = false;
};

} // namespace atlas::net
