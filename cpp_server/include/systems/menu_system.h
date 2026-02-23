#ifndef EVE_SYSTEMS_MENU_SYSTEM_H
#define EVE_SYSTEMS_MENU_SYSTEM_H

#include "ecs/system.h"
#include "ecs/entity.h"
#include "components/game_components.h"
#include <string>

namespace atlas {
namespace systems {

class MenuSystem : public ecs::System {
public:
    explicit MenuSystem(ecs::World* world);
    ~MenuSystem() override = default;

    void update(float delta_time) override;
    std::string getName() const override { return "MenuSystem"; }

    bool navigateTo(const std::string& entity_id, components::MenuState::Screen target);
    bool goBack(const std::string& entity_id);
    components::MenuState::Screen getCurrentScreen(const std::string& entity_id) const;
    bool isInGame(const std::string& entity_id) const;

private:
    float transition_duration_ = 0.5f;
};

} // namespace systems
} // namespace atlas

#endif // EVE_SYSTEMS_MENU_SYSTEM_H
