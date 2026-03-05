#include "systems/menu_system.h"
#include "ecs/world.h"

namespace atlas {
namespace systems {

MenuSystem::MenuSystem(ecs::World* world)
    : SingleComponentSystem(world) {
}

void MenuSystem::updateComponent(ecs::Entity& /*entity*/, components::MenuState& menu, float delta_time) {
    if (menu.transition_active) {
        menu.transition_timer -= delta_time;
        if (menu.transition_timer <= 0.0f) {
            menu.transition_timer = 0.0f;
            menu.transition_active = false;
        }
    }
}

bool MenuSystem::navigateTo(const std::string& entity_id, components::MenuState::Screen target) {
    auto* menu = getComponentFor(entity_id);
    if (!menu) return false;

    menu->previous_screen = menu->current_screen;
    menu->current_screen = target;
    menu->transition_timer = transition_duration_;
    menu->transition_active = true;
    return true;
}

bool MenuSystem::goBack(const std::string& entity_id) {
    auto* menu = getComponentFor(entity_id);
    if (!menu) return false;

    auto target = menu->previous_screen;
    menu->previous_screen = menu->current_screen;
    menu->current_screen = target;
    menu->transition_timer = transition_duration_;
    menu->transition_active = true;
    return true;
}

components::MenuState::Screen MenuSystem::getCurrentScreen(const std::string& entity_id) const {
    const auto* menu = getComponentFor(entity_id);
    if (!menu) return components::MenuState::Screen::TitleScreen;
    return menu->current_screen;
}

bool MenuSystem::isInGame(const std::string& entity_id) const {
    return getCurrentScreen(entity_id) == components::MenuState::Screen::InGame;
}

} // namespace systems
} // namespace atlas
