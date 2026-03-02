#include "systems/dock_node_layout_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>
#include <cmath>

namespace atlas {
namespace systems {

DockNodeLayoutSystem::DockNodeLayoutSystem(ecs::World* world)
    : System(world) {
}

std::string DockNodeLayoutSystem::generateNodeId(components::DockNodeLayout* layout) {
    return "node_" + std::to_string(layout->nodes.size());
}

void DockNodeLayoutSystem::recalculateLayout(components::DockNodeLayout* layout,
                                              components::DockNodeLayout::DockNode* node) {
    if (!node) return;
    using NT = components::DockNodeLayout::NodeType;
    using SD = components::DockNodeLayout::SplitDirection;

    if (node->type == NT::Split) {
        auto* left = layout->findNode(node->left_child_id);
        auto* right = layout->findNode(node->right_child_id);
        if (left && right) {
            if (node->direction == SD::Horizontal) {
                float left_w = node->width * node->split_ratio;
                float right_w = node->width * (1.0f - node->split_ratio);
                left->x = node->x;
                left->y = node->y;
                left->width = left_w;
                left->height = node->height;
                right->x = node->x + left_w;
                right->y = node->y;
                right->width = right_w;
                right->height = node->height;
            } else {
                float top_h = node->height * node->split_ratio;
                float bot_h = node->height * (1.0f - node->split_ratio);
                left->x = node->x;
                left->y = node->y;
                left->width = node->width;
                left->height = top_h;
                right->x = node->x;
                right->y = node->y + top_h;
                right->width = node->width;
                right->height = bot_h;
            }
            recalculateLayout(layout, left);
            recalculateLayout(layout, right);
        }
    }
}

void DockNodeLayoutSystem::update(float delta_time) {
    auto entities = world_->getEntities<components::DockNodeLayout>();
    for (auto* entity : entities) {
        auto* layout = entity->getComponent<components::DockNodeLayout>();
        if (!layout) continue;

        auto* root = layout->findNode(layout->root_node_id);
        if (root) {
            recalculateLayout(layout, root);
        }
    }
}

bool DockNodeLayoutSystem::initializeLayout(const std::string& entity_id,
                                             const std::string& owner_id,
                                             float width, float height) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* existing = entity->getComponent<components::DockNodeLayout>();
    if (existing) return false;

    auto comp = std::make_unique<components::DockNodeLayout>();
    comp->layout_id = entity_id;
    comp->owner_id = owner_id;

    components::DockNodeLayout::DockNode root;
    root.node_id = "root";
    root.type = components::DockNodeLayout::NodeType::Root;
    root.direction = components::DockNodeLayout::SplitDirection::None;
    root.x = 0.0f;
    root.y = 0.0f;
    root.width = width;
    root.height = height;
    comp->nodes.push_back(root);
    comp->root_node_id = "root";

    entity->addComponent(std::move(comp));
    return true;
}

bool DockNodeLayoutSystem::addWindow(const std::string& entity_id,
                                      const std::string& window_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* layout = entity->getComponent<components::DockNodeLayout>();
    if (!layout) return false;

    if (layout->countLeaves() >= layout->max_windows) return false;

    using NT = components::DockNodeLayout::NodeType;

    // Find first leaf or root without a window
    for (auto& node : layout->nodes) {
        if ((node.type == NT::Leaf || node.type == NT::Root) && node.window_id.empty()) {
            node.window_id = window_id;
            if (node.type == NT::Root) {
                node.type = NT::Leaf;
            }
            layout->total_docks++;
            return true;
        }
    }

    // No empty leaf found - split the first leaf
    for (size_t i = 0; i < layout->nodes.size(); i++) {
        if (layout->nodes[i].type == NT::Leaf && !layout->nodes[i].window_id.empty()) {
            std::string target_id = layout->nodes[i].node_id;
            splitNode(entity_id, target_id,
                      components::DockNodeLayout::SplitDirection::Horizontal, 0.5f);
            // Now find the new empty leaf and assign window
            auto* lay = entity->getComponent<components::DockNodeLayout>();
            auto* split = lay->findNode(target_id);
            if (split) {
                auto* right = lay->findNode(split->right_child_id);
                if (right) {
                    right->window_id = window_id;
                    lay->total_docks++;
                    return true;
                }
            }
            break;
        }
    }

    return false;
}

bool DockNodeLayoutSystem::removeWindow(const std::string& entity_id,
                                         const std::string& window_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* layout = entity->getComponent<components::DockNodeLayout>();
    if (!layout) return false;

    for (auto& node : layout->nodes) {
        if (node.window_id == window_id) {
            node.window_id.clear();
            layout->total_undocks++;
            return true;
        }
    }
    return false;
}

bool DockNodeLayoutSystem::splitNode(const std::string& entity_id,
                                      const std::string& node_id,
                                      components::DockNodeLayout::SplitDirection direction,
                                      float ratio) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* layout = entity->getComponent<components::DockNodeLayout>();
    if (!layout) return false;

    auto* node = layout->findNode(node_id);
    if (!node) return false;

    using NT = components::DockNodeLayout::NodeType;
    if (node->type == NT::Split) return false;

    std::string old_window = node->window_id;
    float clamped_ratio = std::max(0.1f, std::min(0.9f, ratio));

    // Create left child
    components::DockNodeLayout::DockNode left;
    left.node_id = generateNodeId(layout);
    left.type = NT::Leaf;
    left.window_id = old_window;
    layout->nodes.push_back(left);

    // Create right child
    components::DockNodeLayout::DockNode right;
    right.node_id = generateNodeId(layout);
    right.type = NT::Leaf;
    layout->nodes.push_back(right);

    // Refind node after push_back (pointers may be invalidated)
    node = layout->findNode(node_id);
    if (!node) return false;

    node->type = NT::Split;
    node->direction = direction;
    node->split_ratio = clamped_ratio;
    node->window_id.clear();
    node->left_child_id = left.node_id;
    node->right_child_id = right.node_id;

    // Recalculate child bounds
    recalculateLayout(layout, node);

    return true;
}

int DockNodeLayoutSystem::getWindowCount(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0;

    auto* layout = entity->getComponent<components::DockNodeLayout>();
    if (!layout) return 0;

    return layout->countLeaves();
}

std::string DockNodeLayoutSystem::getNodeType(const std::string& entity_id,
                                               const std::string& node_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return "unknown";

    auto* layout = entity->getComponent<components::DockNodeLayout>();
    if (!layout) return "unknown";

    const auto* node = layout->findNode(node_id);
    if (!node) return "unknown";

    using NT = components::DockNodeLayout::NodeType;
    switch (node->type) {
        case NT::Root: return "root";
        case NT::Split: return "split";
        case NT::Leaf: return "leaf";
    }
    return "unknown";
}

bool DockNodeLayoutSystem::dockWindow(const std::string& entity_id,
                                       const std::string& window_id,
                                       const std::string& target_node_id,
                                       components::DockNodeLayout::SplitDirection direction) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* layout = entity->getComponent<components::DockNodeLayout>();
    if (!layout) return false;

    if (layout->countLeaves() >= layout->max_windows) return false;

    auto* target = layout->findNode(target_node_id);
    if (!target) return false;

    using NT = components::DockNodeLayout::NodeType;
    if (target->type == NT::Split) return false;

    if (!splitNode(entity_id, target_node_id, direction, 0.5f)) return false;

    // Re-fetch layout (splitNode may invalidate pointers)
    layout = entity->getComponent<components::DockNodeLayout>();
    target = layout->findNode(target_node_id);
    if (!target) return false;

    auto* right = layout->findNode(target->right_child_id);
    if (right) {
        right->window_id = window_id;
        layout->total_docks++;
        return true;
    }
    return false;
}

bool DockNodeLayoutSystem::undockWindow(const std::string& entity_id,
                                         const std::string& window_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* layout = entity->getComponent<components::DockNodeLayout>();
    if (!layout) return false;

    for (auto& node : layout->nodes) {
        if (node.window_id == window_id) {
            node.window_id.clear();
            layout->total_undocks++;
            return true;
        }
    }
    return false;
}

std::tuple<float, float, float, float> DockNodeLayoutSystem::getWindowBounds(
        const std::string& entity_id, const std::string& window_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return std::make_tuple(0.0f, 0.0f, 0.0f, 0.0f);

    auto* layout = entity->getComponent<components::DockNodeLayout>();
    if (!layout) return std::make_tuple(0.0f, 0.0f, 0.0f, 0.0f);

    for (const auto& node : layout->nodes) {
        if (node.window_id == window_id) {
            return std::make_tuple(node.x, node.y, node.width, node.height);
        }
    }
    return std::make_tuple(0.0f, 0.0f, 0.0f, 0.0f);
}

} // namespace systems
} // namespace atlas
