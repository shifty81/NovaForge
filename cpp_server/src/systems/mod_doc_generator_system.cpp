#include "systems/mod_doc_generator_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include "components/game_components.h"
#include <algorithm>

namespace atlas {
namespace systems {

ModDocGeneratorSystem::ModDocGeneratorSystem(ecs::World* world)
    : System(world) {
}

void ModDocGeneratorSystem::update(float delta_time) {
    // No per-tick behavior needed; generation is on-demand
    (void)delta_time;
}

bool ModDocGeneratorSystem::createGenerator(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::ModDocGenerator>();
    entity->addComponent(std::move(comp));
    return true;
}

bool ModDocGeneratorSystem::registerType(const std::string& entity_id,
                                          const std::string& type_name,
                                          const std::string& category,
                                          const std::string& description,
                                          int field_count) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto* mdg = entity->getComponent<components::ModDocGenerator>();
    if (!mdg) return false;

    // Check max entries
    if (static_cast<int>(mdg->entries.size()) >= mdg->max_entries) return false;

    // Check duplicate
    for (const auto& e : mdg->entries) {
        if (e.type_name == type_name) return false;
    }

    components::ModDocGenerator::DocEntry entry;
    entry.type_name = type_name;
    entry.category = category;
    entry.description = description;
    entry.field_count = field_count;
    entry.has_example = false;
    entry.validated = false;

    mdg->entries.push_back(entry);
    mdg->total_entries++;
    return true;
}

bool ModDocGeneratorSystem::addExample(const std::string& entity_id,
                                        const std::string& type_name) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto* mdg = entity->getComponent<components::ModDocGenerator>();
    if (!mdg) return false;

    for (auto& e : mdg->entries) {
        if (e.type_name == type_name) {
            e.has_example = true;
            return true;
        }
    }
    return false;
}

bool ModDocGeneratorSystem::validateEntry(const std::string& entity_id,
                                           const std::string& type_name) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto* mdg = entity->getComponent<components::ModDocGenerator>();
    if (!mdg) return false;

    for (auto& e : mdg->entries) {
        if (e.type_name == type_name) {
            if (e.description.empty()) return false;
            if (e.field_count <= 0) return false;
            if (!e.has_example) return false;
            e.validated = true;
            mdg->total_validated++;
            return true;
        }
    }
    return false;
}

bool ModDocGeneratorSystem::generate(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto* mdg = entity->getComponent<components::ModDocGenerator>();
    if (!mdg) return false;

    if (mdg->entries.empty()) return false;

    for (const auto& e : mdg->entries) {
        if (!e.validated) return false;
    }

    mdg->generated = true;
    mdg->generation_count++;
    return true;
}

int ModDocGeneratorSystem::getEntryCount(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0;
    auto* mdg = entity->getComponent<components::ModDocGenerator>();
    if (!mdg) return 0;
    return static_cast<int>(mdg->entries.size());
}

int ModDocGeneratorSystem::getEntriesByCategory(const std::string& entity_id,
                                                 const std::string& category) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0;
    auto* mdg = entity->getComponent<components::ModDocGenerator>();
    if (!mdg) return 0;
    int count = 0;
    for (const auto& e : mdg->entries) {
        if (e.category == category) count++;
    }
    return count;
}

bool ModDocGeneratorSystem::isGenerated(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto* mdg = entity->getComponent<components::ModDocGenerator>();
    if (!mdg) return false;
    return mdg->generated;
}

int ModDocGeneratorSystem::getValidatedCount(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0;
    auto* mdg = entity->getComponent<components::ModDocGenerator>();
    if (!mdg) return 0;
    return mdg->total_validated;
}

int ModDocGeneratorSystem::getGenerationCount(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0;
    auto* mdg = entity->getComponent<components::ModDocGenerator>();
    if (!mdg) return 0;
    return mdg->generation_count;
}

bool ModDocGeneratorSystem::setTitle(const std::string& entity_id, const std::string& title) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto* mdg = entity->getComponent<components::ModDocGenerator>();
    if (!mdg) return false;
    mdg->title = title;
    return true;
}

bool ModDocGeneratorSystem::setVersion(const std::string& entity_id, const std::string& version) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto* mdg = entity->getComponent<components::ModDocGenerator>();
    if (!mdg) return false;
    mdg->version = version;
    return true;
}

} // namespace systems
} // namespace atlas
