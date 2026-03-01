#ifndef NOVAFORGE_COMPONENTS_NARRATIVE_COMPONENTS_H
#define NOVAFORGE_COMPONENTS_NARRATIVE_COMPONENTS_H

#include "ecs/component.h"
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <cstdint>
#include <cmath>

namespace atlas {
namespace components {

// ==================== Myth & Legend System ====================

class PlayerLegend : public ecs::Component {
public:
    struct LegendEntry {
        std::string event_type;
        std::string description;
        float timestamp;
        std::string system_id;
        int magnitude;
    };

    std::vector<LegendEntry> entries;
    int legend_score = 0;
    std::string title;
    int max_entries = 50;

    void addEntry(const std::string& type, const std::string& desc, float ts,
                  const std::string& sys_id, int mag) {
        LegendEntry entry{type, desc, ts, sys_id, mag};
        entries.push_back(entry);
        legend_score += mag;
        if (static_cast<int>(entries.size()) > max_entries) {
            entries.erase(entries.begin());
        }
    }

    COMPONENT_TYPE(PlayerLegend)
};
// ==================== Race & Lore ====================

class RaceInfo : public ecs::Component {
public:
    enum class RaceName { TerranDescendant, SynthBorn, PureAlien, HybridEvolutionary };

    RaceName race = RaceName::TerranDescendant;

    float learning_rate = 1.0f;
    float diplomacy_modifier = 0.0f;
    float automation_bonus = 0.0f;
    float environmental_resilience = 1.0f;
    float mutation_rate = 0.0f;

    std::string preferred_tech;
    float faction_standing_modifier = 0.0f;

    static void applyRaceDefaults(RaceInfo& info) {
        switch (info.race) {
            case RaceName::TerranDescendant:
                info.learning_rate = 1.2f;
                info.diplomacy_modifier = 0.15f;
                info.preferred_tech = "balanced";
                break;
            case RaceName::SynthBorn:
                info.automation_bonus = 0.25f;
                info.environmental_resilience = 0.8f;
                info.preferred_tech = "drone";
                break;
            case RaceName::PureAlien:
                info.environmental_resilience = 1.3f;
                info.preferred_tech = "exotic";
                break;
            case RaceName::HybridEvolutionary:
                info.learning_rate = 1.1f;
                info.environmental_resilience = 1.1f;
                info.mutation_rate = 0.05f;
                info.preferred_tech = "hybrid";
                break;
        }
    }

    COMPONENT_TYPE(RaceInfo)
};

class LoreEntry : public ecs::Component {
public:
    struct LogEntry {
        std::string title;
        std::string content;
        float discovery_timestamp;
        std::string source;
    };

    std::vector<LogEntry> discovered_lore;
    int max_entries = 100;

    void addLore(const std::string& title, const std::string& content,
                 float ts, const std::string& source) {
        LogEntry entry{title, content, ts, source};
        discovered_lore.push_back(entry);
        if (static_cast<int>(discovered_lore.size()) > max_entries) {
            discovered_lore.erase(discovered_lore.begin());
        }
    }

    int getLoreCount() const { return static_cast<int>(discovered_lore.size()); }

    COMPONENT_TYPE(LoreEntry)
};
// ==================== Phase 15: NPC Dialogue (Legend References) ====================

/**
 * @brief NPC dialogue component that records observed player legend events
 *        and generates contextual remarks about legendary players.
 */
class NPCDialogue : public ecs::Component {
public:
    struct ObservedLegend {
        std::string player_id;
        std::string event_type;
        float timestamp = 0.0f;
    };

    std::vector<ObservedLegend> observed_legends;
    std::vector<std::string> generated_lines;
    int max_lines = 20;

    void observeLegend(const std::string& player_id, const std::string& event_type, float ts) {
        observed_legends.push_back({player_id, event_type, ts});
    }

    void addLine(const std::string& line) {
        generated_lines.push_back(line);
        if (static_cast<int>(generated_lines.size()) > max_lines) {
            generated_lines.erase(generated_lines.begin());
        }
    }

    int getLineCount() const { return static_cast<int>(generated_lines.size()); }
    int getObservedCount() const { return static_cast<int>(observed_legends.size()); }

    COMPONENT_TYPE(NPCDialogue)
};

// ==================== Phase 15: Station Monuments ====================

/**
 * @brief Represents a monument or statue erected in a station for a legendary player.
 */
class StationMonument : public ecs::Component {
public:
    enum class MonumentType { Plaque, Bust, Statue, HeroicStatue, MythicShrine };

    std::string station_id;
    std::string player_id;
    std::string player_name;
    MonumentType type = MonumentType::Plaque;
    int legend_score_at_creation = 0;
    float creation_timestamp = 0.0f;
    std::string inscription;

    static MonumentType scoreToType(int score) {
        if (score >= 500) return MonumentType::MythicShrine;
        if (score >= 200) return MonumentType::HeroicStatue;
        if (score >= 100) return MonumentType::Statue;
        if (score >= 50)  return MonumentType::Bust;
        return MonumentType::Plaque;
    }

    static std::string typeToString(MonumentType t) {
        switch (t) {
            case MonumentType::Plaque:        return "Plaque";
            case MonumentType::Bust:          return "Bust";
            case MonumentType::Statue:        return "Statue";
            case MonumentType::HeroicStatue:  return "HeroicStatue";
            case MonumentType::MythicShrine:  return "MythicShrine";
            default:                          return "Unknown";
        }
    }

    COMPONENT_TYPE(StationMonument)
};
// ==================== Living Universe: Information Propagation ====================

/**
 * @brief Tracks rumors about player actions in a star system.
 * Rumors propagate to neighboring systems over time and decay if unconfirmed.
 */
class InformationPropagation : public ecs::Component {
public:
    struct Rumor {
        std::string rumor_id;
        std::string player_id;
        std::string action_type;  // "combat", "mining", "trade", "exploration", "piracy"
        std::string origin_system;
        float belief_strength = 1.0f;  // 0.0 to 1.0
        float age = 0.0f;              // seconds since creation
        int hops = 0;                  // how many systems this has propagated through
        bool personally_witnessed = false;
    };

    std::vector<Rumor> rumors;
    std::vector<std::string> neighbor_system_ids;  // systems this can propagate to
    float propagation_interval = 30.0f;     // seconds between propagation attempts
    float propagation_timer = 0.0f;
    float decay_rate = 0.01f;               // belief decay per second
    float max_rumor_age = 300.0f;           // rumors older than this are removed
    int max_rumors = 50;
    int max_hops = 5;                       // max propagation distance

    void addRumor(const std::string& rumor_id, const std::string& player_id,
                  const std::string& action_type, const std::string& origin_system,
                  bool witnessed = true) {
        // Don't add duplicate rumors
        for (auto& r : rumors) {
            if (r.rumor_id == rumor_id) {
                if (witnessed) r.belief_strength = (std::min)(r.belief_strength + 0.3f, 1.0f);
                return;
            }
        }
        Rumor rumor;
        rumor.rumor_id = rumor_id;
        rumor.player_id = player_id;
        rumor.action_type = action_type;
        rumor.origin_system = origin_system;
        rumor.belief_strength = witnessed ? 1.0f : 0.5f;
        rumor.personally_witnessed = witnessed;
        rumors.push_back(rumor);
        if (static_cast<int>(rumors.size()) > max_rumors) {
            rumors.erase(rumors.begin());
        }
    }

    int getRumorCount() const { return static_cast<int>(rumors.size()); }

    COMPONENT_TYPE(InformationPropagation)
};
// ==================== Living Universe: Visual Cues ====================

class VisualCue : public ecs::Component {
public:
    // Lockdown visual state
    bool lockdown_active = false;
    float lockdown_intensity = 0.0f;       // 0.0 to 1.0

    // Traffic density visualization
    float traffic_density = 0.0f;          // 0.0 (empty) to 1.0 (congested)
    int traffic_ship_count = 0;

    // Threat visualization
    float threat_glow = 0.0f;              // red glow intensity for dangerous systems

    // Economic state visualization
    float prosperity_indicator = 0.5f;     // 0.0 (depressed) to 1.0 (booming)

    // Pirate presence
    float pirate_warning = 0.0f;           // 0.0 (safe) to 1.0 (infested)

    // Resource availability visualization
    float resource_highlight = 0.5f;       // 0.0 (depleted) to 1.0 (rich)

    // Faction influence coloring
    std::string dominant_faction;
    float faction_influence_strength = 0.0f;

    COMPONENT_TYPE(VisualCue)
};
// ==================== Propaganda & False Myths System ====================

/**
 * @brief Network of NPC-propagated myths, rumors, and misinformation
 * 
 * Tracks fabricated stories about players and factions that NPCs spread
 * through dialogue. Myths have credibility scores that decay over time
 * and can be debunked by players.
 */
class PropagandaNetwork : public ecs::Component {
public:
    enum class MythType {
        Heroic,      // Inflated positive reputation
        Villainous,  // False accusations of wrongdoing
        Mysterious,  // Cryptic rumors about hidden activities
        Exaggerated, // Real events blown out of proportion
        Fabricated   // Completely made up stories
    };

    struct MythEntry {
        std::string myth_id;
        std::string subject_id;        // Who/what the myth is about
        std::string source_faction;    // Faction that originated the myth
        MythType type = MythType::Fabricated;
        std::string content;           // The actual myth text
        std::string base_event;        // Real event being distorted (if any)
        float credibility = 1.0f;      // 0 = debunked, 1 = fully believed
        float timestamp = 0.0f;        // When created
        int spread_count = 1;          // How many NPCs have heard it
        bool debunked = false;
        
        MythEntry() = default;
    };

    std::vector<MythEntry> myths;
    float credibility_decay_rate = 0.001f;  // Credibility lost per update
    int max_myths = 100;

    MythEntry* findMyth(const std::string& myth_id) {
        for (auto& m : myths) {
            if (m.myth_id == myth_id) return &m;
        }
        return nullptr;
    }

    const MythEntry* findMyth(const std::string& myth_id) const {
        for (const auto& m : myths) {
            if (m.myth_id == myth_id) return &m;
        }
        return nullptr;
    }

    std::vector<MythEntry> getMythsAbout(const std::string& subject_id, bool include_debunked = false) const {
        std::vector<MythEntry> result;
        for (const auto& m : myths) {
            if (m.subject_id == subject_id && (include_debunked || !m.debunked)) {
                result.push_back(m);
            }
        }
        return result;
    }

    int getActiveMythCount() const {
        int count = 0;
        for (const auto& m : myths) {
            if (!m.debunked && m.credibility > 0.0f) count++;
        }
        return count;
    }

    COMPONENT_TYPE(PropagandaNetwork)
};
// ==================== Myth Boss Encounters ====================

/**
 * @brief A boss encounter generated from myth/propaganda content
 * 
 * When myths about events, players, or factions reach critical mass,
 * ancient sites manifest as boss encounters reflecting the myth.
 */
class MythBossEncounter : public ecs::Component {
public:
    enum class BossType {
        Guardian,   // From heroic myths - protective ancient entity
        Destroyer,  // From villainous myths - aggressive ancient weapon
        Phantom,    // From mysterious myths - elusive anomaly entity
        Colossus,   // From exaggerated myths - massive ancient construct
        Mirage      // From fabricated myths - illusion-based encounter
    };

    struct LootEntry {
        std::string item_id;
        float drop_chance = 0.5f;
        int quantity = 1;
    };

    std::string encounter_id;
    std::string myth_id;          // Source myth that generated this encounter
    std::string system_id;        // Star system where encounter spawns
    BossType boss_type = BossType::Guardian;
    float difficulty = 1.0f;      // 1.0 = normal, 5.0 = extreme
    float active_time = 0.0f;     // Time since encounter started
    float max_duration = 3600.0f; // Max time before encounter despawns (1 hour)
    bool active = true;
    bool completion_success = false;
    float shield_hp = 1000.0f;
    float armor_hp = 500.0f;
    float hull_hp = 2000.0f;
    std::vector<LootEntry> loot_table;
    int recommended_fleet_size = 3;

    bool isActive() const { return active && active_time < max_duration; }
    bool isExpired() const { return active_time >= max_duration; }

    static std::string getBossTypeName(BossType t) {
        switch (t) {
            case BossType::Guardian: return "Guardian";
            case BossType::Destroyer: return "Destroyer";
            case BossType::Phantom: return "Phantom";
            case BossType::Colossus: return "Colossus";
            case BossType::Mirage: return "Mirage";
            default: return "Unknown";
        }
    }

    COMPONENT_TYPE(MythBossEncounter)
};


} // namespace components
} // namespace atlas

#endif // NOVAFORGE_COMPONENTS_NARRATIVE_COMPONENTS_H
