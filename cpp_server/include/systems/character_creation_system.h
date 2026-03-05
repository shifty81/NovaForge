#ifndef NOVAFORGE_SYSTEMS_CHARACTER_CREATION_SYSTEM_H
#define NOVAFORGE_SYSTEMS_CHARACTER_CREATION_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/game_components.h"
#include <string>

namespace atlas {
namespace systems {

class CharacterCreationSystem : public ecs::SingleComponentSystem<components::CharacterSheet> {
public:
    explicit CharacterCreationSystem(ecs::World* world);
    ~CharacterCreationSystem() override = default;

    std::string getName() const override { return "CharacterCreationSystem"; }

    // Character creation
    bool createCharacter(const std::string& entity_id,
                         const std::string& character_name,
                         const std::string& race,
                         const std::string& bloodline,
                         const std::string& ancestry,
                         const std::string& gender);

    // Implant management
    bool installImplant(const std::string& entity_id,
                        const std::string& implant_id,
                        const std::string& implant_name,
                        int slot,
                        const std::string& attribute_bonus,
                        int bonus_amount);

    bool removeImplant(const std::string& entity_id, int slot);

    // Clone management
    bool setCloneGrade(const std::string& entity_id,
                       const std::string& grade);

    bool setCloneLocation(const std::string& entity_id,
                          const std::string& station_id);

    bool jumpClone(const std::string& entity_id);

    // Attribute queries
    int getEffectiveAttribute(const std::string& entity_id,
                              const std::string& attribute);

    // Security status
    bool modifySecurityStatus(const std::string& entity_id, float delta);

    // Employment
    bool addEmploymentRecord(const std::string& entity_id,
                             const std::string& corp_id,
                             const std::string& corp_name,
                             float join_date);

protected:
    void updateComponent(ecs::Entity& entity, components::CharacterSheet& sheet, float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_CHARACTER_CREATION_SYSTEM_H
