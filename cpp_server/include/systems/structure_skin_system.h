#ifndef NOVAFORGE_SYSTEMS_STRUCTURE_SKIN_SYSTEM_H
#define NOVAFORGE_SYSTEMS_STRUCTURE_SKIN_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/game_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Upwell structure cosmetic skin collection system.
 *
 * A player or corporation may own a collection of structure skins
 * (StructureSkin records).  Each skin is tied to a specific structure
 * type (Astrahus, Fortizar, Keepstar, Athanor, Tatara, Raitaru, Azbel,
 * Sotiyo) and rarity tier.  Only one skin per owner may be applied at
 * a time; applying a new skin automatically removes the previous one.
 * The collection is capped at max_skins (default 50).
 */
class StructureSkinSystem
    : public ecs::SingleComponentSystem<components::StructureSkinCollection> {
public:
    explicit StructureSkinSystem(ecs::World* world);
    ~StructureSkinSystem() override = default;

    std::string getName() const override { return "StructureSkinSystem"; }

    // --- Lifecycle ---
    bool initialize(const std::string& entity_id,
                    const std::string& owner_id);

    // --- Collection management ---
    bool addSkin(const std::string& entity_id,
                 const std::string& skin_id,
                 const std::string& name,
                 components::StructureSkinCollection::StructureType structure_type,
                 components::StructureSkinCollection::Rarity rarity,
                 const std::string& color_primary,
                 const std::string& color_secondary);
    bool removeSkin(const std::string& entity_id,
                    const std::string& skin_id);

    // --- Application ---
    bool applySkin(const std::string& entity_id,
                   const std::string& skin_id);
    bool unapplySkin(const std::string& entity_id);

    // --- Queries ---
    int         getSkinCount(const std::string& entity_id) const;
    std::string getAppliedSkinId(const std::string& entity_id) const;
    int         getSkinCountByType(
                    const std::string& entity_id,
                    components::StructureSkinCollection::StructureType type) const;
    int         getTotalAcquired(const std::string& entity_id) const;
    bool        hasSkin(const std::string& entity_id,
                        const std::string& skin_id) const;

protected:
    void updateComponent(ecs::Entity& entity,
                         components::StructureSkinCollection& comp,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_STRUCTURE_SKIN_SYSTEM_H
