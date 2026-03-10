#ifndef NOVAFORGE_SYSTEMS_D_SCAN_SYSTEM_H
#define NOVAFORGE_SYSTEMS_D_SCAN_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/game_components.h"
#include <string>
#include <vector>

namespace atlas {
namespace systems {

/**
 * @brief Directional scanner system
 *
 * Models EVE Online's directional scanner.  The player configures a range
 * (AU) and cone angle (degrees), then starts a scan.  The system ticks the
 * scan timer and, when complete, marks is_scanning as false.  Contacts are
 * added via addContact() to simulate nearby ships/structures being found.
 * Contacts are cleared at the start of each new scan.
 */
class DScanSystem
    : public ecs::SingleComponentSystem<components::DScanState> {
public:
    explicit DScanSystem(ecs::World* world);
    ~DScanSystem() override = default;

    std::string getName() const override { return "DScanSystem"; }

    // --- Lifecycle ---
    bool initialize(const std::string& entity_id);

    // --- Scanning ---
    bool startScan(const std::string& entity_id,
                   float range_au, float angle_deg);
    bool addContact(const std::string& entity_id,
                    const std::string& contact_entity_id,
                    const std::string& name,
                    int contact_type,
                    float distance_au);
    bool clearContacts(const std::string& entity_id);

    // --- Queries ---
    bool isScanning(const std::string& entity_id) const;
    int  getScanCount(const std::string& entity_id) const;
    int  getContactCount(const std::string& entity_id) const;
    float getScanRange(const std::string& entity_id) const;
    float getScanAngle(const std::string& entity_id) const;
    std::vector<components::DScanState::Contact>
        getContacts(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity,
                         components::DScanState& comp,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_D_SCAN_SYSTEM_H
