#ifndef NOVAFORGE_SYSTEMS_CARGO_MANIFEST_SYSTEM_H
#define NOVAFORGE_SYSTEMS_CARGO_MANIFEST_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/ship_components.h"
#include <string>
#include <vector>

namespace atlas {
namespace systems {

/**
 * @brief Unified cargo management with volume tracking and ore hold separation
 *
 * Manages item storage, volume constraints, ore vs general hold splitting,
 * and jettison/pickup mechanics for ship cargo.
 */
class CargoManifestSystem : public ecs::SingleComponentSystem<components::CargoManifest> {
public:
    explicit CargoManifestSystem(ecs::World* world);
    ~CargoManifestSystem() override = default;

    std::string getName() const override { return "CargoManifestSystem"; }

    bool addItem(const std::string& entity_id, const std::string& item_id,
                 const std::string& item_name, const std::string& category,
                 int quantity, double volume_per_unit);
    bool removeItem(const std::string& entity_id, const std::string& item_id, int quantity);
    int getItemQuantity(const std::string& entity_id, const std::string& item_id) const;
    double getGeneralUsed(const std::string& entity_id) const;
    double getGeneralCapacity(const std::string& entity_id) const;
    double getOreHoldUsed(const std::string& entity_id) const;
    double getOreHoldCapacity(const std::string& entity_id) const;
    int getItemCount(const std::string& entity_id) const;
    bool jettisonItem(const std::string& entity_id, const std::string& item_id, int quantity);
    bool transferItem(const std::string& from_id, const std::string& to_id,
                      const std::string& item_id, int quantity);
    double getTotalVolumeUsed(const std::string& entity_id) const;
    bool hasSpace(const std::string& entity_id, const std::string& category,
                  double volume) const;

protected:
    void updateComponent(ecs::Entity& entity, components::CargoManifest& cargo, float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_CARGO_MANIFEST_SYSTEM_H
