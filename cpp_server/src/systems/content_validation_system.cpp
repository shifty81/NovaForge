#include "systems/content_validation_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include "components/game_components.h"
#include <algorithm>

namespace atlas {
namespace systems {

ContentValidationSystem::ContentValidationSystem(ecs::World* world)
    : SingleComponentSystem(world) {
}

void ContentValidationSystem::updateComponent(ecs::Entity& /*entity*/,
                                               components::ContentValidation& /*cv*/,
                                               float /*delta_time*/) {
    // Validation is on-demand, not tick-driven
}

bool ContentValidationSystem::createValidator(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::ContentValidation>();
    entity->addComponent(std::move(comp));
    return true;
}

bool ContentValidationSystem::submitContent(const std::string& entity_id,
                                             const std::string& content_id,
                                             int content_type,
                                             const std::string& name) {
    auto* cv = getComponentFor(entity_id);
    if (!cv) return false;
    if (content_id.empty() || name.empty()) return false;

    // Check for duplicate content_id
    if (cv->findEntry(content_id) != nullptr) return false;

    components::ContentValidation::ContentEntry entry;
    entry.content_id = content_id;
    entry.name = name;
    entry.content_type = std::max(0, std::min(content_type, 3));
    entry.state = components::ContentValidation::Pending;
    cv->entries.push_back(entry);
    return true;
}

bool ContentValidationSystem::runValidation(const std::string& entity_id,
                                             const std::string& content_id) {
    auto* cv = getComponentFor(entity_id);
    if (!cv) return false;

    auto* entry = cv->findEntry(content_id);
    if (!entry) return false;
    if (entry->state != components::ContentValidation::Pending) return false;

    entry->state = components::ContentValidation::Validating;
    cv->total_validations++;
    return true;
}

bool ContentValidationSystem::approveContent(const std::string& entity_id,
                                              const std::string& content_id) {
    auto* cv = getComponentFor(entity_id);
    if (!cv) return false;

    auto* entry = cv->findEntry(content_id);
    if (!entry) return false;
    if (entry->state != components::ContentValidation::Validating) return false;

    entry->state = components::ContentValidation::Approved;
    cv->approved_count++;
    return true;
}

bool ContentValidationSystem::rejectContent(const std::string& entity_id,
                                             const std::string& content_id,
                                             const std::string& reason) {
    auto* cv = getComponentFor(entity_id);
    if (!cv) return false;

    auto* entry = cv->findEntry(content_id);
    if (!entry) return false;
    if (entry->state != components::ContentValidation::Validating) return false;

    entry->state = components::ContentValidation::Rejected;
    entry->rejection_reason = reason;
    cv->rejected_count++;
    return true;
}

int ContentValidationSystem::getContentState(const std::string& entity_id,
                                              const std::string& content_id) const {
    const auto* cv = getComponentFor(entity_id);
    if (!cv) return -1;

    const auto* entry = cv->findEntry(content_id);
    if (!entry) return -1;
    return entry->state;
}

int ContentValidationSystem::getPendingCount(const std::string& entity_id) const {
    const auto* cv = getComponentFor(entity_id);
    if (!cv) return 0;
    int count = 0;
    for (const auto& e : cv->entries) {
        if (e.state == components::ContentValidation::Pending) count++;
    }
    return count;
}

int ContentValidationSystem::getApprovedCount(const std::string& entity_id) const {
    const auto* cv = getComponentFor(entity_id);
    if (!cv) return 0;
    return cv->approved_count;
}

int ContentValidationSystem::getRejectedCount(const std::string& entity_id) const {
    const auto* cv = getComponentFor(entity_id);
    if (!cv) return 0;
    return cv->rejected_count;
}

std::string ContentValidationSystem::getRejectionReason(const std::string& entity_id,
                                                         const std::string& content_id) const {
    const auto* cv = getComponentFor(entity_id);
    if (!cv) return "";

    const auto* entry = cv->findEntry(content_id);
    if (!entry) return "";
    return entry->rejection_reason;
}

int ContentValidationSystem::getTotalValidations(const std::string& entity_id) const {
    const auto* cv = getComponentFor(entity_id);
    if (!cv) return 0;
    return cv->total_validations;
}

} // namespace systems
} // namespace atlas
