#ifndef NOVAFORGE_SYSTEMS_PI_CUSTOMS_SYSTEM_H
#define NOVAFORGE_SYSTEMS_PI_CUSTOMS_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/economy_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Planetary-Operations customs office system
 *
 * Manages export batches from planetary colonies to orbit.
 * Each batch progresses over time and deducts an export tax from
 * the resulting proceeds.  Player-owned customs offices support
 * configurable tax rates for corp members vs. strangers.
 */
class PiCustomsSystem
    : public ecs::SingleComponentSystem<components::PiCustomsState> {
public:
    explicit PiCustomsSystem(ecs::World* world);
    ~PiCustomsSystem() override = default;

    std::string getName() const override { return "PiCustomsSystem"; }

    // --- public API ---

    /** Attach a PiCustomsState component to an existing entity. */
    bool initialize(const std::string& entity_id,
                    const std::string& customs_office_id,
                    const std::string& system_id,
                    bool player_owned = false);

    /** Queue a new export batch; returns batch_id on success or "" on failure. */
    std::string queueExport(const std::string& entity_id,
                            const std::string& colony_id,
                            const std::string& resource_type,
                            int quantity,
                            bool is_corp_member = false);

    /** Cancel a pending (not-yet-completed) export batch. */
    bool cancelExport(const std::string& entity_id, const std::string& batch_id);

    int   getPendingBatches(const std::string& entity_id) const;
    int   getCompletedBatches(const std::string& entity_id) const;
    int   getTotalExported(const std::string& entity_id) const;
    float getBatchProgress(const std::string& entity_id,
                           const std::string& batch_id) const;

protected:
    void updateComponent(ecs::Entity& entity,
                         components::PiCustomsState& comp,
                         float delta_time) override;

private:
    int batch_counter_ = 0;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_PI_CUSTOMS_SYSTEM_H
