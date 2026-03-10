#include "systems/pi_customs_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

PiCustomsSystem::PiCustomsSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void PiCustomsSystem::updateComponent(ecs::Entity& /*entity*/,
    components::PiCustomsState& comp, float delta_time) {
    if (!comp.active) return;
    comp.elapsed += delta_time;

    for (auto& batch : comp.batches) {
        if (batch.completed || batch.cancelled) continue;

        batch.progress += delta_time / batch.export_duration;
        if (batch.progress >= 1.0f) {
            batch.progress = 1.0f;
            batch.completed = true;
            comp.total_exported += batch.quantity;
        }
    }
}

bool PiCustomsSystem::initialize(const std::string& entity_id,
    const std::string& customs_office_id, const std::string& system_id,
    bool player_owned) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::PiCustomsState>();
    comp->customs_office_id = customs_office_id;
    comp->system_id = system_id;
    comp->player_owned = player_owned;
    entity->addComponent(std::move(comp));
    return true;
}

std::string PiCustomsSystem::queueExport(const std::string& entity_id,
    const std::string& colony_id, const std::string& resource_type,
    int quantity, bool is_corp_member) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return "";
    if (quantity <= 0) return "";

    // Count active (non-complete, non-cancelled) batches
    int active_count = 0;
    for (const auto& b : comp->batches) {
        if (!b.completed && !b.cancelled) active_count++;
    }
    if (active_count >= comp->max_concurrent) return "";

    components::PiCustomsState::ExportBatch batch;
    batch.batch_id = "batch_" + std::to_string(++batch_counter_);
    batch.colony_id = colony_id;
    batch.resource_type = resource_type;
    batch.quantity = quantity;
    batch.tax_rate = is_corp_member ? comp->tax_rate_corp : comp->tax_rate_stranger;
    comp->batches.push_back(batch);
    comp->total_batches++;
    return batch.batch_id;
}

bool PiCustomsSystem::cancelExport(const std::string& entity_id,
    const std::string& batch_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (auto& b : comp->batches) {
        if (b.batch_id == batch_id && !b.completed && !b.cancelled) {
            b.cancelled = true;
            return true;
        }
    }
    return false;
}

int PiCustomsSystem::getPendingBatches(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    int count = 0;
    for (const auto& b : comp->batches) {
        if (!b.completed && !b.cancelled) count++;
    }
    return count;
}

int PiCustomsSystem::getCompletedBatches(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    int count = 0;
    for (const auto& b : comp->batches) {
        if (b.completed) count++;
    }
    return count;
}

int PiCustomsSystem::getTotalExported(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_exported : 0;
}

float PiCustomsSystem::getBatchProgress(const std::string& entity_id,
    const std::string& batch_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0f;
    for (const auto& b : comp->batches) {
        if (b.batch_id == batch_id) return b.progress;
    }
    return 0.0f;
}

} // namespace systems
} // namespace atlas
