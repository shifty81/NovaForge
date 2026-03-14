#include "systems/moon_mining_scheduler_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

MoonMiningSchedulerSystem::MoonMiningSchedulerSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

// ---------------------------------------------------------------------------
// Tick — advance extraction timers
// ---------------------------------------------------------------------------

void MoonMiningSchedulerSystem::updateComponent(
        ecs::Entity& /*entity*/,
        components::MoonMiningSchedulerState& comp,
        float delta_time) {
    if (!comp.active) return;
    comp.elapsed += delta_time;

    for (auto& ex : comp.extractions) {
        if (ex.status == components::MoonMiningSchedulerState::ExtractionStatus::Extracting) {
            ex.time_remaining -= delta_time;
            if (ex.time_remaining <= 0.0f) {
                ex.time_remaining = 0.0f;
                ex.status = components::MoonMiningSchedulerState::ExtractionStatus::Ready;
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Lifecycle
// ---------------------------------------------------------------------------

bool MoonMiningSchedulerSystem::initialize(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::MoonMiningSchedulerState>();
    entity->addComponent(std::move(comp));
    return true;
}

// ---------------------------------------------------------------------------
// Extraction management
// ---------------------------------------------------------------------------

bool MoonMiningSchedulerSystem::startExtraction(
        const std::string& entity_id,
        const std::string& extraction_id,
        const std::string& moon_id,
        const std::string& ore_type,
        float duration,
        float estimated_yield) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (extraction_id.empty()) return false;
    if (moon_id.empty()) return false;
    if (ore_type.empty()) return false;
    if (duration <= 0.0f) return false;
    if (estimated_yield <= 0.0f) return false;

    for (const auto& ex : comp->extractions) {
        if (ex.extraction_id == extraction_id) return false;
    }

    if (static_cast<int>(comp->extractions.size()) >= comp->max_extractions) return false;

    components::MoonMiningSchedulerState::Extraction ex;
    ex.extraction_id   = extraction_id;
    ex.moon_id         = moon_id;
    ex.ore_type        = ore_type;
    ex.duration        = duration;
    ex.time_remaining  = duration;
    ex.estimated_yield = estimated_yield;
    ex.status          = components::MoonMiningSchedulerState::ExtractionStatus::Extracting;
    comp->extractions.push_back(ex);

    comp->total_extractions_started++;
    return true;
}

bool MoonMiningSchedulerSystem::cancelExtraction(
        const std::string& entity_id,
        const std::string& extraction_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;

    auto it = std::find_if(comp->extractions.begin(), comp->extractions.end(),
        [&](const components::MoonMiningSchedulerState::Extraction& ex) {
            return ex.extraction_id == extraction_id;
        });
    if (it == comp->extractions.end()) return false;
    if (it->status != components::MoonMiningSchedulerState::ExtractionStatus::Extracting)
        return false;

    it->status = components::MoonMiningSchedulerState::ExtractionStatus::Idle;
    it->time_remaining = 0.0f;
    return true;
}

bool MoonMiningSchedulerSystem::fractureExtraction(
        const std::string& entity_id,
        const std::string& extraction_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;

    auto it = std::find_if(comp->extractions.begin(), comp->extractions.end(),
        [&](const components::MoonMiningSchedulerState::Extraction& ex) {
            return ex.extraction_id == extraction_id;
        });
    if (it == comp->extractions.end()) return false;
    if (it->status != components::MoonMiningSchedulerState::ExtractionStatus::Ready)
        return false;

    it->status = components::MoonMiningSchedulerState::ExtractionStatus::Fractured;
    comp->total_fractured++;
    comp->total_yield += it->estimated_yield;
    return true;
}

bool MoonMiningSchedulerSystem::removeExtraction(
        const std::string& entity_id,
        const std::string& extraction_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;

    auto it = std::find_if(comp->extractions.begin(), comp->extractions.end(),
        [&](const components::MoonMiningSchedulerState::Extraction& ex) {
            return ex.extraction_id == extraction_id;
        });
    if (it == comp->extractions.end()) return false;
    comp->extractions.erase(it);
    return true;
}

bool MoonMiningSchedulerSystem::clearExtractions(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->extractions.clear();
    return true;
}

// ---------------------------------------------------------------------------
// Configuration
// ---------------------------------------------------------------------------

bool MoonMiningSchedulerSystem::setStructureId(const std::string& entity_id,
                                                const std::string& structure_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (structure_id.empty()) return false;
    comp->structure_id = structure_id;
    return true;
}

bool MoonMiningSchedulerSystem::setMaxExtractions(const std::string& entity_id,
                                                   int max_extractions) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (max_extractions <= 0) return false;
    comp->max_extractions = max_extractions;
    return true;
}

// ---------------------------------------------------------------------------
// Queries
// ---------------------------------------------------------------------------

int MoonMiningSchedulerSystem::getExtractionCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? static_cast<int>(comp->extractions.size()) : 0;
}

int MoonMiningSchedulerSystem::getActiveExtractionCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    int count = 0;
    for (const auto& ex : comp->extractions) {
        if (ex.status == components::MoonMiningSchedulerState::ExtractionStatus::Extracting)
            count++;
    }
    return count;
}

int MoonMiningSchedulerSystem::getTotalExtractionsStarted(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_extractions_started : 0;
}

int MoonMiningSchedulerSystem::getTotalFractured(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_fractured : 0;
}

float MoonMiningSchedulerSystem::getTotalYield(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_yield : 0.0f;
}

float MoonMiningSchedulerSystem::getTimeRemaining(const std::string& entity_id,
                                                    const std::string& extraction_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0f;
    for (const auto& ex : comp->extractions) {
        if (ex.extraction_id == extraction_id) return ex.time_remaining;
    }
    return 0.0f;
}

bool MoonMiningSchedulerSystem::hasExtraction(const std::string& entity_id,
                                               const std::string& extraction_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (const auto& ex : comp->extractions) {
        if (ex.extraction_id == extraction_id) return true;
    }
    return false;
}

std::string MoonMiningSchedulerSystem::getStructureId(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->structure_id : "";
}

int MoonMiningSchedulerSystem::getCountByStatus(
        const std::string& entity_id,
        components::MoonMiningSchedulerState::ExtractionStatus status) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    int count = 0;
    for (const auto& ex : comp->extractions) {
        if (ex.status == status) count++;
    }
    return count;
}

std::string MoonMiningSchedulerSystem::getExtractionStatus(
        const std::string& entity_id,
        const std::string& extraction_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return "Unknown";
    for (const auto& ex : comp->extractions) {
        if (ex.extraction_id == extraction_id) {
            switch (ex.status) {
                case components::MoonMiningSchedulerState::ExtractionStatus::Idle:
                    return "Idle";
                case components::MoonMiningSchedulerState::ExtractionStatus::Extracting:
                    return "Extracting";
                case components::MoonMiningSchedulerState::ExtractionStatus::Ready:
                    return "Ready";
                case components::MoonMiningSchedulerState::ExtractionStatus::Fractured:
                    return "Fractured";
            }
        }
    }
    return "Unknown";
}

} // namespace systems
} // namespace atlas
