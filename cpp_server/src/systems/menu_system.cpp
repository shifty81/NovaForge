#include "systems/menu_system.h"
#include "ecs/world.h"

namespace atlas {
namespace systems {

MenuSystem::MenuSystem(ecs::World* world)
    : System(world) {
}

void MenuSystem::update(float delta_time) {
    auto entities = world_->getEntities<components::MenuState>();
    for (auto* entity : entities) {
        auto* menu = entity->getComponent<components::MenuState>();
        if (!menu) continue;

        if (menu->transition_active) {
            menu->transition_timer -= delta_time;
            if (menu->transition_timer <= 0.0f) {
                menu->transition_timer = 0.0f;
                menu->transition_active = false;
            }
        }
    }
}

bool MenuSystem::navigateTo(const std::string& entity_id, components::MenuState::Screen target) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto* menu = entity->getComponent<components::MenuState>();
    if (!menu) return false;

    menu->previous_screen = menu->current_screen;
    menu->current_screen = target;
    menu->transition_timer = transition_duration_;
    menu->transition_active = true;
    return true;
}

bool MenuSystem::goBack(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto* menu = entity->getComponent<components::MenuState>();
    if (!menu) return false;

    auto target = menu->previous_screen;
    menu->previous_screen = menu->current_screen;
    menu->current_screen = target;
    menu->transition_timer = transition_duration_;
    menu->transition_active = true;
    return true;
}

components::MenuState::Screen MenuSystem::getCurrentScreen(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return components::MenuState::Screen::TitleScreen;
    auto* menu = entity->getComponent<components::MenuState>();
    if (!menu) return components::MenuState::Screen::TitleScreen;
    return menu->current_screen;
}

bool MenuSystem::isInGame(const std::string& entity_id) const {
    return getCurrentScreen(entity_id) == components::MenuState::Screen::InGame;
}

} // namespace systems
} // namespace atlas
