#include "systems/jitter_buffer_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include "components/navigation_components.h"
#include <algorithm>
#include <cmath>

namespace atlas {
namespace systems {

namespace {

using JB = components::JitterBuffer;
using Sample = components::JitterBuffer::PacketSample;

void recomputeMetrics(JB* jb) {
    if (jb->samples.empty()) {
        jb->average_jitter = 0.0f;
        jb->peak_jitter = 0.0f;
        return;
    }
    float sum = 0.0f;
    float peak = 0.0f;
    for (const auto& s : jb->samples) {
        sum += s.jitter;
        if (s.jitter > peak) peak = s.jitter;
    }
    jb->average_jitter = sum / static_cast<float>(jb->samples.size());
    jb->peak_jitter = peak;
}

} // anonymous namespace

JitterBufferSystem::JitterBufferSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void JitterBufferSystem::updateComponent(ecs::Entity& entity,
    components::JitterBuffer& jb, float delta_time) {
    if (!jb.active) return;

    jb.elapsed += delta_time;

    // Adaptive buffer sizing: move buffer_size towards average_jitter × 2
    // (with some headroom) at the configured adaptation rate
    if (!jb.samples.empty()) {
        float target = jb.average_jitter * 2.0f;
        target = std::max(jb.min_buffer_ms, std::min(jb.max_buffer_ms, target));
        jb.buffer_size_ms += (target - jb.buffer_size_ms) * jb.adaptation_rate;
        jb.buffer_size_ms = std::max(jb.min_buffer_ms, std::min(jb.max_buffer_ms, jb.buffer_size_ms));
    }
}

bool JitterBufferSystem::initialize(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::JitterBuffer>();
    entity->addComponent(std::move(comp));
    return true;
}

bool JitterBufferSystem::recordPacket(const std::string& entity_id, int sequence,
    float arrival_time, float expected_time) {
    auto* jb = getComponentFor(entity_id);
    if (!jb) return false;

    Sample s;
    s.sequence = sequence;
    s.arrival_time = arrival_time;
    s.expected_time = expected_time;
    s.jitter = std::fabs(arrival_time - expected_time);

    // Evict oldest if at capacity
    if (static_cast<int>(jb->samples.size()) >= jb->max_samples) {
        jb->samples.erase(jb->samples.begin());
    }
    jb->samples.push_back(s);
    jb->total_packets++;
    jb->last_sequence = sequence;

    recomputeMetrics(jb);
    return true;
}

bool JitterBufferSystem::recordLoss(const std::string& entity_id) {
    auto* jb = getComponentFor(entity_id);
    if (!jb) return false;
    jb->lost_packets++;
    jb->total_packets++;
    return true;
}

bool JitterBufferSystem::recordUnderrun(const std::string& entity_id) {
    auto* jb = getComponentFor(entity_id);
    if (!jb) return false;
    jb->underruns++;
    return true;
}

bool JitterBufferSystem::recordOverrun(const std::string& entity_id) {
    auto* jb = getComponentFor(entity_id);
    if (!jb) return false;
    jb->overruns++;
    return true;
}

bool JitterBufferSystem::setBufferBounds(const std::string& entity_id,
    float min_ms, float max_ms) {
    auto* jb = getComponentFor(entity_id);
    if (!jb) return false;
    if (min_ms < 0.0f || max_ms < min_ms) return false;
    jb->min_buffer_ms = min_ms;
    jb->max_buffer_ms = max_ms;
    jb->buffer_size_ms = std::max(min_ms, std::min(max_ms, jb->buffer_size_ms));
    return true;
}

bool JitterBufferSystem::setAdaptationRate(const std::string& entity_id, float rate) {
    auto* jb = getComponentFor(entity_id);
    if (!jb) return false;
    jb->adaptation_rate = std::max(0.0f, std::min(1.0f, rate));
    return true;
}

bool JitterBufferSystem::resetMetrics(const std::string& entity_id) {
    auto* jb = getComponentFor(entity_id);
    if (!jb) return false;
    jb->samples.clear();
    jb->average_jitter = 0.0f;
    jb->peak_jitter = 0.0f;
    jb->total_packets = 0;
    jb->lost_packets = 0;
    jb->underruns = 0;
    jb->overruns = 0;
    jb->last_sequence = 0;
    return true;
}

int JitterBufferSystem::getSampleCount(const std::string& entity_id) const {
    auto* jb = getComponentFor(entity_id);
    return jb ? static_cast<int>(jb->samples.size()) : 0;
}

float JitterBufferSystem::getBufferSize(const std::string& entity_id) const {
    auto* jb = getComponentFor(entity_id);
    return jb ? jb->buffer_size_ms : 0.0f;
}

float JitterBufferSystem::getAverageJitter(const std::string& entity_id) const {
    auto* jb = getComponentFor(entity_id);
    return jb ? jb->average_jitter : 0.0f;
}

float JitterBufferSystem::getPeakJitter(const std::string& entity_id) const {
    auto* jb = getComponentFor(entity_id);
    return jb ? jb->peak_jitter : 0.0f;
}

int JitterBufferSystem::getTotalPackets(const std::string& entity_id) const {
    auto* jb = getComponentFor(entity_id);
    return jb ? jb->total_packets : 0;
}

int JitterBufferSystem::getLostPackets(const std::string& entity_id) const {
    auto* jb = getComponentFor(entity_id);
    return jb ? jb->lost_packets : 0;
}

int JitterBufferSystem::getUnderruns(const std::string& entity_id) const {
    auto* jb = getComponentFor(entity_id);
    return jb ? jb->underruns : 0;
}

int JitterBufferSystem::getOverruns(const std::string& entity_id) const {
    auto* jb = getComponentFor(entity_id);
    return jb ? jb->overruns : 0;
}

float JitterBufferSystem::getPacketLossRate(const std::string& entity_id) const {
    auto* jb = getComponentFor(entity_id);
    if (!jb || jb->total_packets == 0) return 0.0f;
    return static_cast<float>(jb->lost_packets) / static_cast<float>(jb->total_packets);
}

} // namespace systems
} // namespace atlas
