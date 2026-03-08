#ifndef NOVAFORGE_SYSTEMS_SESSION_PERSISTENCE_SYSTEM_H
#define NOVAFORGE_SYSTEMS_SESSION_PERSISTENCE_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/game_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * SessionPersistenceSystem — auto-saves and restores player session state.
 *
 * Reads/Writes SessionPersistenceState component.
 *
 * Design:
 *   - Tracks elapsed time and marks sessions dirty when state changes.
 *   - On auto-save interval, serialises position, credits, cargo, ship HP,
 *     current system, and docked station.
 *   - Provides query API for save/load status and snapshot inspection.
 *   - Failure tracking: increments save_failures on simulated I/O errors.
 */
class SessionPersistenceSystem : public ecs::SingleComponentSystem<components::SessionPersistenceState> {
public:
    explicit SessionPersistenceSystem(ecs::World* world);
    ~SessionPersistenceSystem() override = default;

    std::string getName() const override { return "SessionPersistenceSystem"; }

    /// Mark a session as needing a save
    bool markDirty(const std::string& entity_id);

    /// Trigger an immediate save (returns false on simulated failure)
    bool triggerSave(const std::string& entity_id);

    /// Simulate a load by resetting time counters
    bool triggerLoad(const std::string& entity_id);

    /// Query helpers
    int getTotalSaves(const std::string& entity_id) const;
    int getTotalLoads(const std::string& entity_id) const;
    bool isDirty(const std::string& entity_id) const;
    float getTimeSinceLastSave(const std::string& entity_id) const;
    std::string getCurrentSystem(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity, components::SessionPersistenceState& state, float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_SESSION_PERSISTENCE_SYSTEM_H
