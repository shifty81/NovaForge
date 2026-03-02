#ifndef NOVAFORGE_SYSTEMS_KEYBOARD_NAVIGATION_SYSTEM_H
#define NOVAFORGE_SYSTEMS_KEYBOARD_NAVIGATION_SYSTEM_H

#include "ecs/system.h"
#include "components/ui_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Keyboard-first navigation system (Phase 15)
 *
 * Manages keyboard focus, key bindings, tab order, modal state,
 * and cursor blinking for keyboard-driven UI navigation.
 */
class KeyboardNavigationSystem : public ecs::System {
public:
    explicit KeyboardNavigationSystem(ecs::World* world);
    ~KeyboardNavigationSystem() override = default;

    void update(float delta_time) override;
    std::string getName() const override { return "KeyboardNavigationSystem"; }

    // Initialization
    bool initializeNavigation(const std::string& entity_id, const std::string& owner_id);

    // Focus management
    bool setFocusPanel(const std::string& entity_id, const std::string& panel_id);
    bool moveFocus(const std::string& entity_id, const std::string& direction);
    bool activateFocus(const std::string& entity_id);
    bool pushFocusStack(const std::string& entity_id, const std::string& panel_id);
    bool popFocusStack(const std::string& entity_id);

    // Key bindings
    bool bindKey(const std::string& entity_id, const std::string& key,
                 const std::string& action);

    // Query
    std::string getActivePanel(const std::string& entity_id) const;
    int getFocusIndex(const std::string& entity_id) const;
    bool isModal(const std::string& entity_id) const;

    // Modal
    bool setModal(const std::string& entity_id, bool modal,
                  const std::string& modal_panel_id);

    // Input
    bool handleKeyInput(const std::string& entity_id, const std::string& key);
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_KEYBOARD_NAVIGATION_SYSTEM_H
