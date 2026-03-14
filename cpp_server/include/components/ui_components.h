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

// ==================== Overview Filter ====================

/**
 * @brief Overview window entity filtering and column sorting
 *
 * Manages overview tab presets with type/distance/standing filters.
 * Supports column-based sorting and entry count limits.
 */
class OverviewFilter : public ecs::Component {
public:
    struct FilterPreset {
        std::string preset_id;
        std::string name;
        std::vector<std::string> shown_types;   // "Ship", "NPC", "Wreck", "Celestial", "Drone", "Structure"
        float max_distance = 0.0f;              // 0 = unlimited
        bool show_friendly = true;
        bool show_neutral = true;
        bool show_hostile = true;
    };

    struct OverviewEntry {
        std::string entity_id;
        std::string name;
        std::string type;
        float distance = 0.0f;
        float angular_velocity = 0.0f;
        std::string standing;    // "Friendly", "Neutral", "Hostile"
        bool is_locked = false;
    };

    std::vector<FilterPreset> presets;
    std::vector<OverviewEntry> entries;
    std::string active_preset_id;
    std::string sort_column = "distance";  // "name", "type", "distance", "angular_velocity"
    bool sort_ascending = true;
    int max_presets = 10;
    int max_entries = 200;
    int total_entries_filtered = 0;
    float update_interval = 1.0f;
    float update_timer = 0.0f;
    bool active = true;

    COMPONENT_TYPE(OverviewFilter)
};

// ==================== Shield Arc HUD ====================

/**
 * @brief Circular shield/armor/hull arc display state for HUD
 *
 * Renders concentric arcs (shield outermost, hull innermost) that
 * deplete clockwise from 12 o'clock.  Used by the Ship HUD Control Ring.
 */
class ShieldArcHud : public ecs::Component {
public:
    std::string owner_id;

    float shield_percent = 100.0f;   // 0-100
    float armor_percent  = 100.0f;
    float hull_percent   = 100.0f;

    // Visual state
    float critical_threshold = 25.0f;  // percent below which a layer is critical
    bool  shield_critical = false;   // shield < critical_threshold
    bool  armor_critical  = false;
    bool  hull_critical   = false;
    float flash_timer     = 0.0f;    // pulse animation timer
    float flash_interval  = 0.5f;    // seconds between flashes when critical
    bool  visible         = true;
    bool  active          = true;

    COMPONENT_TYPE(ShieldArcHud)
};

// ==================== Capacitor HUD Bar ====================

/**
 * @brief Vertical capacitor bar state for HUD
 *
 * Displays a vertical bar showing current vs max capacitor with
 * color transitions: green → yellow → red as charge depletes.
 */
class CapacitorHudBar : public ecs::Component {
public:
    std::string owner_id;

    float current    = 100.0f;
    float maximum    = 100.0f;
    float percent    = 100.0f;       // derived: current / maximum * 100

    // Color thresholds (percent values)
    float green_threshold  = 50.0f;  // strictly above this → green
    float yellow_threshold = 25.0f;  // strictly above this → yellow, else red

    enum class BarColor { Green, Yellow, Red };
    int color_state = 0;             // 0=Green, 1=Yellow, 2=Red

    bool  warning_active = false;    // true when below yellow threshold
    float drain_rate     = 0.0f;     // GJ/s currently being consumed
    bool  visible        = true;
    bool  active         = true;

    COMPONENT_TYPE(CapacitorHudBar)
};

// ==================== Velocity Arc HUD ====================

/**
 * @brief Velocity arc indicator for HUD
 *
 * Shows current speed as a sweeping arc with color-coded states:
 * green (normal), yellow (approaching max), red (max / overloaded).
 */
class VelocityArcHud : public ecs::Component {
public:
    std::string owner_id;

    float current_speed = 0.0f;
    float max_speed     = 100.0f;
    float speed_percent = 0.0f;      // derived: current / max * 100

    enum class SpeedState { Idle, Normal, Approaching, AtMax };
    int   speed_state = 0;           // 0=Idle, 1=Normal, 2=Approaching, 3=AtMax

    float approach_threshold = 80.0f; // percent → Approaching state
    float idle_threshold     = 1.0f;  // speed below this → Idle

    float warp_prep_progress = 0.0f;  // 0-1, warp charge-up visualisation
    bool  afterburner_active = false;
    bool  visible            = true;
    bool  active             = true;

    COMPONENT_TYPE(VelocityArcHud)
};

// ==================== Alert Stack HUD ====================

/**
 * @brief Warning / notification alert stack for HUD
 *
 * Manages a stack of timed alerts (shield low, cargo full, etc.)
 * ordered by priority.  Oldest alerts expire automatically.
 */
class AlertStackHud : public ecs::Component {
public:
    enum class AlertLevel { Info = 0, Warning = 1, Critical = 2 };

    struct Alert {
        int    id       = 0;
        int    level    = 0;      // AlertLevel as int
        std::string message;
        float  lifetime     = 5.0f;
        float  max_lifetime = 5.0f;
        bool   persistent   = false;   // if true, never auto-expires
        bool   dismissed    = false;
    };

    std::vector<Alert> alerts;
    int  next_alert_id  = 1;
    int  max_alerts     = 10;
    int  total_shown    = 0;
    int  total_expired  = 0;
    int  total_dismissed = 0;
    bool visible        = true;
    bool active         = true;

    Alert* findAlert(int alert_id) {
        for (auto& a : alerts) {
            if (a.id == alert_id) return &a;
        }
        return nullptr;
    }

    const Alert* findAlert(int alert_id) const {
        for (const auto& a : alerts) {
            if (a.id == alert_id) return &a;
        }
        return nullptr;
    }

    COMPONENT_TYPE(AlertStackHud)
};

// ==================== Damage Feedback HUD ====================

/**
 * @brief Damage feedback overlay state for HUD
 *
 * Manages screen-space damage effects: shield ripple (blue),
 * armor flash (yellow / orange), and hull shake (red + screen shake).
 * Effects are layered and decay over time.
 */
class DamageFeedbackHud : public ecs::Component {
public:
    struct FeedbackLayer {
        std::string layer_type;        // "shield", "armor", "hull"
        float       intensity  = 0.0f; // 0-1 visual strength
        float       decay_rate = 2.0f; // intensity lost per second
        float       duration   = 0.0f; // seconds since trigger
        bool        active     = false;
    };

    std::string owner_id;

    FeedbackLayer shield_feedback{"shield", 0.0f, 2.0f, 0.0f, false};
    FeedbackLayer armor_feedback {"armor",  0.0f, 1.5f, 0.0f, false};
    FeedbackLayer hull_feedback  {"hull",   0.0f, 1.0f, 0.0f, false};

    float screen_shake_intensity = 0.0f;
    float screen_shake_decay     = 3.0f;

    int   total_shield_hits = 0;
    int   total_armor_hits  = 0;
    int   total_hull_hits   = 0;
    bool  visible           = true;
    bool  active            = true;

    COMPONENT_TYPE(DamageFeedbackHud)
};

// ==================== Relay Clone Installation UI ====================

/**
 * @brief UI state for relay clone installation dialog
 *
 * Drives the "Install Relay Clone" panel that a player uses while docked
 * at a station with a Clone Bay.  Manages the multi-step flow:
 *   1. Select destination station (from a searchable list)
 *   2. Confirm cost (skill-based fee)
 *   3. Await server acknowledgement (pending state)
 *   4. Display success / error message
 * Tracks previously installed clones for display and allows cancellation
 * at any step before the final confirm.
 */
class RelayCloneInstallUiState : public ecs::Component {
public:
    enum class UiStep {
        Idle,           // panel not open
        SelectStation,  // player is browsing station list
        ConfirmCost,    // cost breakdown shown, awaiting confirm
        Pending,        // request sent to server
        Success,        // server confirmed installation
        Error           // server returned an error
    };

    struct StationEntry {
        std::string station_id;
        std::string station_name;
        std::string region;
        float       install_cost = 0.0f;
        bool        available    = true;
    };

    struct InstalledCloneEntry {
        std::string clone_id;
        std::string station_id;
        std::string station_name;
    };

    std::string character_id;
    UiStep      current_step         = UiStep::Idle;
    std::string selected_station_id;
    float       pending_cost         = 0.0f;
    std::string last_error_message;
    std::string station_search_filter;

    std::vector<StationEntry>       available_stations;
    std::vector<InstalledCloneEntry> installed_clones;
    std::vector<StationEntry>       filtered_stations; // driven by search filter

    int   max_stations         = 200;
    int   total_installs       = 0;
    int   total_cancels        = 0;
    float pending_timeout      = 10.0f;  // seconds before timeout error
    float pending_elapsed      = 0.0f;
    bool  panel_open           = false;
    bool  active               = true;

    COMPONENT_TYPE(RelayCloneInstallUiState)
};

// ---------------------------------------------------------------------------
// NotificationState — in-game notification management
// ---------------------------------------------------------------------------
class NotificationState : public ecs::Component {
public:
    enum class NotifType { Info, Warning, Error, Achievement, Combat, Trade };

    struct Notification {
        std::string notif_id;
        std::string message;
        NotifType   type       = NotifType::Info;
        bool        read       = false;
        float       timestamp  = 0.0f;
        float       lifetime   = 60.0f;
    };

    std::vector<Notification> notifications;
    int   max_notifications        = 100;
    int   total_notifications_sent = 0;
    int   total_expired            = 0;
    float elapsed                  = 0.0f;
    bool  active                   = true;

    COMPONENT_TYPE(NotificationState)
};

} // namespace components
} // namespace atlas

#endif // NOVAFORGE_COMPONENTS_UI_COMPONENTS_H
