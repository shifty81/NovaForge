#include "systems/cargo_transfer_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

CargoTransferSystem::CargoTransferSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

bool CargoTransferSystem::initializeTransfers(const std::string& entity_id,
                                               int max_concurrent) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    if (entity->hasComponent<components::CargoTransfer>()) return false;

    auto ct = std::make_unique<components::CargoTransfer>();
    ct->max_concurrent_transfers = max_concurrent;
    entity->addComponent(std::move(ct));
    return true;
}

bool CargoTransferSystem::startTransfer(const std::string& entity_id,
                                         const std::string& target_id,
                                         const std::string& item_type,
                                         float amount,
                                         float speed) {
    auto* ct = getComponentFor(entity_id);
    if (!ct || !ct->active) return false;
    if (target_id.empty() || item_type.empty() || amount <= 0.0f || speed <= 0.0f) return false;

    // Verify target exists
    if (!world_->getEntity(target_id)) return false;

    // Count active transfers
    int active = 0;
    for (const auto& j : ct->jobs) {
        if (!j.completed) ++active;
    }
    if (active >= ct->max_concurrent_transfers) return false;

    components::CargoTransfer::TransferJob job;
    job.target_id = target_id;
    job.item_type = item_type;
    job.amount = amount;
    job.transfer_speed = speed;
    ct->jobs.push_back(job);
    return true;
}

int CargoTransferSystem::getActiveTransferCount(const std::string& entity_id) const {
    auto* ct = getComponentFor(entity_id);
    if (!ct) return 0;
    int count = 0;
    for (const auto& j : ct->jobs) {
        if (!j.completed) ++count;
    }
    return count;
}

int CargoTransferSystem::getTotalCompleted(const std::string& entity_id) const {
    auto* ct = getComponentFor(entity_id);
    return ct ? ct->total_transfers_completed : 0;
}

float CargoTransferSystem::getTotalUnitsTransferred(const std::string& entity_id) const {
    auto* ct = getComponentFor(entity_id);
    return ct ? ct->total_units_transferred : 0.0f;
}

float CargoTransferSystem::getTransferProgress(const std::string& entity_id,
                                                int job_index) const {
    auto* ct = getComponentFor(entity_id);
    if (!ct || job_index < 0 || job_index >= static_cast<int>(ct->jobs.size())) return 0.0f;
    const auto& job = ct->jobs[job_index];
    return job.amount > 0.0f ? job.transferred / job.amount : 0.0f;
}

bool CargoTransferSystem::cancelAllTransfers(const std::string& entity_id) {
    auto* ct = getComponentFor(entity_id);
    if (!ct) return false;
    ct->jobs.clear();
    return true;
}

void CargoTransferSystem::updateComponent(ecs::Entity& /*entity*/,
                                           components::CargoTransfer& ct,
                                           float delta_time) {
    if (!ct.active) return;
    ct.elapsed += delta_time;

    for (auto& job : ct.jobs) {
        if (job.completed) continue;

        float advance = job.transfer_speed * delta_time;
        job.transferred = std::min(job.transferred + advance, job.amount);

        if (job.transferred >= job.amount - 1e-6f) {
            job.transferred = job.amount;
            job.completed = true;
            ct.total_transfers_completed++;
            ct.total_units_transferred += job.amount;
        }
    }
}

} // namespace systems
} // namespace atlas
