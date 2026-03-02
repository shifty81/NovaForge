#ifndef NOVAFORGE_COMPONENTS_CREW_COMPONENTS_H
#define NOVAFORGE_COMPONENTS_CREW_COMPONENTS_H

#include "ecs/component.h"
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <cstdint>
#include <cmath>

namespace atlas {
namespace components {

// ==================== Survival Module ====================

class SurvivalNeeds : public ecs::Component {
public:
    float oxygen = 100.0f;
    float hunger = 0.0f;
    float fatigue = 0.0f;
    float oxygen_drain_rate = 0.5f;
    float hunger_rate = 0.1f;
    float fatigue_rate = 0.05f;

    bool isAlive() const { return oxygen > 0.0f; }
    bool isStarving() const { return hunger >= 80.0f; }
    bool isExhausted() const { return fatigue >= 80.0f; }

    COMPONENT_TYPE(SurvivalNeeds)
};

class Fabricator : public ecs::Component {
public:
    bool is_active = false;
    std::string current_recipe;
    float progress = 0.0f;
    float craft_speed = 1.0f;
    std::vector<std::string> known_recipes;
    int max_queue = 5;

    COMPONENT_TYPE(Fabricator)
};
// ==================== NPC Crew Simulation ====================

class CrewMember : public ecs::Component {
public:
    enum class CrewRole {
        Engineer, Pilot, Gunner, Medic,
        Scientist, Miner, Cook, Security
    };
    enum class Activity {
        Idle, Working, Walking, Resting, Eating, Repairing, Manning
    };

    CrewRole role = CrewRole::Engineer;
    Activity current_activity = Activity::Idle;
    std::string assigned_room_id;
    std::string current_room_id;
    float skill_level = 1.0f;
    float morale = 50.0f;
    float efficiency_bonus = 0.0f;

    COMPONENT_TYPE(CrewMember)
};

class ShipCrew : public ecs::Component {
public:
    int max_crew = 10;
    int current_crew = 0;
    std::vector<std::string> crew_member_ids;
    float overall_efficiency = 1.0f;
    float morale_average = 50.0f;

    COMPONENT_TYPE(ShipCrew)
};
// ==================== Phase 13: Crew Activity AI ====================

/**
 * @brief Tracks crew member activity state and room assignment.
 * Crew AI transitions between activities based on ship state and needs.
 */
class CrewActivity : public ecs::Component {
public:
    enum class Activity { Idle, Working, Walking, Resting, Eating, Repairing, Manning };

    std::string crew_member_id;
    std::string assigned_room_id;
    Activity current_activity = Activity::Idle;
    float activity_timer = 0.0f;        // time in current activity
    float activity_duration = 60.0f;    // how long to stay in activity
    float fatigue = 0.0f;               // 0.0 to 1.0
    float hunger = 0.0f;                // 0.0 to 1.0
    bool ship_damaged = false;          // triggers repair activity
    bool station_manned = false;        // at workstation

    static std::string activityToString(Activity a) {
        switch (a) {
            case Activity::Idle:      return "Idle";
            case Activity::Working:   return "Working";
            case Activity::Walking:   return "Walking";
            case Activity::Resting:   return "Resting";
            case Activity::Eating:    return "Eating";
            case Activity::Repairing: return "Repairing";
            case Activity::Manning:   return "Manning";
            default:                  return "Unknown";
        }
    }

    COMPONENT_TYPE(CrewActivity)
};
// ==================== Rest Station (Bed & Quarters) ====================

/**
 * @brief A rest facility (bed, quarters, etc.) for fatigue recovery
 * 
 * Provides mechanics for characters to rest and recover fatigue.
 * Quality level affects recovery rate.
 */
class RestStation : public ecs::Component {
public:
    enum class StationType {
        Bunk,       // Basic bunk bed (quality 0.5)
        Bed,        // Standard bed (quality 1.0)
        Quarters,   // Private quarters (quality 1.5)
        Luxury      // Luxury cabin (quality 2.0)
    };

    StationType type = StationType::Bed;
    float quality = 1.0f;              // Recovery rate multiplier (0.5 - 2.0)
    std::string occupant_id;           // Entity currently using this station
    bool is_occupied = false;
    float rest_start_time = 0.0f;      // When current rest session started
    float total_rest_time = 0.0f;      // Accumulated rest time this session

    bool isAvailable() const { return !is_occupied; }

    void startRest(const std::string& entity_id, float current_time) {
        occupant_id = entity_id;
        is_occupied = true;
        rest_start_time = current_time;
        total_rest_time = 0.0f;
    }

    void endRest() {
        occupant_id.clear();
        is_occupied = false;
        rest_start_time = 0.0f;
        total_rest_time = 0.0f;
    }

    static float getQualityForType(StationType t) {
        switch (t) {
            case StationType::Bunk: return 0.5f;
            case StationType::Bed: return 1.0f;
            case StationType::Quarters: return 1.5f;
            case StationType::Luxury: return 2.0f;
            default: return 1.0f;
        }
    }

    static std::string getTypeName(StationType t) {
        switch (t) {
            case StationType::Bunk: return "Bunk";
            case StationType::Bed: return "Bed";
            case StationType::Quarters: return "Quarters";
            case StationType::Luxury: return "Luxury Cabin";
            default: return "Unknown";
        }
    }

    COMPONENT_TYPE(RestStation)
};

/**
 * @brief Tracks an entity's current resting state
 * 
 * Added to entities that are currently resting at a RestStation.
 */
class RestingState : public ecs::Component {
public:
    std::string rest_station_id;   // RestStation entity being used
    float rest_start_time = 0.0f;  // When rest started
    float fatigue_at_start = 0.0f; // Fatigue level when rest began
    bool is_resting = false;

    COMPONENT_TYPE(RestingState)
};

/**
 * @brief Crew member training with skill progression
 *
 * Tracks trainees progressing through skill training over time.
 */
class CrewTraining : public ecs::Component {
public:
    struct TrainingSlot {
        std::string trainee_id;
        std::string skill_name;
        float progress = 0.0f;        // 0-1
        float training_rate = 0.01f;   // per second
        bool completed = false;
    };

    std::vector<TrainingSlot> trainees;
    int max_trainees = 5;
    int total_completed = 0;
    float xp_bonus = 1.0f; // training rate multiplier
    bool active = true;

    COMPONENT_TYPE(CrewTraining)
};


} // namespace components
} // namespace atlas

#endif // NOVAFORGE_COMPONENTS_CREW_COMPONENTS_H
