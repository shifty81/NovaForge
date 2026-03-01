#ifndef NOVAFORGE_COMPONENTS_FLEET_COMPONENTS_H
#define NOVAFORGE_COMPONENTS_FLEET_COMPONENTS_H

#include "ecs/component.h"
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <cstdint>
#include <cmath>

namespace atlas {
namespace components {

/**
 * @brief Fleet membership for an entity (attached to each fleet member)
 *
 * Tracks which fleet a player belongs to, their role, and any
 * active fleet bonuses being applied.
 */
class FleetMembership : public ecs::Component {
public:
    std::string fleet_id;
    std::string role = "Member";  // "FleetCommander", "WingCommander", "SquadCommander", "Member"
    std::string squad_id;
    std::string wing_id;
    std::map<std::string, float> active_bonuses;  // e.g. "armor_hp_bonus" -> 0.10
    
    COMPONENT_TYPE(FleetMembership)
};
/**
 * @brief Personality axes for AI fleet captains
 *
 * Eight axes capturing both behavioural style and deeper psychology.
 * The original four (aggression, sociability, optimism, professionalism)
 * describe observable behaviour; the new four (loyalty, paranoia, ambition,
 * adaptability) drive long-term decision making and relationship dynamics.
 */
class CaptainPersonality : public ecs::Component {
public:
    // Behavioural axes
    float aggression = 0.5f;        // 0=cautious, 1=bold
    float sociability = 0.5f;       // 0=quiet, 1=talkative
    float optimism = 0.5f;          // 0=grim, 1=hopeful
    float professionalism = 0.5f;   // 0=casual, 1=formal

    // Psychological axes (Phase 1)
    float loyalty = 0.5f;           // 0=self-serving, 1=devoted to fleet
    float paranoia = 0.5f;          // 0=trusting, 1=suspicious/cautious
    float ambition = 0.5f;          // 0=content, 1=driven/power-seeking
    float adaptability = 0.5f;      // 0=rigid, 1=flexible

    std::string captain_name;
    std::string faction;            // Solari, Veyren, Aurelian, Keldari

    COMPONENT_TYPE(CaptainPersonality)
};

/**
 * @brief Tracks fleet morale for an entity
 */
class FleetMorale : public ecs::Component {
public:
    float morale_score = 0.0f;              // clamped -100 to +100
    int wins = 0;
    int losses = 0;
    int ships_lost = 0;
    int times_saved_by_player = 0;
    int times_player_saved = 0;
    int missions_together = 0;
    std::string morale_state = "Steady";    // Inspired/Steady/Doubtful/Disengaged

    void updateMoraleScore() {
        morale_score = static_cast<float>(wins) * 1.0f
                     - static_cast<float>(losses) * 1.5f
                     - static_cast<float>(ships_lost) * 2.0f
                     + static_cast<float>(times_saved_by_player) * 1.2f;
        morale_score = std::clamp(morale_score, -100.0f, 100.0f);
        if (morale_score >= 50.0f) {
            morale_state = "Inspired";
        } else if (morale_score >= 0.0f) {
            morale_state = "Steady";
        } else if (morale_score >= -50.0f) {
            morale_state = "Doubtful";
        } else {
            morale_state = "Disengaged";
        }
    }

    COMPONENT_TYPE(FleetMorale)
};

/**
 * @brief Social graph for fleet captains
 */
class CaptainRelationship : public ecs::Component {
public:
    struct Relationship {
        std::string other_captain_id;
        float affinity = 0.0f;  // -100 to +100
    };

    std::vector<Relationship> relationships;

    float getAffinityWith(const std::string& id) const {
        for (const auto& r : relationships) {
            if (r.other_captain_id == id) {
                return r.affinity;
            }
        }
        return 0.0f;
    }

    void modifyAffinity(const std::string& id, float change) {
        for (auto& r : relationships) {
            if (r.other_captain_id == id) {
                r.affinity = std::clamp(r.affinity + change, -100.0f, 100.0f);
                return;
            }
        }
        Relationship rel;
        rel.other_captain_id = id;
        rel.affinity = std::clamp(change, -100.0f, 100.0f);
        relationships.push_back(rel);
    }

    COMPONENT_TYPE(CaptainRelationship)
};

/**
 * @brief Long-term emotional arcs
 */
class EmotionalState : public ecs::Component {
public:
    float confidence = 50.0f;       // 0-100
    float trust_in_player = 50.0f;  // 0-100
    float fatigue = 0.0f;           // 0-100
    float hope = 50.0f;             // 0-100

    COMPONENT_TYPE(EmotionalState)
};
/**
 * @brief Chatter state for an entity
 */
class FleetChatterState : public ecs::Component {
public:
    float chatter_cooldown = 0.0f;
    bool is_speaking = false;
    float priority = 0.0f;
    std::string current_activity = "Idle";  // Warp/Mining/Combat/Travel/Idle
    std::string last_line_spoken;
    int lines_spoken_total = 0;

    // Interruptible chatter support (Phase 9)
    float speaking_priority = 0.0f;   // priority of the line currently being spoken
    bool was_interrupted = false;     // true if last speech was cut off

    COMPONENT_TYPE(FleetChatterState)
};

/**
 * @brief Rumors heard/witnessed by a captain
 */
class RumorLog : public ecs::Component {
public:
    struct Rumor {
        std::string rumor_id;
        std::string text;
        float belief_strength = 0.5f;
        bool personally_witnessed = false;
        int times_heard = 0;
    };

    std::vector<Rumor> rumors;

    bool hasRumor(const std::string& id) const {
        for (const auto& r : rumors) {
            if (r.rumor_id == id) {
                return true;
            }
        }
        return false;
    }

    void addRumor(const std::string& id, const std::string& text, bool witnessed) {
        for (auto& r : rumors) {
            if (r.rumor_id == id) {
                r.times_heard++;
                return;
            }
        }
        Rumor rumor;
        rumor.rumor_id = id;
        rumor.text = text;
        rumor.belief_strength = 0.5f;
        rumor.personally_witnessed = witnessed;
        rumor.times_heard = 1;
        rumors.push_back(rumor);
    }

    COMPONENT_TYPE(RumorLog)
};

/**
 * @brief Aggregated fleet cargo
 */
class FleetCargoPool : public ecs::Component {
public:
    uint64_t total_capacity = 0;
    uint64_t used_capacity = 0;
    std::map<std::string, uint64_t> pooled_items;   // item_type -> quantity
    std::vector<std::string> contributor_ship_ids;

    COMPONENT_TYPE(FleetCargoPool)
};
/**
 * @brief Fleet formation assignment for an entity
 *
 * Stores the formation type the fleet is using and this member's
 * computed offset relative to the fleet commander.
 */
class FleetFormation : public ecs::Component {
public:
    enum class FormationType { None, Arrow, Line, Wedge, Spread, Diamond };

    FormationType formation = FormationType::Arrow;
    int slot_index = 0;           // position within the formation (0 = leader)
    float offset_x = 0.0f;       // metres relative to commander
    float offset_y = 0.0f;
    float offset_z = 0.0f;

    // Relationship-driven spacing modifier (Phase 9)
    // 1.0 = normal spacing, <1.0 = closer (friends), >1.0 = wider (grudges)
    float spacing_modifier = 1.0f;

    COMPONENT_TYPE(FleetFormation)
};

/**
 * @brief Persistent memory for an AI fleet captain
 *
 * Records significant events so the captain can reference them in chatter
 * and factor them into personality-driven decisions.
 */
class CaptainMemory : public ecs::Component {
public:
    struct MemoryEntry {
        std::string event_type;     // "combat_win", "combat_loss", "ship_lost", "saved_by_player", "warp_anomaly"
        std::string context;        // free-form detail (e.g. enemy name)
        float timestamp = 0.0f;     // in-game seconds since session start
        float emotional_weight = 0.0f; // -1=traumatic, +1=uplifting
    };

    std::vector<MemoryEntry> memories;
    int max_memories = 50;          // cap to prevent unbounded growth

    void addMemory(const std::string& event, const std::string& ctx,
                   float time, float weight) {
        if (static_cast<int>(memories.size()) >= max_memories) {
            memories.erase(memories.begin()); // drop oldest
        }
        memories.push_back({event, ctx, time, weight});
    }

    int countByType(const std::string& event_type) const {
        int n = 0;
        for (const auto& m : memories) {
            if (m.event_type == event_type) ++n;
        }
        return n;
    }

    float averageWeight() const {
        if (memories.empty()) return 0.0f;
        float sum = 0.0f;
        for (const auto& m : memories) sum += m.emotional_weight;
        return sum / static_cast<float>(memories.size());
    }

    COMPONENT_TYPE(CaptainMemory)
};
// ==================== Phase 9: Rumor-to-Questline, Departure, Transfer ====================

class RumorQuestline : public ecs::Component {
public:
    std::string rumor_id;
    std::string questline_id;
    int required_confirmations = 3;
    bool graduated = false;
    std::string quest_description;

    COMPONENT_TYPE(RumorQuestline)
};

class CaptainDepartureState : public ecs::Component {
public:
    enum class DeparturePhase { None, Grumbling, FormalRequest, Departing };

    DeparturePhase phase = DeparturePhase::None;
    float disagreement_score = 0.0f;
    float grumble_threshold = 5.0f;
    float request_threshold = 10.0f;
    float departure_timer = 0.0f;
    float departure_delay = 120.0f;
    bool player_acknowledged = false;

    COMPONENT_TYPE(CaptainDepartureState)
};

class CaptainTransferRequest : public ecs::Component {
public:
    enum class TransferType { BiggerShip, EscortOnly, RoleChange };

    TransferType request_type = TransferType::BiggerShip;
    bool request_pending = false;
    std::string requested_ship_class;
    std::string requested_role;
    float morale_at_request = 0.0f;

    COMPONENT_TYPE(CaptainTransferRequest)
};
// ==================== Phase 11: Fleet History ====================

struct FleetHistoryEntry {
    std::string event_type;
    std::string description;
    float timestamp = 0.0f;
    std::string involved_entity_id;
};

class FleetHistory : public ecs::Component {
public:
    std::vector<FleetHistoryEntry> events;
    int max_events = 100;

    void addEvent(const std::string& type, const std::string& desc,
                  float ts, const std::string& entity_id) {
        FleetHistoryEntry entry;
        entry.event_type = type;
        entry.description = desc;
        entry.timestamp = ts;
        entry.involved_entity_id = entity_id;
        events.push_back(entry);
        while (static_cast<int>(events.size()) > max_events) {
            events.erase(events.begin());
        }
    }

    COMPONENT_TYPE(FleetHistory)
};
// ==================== Phase 11: Fleet-as-Civilization ====================

/**
 * @brief Fleet progression stage tracking
 *
 * Tracks fleet growth through 3 stages:
 *   Early (max 5 ships)  — Player + 4 captains, basic personalities + chatter
 *   Mid   (max 15 ships) — 3 wings × 5, wing commanders, role specialization
 *   End   (max 25 ships) — 5 wings × 5, full doctrine (mining, salvage, logistics, escort, construction)
 */
class FleetProgression : public ecs::Component {
public:
    enum class Stage { Early, Mid, End };

    Stage stage = Stage::Early;
    int max_ships = 5;
    int max_wings = 1;
    int current_ship_count = 1;       // starts with player ship
    int ships_per_wing = 5;

    // Stage thresholds (measured by cumulative fleet experience)
    float fleet_experience = 0.0f;    // accumulated from missions, combat, etc.
    float mid_threshold = 100.0f;     // XP required to unlock Mid stage
    float end_threshold = 500.0f;     // XP required to unlock End stage

    // Role specialization unlocks (Mid stage+)
    bool mining_wing_unlocked = false;
    bool combat_wing_unlocked = false;
    bool logistics_wing_unlocked = false;
    bool salvage_wing_unlocked = false;   // End stage
    bool construction_wing_unlocked = false; // End stage

    void updateStage() {
        if (fleet_experience >= end_threshold) {
            stage = Stage::End;
            max_ships = 25;
            max_wings = 5;
        } else if (fleet_experience >= mid_threshold) {
            stage = Stage::Mid;
            max_ships = 15;
            max_wings = 3;
        } else {
            stage = Stage::Early;
            max_ships = 5;
            max_wings = 1;
        }
    }

    COMPONENT_TYPE(FleetProgression)
};

/**
 * @brief Station deployment component
 *
 * Attached to ships capable of deploying into permanent stations.
 * Tracks deployment state, attached modules, and system upgrade effects.
 */
class StationDeployment : public ecs::Component {
public:
    enum class DeployState { Mobile, Deploying, Deployed };

    DeployState deploy_state = DeployState::Mobile;
    float deploy_timer = 0.0f;          // seconds remaining in deployment
    float deploy_duration = 300.0f;     // 5 minutes to deploy

    // Location (set on deployment)
    std::string system_id;
    float deploy_x = 0.0f;
    float deploy_y = 0.0f;
    float deploy_z = 0.0f;

    // Attached station modules (module_type -> count)
    std::map<std::string, int> attached_modules;
    int max_module_slots = 3;

    // System upgrading effects
    float security_bonus = 0.0f;        // +security to the system
    float economy_bonus = 0.0f;         // +economy to the system
    float resource_bonus = 0.0f;        // +resource availability

    int getTotalAttachedModules() const {
        int total = 0;
        for (const auto& pair : attached_modules)
            total += pair.second;
        return total;
    }

    bool canAttachModule() const {
        return getTotalAttachedModules() < max_module_slots;
    }

    COMPONENT_TYPE(StationDeployment)
};

/**
 * @brief Fleet warp formation state
 *
 * Extends FleetFormation with warp-specific behavior:
 * - Formation type selection based on ship class
 * - Breathing oscillation for organic feel (0.02–0.05 Hz)
 * - Visual interaction data (distortion bending, wake ripples)
 */
class FleetWarpState : public ecs::Component {
public:
    enum class WarpFormationType { TightEchelon, LooseDiamond, WideCapital };

    WarpFormationType warp_formation = WarpFormationType::TightEchelon;
    bool in_fleet_warp = false;

    // Breathing oscillation (organic feel)
    float breathing_frequency = 0.03f;  // Hz (0.02–0.05)
    float breathing_amplitude = 0.1f;   // fraction of spacing
    float breathing_phase = 0.0f;       // current phase (radians)

    // Visual interaction
    float distortion_bend = 0.0f;       // warp distortion bending factor (mass-based)
    float wake_ripple = 0.0f;           // wake ripple intensity

    // Ship class for formation selection
    std::string ship_class = "Frigate";

    float computeBreathingOffset(float elapsed_time) const {
        static constexpr float kTwoPi = 6.2831853f;
        float phase = breathing_phase + elapsed_time * breathing_frequency * kTwoPi;
        return breathing_amplitude * std::sin(phase);
    }

    COMPONENT_TYPE(FleetWarpState)
};

/**
 * @brief Fleet civilization-scale tracking
 *
 * Tracks the metrics required for a fleet to reach civilization status:
 * titan threshold, fleet stability, and persistent history.
 */
class FleetCivilization : public ecs::Component {
public:
    // Titan threshold requirements
    bool has_stable_logistics = false;     // sustained supply chain
    bool has_loyal_captains = false;       // average morale >= "Steady"
    bool has_fleet_history = false;        // >= 20 missions completed together
    bool has_fleet_industry = false;       // fleet-scale manufacturing active

    // Civilization metrics
    int total_missions_completed = 0;
    int total_stations_deployed = 0;
    float fleet_stability_score = 0.0f;   // 0-100 stability index
    float economic_output = 0.0f;         // cumulative fleet economic value

    // Persistent history
    int major_events_count = 0;
    int ships_ever_owned = 0;
    int captains_ever_served = 0;

    bool isCivilizationThresholdMet() const {
        return has_stable_logistics && has_loyal_captains &&
               has_fleet_history && has_fleet_industry;
    }

    COMPONENT_TYPE(FleetCivilization)
};


} // namespace components
} // namespace atlas

#endif // NOVAFORGE_COMPONENTS_FLEET_COMPONENTS_H
