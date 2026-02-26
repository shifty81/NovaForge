#ifndef NOVAFORGE_SYSTEMS_MYTH_BOSS_SYSTEM_H
#define NOVAFORGE_SYSTEMS_MYTH_BOSS_SYSTEM_H

#include "ecs/system.h"
#include "ecs/entity.h"
#include "components/game_components.h"
#include <string>
#include <vector>

namespace atlas {
namespace systems {

/**
 * @brief Generates boss encounters from myth/propaganda content
 *
 * When myths about events, players, or factions reach critical mass,
 * ancient sites manifest as boss encounters reflecting the myth.
 * Boss type is derived from the myth type (Heroic → Guardian, etc.).
 */
class MythBossSystem : public ecs::System {
public:
    explicit MythBossSystem(ecs::World* world);
    ~MythBossSystem() override = default;

    void update(float delta_time) override;
    std::string getName() const override { return "MythBossSystem"; }

    /**
     * @brief Create a boss encounter from a myth
     * @param myth_id Source myth that generates this encounter
     * @param system_id Star system where encounter spawns
     * @return Generated encounter ID
     */
    std::string generateEncounter(const std::string& myth_id, const std::string& system_id);

    /**
     * @brief Check if an encounter is still active
     */
    bool isEncounterActive(const std::string& encounter_id) const;

    /**
     * @brief Mark encounter as complete
     * @param encounter_id Encounter to complete
     * @param success Whether the encounter was completed successfully
     * @return true if encounter was found and completed
     */
    bool completeEncounter(const std::string& encounter_id, bool success);

    /**
     * @brief Get count of active boss encounters
     */
    int getActiveBossCount() const;

    /**
     * @brief Get difficulty rating of an encounter
     */
    float getBossDifficulty(const std::string& encounter_id) const;

    /**
     * @brief Get the myth that generated this encounter
     */
    std::string getEncounterMythId(const std::string& encounter_id) const;

    /**
     * @brief Get boss type name for a type index
     */
    static std::string getBossTypeName(int type_index);

private:
    int encounter_counter_ = 0;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_MYTH_BOSS_SYSTEM_H
