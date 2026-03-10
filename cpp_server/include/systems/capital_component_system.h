#ifndef NOVAFORGE_SYSTEMS_CAPITAL_COMPONENT_SYSTEM_H
#define NOVAFORGE_SYSTEMS_CAPITAL_COMPONENT_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/economy_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Capital ship component manufacturing system
 *
 * Manages production jobs for capital-specific components
 * (e.g. Capital Armor Plates, Capital Capacitor Batteries, Capital
 * Shield Emitters).  Each job consumes raw materials and produces a
 * capital component after a time scaled by ME/TE research bonuses.
 */
class CapitalComponentSystem
    : public ecs::SingleComponentSystem<components::CapitalComponentState> {
public:
    explicit CapitalComponentSystem(ecs::World* world);
    ~CapitalComponentSystem() override = default;

    std::string getName() const override { return "CapitalComponentSystem"; }

    // --- public API ---

    /** Attach a CapitalComponentState component to an existing entity. */
    bool initialize(const std::string& entity_id,
                    const std::string& facility_id);

    /**
     * Start a new capital component job.
     * @param component_type  e.g. "cap_armor_plate"
     * @param blueprint_id    blueprint entity id
     * @param runs            number of runs
     * @param me_bonus        material efficiency 0.0–1.0
     * @param te_bonus        time efficiency 0.0–1.0
     * @return job_id on success, "" on failure
     */
    std::string startJob(const std::string& entity_id,
                         const std::string& component_type,
                         const std::string& blueprint_id,
                         int runs,
                         float me_bonus,
                         float te_bonus);

    /** Cancel a running job. */
    bool cancelJob(const std::string& entity_id, const std::string& job_id);

    int   getActiveJobCount(const std::string& entity_id) const;
    int   getCompletedJobCount(const std::string& entity_id) const;
    int   getTotalUnitsProduced(const std::string& entity_id) const;
    float getJobProgress(const std::string& entity_id,
                         const std::string& job_id) const;
    bool  isJobComplete(const std::string& entity_id,
                        const std::string& job_id) const;

protected:
    void updateComponent(ecs::Entity& entity,
                         components::CapitalComponentState& comp,
                         float delta_time) override;

private:
    int job_counter_ = 0;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_CAPITAL_COMPONENT_SYSTEM_H
