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
    int current_mode = 1;    // Mode enum as int (Interior/FPS)
    int previous_mode = 0;
    int target_mode = 0;
    bool transitioning = false;
    float transition_progress = 0.0f;
    float transition_duration = 1.5f;
    float cooldown_remaining = 0.0f;

    COMPONENT_TYPE(ViewModeState)
};

// ==================== Dock Node Layout ====================

class DockNodeLayout : public ecs::Component {
public:
    std::string layout_id;
    std::string owner_id;

    enum class NodeType { Root, Split, Leaf };
    enum class SplitDirection { Horizontal, Vertical, None };

    struct DockNode {
        std::string node_id;
        NodeType type = NodeType::Leaf;
        SplitDirection direction = SplitDirection::None;
        float split_ratio = 0.5f;
        std::string left_child_id;
        std::string right_child_id;
        std::string window_id;
        float x = 0.0f;
        float y = 0.0f;
        float width = 0.0f;
        float height = 0.0f;
    };

    std::vector<DockNode> nodes;
    std::string root_node_id;
    int max_windows = 20;  // inclusive limit: up to 20 windows allowed
    int total_docks = 0;
    int total_undocks = 0;

    DockNode* findNode(const std::string& nid) {
        for (auto& n : nodes) {
            if (n.node_id == nid) return &n;
        }
        return nullptr;
    }

    const DockNode* findNode(const std::string& nid) const {
        for (const auto& n : nodes) {
            if (n.node_id == nid) return &n;
        }
        return nullptr;
    }

    int countLeaves() const {
        int c = 0;
        for (const auto& n : nodes) {
            if (n.type == NodeType::Leaf && !n.window_id.empty()) c++;
        }
        return c;
    }

    COMPONENT_TYPE(DockNodeLayout)
};

// ==================== Atlas UI Panel ====================

/**
 * @brief UI panel state for inventory, fitting, market, and other panels
 *
 * Tracks panel open/close state, position, size, scroll, selection,
 * filtering, and the items displayed within each panel type.
 */
class AtlasUIPanel : public ecs::Component {
public:
    enum class PanelType {
        Inventory,
        Fitting,
        Market,
        Overview,
        Chat,
        Drone
    };

    struct PanelItem {
        std::string item_id;
        std::string name;
        int quantity = 0;
        float value = 0.0f;
    };

    std::string panel_id;
    std::string owner_id;
    PanelType panel_type = PanelType::Inventory;
    bool is_open = false;
    bool is_docked = false;

    float position_x = 0.0f;
    float position_y = 0.0f;
    float size_w = 300.0f;
    float size_h = 400.0f;

    float scroll_offset = 0.0f;
    int selected_index = -1;
    std::string filter_text;

    std::vector<PanelItem> items;
    int max_items = 100;

    std::string sort_field;
    bool sort_ascending = true;

    PanelItem* findItem(const std::string& item_id) {
        for (auto& i : items) {
            if (i.item_id == item_id) return &i;
        }
        return nullptr;
    }

    const PanelItem* findItem(const std::string& item_id) const {
        for (const auto& i : items) {
            if (i.item_id == item_id) return &i;
        }
        return nullptr;
    }

    // Returns 1 if this panel is open, 0 otherwise (for aggregation across entities)
    int countOpenPanels() const {
        return is_open ? 1 : 0;
    }

    COMPONENT_TYPE(AtlasUIPanel)
};

// ==================== Keyboard Navigation ====================

/**
 * @brief Keyboard-first navigation state for UI panels
 *
 * Manages focus tracking, key bindings, tab order, modal state,
 * and input buffering for keyboard-driven UI navigation.
 */
class KeyboardNavigation : public ecs::Component {
public:
    std::string nav_id;
    std::string owner_id;
    std::string active_panel_id;
    int focus_index = 0;

    std::vector<std::string> focus_stack;
    std::map<std::string, std::string> key_bindings;
    std::vector<std::string> tab_order;

    bool is_modal = false;
    std::string modal_panel_id;
    bool cursor_visible = true;
    float cursor_blink_timer = 0.0f;
    std::string input_buffer;

    std::string findBinding(const std::string& key) const {
        auto it = key_bindings.find(key);
        if (it != key_bindings.end()) return it->second;
        return "";
    }

    COMPONENT_TYPE(KeyboardNavigation)
};

// ==================== Data Binding ====================

/**
 * @brief Observer-pattern data binding for UI widgets
 *
 * Maintains bindings between data sources and UI widgets with
 * dirty tracking, observer notifications, and transform functions.
 */
class DataBinding : public ecs::Component {
public:
    struct Binding {
        std::string binding_id;
        std::string source_path;
        std::string target_widget;
        std::string transform_func;
        std::string last_value;
        bool dirty = false;
    };

    struct Observer {
        std::string observer_id;
        std::string pattern;
        std::string callback_id;
        bool active = true;
    };

    std::string binding_id;
    std::string owner_id;

    std::vector<Binding> bindings;
    std::vector<Observer> observers;
    std::vector<std::string> pending_notifications;

    int max_bindings = 50;
    int total_updates = 0;
    int total_notifications = 0;

    Binding* findBinding(const std::string& bid) {
        for (auto& b : bindings) {
            if (b.binding_id == bid) return &b;
        }
        return nullptr;
    }

    const Binding* findBinding(const std::string& bid) const {
        for (const auto& b : bindings) {
            if (b.binding_id == bid) return &b;
        }
        return nullptr;
    }

    Observer* findObserver(const std::string& oid) {
        for (auto& o : observers) {
            if (o.observer_id == oid) return &o;
        }
        return nullptr;
    }

    const Observer* findObserver(const std::string& oid) const {
        for (const auto& o : observers) {
            if (o.observer_id == oid) return &o;
        }
        return nullptr;
    }

    COMPONENT_TYPE(DataBinding)
};

} // namespace components
} // namespace atlas

#endif // NOVAFORGE_COMPONENTS_UI_COMPONENTS_H
