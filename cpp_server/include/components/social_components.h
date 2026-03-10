#ifndef NOVAFORGE_COMPONENTS_SOCIAL_COMPONENTS_H
#define NOVAFORGE_COMPONENTS_SOCIAL_COMPONENTS_H

#include "ecs/component.h"
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <cstdint>
#include <cmath>

namespace atlas {
namespace components {

class Corporation : public ecs::Component {
public:
    std::string corp_id;
    std::string corp_name;
    std::string ticker;
    std::string ceo_id;
    float tax_rate = 0.05f;
    std::vector<std::string> member_ids;
    double corp_wallet = 0.0;

    struct CorpHangarItem {
        std::string item_id;
        std::string name;
        std::string type;
        int quantity = 1;
        float volume = 1.0f;
    };

    std::vector<CorpHangarItem> hangar_items;

    COMPONENT_TYPE(Corporation)
};
/**
 * @brief Chat channel for persistent messaging
 *
 * Supports multiple channel types (local, corp, fleet, private)
 * with message history, moderation, and member tracking.
 */
class ChatChannel : public ecs::Component {
public:
    std::string channel_id;
    std::string channel_name;
    std::string channel_type = "local";  // "local", "corp", "fleet", "alliance", "private"
    std::string owner_id;                // channel creator/owner
    std::string motd;                    // message of the day
    int max_members = 0;                 // 0 = unlimited
    bool is_moderated = false;

    struct ChatMessage {
        std::string message_id;
        std::string sender_id;
        std::string sender_name;
        std::string content;
        std::string timestamp;
        bool is_system_message = false;
    };

    struct ChannelMember {
        std::string player_id;
        std::string player_name;
        std::string role = "member";     // "member", "moderator", "operator", "owner"
        bool is_muted = false;
    };

    std::vector<ChatMessage> messages;
    std::vector<ChannelMember> members;
    int max_history = 200;               // max messages to keep

    int memberCount() const { return static_cast<int>(members.size()); }

    COMPONENT_TYPE(ChatChannel)
};

/**
 * @brief Character sheet for player identity and attributes
 *
 * Tracks race, bloodline, ancestry, clone, implants, and
 * character attributes for creation and progression.
 */
class CharacterSheet : public ecs::Component {
public:
    std::string character_id;
    std::string character_name;
    std::string race = "Caldari";           // "Caldari", "Amarr", "Gallente", "Minmatar"
    std::string bloodline;                   // race-specific bloodline
    std::string ancestry;                    // background/origin
    std::string gender = "male";             // "male", "female"
    float date_of_birth = 0.0f;              // simulation time of creation

    // Attributes (base values, modified by implants)
    int intelligence = 20;
    int perception = 20;
    int charisma = 19;
    int willpower = 20;
    int memory = 20;

    // Clone
    std::string clone_grade = "foundry";       // "foundry", "apex"
    std::string clone_location;              // station ID for medical clone
    int clone_jump_cooldown = 0;             // seconds remaining

    // Implants (slots 1-10)
    struct Implant {
        std::string implant_id;
        std::string implant_name;
        int slot = 0;                        // 1-10
        std::string attribute_bonus;         // attribute boosted
        int bonus_amount = 0;
    };
    std::vector<Implant> implants;

    // Security status
    float security_status = 0.0f;            // -10.0 to 10.0

    // Employment history
    struct EmploymentRecord {
        std::string corp_id;
        std::string corp_name;
        float join_date = 0.0f;
        float leave_date = 0.0f;
    };
    std::vector<EmploymentRecord> employment_history;

    int getEffectiveAttribute(const std::string& attr) const {
        int base = 0;
        if (attr == "intelligence") base = intelligence;
        else if (attr == "perception") base = perception;
        else if (attr == "charisma") base = charisma;
        else if (attr == "willpower") base = willpower;
        else if (attr == "memory") base = memory;

        for (const auto& imp : implants) {
            if (imp.attribute_bonus == attr) base += imp.bonus_amount;
        }
        return base;
    }

    COMPONENT_TYPE(CharacterSheet)
};

/**
 * @brief Tournament bracket for competitive PvE events
 *
 * Tracks tournament lifecycle: registration, active rounds,
 * participant scoring, and final results with rewards.
 */
class Tournament : public ecs::Component {
public:
    std::string tournament_id;
    std::string name;
    std::string status = "registration";  // "registration", "active", "completed", "cancelled"

    int max_participants = 16;
    int current_round = 0;
    int total_rounds = 0;
    float round_duration = 600.0f;        // seconds per round
    float round_timer = 0.0f;             // countdown for current round

    double entry_fee = 0.0;
    double prize_pool = 0.0;

    struct Participant {
        std::string player_id;
        std::string player_name;
        int score = 0;
        int kills = 0;
        bool eliminated = false;
    };

    std::vector<Participant> participants;

    struct RoundResult {
        int round_number = 0;
        std::string winner_id;
        int winner_score = 0;
        int participant_count = 0;
    };

    std::vector<RoundResult> round_results;

    COMPONENT_TYPE(Tournament)
};

/**
 * @brief Leaderboard for tracking player rankings and achievements
 *
 * Aggregates player stats across categories (kills, Credits earned,
 * missions completed, etc.) and tracks unlocked achievements.
 */
class Leaderboard : public ecs::Component {
public:
    struct PlayerEntry {
        std::string player_id;
        std::string player_name;
        int total_kills = 0;
        double total_isc_earned = 0.0;
        int missions_completed = 0;
        int tournaments_won = 0;
        double total_bounty = 0.0;
        int ships_destroyed = 0;
        int ships_lost = 0;
        double total_damage_dealt = 0.0;
    };

    struct Achievement {
        std::string achievement_id;
        std::string name;
        std::string description;
        std::string category;             // "combat", "industry", "exploration", "social"
        int requirement = 1;              // threshold to unlock
        std::string stat_key;             // which stat to check against requirement
    };

    struct UnlockedAchievement {
        std::string achievement_id;
        std::string player_id;
        float unlock_time = 0.0f;
    };

    std::string board_id;
    std::vector<PlayerEntry> entries;
    std::vector<Achievement> achievements;
    std::vector<UnlockedAchievement> unlocked;

    COMPONENT_TYPE(Leaderboard)
};
class Alliance : public ecs::Component {
public:
    std::string alliance_id;
    std::string alliance_name;
    std::string ticker;
    std::string executor_corp_id;  // The corp that leads the alliance
    std::vector<std::string> member_corp_ids;
    double alliance_wallet = 0.0;
    float tax_rate = 0.0f;
    std::string founded_date;
    bool is_open = false;  // Open for new applications
    int max_corps = 100;
    COMPONENT_TYPE(Alliance)
};

class Sovereignty : public ecs::Component {
public:
    std::string system_id;        // Star system this sovereignty covers
    std::string owner_id;         // Alliance or corp entity that owns it
    std::string system_name;
    float control_level = 0.0f;   // 0.0 to 1.0 (contested to full control)
    float vulnerability_timer = 0.0f; // hours until vulnerable
    bool is_contested = false;
    float strategic_index = 0.0f; // 0.0 to 5.0 based on activity
    float military_index = 0.0f;  // 0.0 to 5.0 based on NPC kills
    float industrial_index = 0.0f;// 0.0 to 5.0 based on mining/production
    std::string infrastructure_hub_id;
    int upgrade_level = 0;        // 0-5
    COMPONENT_TYPE(Sovereignty)
};

class WarDeclaration : public ecs::Component {
public:
    std::string war_id;
    std::string aggressor_id;     // Corp or alliance that declared war
    std::string defender_id;      // Corp or alliance being attacked
    std::string status = "pending"; // "pending", "active", "mutual", "surrendered", "retracted", "finished"
    float duration_hours = 168.0f; // Default 1 week
    float elapsed_hours = 0.0f;
    double war_cost = 100000000.0; // Credits cost to declare war (100M)
    int aggressor_kills = 0;
    int defender_kills = 0;
    double aggressor_isc_destroyed = 0.0;
    double defender_isc_destroyed = 0.0;
    bool is_mutual = false;
    COMPONENT_TYPE(WarDeclaration)
};

/**
 * @brief Player faction standing and reputation aggregation
 */
class PlayerStanding : public ecs::Component {
public:
    struct FactionStanding {
        std::string faction_id;
        std::string faction_name;
        double standing = 0.0;     // -10.0 to 10.0
        int rank = 0;              // 0=Neutral, 1-5=Positive ranks, -1 to -5=Negative ranks
    };

    struct StandingNotification {
        std::string faction_id;
        int old_rank = 0;
        int new_rank = 0;
        float timestamp = 0.0f;
    };

    float elapsed = 0.0f;
    bool active = true;
    int max_notifications = 20;
    std::vector<FactionStanding> factions;
    std::vector<StandingNotification> notifications;

    COMPONENT_TYPE(PlayerStanding)
};


// ---------------------------------------------------------------------------
// CitadelState — player-owned Upwell structure management
// ---------------------------------------------------------------------------
/**
 * @brief Player-owned citadel / Upwell structure state.
 *
 * Models three sizes (Astrahus / Fortizar / Keepstar) with services,
 * fuel consumption, vulnerability windows, and reinforcement timers.
 * Fuel is consumed per tick based on active services.  When fuel runs
 * out, all services go offline.  Vulnerability windows allow attacks
 * that may trigger reinforcement timers.
 */
class CitadelState : public ecs::Component {
public:
    enum class CitadelType { Astrahus, Fortizar, Keepstar };
    enum class StructureState { Online, Vulnerable, Reinforced, Destroyed };

    struct Service {
        std::string service_id;
        std::string service_name;
        float       fuel_per_hour = 5.0f;
        bool        online        = true;
    };

    CitadelType    type              = CitadelType::Astrahus;
    StructureState state             = StructureState::Online;
    std::string    owner_corp_id;
    std::string    structure_name;
    float          shield_hp         = 10000.0f;
    float          shield_hp_max     = 10000.0f;
    float          armor_hp          = 10000.0f;
    float          armor_hp_max      = 10000.0f;
    float          hull_hp           = 10000.0f;
    float          hull_hp_max       = 10000.0f;
    std::vector<Service> services;
    int            max_services      = 5;
    float          fuel_remaining    = 1000.0f;  // fuel units
    float          fuel_capacity     = 5000.0f;
    float          vulnerability_hours = 3.0f;   // hours per week
    float          reinforcement_timer = 0.0f;   // seconds remaining
    float          reinforcement_duration = 86400.0f; // default 24h
    int            total_reinforcements = 0;
    int            total_services_installed = 0;
    float          elapsed           = 0.0f;
    bool           active            = true;

    COMPONENT_TYPE(CitadelState)
};

} // namespace components
} // namespace atlas

#endif // NOVAFORGE_COMPONENTS_SOCIAL_COMPONENTS_H
