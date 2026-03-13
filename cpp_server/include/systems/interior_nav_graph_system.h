#ifndef NOVAFORGE_SYSTEMS_INTERIOR_NAV_GRAPH_SYSTEM_H
#define NOVAFORGE_SYSTEMS_INTERIOR_NAV_GRAPH_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/navigation_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Walkable interior navigation graph for ships and stations
 *
 * Models the interior layout as a graph of NavNodes (doors, ramps, ladders,
 * elevators, airlocks, corridors, platforms) with bidirectional connections
 * and traversal costs.  Supports BFS-based connectivity validation.
 */
class InteriorNavGraphSystem
    : public ecs::SingleComponentSystem<components::InteriorNavGraphState> {
public:
    explicit InteriorNavGraphSystem(ecs::World* world);
    ~InteriorNavGraphSystem() override = default;

    std::string getName() const override { return "InteriorNavGraphSystem"; }

    // --- Lifecycle ---
    bool initialize(const std::string& entity_id);

    // --- Node management ---
    bool add_node(const std::string& entity_id,
                  const std::string& node_id,
                  int node_type, float x, float y, float z,
                  float traversal_cost);
    bool remove_node(const std::string& entity_id,
                     const std::string& node_id);
    bool connect_nodes(const std::string& entity_id,
                       const std::string& node_a,
                       const std::string& node_b);
    bool disconnect_nodes(const std::string& entity_id,
                          const std::string& node_a,
                          const std::string& node_b);
    bool clear_graph(const std::string& entity_id);

    // --- Queries ---
    int  get_node_count(const std::string& entity_id) const;
    bool has_node(const std::string& entity_id,
                  const std::string& node_id) const;
    int  get_connection_count(const std::string& entity_id,
                              const std::string& node_id) const;
    bool is_connected(const std::string& entity_id,
                      const std::string& node_a,
                      const std::string& node_b) const;
    int  get_node_type(const std::string& entity_id,
                       const std::string& node_id) const;
    int  get_total_path_queries(const std::string& entity_id) const;

    // --- Validation ---
    bool validate_connectivity(const std::string& entity_id,
                               const std::string& start_node_id);

protected:
    void updateComponent(ecs::Entity& entity,
                         components::InteriorNavGraphState& comp,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_INTERIOR_NAV_GRAPH_SYSTEM_H
