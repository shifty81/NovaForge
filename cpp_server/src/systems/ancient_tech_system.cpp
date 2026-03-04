#include "systems/ancient_tech_system.h"
#include "ecs/world.h"

namespace atlas {
namespace systems {

AncientTechSystem::AncientTechSystem(ecs::World* world)
    : SingleComponentSystem(world) {
}

void AncientTechSystem::updateComponent(ecs::Entity& /*entity*/, components::AncientTechModule& tech, float delta_time) {
    if (tech.state == components::AncientTechModule::TechState::Repairing) {
        tech.repair_progress += delta_time / (tech.repair_cost * 0.5f);
        if (tech.repair_progress >= 1.0f) {
            tech.repair_progress = 1.0f;
            tech.state = components::AncientTechModule::TechState::Repaired;
        }
    }
}

bool AncientTechSystem::startRepair(const std::string& entity_id) {
    auto* tech = getComponentFor(entity_id);
    if (!tech) return false;
    if (tech->state != components::AncientTechModule::TechState::Broken) return false;
    tech->state = components::AncientTechModule::TechState::Repairing;
    return true;
}

std::string AncientTechSystem::reverseEngineer(const std::string& entity_id) {
    auto* tech = getComponentFor(entity_id);
    if (!tech) return "";
    if (tech->state != components::AncientTechModule::TechState::Repaired) return "";
    tech->reverse_engineered = true;
    return tech->blueprint_id;
}

components::AncientTechModule::TechState
AncientTechSystem::getState(const std::string& entity_id) const {
    const auto* tech = getComponentFor(entity_id);
    if (!tech) return components::AncientTechModule::TechState::Broken;
    return tech->state;
}

bool AncientTechSystem::isUsable(const std::string& entity_id) const {
    const auto* tech = getComponentFor(entity_id);
    if (!tech) return false;
    return tech->isUsable();
}

} // namespace systems
} // namespace atlas
