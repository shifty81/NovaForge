#include "systems/ancient_module_discovery_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

AncientModuleDiscoverySystem::AncientModuleDiscoverySystem(ecs::World* world)
    : System(world) {
}

void AncientModuleDiscoverySystem::update(float delta_time) {
    auto entities = world_->getEntities<components::AncientModuleDiscovery>();
    for (auto* entity : entities) {
        auto* disc = entity->getComponent<components::AncientModuleDiscovery>();
        if (!disc || !disc->active) continue;

        using DS = components::AncientModuleDiscovery::DiscoveryState;

        for (auto& mod : disc->modules) {
            auto state = static_cast<DS>(mod.state);
            if (state == DS::Scanning) {
                mod.scan_progress += delta_time;
                if (mod.scan_progress >= 1.0f) {
                    mod.scan_progress = 1.0f;
                    mod.state = static_cast<int>(DS::Discovered);
                }
            } else if (state == DS::Extracting) {
                mod.extract_progress += delta_time;
                if (mod.extract_progress >= mod.extract_required) {
                    mod.extract_progress = mod.extract_required;
                    mod.state = static_cast<int>(DS::Extracted);
                    disc->total_extractions++;
                }
            }
        }
    }
}

bool AncientModuleDiscoverySystem::initializeSite(const std::string& entity_id,
                                                    const std::string& site_id,
                                                    const std::string& explorer_id,
                                                    float scan_range) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    if (entity->getComponent<components::AncientModuleDiscovery>()) return false;

    auto comp = std::make_unique<components::AncientModuleDiscovery>();
    comp->site_id = site_id;
    comp->explorer_id = explorer_id;
    comp->scan_range = scan_range;
    entity->addComponent(std::move(comp));
    return true;
}

bool AncientModuleDiscoverySystem::addHiddenModule(const std::string& entity_id,
                                                    const std::string& module_id,
                                                    const std::string& tech_type,
                                                    float repair_difficulty,
                                                    float extract_required,
                                                    float estimated_value) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* disc = entity->getComponent<components::AncientModuleDiscovery>();
    if (!disc) return false;

    if (disc->findModule(module_id)) return false;
    if (static_cast<int>(disc->modules.size()) >= disc->max_modules) return false;

    components::AncientModuleDiscovery::DiscoveredModule mod;
    mod.module_id = module_id;
    mod.tech_type = tech_type;
    mod.repair_difficulty = repair_difficulty;
    mod.extract_required = extract_required;
    mod.estimated_value = estimated_value;
    disc->modules.push_back(mod);
    return true;
}

bool AncientModuleDiscoverySystem::beginScan(const std::string& entity_id,
                                              const std::string& module_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* disc = entity->getComponent<components::AncientModuleDiscovery>();
    if (!disc) return false;

    auto* mod = disc->findModule(module_id);
    if (!mod) return false;

    using DS = components::AncientModuleDiscovery::DiscoveryState;
    if (mod->state != static_cast<int>(DS::Undiscovered)) return false;

    mod->state = static_cast<int>(DS::Scanning);
    mod->scan_progress = 0.0f;
    return true;
}

bool AncientModuleDiscoverySystem::beginExtraction(const std::string& entity_id,
                                                    const std::string& module_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* disc = entity->getComponent<components::AncientModuleDiscovery>();
    if (!disc) return false;

    auto* mod = disc->findModule(module_id);
    if (!mod) return false;

    using DS = components::AncientModuleDiscovery::DiscoveryState;
    if (mod->state != static_cast<int>(DS::Discovered)) return false;

    mod->state = static_cast<int>(DS::Extracting);
    mod->extract_progress = 0.0f;
    return true;
}

bool AncientModuleDiscoverySystem::analyzeModule(const std::string& entity_id,
                                                  const std::string& module_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* disc = entity->getComponent<components::AncientModuleDiscovery>();
    if (!disc) return false;

    auto* mod = disc->findModule(module_id);
    if (!mod) return false;

    using DS = components::AncientModuleDiscovery::DiscoveryState;
    if (mod->state != static_cast<int>(DS::Extracted)) return false;

    mod->state = static_cast<int>(DS::Analyzed);
    return true;
}

bool AncientModuleDiscoverySystem::setActive(const std::string& entity_id, bool active) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* disc = entity->getComponent<components::AncientModuleDiscovery>();
    if (!disc) return false;

    disc->active = active;
    return true;
}

int AncientModuleDiscoverySystem::getModuleState(const std::string& entity_id,
                                                  const std::string& module_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0;

    auto* disc = entity->getComponent<components::AncientModuleDiscovery>();
    if (!disc) return 0;

    const auto* mod = disc->findModule(module_id);
    if (!mod) return 0;

    return mod->state;
}

int AncientModuleDiscoverySystem::getDiscoveredCount(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0;

    auto* disc = entity->getComponent<components::AncientModuleDiscovery>();
    if (!disc) return 0;

    return disc->discoveredCount();
}

int AncientModuleDiscoverySystem::getExtractedCount(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0;

    auto* disc = entity->getComponent<components::AncientModuleDiscovery>();
    if (!disc) return 0;

    return disc->extractedCount();
}

int AncientModuleDiscoverySystem::getTotalModules(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0;

    auto* disc = entity->getComponent<components::AncientModuleDiscovery>();
    if (!disc) return 0;

    return static_cast<int>(disc->modules.size());
}

float AncientModuleDiscoverySystem::getScanRange(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0.0f;

    auto* disc = entity->getComponent<components::AncientModuleDiscovery>();
    if (!disc) return 0.0f;

    return disc->scan_range;
}

std::string AncientModuleDiscoverySystem::stateName(int state) {
    using DS = components::AncientModuleDiscovery::DiscoveryState;
    switch (static_cast<DS>(state)) {
        case DS::Undiscovered: return "undiscovered";
        case DS::Scanning: return "scanning";
        case DS::Discovered: return "discovered";
        case DS::Extracting: return "extracting";
        case DS::Extracted: return "extracted";
        case DS::Analyzed: return "analyzed";
    }
    return "unknown";
}

} // namespace systems
} // namespace atlas
