#ifndef NOVAFORGE_SYSTEMS_MENU_SYSTEM_H
#define NOVAFORGE_SYSTEMS_MENU_SYSTEM_H

#include "ecs/single_component_system.h"
#include "ecs/entity.h"
#include "components/game_components.h"
#include <string>

namespace atlas {
namespace systems {

class MenuSystem : public ecs::SingleComponentSystem<components::MenuState> {
public:
    explicit MenuSystem(ecs::World* world);
    ~MenuSystem() override = default;

    std::string getName() const override { return "MenuSystem"; }

    bool navigateTo(const std::string& entity_id, components::MenuState::Screen target);
    bool goBack(const std::string& entity_id);
    components::MenuState::Screen getCurrentScreen(const std::string& entity_id) const;
    bool isInGame(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity, components::MenuState& menu, float delta_time) override;

private:
    float transition_duration_ = 0.5f;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_MENU_SYSTEM_H
