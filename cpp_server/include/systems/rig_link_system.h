#ifndef NOVAFORGE_SYSTEMS_RIG_LINK_SYSTEM_H
#define NOVAFORGE_SYSTEMS_RIG_LINK_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/fps_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Links a player rig to a ship port and manages stat bonuses
 *
 * When linked the player receives stat bonuses modulated by link quality
 * and interface level.  Stats have a base value and bonus component.
 */
class RigLinkSystem
    : public ecs::SingleComponentSystem<components::RigLinkState> {
public:
    explicit RigLinkSystem(ecs::World* world);
    ~RigLinkSystem() override = default;

    std::string getName() const override { return "RigLinkSystem"; }

    // --- Lifecycle ---
    bool initialize(const std::string& entity_id);

    // --- Linking ---
    bool link_to_ship(const std::string& entity_id,
                      const std::string& ship_id,
                      const std::string& port_id);
    bool unlink(const std::string& entity_id);

    // --- Stat management ---
    bool add_stat(const std::string& entity_id,
                  const std::string& stat_name,
                  float base_value);
    bool remove_stat(const std::string& entity_id,
                     const std::string& stat_name);
    bool set_stat_bonus(const std::string& entity_id,
                        const std::string& stat_name,
                        float bonus);
    bool clear_stats(const std::string& entity_id);

    // --- Configuration ---
    bool set_interface_level(const std::string& entity_id, int level);
    bool set_link_quality(const std::string& entity_id, float quality);

    // --- Queries ---
    bool        is_linked(const std::string& entity_id) const;
    std::string get_linked_ship(const std::string& entity_id) const;
    std::string get_linked_port(const std::string& entity_id) const;
    float       get_stat_value(const std::string& entity_id,
                               const std::string& stat_name) const;
    int         get_stat_count(const std::string& entity_id) const;
    int         get_interface_level(const std::string& entity_id) const;
    float       get_link_quality(const std::string& entity_id) const;
    int         get_total_links(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity,
                         components::RigLinkState& comp,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_RIG_LINK_SYSTEM_H
