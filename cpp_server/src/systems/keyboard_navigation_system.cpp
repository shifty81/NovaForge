#include "systems/keyboard_navigation_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

KeyboardNavigationSystem::KeyboardNavigationSystem(ecs::World* world)
    : SingleComponentSystem(world) {
}

void KeyboardNavigationSystem::updateComponent(ecs::Entity& /*entity*/,
                                                components::KeyboardNavigation& nav,
                                                float delta_time) {
    // Update cursor blink timer
    nav.cursor_blink_timer += delta_time;
    if (nav.cursor_blink_timer >= 1.0f) {
        nav.cursor_blink_timer -= 1.0f;
        nav.cursor_visible = !nav.cursor_visible;
    }
}

bool KeyboardNavigationSystem::initializeNavigation(const std::string& entity_id,
                                                     const std::string& owner_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* existing = entity->getComponent<components::KeyboardNavigation>();
    if (existing) return false;

    auto comp = std::make_unique<components::KeyboardNavigation>();
    comp->nav_id = entity_id;
    comp->owner_id = owner_id;
    entity->addComponent(std::move(comp));
    return true;
}

bool KeyboardNavigationSystem::setFocusPanel(const std::string& entity_id,
                                              const std::string& panel_id) {
    auto* nav = getComponentFor(entity_id);
    if (!nav) return false;

    nav->active_panel_id = panel_id;
    nav->focus_index = 0;
    return true;
}

bool KeyboardNavigationSystem::moveFocus(const std::string& entity_id,
                                          const std::string& direction) {
    auto* nav = getComponentFor(entity_id);
    if (!nav) return false;

    if (nav->tab_order.empty()) return false;

    int max_index = static_cast<int>(nav->tab_order.size()) - 1;

    if (direction == "Up" || direction == "Left") {
        nav->focus_index = std::max(0, nav->focus_index - 1);
    } else if (direction == "Down" || direction == "Right") {
        nav->focus_index = std::min(max_index, nav->focus_index + 1);
    } else {
        return false;
    }
    return true;
}

bool KeyboardNavigationSystem::activateFocus(const std::string& entity_id) {
    auto* nav = getComponentFor(entity_id);
    if (!nav) return false;

    if (nav->tab_order.empty()) return false;
    if (nav->focus_index < 0 || nav->focus_index >= static_cast<int>(nav->tab_order.size())) {
        return false;
    }

    return true;
}

bool KeyboardNavigationSystem::pushFocusStack(const std::string& entity_id,
                                               const std::string& panel_id) {
    auto* nav = getComponentFor(entity_id);
    if (!nav) return false;

    nav->focus_stack.push_back(nav->active_panel_id);
    nav->active_panel_id = panel_id;
    nav->focus_index = 0;
    return true;
}

bool KeyboardNavigationSystem::popFocusStack(const std::string& entity_id) {
    auto* nav = getComponentFor(entity_id);
    if (!nav) return false;

    if (nav->focus_stack.empty()) return false;

    nav->active_panel_id = nav->focus_stack.back();
    nav->focus_stack.pop_back();
    nav->focus_index = 0;
    return true;
}

bool KeyboardNavigationSystem::bindKey(const std::string& entity_id,
                                        const std::string& key,
                                        const std::string& action) {
    auto* nav = getComponentFor(entity_id);
    if (!nav) return false;

    nav->key_bindings[key] = action;
    return true;
}

std::string KeyboardNavigationSystem::getActivePanel(const std::string& entity_id) const {
    const auto* nav = getComponentFor(entity_id);
    if (!nav) return "";

    return nav->active_panel_id;
}

int KeyboardNavigationSystem::getFocusIndex(const std::string& entity_id) const {
    const auto* nav = getComponentFor(entity_id);
    if (!nav) return 0;

    return nav->focus_index;
}

bool KeyboardNavigationSystem::isModal(const std::string& entity_id) const {
    const auto* nav = getComponentFor(entity_id);
    if (!nav) return false;

    return nav->is_modal;
}

bool KeyboardNavigationSystem::setModal(const std::string& entity_id, bool modal,
                                         const std::string& modal_panel_id) {
    auto* nav = getComponentFor(entity_id);
    if (!nav) return false;

    nav->is_modal = modal;
    nav->modal_panel_id = modal ? modal_panel_id : "";
    return true;
}

bool KeyboardNavigationSystem::handleKeyInput(const std::string& entity_id,
                                               const std::string& key) {
    auto* nav = getComponentFor(entity_id);
    if (!nav) return false;

    // Check for key binding
    std::string action = nav->findBinding(key);
    if (!action.empty()) {
        // Process bound action
        if (action == "move_up") return moveFocus(entity_id, "Up");
        if (action == "move_down") return moveFocus(entity_id, "Down");
        if (action == "move_left") return moveFocus(entity_id, "Left");
        if (action == "move_right") return moveFocus(entity_id, "Right");
        if (action == "activate") return activateFocus(entity_id);
        return true;
    }

    // Buffer unbound key input
    nav->input_buffer += key;
    return true;
}

} // namespace systems
} // namespace atlas
