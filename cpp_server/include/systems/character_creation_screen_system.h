#ifndef NOVAFORGE_SYSTEMS_CHARACTER_CREATION_SCREEN_SYSTEM_H
#define NOVAFORGE_SYSTEMS_CHARACTER_CREATION_SCREEN_SYSTEM_H

#include "ecs/system.h"
#include "ecs/entity.h"
#include "components/game_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Manages the character creation screen state
 *
 * Tracks the server-side state of the character creation screen including
 * race/faction selection, attribute sliders, appearance customization,
 * and final validation before character finalization.
 */
class CharacterCreationScreenSystem : public ecs::System {
public:
    explicit CharacterCreationScreenSystem(ecs::World* world);
    ~CharacterCreationScreenSystem() override = default;

    void update(float delta_time) override;
    std::string getName() const override { return "CharacterCreationScreenSystem"; }

    /**
     * @brief Open the character creation screen for a player
     * @return true if screen was opened successfully
     */
    bool openScreen(const std::string& player_id);

    /**
     * @brief Select a race for the character
     */
    bool selectRace(const std::string& player_id, const std::string& race);

    /**
     * @brief Select a faction for the character
     */
    bool selectFaction(const std::string& player_id, const std::string& faction);

    /**
     * @brief Set an attribute slider value (0.0 - 1.0)
     */
    bool setAttributeSlider(const std::string& player_id, const std::string& attribute, float value);

    /**
     * @brief Get an attribute slider value
     */
    float getAttributeSlider(const std::string& player_id, const std::string& attribute) const;

    /**
     * @brief Set an appearance slider value (0.0 - 1.0)
     */
    bool setAppearanceSlider(const std::string& player_id, const std::string& feature, float value);

    /**
     * @brief Validate all selections and check if character can be finalized
     */
    bool validateSelections(const std::string& player_id) const;

    /**
     * @brief Finalize the character and close the creation screen
     * @return true if character was finalized successfully
     */
    bool finalizeCharacter(const std::string& player_id, const std::string& character_name);

    /**
     * @brief Check if the screen is open for a player
     */
    bool isScreenOpen(const std::string& player_id) const;

    /**
     * @brief Get the currently selected race
     */
    std::string getSelectedRace(const std::string& player_id) const;

    /**
     * @brief Get the currently selected faction
     */
    std::string getSelectedFaction(const std::string& player_id) const;

private:
    static constexpr const char* SCREEN_ENTITY_PREFIX = "char_screen_";
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_CHARACTER_CREATION_SCREEN_SYSTEM_H
