#ifndef NOVAFORGE_SYSTEMS_CONTROL_PORT_SYSTEM_H
#define NOVAFORGE_SYSTEMS_CONTROL_PORT_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/fps_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Manages interactive control ports on ships and structures
 *
 * Ships and stations expose named ports (turrets, pilot seats, engineering
 * consoles, etc.) that players can occupy.  Each port has a type and an
 * associated enter-mode used by PlayerModeSystem.
 */
class ControlPortSystem
    : public ecs::SingleComponentSystem<components::ControlPortState> {
public:
    explicit ControlPortSystem(ecs::World* world);
    ~ControlPortSystem() override = default;

    std::string getName() const override { return "ControlPortSystem"; }

    // --- Lifecycle ---
    bool initialize(const std::string& entity_id);

    // --- Port management ---
    bool add_port(const std::string& entity_id,
                  const std::string& port_id,
                  const std::string& port_type,
                  int enter_mode);
    bool remove_port(const std::string& entity_id,
                     const std::string& port_id);
    bool occupy_port(const std::string& entity_id,
                     const std::string& port_id,
                     const std::string& occupant_id);
    bool vacate_port(const std::string& entity_id,
                     const std::string& port_id);
    bool clear_ports(const std::string& entity_id);

    // --- Queries ---
    bool        is_occupied(const std::string& entity_id,
                            const std::string& port_id) const;
    std::string get_occupant(const std::string& entity_id,
                             const std::string& port_id) const;
    int         get_port_count(const std::string& entity_id) const;
    std::string get_port_type(const std::string& entity_id,
                              const std::string& port_id) const;
    int         get_total_uses(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity,
                         components::ControlPortState& comp,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_CONTROL_PORT_SYSTEM_H
