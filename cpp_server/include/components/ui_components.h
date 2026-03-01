#ifndef NOVAFORGE_COMPONENTS_UI_COMPONENTS_H
#define NOVAFORGE_COMPONENTS_UI_COMPONENTS_H

#include "ecs/component.h"
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <cstdint>
#include <cmath>

namespace atlas {
namespace components {

// ==================== Menu & Game Flow ====================

class MenuState : public ecs::Component {
public:
    enum class Screen {
        TitleScreen,
        NewGame,
        LoadGame,
        ModMenu,
        MultiplayerMenu,
        CharacterCreation,
        ShipSelection,
        HangarSpawn,
        FleetCommand,
        InGame,
        PauseMenu
    };

    Screen current_screen = Screen::TitleScreen;
    Screen previous_screen = Screen::TitleScreen;
    float transition_timer = 0.0f;
    bool transition_active = false;

    COMPONENT_TYPE(MenuState)
};

class MultiplayerSession : public ecs::Component {
public:
    enum class Role { None, Host, Client };

    Role role = Role::None;
    std::string host_address;
    int port = 7777;
    int max_players = 20;
    int connected_players = 0;
    bool mod_validation_passed = false;
    uint64_t world_seed = 0;

    COMPONENT_TYPE(MultiplayerSession)
};
// ==================== Mod Registry ====================

/**
 * @brief Registry of installed mod manifests
 * 
 * Stores all registered mod manifests with their metadata, dependencies,
 * and enabled/disabled state. Used by ModManifestSystem for validation
 * and load ordering.
 */
class ModRegistry : public ecs::Component {
public:
    struct ModInfo {
        std::string mod_id;
        std::string name;
        std::string version;
        std::string author;
        std::vector<std::string> dependencies;
        bool enabled = true;
    };

    std::vector<ModInfo> mods;
    int max_mods = 50;

    ModInfo* findMod(const std::string& mod_id) {
        for (auto& m : mods) {
            if (m.mod_id == mod_id) return &m;
        }
        return nullptr;
    }

    const ModInfo* findMod(const std::string& mod_id) const {
        for (const auto& m : mods) {
            if (m.mod_id == mod_id) return &m;
        }
        return nullptr;
    }

    bool removeMod(const std::string& mod_id) {
        for (auto it = mods.begin(); it != mods.end(); ++it) {
            if (it->mod_id == mod_id) {
                mods.erase(it);
                return true;
            }
        }
        return false;
    }

    COMPONENT_TYPE(ModRegistry)
};
// ==================== Character Creation Screen ====================

/**
 * @brief Server-side state for the character creation screen
 *
 * Tracks race/faction selection, attribute sliders, appearance
 * customization, and validation state during character creation.
 */
class CharacterCreationScreen : public ecs::Component {
public:
    std::string player_id;
    bool is_open = false;
    bool finalized = false;
    float time_open = 0.0f;

    std::string selected_race;
    std::string selected_faction;
    std::string character_name;

    std::map<std::string, float> attribute_sliders;    // attribute_name -> value (0.0 - 1.0)
    std::map<std::string, float> appearance_sliders;   // feature_name -> value (0.0 - 1.0)

    COMPONENT_TYPE(CharacterCreationScreen)
};

// ==================== View Mode State ====================

/**
 * @brief Tracks the current view mode and transition state
 *
 * Manages seamless transitions between Cockpit, Interior, EVA,
 * and RTS Overlay view modes with transition progress tracking.
 */
class ViewModeState : public ecs::Component {
public:
    enum class Mode {
        Cockpit = 0,    // Ship piloting view
        Interior = 1,   // FPS walking inside ship
        EVA = 2,        // Space walk / EVA
        RTSOverlay = 3  // Tactical fleet command overlay
    };

    std::string player_id;
    int current_mode = 0;    // Mode enum as int
    int previous_mode = 0;
    int target_mode = 0;
    bool transitioning = false;
    float transition_progress = 0.0f;
    float transition_duration = 1.5f;
    float cooldown_remaining = 0.0f;

    COMPONENT_TYPE(ViewModeState)
};


} // namespace components
} // namespace atlas

#endif // NOVAFORGE_COMPONENTS_UI_COMPONENTS_H
