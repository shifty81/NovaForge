#include "systems/ore_compression_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include "components/economy_components.h"
#include <algorithm>
#include <cmath>

namespace atlas {
namespace systems {

namespace {

using OC = components::OreCompression;

const char* stateToString(OC::CompressionState s) {
    switch (s) {
        case OC::CompressionState::Idle:        return "Idle";
        case OC::CompressionState::Compressing:  return "Compressing";
        case OC::CompressionState::Complete:     return "Complete";
        case OC::CompressionState::Failed:       return "Failed";
    }
    return "Unknown";
}

} // anonymous namespace

OreCompressionSystem::OreCompressionSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void OreCompressionSystem::updateComponent(ecs::Entity& entity,
    components::OreCompression& comp, float delta_time) {
    if (!comp.active) return;
    comp.elapsed += delta_time;

    if (comp.state == OC::CompressionState::Compressing) {
        comp.process_timer -= delta_time;
        if (comp.process_timer <= 0.0f) {
            comp.process_timer = 0.0f;

            // Find the ore type to compute output
            for (const auto& ot : comp.ore_types) {
                if (ot.ore_name == comp.current_ore) {
                    int compressed = static_cast<int>(
                        std::floor(static_cast<float>(comp.raw_units_queued) / ot.compression_ratio));
                    comp.compressed_units_produced = compressed;
                    comp.total_compressed_produced += compressed;
                    comp.total_raw_consumed += comp.raw_units_queued;
                    comp.total_isc_spent += ot.cost_per_batch;
                    comp.total_batches_processed++;
                    comp.raw_units_queued = 0;
                    comp.state = OC::CompressionState::Complete;
                    return;
                }
            }
            // Ore type vanished — shouldn't happen, but mark failed
            comp.state = OC::CompressionState::Failed;
        }
    }
}

bool OreCompressionSystem::initialize(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::OreCompression>();
    entity->addComponent(std::move(comp));
    return true;
}

bool OreCompressionSystem::addOreType(const std::string& entity_id,
    const std::string& ore_name, float compression_ratio,
    float process_time, double cost_per_batch) {
    auto* oc = getComponentFor(entity_id);
    if (!oc) return false;
    if (compression_ratio <= 0.0f || process_time <= 0.0f) return false;
    for (const auto& ot : oc->ore_types) {
        if (ot.ore_name == ore_name) return false;
    }
    OC::OreType ot;
    ot.ore_name = ore_name;
    ot.compression_ratio = compression_ratio;
    ot.process_time = process_time;
    ot.cost_per_batch = cost_per_batch;
    oc->ore_types.push_back(ot);
    return true;
}

bool OreCompressionSystem::startCompression(const std::string& entity_id,
    const std::string& ore_name, int raw_units) {
    auto* oc = getComponentFor(entity_id);
    if (!oc) return false;
    if (oc->state == OC::CompressionState::Compressing) return false;
    if (raw_units <= 0 || raw_units > oc->max_queue) return false;

    const OC::OreType* found = nullptr;
    for (const auto& ot : oc->ore_types) {
        if (ot.ore_name == ore_name) { found = &ot; break; }
    }
    if (!found) return false;

    oc->current_ore = ore_name;
    oc->raw_units_queued = raw_units;
    oc->compressed_units_produced = 0;
    oc->process_timer = found->process_time;
    oc->state = OC::CompressionState::Compressing;
    return true;
}

bool OreCompressionSystem::cancelCompression(const std::string& entity_id) {
    auto* oc = getComponentFor(entity_id);
    if (!oc) return false;
    if (oc->state != OC::CompressionState::Compressing) return false;
    oc->state = OC::CompressionState::Idle;
    oc->raw_units_queued = 0;
    oc->process_timer = 0.0f;
    return true;
}

bool OreCompressionSystem::collectCompressed(const std::string& entity_id) {
    auto* oc = getComponentFor(entity_id);
    if (!oc) return false;
    if (oc->state != OC::CompressionState::Complete) return false;
    oc->compressed_units_produced = 0;
    oc->current_ore.clear();
    oc->state = OC::CompressionState::Idle;
    return true;
}

int OreCompressionSystem::getOreTypeCount(const std::string& entity_id) const {
    auto* oc = getComponentFor(entity_id);
    return oc ? static_cast<int>(oc->ore_types.size()) : 0;
}

std::string OreCompressionSystem::getCompressionState(const std::string& entity_id) const {
    auto* oc = getComponentFor(entity_id);
    if (!oc) return "Unknown";
    return stateToString(oc->state);
}

std::string OreCompressionSystem::getCurrentOre(const std::string& entity_id) const {
    auto* oc = getComponentFor(entity_id);
    if (!oc) return "";
    return oc->current_ore;
}

int OreCompressionSystem::getRawQueued(const std::string& entity_id) const {
    auto* oc = getComponentFor(entity_id);
    return oc ? oc->raw_units_queued : 0;
}

int OreCompressionSystem::getCompressedProduced(const std::string& entity_id) const {
    auto* oc = getComponentFor(entity_id);
    return oc ? oc->compressed_units_produced : 0;
}

float OreCompressionSystem::getProcessTimer(const std::string& entity_id) const {
    auto* oc = getComponentFor(entity_id);
    return oc ? oc->process_timer : 0.0f;
}

double OreCompressionSystem::getTotalIscSpent(const std::string& entity_id) const {
    auto* oc = getComponentFor(entity_id);
    return oc ? oc->total_isc_spent : 0.0;
}

int OreCompressionSystem::getTotalBatchesProcessed(const std::string& entity_id) const {
    auto* oc = getComponentFor(entity_id);
    return oc ? oc->total_batches_processed : 0;
}

int OreCompressionSystem::getTotalRawConsumed(const std::string& entity_id) const {
    auto* oc = getComponentFor(entity_id);
    return oc ? oc->total_raw_consumed : 0;
}

int OreCompressionSystem::getTotalCompressedProduced(const std::string& entity_id) const {
    auto* oc = getComponentFor(entity_id);
    return oc ? oc->total_compressed_produced : 0;
}

} // namespace systems
} // namespace atlas
