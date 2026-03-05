#include "systems/character_creation_screen_system.h"
#include "ecs/world.h"
#include <memory>
#include <algorithm>

namespace atlas {
namespace systems {

// Valid races and factions
static const std::vector<std::string> VALID_RACES = {
    "Terran Descendant", "Synth-Born", "Pure Alien", "Hybrid Evolutionary"
};

static const std::vector<std::string> VALID_FACTIONS = {
    "Solari", "Veyren", "Aurelian", "Keldari"
};

static const std::vector<std::string> VALID_ATTRIBUTES = {
    "strength", "agility", "intelligence", "perception", "willpower"
};

CharacterCreationScreenSystem::CharacterCreationScreenSystem(ecs::World* world)
    : SingleComponentSystem(world) {
}

void CharacterCreationScreenSystem::updateComponent(ecs::Entity& /*entity*/, components::CharacterCreationScreen& screen, float delta_time) {
    if (!screen.is_open) return;
    screen.time_open += delta_time;
}

bool CharacterCreationScreenSystem::openScreen(const std::string& player_id) {
    std::string entity_id = std::string(SCREEN_ENTITY_PREFIX) + player_id;
    auto* entity = world_->getEntity(entity_id);
    if (!entity) {
        entity = world_->createEntity(entity_id);
    }

    auto* screen = entity->getComponent<components::CharacterCreationScreen>();
    if (screen && screen->is_open) return false; // Already open

    if (!screen) {
        auto comp = std::make_unique<components::CharacterCreationScreen>();
        comp->player_id = player_id;
        comp->is_open = true;
        entity->addComponent(std::move(comp));
    } else {
        screen->is_open = true;
        screen->player_id = player_id;
        screen->time_open = 0.0f;
    }
    return true;
}

bool CharacterCreationScreenSystem::selectRace(const std::string& player_id, const std::string& race) {
    auto found = std::find(VALID_RACES.begin(), VALID_RACES.end(), race);
    if (found == VALID_RACES.end()) return false;

    std::string entity_id = std::string(SCREEN_ENTITY_PREFIX) + player_id;
    auto* screen = getComponentFor(entity_id);
    if (!screen || !screen->is_open) return false;

    screen->selected_race = race;
    return true;
}

bool CharacterCreationScreenSystem::selectFaction(const std::string& player_id, const std::string& faction) {
    auto found = std::find(VALID_FACTIONS.begin(), VALID_FACTIONS.end(), faction);
    if (found == VALID_FACTIONS.end()) return false;

    std::string entity_id = std::string(SCREEN_ENTITY_PREFIX) + player_id;
    auto* screen = getComponentFor(entity_id);
    if (!screen || !screen->is_open) return false;

    screen->selected_faction = faction;
    return true;
}

bool CharacterCreationScreenSystem::setAttributeSlider(const std::string& player_id,
                                                        const std::string& attribute,
                                                        float value) {
    auto found = std::find(VALID_ATTRIBUTES.begin(), VALID_ATTRIBUTES.end(), attribute);
    if (found == VALID_ATTRIBUTES.end()) return false;

    std::string entity_id = std::string(SCREEN_ENTITY_PREFIX) + player_id;
    auto* screen = getComponentFor(entity_id);
    if (!screen || !screen->is_open) return false;

    float clamped = std::max(0.0f, std::min(1.0f, value));
    screen->attribute_sliders[attribute] = clamped;
    return true;
}

float CharacterCreationScreenSystem::getAttributeSlider(const std::string& player_id,
                                                         const std::string& attribute) const {
    std::string entity_id = std::string(SCREEN_ENTITY_PREFIX) + player_id;
    const auto* screen = getComponentFor(entity_id);
    if (!screen) return 0.0f;

    auto it = screen->attribute_sliders.find(attribute);
    if (it != screen->attribute_sliders.end()) return it->second;
    return 0.0f;
}

bool CharacterCreationScreenSystem::setAppearanceSlider(const std::string& player_id,
                                                         const std::string& feature,
                                                         float value) {
    std::string entity_id = std::string(SCREEN_ENTITY_PREFIX) + player_id;
    auto* screen = getComponentFor(entity_id);
    if (!screen || !screen->is_open) return false;

    float clamped = std::max(0.0f, std::min(1.0f, value));
    screen->appearance_sliders[feature] = clamped;
    return true;
}

bool CharacterCreationScreenSystem::validateSelections(const std::string& player_id) const {
    std::string entity_id = std::string(SCREEN_ENTITY_PREFIX) + player_id;
    const auto* screen = getComponentFor(entity_id);
    if (!screen || !screen->is_open) return false;

    // Must have race and faction selected
    if (screen->selected_race.empty()) return false;
    if (screen->selected_faction.empty()) return false;

    return true;
}

bool CharacterCreationScreenSystem::finalizeCharacter(const std::string& player_id,
                                                       const std::string& character_name) {
    if (character_name.empty()) return false;
    if (!validateSelections(player_id)) return false;

    std::string entity_id = std::string(SCREEN_ENTITY_PREFIX) + player_id;
    auto* screen = getComponentFor(entity_id);
    if (!screen || !screen->is_open) return false;

    screen->character_name = character_name;
    screen->finalized = true;
    screen->is_open = false;
    return true;
}

bool CharacterCreationScreenSystem::isScreenOpen(const std::string& player_id) const {
    std::string entity_id = std::string(SCREEN_ENTITY_PREFIX) + player_id;
    const auto* screen = getComponentFor(entity_id);
    return screen && screen->is_open;
}

std::string CharacterCreationScreenSystem::getSelectedRace(const std::string& player_id) const {
    std::string entity_id = std::string(SCREEN_ENTITY_PREFIX) + player_id;
    const auto* screen = getComponentFor(entity_id);
    return screen ? screen->selected_race : "";
}

std::string CharacterCreationScreenSystem::getSelectedFaction(const std::string& player_id) const {
    std::string entity_id = std::string(SCREEN_ENTITY_PREFIX) + player_id;
    const auto* screen = getComponentFor(entity_id);
    return screen ? screen->selected_faction : "";
}

} // namespace systems
} // namespace atlas
