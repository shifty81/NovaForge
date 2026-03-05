#include "systems/grid_construction_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>
#include <cmath>

namespace atlas {
namespace systems {

GridConstructionSystem::GridConstructionSystem(ecs::World* world)
    : SingleComponentSystem(world) {
}

void GridConstructionSystem::updateComponent(ecs::Entity& /*entity*/, components::GridConstruction& grid, float /*delta_time*/) {
    if (!grid.is_powered) {
        // Unpower all cells when grid power is off
        for (int y = 0; y < grid.grid_height; y++) {
            for (int x = 0; x < grid.grid_width; x++) {
                grid.cells[y][x].is_powered = false;
            }
        }
        grid.powered_cell_count = 0;
        return;
    }

    // Propagate power from PowerNodes to adjacent modules
    // First reset power state
    for (int y = 0; y < grid.grid_height; y++) {
        for (int x = 0; x < grid.grid_width; x++) {
            grid.cells[y][x].is_powered = false;
        }
    }

    // Power nodes are always powered
    for (int y = 0; y < grid.grid_height; y++) {
        for (int x = 0; x < grid.grid_width; x++) {
            if (grid.cells[y][x].module_type == components::GridConstruction::ModuleType::PowerNode) {
                grid.cells[y][x].is_powered = true;
                // Power adjacent cells
                int dx[] = {-1, 1, 0, 0};
                int dy[] = {0, 0, -1, 1};
                for (int d = 0; d < 4; d++) {
                    int nx = x + dx[d];
                    int ny = y + dy[d];
                    if (grid.inBounds(nx, ny) &&
                        grid.cells[ny][nx].module_type != components::GridConstruction::ModuleType::Empty) {
                        grid.cells[ny][nx].is_powered = true;
                    }
                }
            }
        }
    }

    // Count powered cells and calculate power balance
    grid.powered_cell_count = 0;
    grid.total_power_generation = 0.0f;
    grid.total_power_consumption = 0.0f;
    for (int y = 0; y < grid.grid_height; y++) {
        for (int x = 0; x < grid.grid_width; x++) {
            if (grid.cells[y][x].is_powered) {
                grid.powered_cell_count++;
            }
            if (grid.cells[y][x].module_type == components::GridConstruction::ModuleType::PowerNode) {
                grid.total_power_generation += 10.0f;
            } else if (grid.cells[y][x].module_type != components::GridConstruction::ModuleType::Empty) {
                grid.total_power_consumption += 2.0f;
            }
        }
    }
}

bool GridConstructionSystem::initializeGrid(const std::string& entity_id,
                                             const std::string& owner_id,
                                             int width, int height) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* existing = entity->getComponent<components::GridConstruction>();
    if (existing) return false;

    auto comp = std::make_unique<components::GridConstruction>();
    comp->owner_entity_id = owner_id;
    comp->initCells(std::max(1, width), std::max(1, height));
    entity->addComponent(std::move(comp));
    return true;
}

bool GridConstructionSystem::placeModule(const std::string& entity_id, int x, int y,
                                          components::GridConstruction::ModuleType module_type) {
    auto* grid = getComponentFor(entity_id);
    if (!grid) return false;

    if (!grid->inBounds(x, y)) return false;
    if (grid->cells[y][x].module_type != components::GridConstruction::ModuleType::Empty) return false;

    grid->cells[y][x].module_type = module_type;
    grid->cells[y][x].health = 1.0f;
    grid->cells[y][x].is_powered = false;
    grid->total_modules_placed++;
    return true;
}

bool GridConstructionSystem::removeModule(const std::string& entity_id, int x, int y) {
    auto* grid = getComponentFor(entity_id);
    if (!grid) return false;

    if (!grid->inBounds(x, y)) return false;
    if (grid->cells[y][x].module_type == components::GridConstruction::ModuleType::Empty) return false;

    grid->cells[y][x].module_type = components::GridConstruction::ModuleType::Empty;
    grid->cells[y][x].module_id.clear();
    grid->cells[y][x].health = 1.0f;
    grid->cells[y][x].is_powered = false;
    return true;
}

std::string GridConstructionSystem::getModuleAt(const std::string& entity_id, int x, int y) const {
    const auto* grid = getComponentFor(entity_id);
    if (!grid) return "unknown";

    if (!grid->inBounds(x, y)) return "unknown";

    return components::GridConstruction::moduleTypeToString(grid->cells[y][x].module_type);
}

float GridConstructionSystem::getModuleHealth(const std::string& entity_id, int x, int y) const {
    const auto* grid = getComponentFor(entity_id);
    if (!grid) return 0.0f;

    if (!grid->inBounds(x, y)) return 0.0f;

    return grid->cells[y][x].health;
}

int GridConstructionSystem::getModuleCount(const std::string& entity_id) const {
    const auto* grid = getComponentFor(entity_id);
    if (!grid) return 0;

    return grid->getModuleCount();
}

int GridConstructionSystem::getPoweredCount(const std::string& entity_id) const {
    const auto* grid = getComponentFor(entity_id);
    if (!grid) return 0;

    return grid->powered_cell_count;
}

int GridConstructionSystem::getGridWidth(const std::string& entity_id) const {
    const auto* grid = getComponentFor(entity_id);
    if (!grid) return 0;

    return grid->grid_width;
}

int GridConstructionSystem::getGridHeight(const std::string& entity_id) const {
    const auto* grid = getComponentFor(entity_id);
    if (!grid) return 0;

    return grid->grid_height;
}

bool GridConstructionSystem::damageModule(const std::string& entity_id, int x, int y, float amount) {
    auto* grid = getComponentFor(entity_id);
    if (!grid) return false;

    if (!grid->inBounds(x, y)) return false;
    if (grid->cells[y][x].module_type == components::GridConstruction::ModuleType::Empty) return false;

    grid->cells[y][x].health = std::max(0.0f, grid->cells[y][x].health - amount);
    return true;
}

bool GridConstructionSystem::repairModule(const std::string& entity_id, int x, int y, float amount) {
    auto* grid = getComponentFor(entity_id);
    if (!grid) return false;

    if (!grid->inBounds(x, y)) return false;
    if (grid->cells[y][x].module_type == components::GridConstruction::ModuleType::Empty) return false;

    grid->cells[y][x].health = std::min(1.0f, grid->cells[y][x].health + amount);
    return true;
}

float GridConstructionSystem::calculateIntegrity(const std::string& entity_id) {
    auto* grid = getComponentFor(entity_id);
    if (!grid) return 0.0f;

    int module_count = 0;
    float total_integrity = 0.0f;

    for (int y = 0; y < grid->grid_height; y++) {
        for (int x = 0; x < grid->grid_width; x++) {
            if (grid->cells[y][x].module_type == components::GridConstruction::ModuleType::Empty) continue;

            module_count++;
            float base = grid->cells[y][x].health;

            // Count adjacent non-empty neighbors
            int adjacent = 0;
            int dx[] = {-1, 1, 0, 0};
            int dy[] = {0, 0, -1, 1};
            for (int d = 0; d < 4; d++) {
                int nx = x + dx[d];
                int ny = y + dy[d];
                if (grid->inBounds(nx, ny) &&
                    grid->cells[ny][nx].module_type != components::GridConstruction::ModuleType::Empty) {
                    adjacent++;
                }
            }

            // Modules with 3+ adjacent non-empty neighbors get +0.1 bonus
            if (adjacent >= 3) {
                base = std::min(1.0f, base + 0.1f);
            }

            total_integrity += base;
        }
    }

    if (module_count == 0) {
        grid->structural_integrity = 0.0f;
    } else {
        grid->structural_integrity = total_integrity / static_cast<float>(module_count);
    }

    return grid->structural_integrity;
}

float GridConstructionSystem::calculatePower(const std::string& entity_id) {
    auto* grid = getComponentFor(entity_id);
    if (!grid) return 0.0f;

    grid->total_power_generation = 0.0f;
    grid->total_power_consumption = 0.0f;

    for (int y = 0; y < grid->grid_height; y++) {
        for (int x = 0; x < grid->grid_width; x++) {
            if (grid->cells[y][x].module_type == components::GridConstruction::ModuleType::PowerNode) {
                grid->total_power_generation += 10.0f;
            } else if (grid->cells[y][x].module_type != components::GridConstruction::ModuleType::Empty) {
                grid->total_power_consumption += 2.0f;
            }
        }
    }

    return grid->total_power_generation - grid->total_power_consumption;
}

bool GridConstructionSystem::setPowerEnabled(const std::string& entity_id, bool enabled) {
    auto* grid = getComponentFor(entity_id);
    if (!grid) return false;

    grid->is_powered = enabled;
    return true;
}

} // namespace systems
} // namespace atlas
