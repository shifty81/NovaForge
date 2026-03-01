#include "JitterBuffer.h"
#include <algorithm>
#include <cmath>

namespace atlas::net {

JitterBuffer::JitterBuffer(float targetDelay,
                           size_t maxBufferSize,
                           bool adaptive)
    : m_targetDelay(std::max(targetDelay, kMinTargetDelay))
    , m_maxBufferSize(maxBufferSize > 0 ? maxBufferSize : 1)
    , m_adaptive(adaptive)
{}

// ── Push ───────────────────────────────────────────────────────────

void JitterBuffer::Push(uint32_t tick, float arrivalTime,
                        const std::vector<uint8_t>& payload)
{
    ++m_totalPushed;

    // Drop late packets (already released a newer tick)
    if (tick <= m_lastReleasedTick && m_lastReleasedTick != 0) {
        ++m_totalDropped;
        return;
    }

    // Adaptive jitter estimation
    if (m_adaptive) {
        if (m_firstPush) {
            m_lastArrivalTime = arrivalTime;
            m_firstPush       = false;
        } else {
            float interArrival = arrivalTime - m_lastArrivalTime;
            if (interArrival >= 0.0f) {
                float jitterSample = std::fabs(interArrival - m_lastInterArrival);
                adaptDelay(jitterSample);
                m_lastInterArrival = interArrival;
            }
            m_lastArrivalTime = arrivalTime;
        }
    }

    // Insert in tick order
    JitterEntry entry;
    entry.tick        = tick;
    entry.arrivalTime = arrivalTime;
    entry.payload     = payload;

    auto it = std::lower_bound(
        m_buffer.begin(), m_buffer.end(), entry,
        [](const JitterEntry& a, const JitterEntry& b) {
            return a.tick < b.tick;
        });

    // Replace if same tick already exists
    if (it != m_buffer.end() && it->tick == tick) {
        *it = entry;
    } else {
        m_buffer.insert(it, entry);
    }

    trimOverflow();
}

// ── Flush ──────────────────────────────────────────────────────────

std::vector<JitterEntry> JitterBuffer::Flush(float currentTime)
{
    std::vector<JitterEntry> ready;

    while (!m_buffer.empty()) {
        const auto& front = m_buffer.front();
        float holdTime = currentTime - front.arrivalTime;

        if (holdTime >= m_targetDelay) {
            m_lastReleasedTick = front.tick;
            ready.push_back(front);
            m_buffer.pop_front();
        } else {
            break;  // entries are sorted, so no later ones are ready
        }
    }

    return ready;
}

// ── Reset ──────────────────────────────────────────────────────────

void JitterBuffer::Reset() {
    m_buffer.clear();
    m_smoothedJitter   = 0.0f;
    m_lastArrivalTime  = -1.0f;
    m_lastInterArrival = 0.0f;
    m_firstPush        = true;
    m_lastReleasedTick = 0;
    m_totalPushed      = 0;
    m_totalDropped     = 0;
}

// ── Internals ──────────────────────────────────────────────────────

void JitterBuffer::adaptDelay(float interArrivalJitter) {
    // Exponential moving average of jitter
    m_smoothedJitter = m_smoothedJitter
        + kJitterAlpha * (interArrivalJitter - m_smoothedJitter);

    // Scale jitter to target delay
    float adapted = m_smoothedJitter * kAdaptiveScale;
    m_targetDelay = std::clamp(adapted, kMinTargetDelay, kMaxTargetDelay);
}

void JitterBuffer::trimOverflow() {
    while (m_buffer.size() > m_maxBufferSize) {
        ++m_totalDropped;
        m_buffer.pop_front();
    }
}

} // namespace atlas::net
