#include "systems/interior_nav_graph_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>
#include <queue>
#include <unordered_set>

namespace atlas {
namespace systems {

InteriorNavGraphSystem::InteriorNavGraphSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

// ---------------------------------------------------------------------------
// Tick
// ---------------------------------------------------------------------------

void InteriorNavGraphSystem::updateComponent(ecs::Entity& /*entity*/,
                                              components::InteriorNavGraphState& comp,
                                              float delta_time) {
    if (!comp.active) return;
    comp.elapsed += delta_time;
}

// ---------------------------------------------------------------------------
// Lifecycle
// ---------------------------------------------------------------------------

bool InteriorNavGraphSystem::initialize(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::InteriorNavGraphState>();
    entity->addComponent(std::move(comp));
    return true;
}

// ---------------------------------------------------------------------------
// Node management
// ---------------------------------------------------------------------------

bool InteriorNavGraphSystem::add_node(const std::string& entity_id,
                                       const std::string& node_id,
                                       int node_type,
                                       float x, float y, float z,
                                       float traversal_cost) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (node_id.empty()) return false;
    if (static_cast<int>(comp->nodes.size()) >= comp->max_nodes) return false;

    for (const auto& n : comp->nodes) {
        if (n.node_id == node_id) return false;
    }

    components::InteriorNavGraphState::NavNode node;
    node.node_id        = node_id;
    node.node_type      = node_type;
    node.x              = x;
    node.y              = y;
    node.z              = z;
    node.traversal_cost = traversal_cost;
    comp->nodes.push_back(node);
    return true;
}

bool InteriorNavGraphSystem::remove_node(const std::string& entity_id,
                                          const std::string& node_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;

    // Remove from all connection lists first
    for (auto& n : comp->nodes) {
        auto& conns = n.connections;
        conns.erase(std::remove(conns.begin(), conns.end(), node_id),
                    conns.end());
    }

    auto it = std::find_if(comp->nodes.begin(), comp->nodes.end(),
                           [&](const auto& n) { return n.node_id == node_id; });
    if (it == comp->nodes.end()) return false;
    comp->nodes.erase(it);
    return true;
}

bool InteriorNavGraphSystem::connect_nodes(const std::string& entity_id,
                                            const std::string& node_a,
                                            const std::string& node_b) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (node_a == node_b) return false;

    components::InteriorNavGraphState::NavNode* na = nullptr;
    components::InteriorNavGraphState::NavNode* nb = nullptr;

    for (auto& n : comp->nodes) {
        if (n.node_id == node_a) na = &n;
        if (n.node_id == node_b) nb = &n;
    }
    if (!na || !nb) return false;

    // Check if already connected
    if (std::find(na->connections.begin(), na->connections.end(), node_b) !=
        na->connections.end()) {
        return false;
    }

    na->connections.push_back(node_b);
    nb->connections.push_back(node_a);
    return true;
}

bool InteriorNavGraphSystem::disconnect_nodes(const std::string& entity_id,
                                               const std::string& node_a,
                                               const std::string& node_b) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;

    components::InteriorNavGraphState::NavNode* na = nullptr;
    components::InteriorNavGraphState::NavNode* nb = nullptr;

    for (auto& n : comp->nodes) {
        if (n.node_id == node_a) na = &n;
        if (n.node_id == node_b) nb = &n;
    }
    if (!na || !nb) return false;

    auto ita = std::find(na->connections.begin(), na->connections.end(), node_b);
    auto itb = std::find(nb->connections.begin(), nb->connections.end(), node_a);
    if (ita == na->connections.end()) return false;

    na->connections.erase(ita);
    if (itb != nb->connections.end()) {
        nb->connections.erase(itb);
    }
    return true;
}

bool InteriorNavGraphSystem::clear_graph(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->nodes.clear();
    return true;
}

// ---------------------------------------------------------------------------
// Queries
// ---------------------------------------------------------------------------

int InteriorNavGraphSystem::get_node_count(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    return static_cast<int>(comp->nodes.size());
}

bool InteriorNavGraphSystem::has_node(const std::string& entity_id,
                                       const std::string& node_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (const auto& n : comp->nodes) {
        if (n.node_id == node_id) return true;
    }
    return false;
}

int InteriorNavGraphSystem::get_connection_count(const std::string& entity_id,
                                                  const std::string& node_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    for (const auto& n : comp->nodes) {
        if (n.node_id == node_id)
            return static_cast<int>(n.connections.size());
    }
    return 0;
}

bool InteriorNavGraphSystem::is_connected(const std::string& entity_id,
                                           const std::string& node_a,
                                           const std::string& node_b) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (const auto& n : comp->nodes) {
        if (n.node_id == node_a) {
            return std::find(n.connections.begin(), n.connections.end(), node_b) !=
                   n.connections.end();
        }
    }
    return false;
}

int InteriorNavGraphSystem::get_node_type(const std::string& entity_id,
                                           const std::string& node_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    for (const auto& n : comp->nodes) {
        if (n.node_id == node_id) return n.node_type;
    }
    return 0;
}

int InteriorNavGraphSystem::get_total_path_queries(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    return comp->total_path_queries;
}

// ---------------------------------------------------------------------------
// Validation — BFS from start_node, return true if all nodes reachable
// ---------------------------------------------------------------------------

bool InteriorNavGraphSystem::validate_connectivity(
        const std::string& entity_id,
        const std::string& start_node_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (comp->nodes.empty()) return true;

    comp->total_path_queries++;

    // Verify start node exists
    bool start_found = false;
    for (const auto& n : comp->nodes) {
        if (n.node_id == start_node_id) { start_found = true; break; }
    }
    if (!start_found) return false;

    // BFS
    std::unordered_set<std::string> visited;
    std::queue<std::string> frontier;
    frontier.push(start_node_id);
    visited.insert(start_node_id);

    while (!frontier.empty()) {
        std::string current = frontier.front();
        frontier.pop();

        for (const auto& n : comp->nodes) {
            if (n.node_id == current) {
                for (const auto& conn : n.connections) {
                    if (visited.find(conn) == visited.end()) {
                        visited.insert(conn);
                        frontier.push(conn);
                    }
                }
                break;
            }
        }
    }

    return static_cast<int>(visited.size()) == static_cast<int>(comp->nodes.size());
}

} // namespace systems
} // namespace atlas
