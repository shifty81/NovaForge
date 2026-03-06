#ifndef NOVAFORGE_SYSTEMS_SAVE_GAME_SYSTEM_H
#define NOVAFORGE_SYSTEMS_SAVE_GAME_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/game_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Save game orchestration for session persistence
 *
 * Manages save slots, dirty-flag tracking, auto-save timing, and
 * save/load lifecycle.  Enables player progress to persist between
 * game sessions.
 */
class SaveGameSystem : public ecs::SingleComponentSystem<components::SaveGameState> {
public:
    explicit SaveGameSystem(ecs::World* world);
    ~SaveGameSystem() override = default;

    std::string getName() const override { return "SaveGameSystem"; }

public:
    bool initialize(const std::string& entity_id, const std::string& owner_id);

    // Save slot management
    bool createSaveSlot(const std::string& entity_id, const std::string& slot_id,
                        const std::string& character_name, const std::string& location,
                        const std::string& ship_type, double wallet_balance,
                        int skill_points, float play_time);
    bool deleteSaveSlot(const std::string& entity_id, const std::string& slot_id);
    bool overwriteSaveSlot(const std::string& entity_id, const std::string& slot_id,
                           const std::string& character_name, const std::string& location,
                           const std::string& ship_type, double wallet_balance,
                           int skill_points, float play_time);
    bool loadSaveSlot(const std::string& entity_id, const std::string& slot_id);

    // Dirty flag management
    bool markDirty(const std::string& entity_id, const std::string& category);
    bool clearDirty(const std::string& entity_id);
    bool isDirty(const std::string& entity_id) const;

    // Save/load lifecycle
    bool beginSave(const std::string& entity_id);
    bool completeSave(const std::string& entity_id, float timestamp);
    bool beginLoad(const std::string& entity_id);
    bool completeLoad(const std::string& entity_id);
    bool reportError(const std::string& entity_id);

    // Queries
    int getSlotCount(const std::string& entity_id) const;
    int getOccupiedSlotCount(const std::string& entity_id) const;
    int getSaveStatus(const std::string& entity_id) const;
    int getTotalSaves(const std::string& entity_id) const;
    int getTotalLoads(const std::string& entity_id) const;
    int getSaveErrors(const std::string& entity_id) const;
    float getLastSaveTime(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity, components::SaveGameState& state, float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_SAVE_GAME_SYSTEM_H
