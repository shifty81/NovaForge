#include "DeltaCompression.h"
#include <cmath>

namespace atlas::net {

DeltaCompression::DeltaCompression(uint32_t keyframeInterval)
    : m_keyframeInterval(keyframeInterval > 0 ? keyframeInterval : 1)
{}

// ── Quantisation ───────────────────────────────────────────────────

int32_t DeltaCompression::QuantizePosition(float v) {
    return static_cast<int32_t>(std::round(v * kPositionScale));
}

float DeltaCompression::DequantizePosition(int32_t v) {
    return static_cast<float>(v) / kPositionScale;
}

int32_t DeltaCompression::QuantizeRotation(float deg) {
    return static_cast<int32_t>(std::round(deg * kRotationScale));
}

float DeltaCompression::DequantizeRotation(int32_t v) {
    return static_cast<float>(v) / kRotationScale;
}

// ── Encode ─────────────────────────────────────────────────────────

CompressedFrame DeltaCompression::Encode(
    const std::vector<EntitySnapshot>& snapshots)
{
    CompressedFrame frame;
    if (!snapshots.empty()) {
        frame.tick = snapshots.front().tick;
    }

    for (const auto& snap : snapshots) {
        CompressedSnapshot cs;
        cs.entityId = snap.entityId;
        cs.tick     = snap.tick;

        if (needsKeyframe(snap.entityId, snap.tick)) {
            // Emit keyframe — full quantised state
            cs.frameType        = FrameType::Keyframe;
            cs.delta.dPosX      = QuantizePosition(snap.posX);
            cs.delta.dPosY      = QuantizePosition(snap.posY);
            cs.delta.dPosZ      = QuantizePosition(snap.posZ);
            cs.delta.dVelX      = QuantizePosition(snap.velX);
            cs.delta.dVelY      = QuantizePosition(snap.velY);
            cs.delta.dVelZ      = QuantizePosition(snap.velZ);
            cs.delta.dRotYaw    = QuantizeRotation(snap.rotYaw);

            // Update baseline
            Baseline& bl       = m_baselines[snap.entityId];
            bl.snapshot        = snap;
            bl.lastKeyframeTick = snap.tick;
            bl.forceKeyframe   = false;
        } else {
            // Emit delta from baseline
            cs.frameType = FrameType::Delta;
            const auto& base = m_baselines[snap.entityId].snapshot;
            cs.delta.dPosX   = QuantizePosition(snap.posX)   - QuantizePosition(base.posX);
            cs.delta.dPosY   = QuantizePosition(snap.posY)   - QuantizePosition(base.posY);
            cs.delta.dPosZ   = QuantizePosition(snap.posZ)   - QuantizePosition(base.posZ);
            cs.delta.dVelX   = QuantizePosition(snap.velX)   - QuantizePosition(base.velX);
            cs.delta.dVelY   = QuantizePosition(snap.velY)   - QuantizePosition(base.velY);
            cs.delta.dVelZ   = QuantizePosition(snap.velZ)   - QuantizePosition(base.velZ);
            cs.delta.dRotYaw = QuantizeRotation(snap.rotYaw) - QuantizeRotation(base.rotYaw);

            // Advance baseline to current
            m_baselines[snap.entityId].snapshot = snap;
        }

        frame.entries.push_back(cs);
    }

    m_forceAll = false;
    return frame;
}

// ── Decode ─────────────────────────────────────────────────────────

std::vector<EntitySnapshot> DeltaCompression::Decode(
    const CompressedFrame& frame)
{
    std::vector<EntitySnapshot> result;
    result.reserve(frame.entries.size());

    for (const auto& cs : frame.entries) {
        EntitySnapshot snap;
        snap.entityId = cs.entityId;
        snap.tick     = cs.tick;

        if (cs.frameType == FrameType::Keyframe) {
            snap.posX   = DequantizePosition(cs.delta.dPosX);
            snap.posY   = DequantizePosition(cs.delta.dPosY);
            snap.posZ   = DequantizePosition(cs.delta.dPosZ);
            snap.velX   = DequantizePosition(cs.delta.dVelX);
            snap.velY   = DequantizePosition(cs.delta.dVelY);
            snap.velZ   = DequantizePosition(cs.delta.dVelZ);
            snap.rotYaw = DequantizeRotation(cs.delta.dRotYaw);

            // Store as baseline
            Baseline& bl       = m_baselines[cs.entityId];
            bl.snapshot        = snap;
            bl.lastKeyframeTick = cs.tick;
        } else {
            // Delta — reconstruct from baseline
            auto it = m_baselines.find(cs.entityId);
            if (it == m_baselines.end()) {
                // No baseline — cannot decode delta, skip
                continue;
            }
            const auto& base = it->second.snapshot;
            snap.posX   = DequantizePosition(QuantizePosition(base.posX)   + cs.delta.dPosX);
            snap.posY   = DequantizePosition(QuantizePosition(base.posY)   + cs.delta.dPosY);
            snap.posZ   = DequantizePosition(QuantizePosition(base.posZ)   + cs.delta.dPosZ);
            snap.velX   = DequantizePosition(QuantizePosition(base.velX)   + cs.delta.dVelX);
            snap.velY   = DequantizePosition(QuantizePosition(base.velY)   + cs.delta.dVelY);
            snap.velZ   = DequantizePosition(QuantizePosition(base.velZ)   + cs.delta.dVelZ);
            snap.rotYaw = DequantizeRotation(QuantizeRotation(base.rotYaw) + cs.delta.dRotYaw);

            // Update baseline
            it->second.snapshot = snap;
        }

        result.push_back(snap);
    }

    return result;
}

// ── Control ────────────────────────────────────────────────────────

void DeltaCompression::ForceKeyframe(uint32_t entityId) {
    auto it = m_baselines.find(entityId);
    if (it != m_baselines.end()) {
        it->second.forceKeyframe = true;
    }
    // If no baseline yet, it will naturally be a keyframe on first encode.
}

void DeltaCompression::ForceAllKeyframes() {
    m_forceAll = true;
    for (auto& [id, bl] : m_baselines) {
        bl.forceKeyframe = true;
    }
}

void DeltaCompression::RemoveEntity(uint32_t entityId) {
    m_baselines.erase(entityId);
}

void DeltaCompression::Clear() {
    m_baselines.clear();
    m_forceAll = false;
}

// ── Internals ──────────────────────────────────────────────────────

bool DeltaCompression::needsKeyframe(uint32_t entityId,
                                      uint32_t currentTick) const
{
    if (m_forceAll) return true;

    auto it = m_baselines.find(entityId);
    if (it == m_baselines.end()) return true;            // no baseline yet

    const auto& bl = it->second;
    if (bl.forceKeyframe) return true;                   // explicit force

    // Periodic keyframe interval
    if (currentTick >= bl.lastKeyframeTick + m_keyframeInterval) return true;

    return false;
}

} // namespace atlas::net
