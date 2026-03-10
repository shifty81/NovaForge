#ifndef NOVAFORGE_SYSTEMS_SKILL_INJECTOR_SYSTEM_H
#define NOVAFORGE_SYSTEMS_SKILL_INJECTOR_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/game_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Skill-point extractor and injector management system.
 *
 * Models the EVE Online skill injector / extractor mechanic:
 *
 *  - extractSP: moves SP from unallocated_sp into a new InjectorItem.
 *    Each extraction creates exactly one injector holding 500,000 SP.
 *    Extraction fails if unallocated_sp < 500,000 or capacity is full.
 *
 *  - injectSP: applies an owned injector to the character.  The dose
 *    delivered depends on the character's total_sp at the time of
 *    injection (EVE-style diminishing returns):
 *      ≤  5,000,000 SP → 500,000 SP added  (full dose)
 *      ≤ 50,000,000 SP → 400,000 SP added  (80 %)
 *      ≤ 80,000,000 SP → 300,000 SP added  (60 %)
 *      >  80,000,000 SP → 200,000 SP added  (40 %)
 *
 *  - addUnallocatedSP / setTotalSP for test/configuration helpers.
 *
 * Injectors are kept in a list capped at max_injectors (default 20).
 * total_extracted / total_injected are cumulative SP counters.
 */
class SkillInjectorSystem
    : public ecs::SingleComponentSystem<components::SkillInjectorState> {
public:
    explicit SkillInjectorSystem(ecs::World* world);
    ~SkillInjectorSystem() override = default;

    std::string getName() const override { return "SkillInjectorSystem"; }

    // --- Lifecycle ---
    bool initialize(const std::string& entity_id,
                    int unallocated_sp,
                    int total_sp);

    // --- Core mechanics ---
    /// Extract 500,000 unallocated SP into a new injector.
    bool extractSP(const std::string& entity_id,
                   const std::string& injector_id,
                   const std::string& source_pilot_id);

    /// Apply an owned injector to this character.
    bool injectSP(const std::string& entity_id,
                  const std::string& injector_id);

    // --- Configuration helpers ---
    bool addUnallocatedSP(const std::string& entity_id, int amount);
    bool setTotalSP(const std::string& entity_id, int total_sp);

    // --- Queries ---
    int  getInjectorCount(const std::string& entity_id) const;
    int  getUnallocatedSP(const std::string& entity_id) const;
    int  getTotalSP(const std::string& entity_id) const;
    int  getTotalExtracted(const std::string& entity_id) const;
    int  getTotalInjected(const std::string& entity_id) const;
    bool hasInjector(const std::string& entity_id,
                     const std::string& injector_id) const;

    /// Returns the SP dose that would be delivered given current total_sp.
    int  computeDose(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity,
                         components::SkillInjectorState& comp,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_SKILL_INJECTOR_SYSTEM_H
