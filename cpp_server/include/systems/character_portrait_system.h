#ifndef NOVAFORGE_SYSTEMS_CHARACTER_PORTRAIT_SYSTEM_H
#define NOVAFORGE_SYSTEMS_CHARACTER_PORTRAIT_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/game_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Character portrait customization system
 *
 * Manages a character's portrait presets — background, lighting, pose,
 * expression, and camera angle.  Multiple presets can be stored up to a
 * configurable cap (max_presets, default 10).  One preset is designated
 * the active portrait; changing customization fields on-the-fly is
 * supported only on the active preset.
 */
class CharacterPortraitSystem
    : public ecs::SingleComponentSystem<components::CharacterPortrait> {
public:
    explicit CharacterPortraitSystem(ecs::World* world);
    ~CharacterPortraitSystem() override = default;

    std::string getName() const override { return "CharacterPortraitSystem"; }

    // --- Lifecycle ---
    bool initialize(const std::string& entity_id,
                    const std::string& character_name);

    // --- Preset management ---
    bool addPreset(const std::string& entity_id,
                   const std::string& preset_id,
                   const std::string& name,
                   const std::string& background,
                   const std::string& lighting,
                   const std::string& pose,
                   const std::string& expression,
                   float camera_angle);
    bool removePreset(const std::string& entity_id,
                      const std::string& preset_id);
    bool setActivePortrait(const std::string& entity_id,
                           const std::string& preset_id);

    // --- Active portrait editing ---
    bool setBackground(const std::string& entity_id, const std::string& bg);
    bool setLighting(const std::string& entity_id, const std::string& lighting);
    bool setPose(const std::string& entity_id, const std::string& pose);
    bool setExpression(const std::string& entity_id, const std::string& expr);
    bool setCameraAngle(const std::string& entity_id, float angle);

    // --- Queries ---
    int         getPresetCount(const std::string& entity_id) const;
    std::string getActivePresetId(const std::string& entity_id) const;
    std::string getCharacterName(const std::string& entity_id) const;
    int         getTotalUpdates(const std::string& entity_id) const;
    bool        hasPreset(const std::string& entity_id,
                          const std::string& preset_id) const;

protected:
    void updateComponent(ecs::Entity& entity,
                         components::CharacterPortrait& comp,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_CHARACTER_PORTRAIT_SYSTEM_H
