#include "LagCompensation.h"
#include <algorithm>

namespace atlas::net {

LagCompensation::LagCompensation(size_t historyDuration,
                                  uint32_t maxRewindTicks)
    : m_historyDuration(historyDuration > 0 ? historyDuration : 1)
    , m_maxRewindTicks(maxRewindTicks)
{}

// ── Store ──────────────────────────────────────────────────────────

void LagCompensation::StoreSnapshot(const EntitySnapshot& snap) {
    if (snap.tick > m_currentTick) {
        m_currentTick = snap.tick;
    }

    auto& eh = m_history[snap.entityId];
    eh.entityId = snap.entityId;

    // Insert in tick order (usually appended at the end)
    if (eh.snapshots.empty() || snap.tick >= eh.snapshots.back().tick) {
        eh.snapshots.push_back(snap);
    } else {
        // Out-of-order — find correct insertion point
        auto it = std::lower_bound(
            eh.snapshots.begin(), eh.snapshots.end(), snap.tick,
            [](const EntitySnapshot& s, uint32_t t) { return s.tick < t; });
        eh.snapshots.insert(it, snap);
    }

    prune(eh);
}

// ── Query ──────────────────────────────────────────────────────────

InterpolatedState LagCompensation::GetStateAtTick(uint32_t entityId,
                                                   float tick) const
{
    InterpolatedState result;
    result.entityId = entityId;

    const EntityHistory* eh = findEntity(entityId);
    if (!eh || eh->snapshots.empty()) return result;

    const auto& snaps = eh->snapshots;

    // Before earliest snapshot
    if (tick <= static_cast<float>(snaps.front().tick)) {
        const auto& s = snaps.front();
        result.posX   = s.posX;
        result.posY   = s.posY;
        result.posZ   = s.posZ;
        result.rotYaw = s.rotYaw;
        result.valid  = (tick == static_cast<float>(snaps.front().tick));
        return result;
    }

    // After latest snapshot
    if (tick >= static_cast<float>(snaps.back().tick)) {
        const auto& s = snaps.back();
        result.posX   = s.posX;
        result.posY   = s.posY;
        result.posZ   = s.posZ;
        result.rotYaw = s.rotYaw;
        result.valid  = true;
        return result;
    }

    // Find the two surrounding snapshots and LERP
    for (size_t i = 0; i + 1 < snaps.size(); ++i) {
        float tA = static_cast<float>(snaps[i].tick);
        float tB = static_cast<float>(snaps[i + 1].tick);
        if (tick >= tA && tick <= tB) {
            float span = tB - tA;
            float t    = (span > 0.0f) ? (tick - tA) / span : 0.0f;

            const auto& a = snaps[i];
            const auto& b = snaps[i + 1];
            result.posX   = a.posX   + (b.posX   - a.posX)   * t;
            result.posY   = a.posY   + (b.posY   - a.posY)   * t;
            result.posZ   = a.posZ   + (b.posZ   - a.posZ)   * t;
            result.rotYaw = a.rotYaw + (b.rotYaw - a.rotYaw) * t;
            result.valid  = true;
            return result;
        }
    }

    return result; // should not reach here
}

// ── Hit Test ───────────────────────────────────────────────────────

HitTestResult LagCompensation::HitTest(uint32_t targetId,
                                        float originX, float originY,
                                        float originZ,
                                        uint32_t rewindTick,
                                        float hitRadius) const
{
    HitTestResult result;
    result.entityId = targetId;

    uint32_t clampedTick = clampRewindTick(rewindTick);
    result.rewindTick    = clampedTick;

    InterpolatedState state = GetStateAtTick(targetId,
                                             static_cast<float>(clampedTick));
    if (!state.valid) return result;

    float dx = state.posX - originX;
    float dy = state.posY - originY;
    float dz = state.posZ - originZ;
    float distSq = dx * dx + dy * dy + dz * dz;
    float dist   = std::sqrt(distSq);

    result.distance = dist;
    result.hit      = (dist <= hitRadius);
    return result;
}

// ── Control ────────────────────────────────────────────────────────

void LagCompensation::RemoveEntity(uint32_t entityId) {
    m_history.erase(entityId);
}

void LagCompensation::Clear() {
    m_history.clear();
    m_currentTick = 0;
}

size_t LagCompensation::SnapshotCount(uint32_t entityId) const {
    const EntityHistory* eh = findEntity(entityId);
    return eh ? eh->snapshots.size() : 0;
}

// ── Internals ──────────────────────────────────────────────────────

LagCompensation::EntityHistory* LagCompensation::findEntity(uint32_t id) {
    auto it = m_history.find(id);
    return (it != m_history.end()) ? &it->second : nullptr;
}

const LagCompensation::EntityHistory* LagCompensation::findEntity(
    uint32_t id) const
{
    auto it = m_history.find(id);
    return (it != m_history.end()) ? &it->second : nullptr;
}

void LagCompensation::prune(EntityHistory& eh) const {
    while (eh.snapshots.size() > m_historyDuration) {
        eh.snapshots.pop_front();
    }
}

uint32_t LagCompensation::clampRewindTick(uint32_t requested) const {
    if (m_currentTick == 0) return requested;

    uint32_t minTick = (m_currentTick > m_maxRewindTicks)
                       ? m_currentTick - m_maxRewindTicks
                       : 0;
    if (requested < minTick) return minTick;
    if (requested > m_currentTick) return m_currentTick;
    return requested;
}

} // namespace atlas::net
