#ifndef NOVAFORGE_SYSTEMS_PROPAGANDA_SYSTEM_H
#define NOVAFORGE_SYSTEMS_PROPAGANDA_SYSTEM_H

#include "ecs/system.h"
#include "ecs/entity.h"
#include "components/game_components.h"
#include <string>
#include <vector>

namespace atlas {
namespace systems {

/**
 * @brief Generates and manages NPC-propagated false myths and misinformation
 *
 * Creates fabricated stories about events, players, and factions that NPCs
 * spread through dialogue. Myths can be beneficial (inflated reputation) or
 * harmful (false accusations), and have a credibility score that decays over time.
 */
class PropagandaSystem : public ecs::System {
public:
    explicit PropagandaSystem(ecs::World* world);
    ~PropagandaSystem() override = default;

    void update(float delta_time) override;
    std::string getName() const override { return "PropagandaSystem"; }

    /**
     * @brief Generate a propaganda myth about a player or faction
     * @param subject_id Entity or faction being talked about
     * @param source_faction Faction spreading the propaganda
     * @param myth_type Type of myth (heroic, villainous, mysterious, exaggerated)
     * @param base_event Optional real event being distorted
     * @return Generated myth ID
     */
    std::string generateMyth(const std::string& subject_id,
                              const std::string& source_faction,
                              const std::string& myth_type,
                              const std::string& base_event = "");

    /**
     * @brief Spread an existing myth to additional NPCs
     * @param myth_id The myth to spread
     * @param target_system_id System where myth spreads
     * @param spread_factor How much the myth mutates (0 = exact, 1 = heavily distorted)
     */
    void spreadMyth(const std::string& myth_id,
                    const std::string& target_system_id,
                    float spread_factor = 0.1f);

    /**
     * @brief Get myths known about a subject
     * @param subject_id Entity or faction being discussed
     * @param include_debunked Whether to include debunked myths
     * @return Vector of myth entries
     */
    std::vector<components::PropagandaNetwork::MythEntry> getMythsAbout(
        const std::string& subject_id,
        bool include_debunked = false) const;

    /**
     * @brief Debunk a myth with counter-evidence
     * @param myth_id Myth to debunk
     * @param evidence_strength How strong the counter-evidence (0-1)
     * @return New credibility of the myth after debunking
     */
    float debunkMyth(const std::string& myth_id, float evidence_strength);

    /**
     * @brief Get current credibility of a myth
     * @return Credibility from 0 (debunked) to 1 (fully believed)
     */
    float getMythCredibility(const std::string& myth_id) const;

    /**
     * @brief Check if an NPC believes a particular myth
     */
    bool npcBelievesMyth(const std::string& npc_id, const std::string& myth_id) const;

    /**
     * @brief Get count of active myths about a subject
     */
    int getActiveMythCount(const std::string& subject_id) const;

    /**
     * @brief Get myth type name
     */
    static std::string getMythTypeName(int type_index);

private:
    int myth_counter_ = 0;
    float credibility_decay_rate_ = 0.01f;  // Per update tick

    std::string generateMythContent(const std::string& subject_id,
                                     const std::string& myth_type,
                                     const std::string& base_event);
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_PROPAGANDA_SYSTEM_H
