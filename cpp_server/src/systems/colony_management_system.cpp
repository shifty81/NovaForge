#include "systems/colony_management_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include "components/economy_components.h"
#include <algorithm>
#include <cmath>

namespace atlas {
namespace systems {

ColonyManagementSystem::ColonyManagementSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void ColonyManagementSystem::updateComponent(ecs::Entity& entity,
    components::ColonyManagementState& state, float delta_time) {
    if (!state.active) return;

    // Recalculate power usage
    float power = 0.0f;
    for (const auto& b : state.buildings) {
        if (b.online) power += b.power_usage;
    }
    state.power_used = power;

    // Production cycle: online buildings produce goods over time
    bool any_production = false;
    for (const auto& b : state.buildings) {
        if (b.online && b.building_type == "extractor") {
            any_production = true;
        }
    }
    if (any_production) {
        state.total_production_cycles++;
    }

    state.elapsed += delta_time;
}

bool ColonyManagementSystem::initialize(const std::string& entity_id,
    const std::string& colony_name, const std::string& planet_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    if (colony_name.empty() || planet_id.empty()) return false;
    auto comp = std::make_unique<components::ColonyManagementState>();
    comp->colony_name = colony_name;
    comp->planet_id = planet_id;
    entity->addComponent(std::move(comp));
    return true;
}

bool ColonyManagementSystem::addBuilding(const std::string& entity_id,
    const std::string& building_id, const std::string& building_type,
    float production_rate, float power_usage) {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    if (building_id.empty() || building_type.empty()) return false;
    if (static_cast<int>(state->buildings.size()) >= state->max_buildings) return false;
    // Check duplicate
    for (const auto& b : state->buildings) {
        if (b.building_id == building_id) return false;
    }
    // Check power budget
    if (state->power_used + power_usage > state->power_capacity) return false;
    components::ColonyManagementState::Building building;
    building.building_id = building_id;
    building.building_type = building_type;
    building.production_rate = std::max(0.1f, production_rate);
    building.power_usage = std::max(0.0f, power_usage);
    state->buildings.push_back(building);
    state->power_used += building.power_usage;
    return true;
}

bool ColonyManagementSystem::removeBuilding(const std::string& entity_id,
    const std::string& building_id) {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    auto it = std::find_if(state->buildings.begin(), state->buildings.end(),
        [&](const components::ColonyManagementState::Building& b) {
            return b.building_id == building_id;
        });
    if (it == state->buildings.end()) return false;
    if (it->online) state->power_used -= it->power_usage;
    state->buildings.erase(it);
    return true;
}

bool ColonyManagementSystem::toggleBuilding(const std::string& entity_id,
    const std::string& building_id) {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    for (auto& b : state->buildings) {
        if (b.building_id == building_id) {
            if (b.online) {
                b.online = false;
                state->power_used -= b.power_usage;
            } else {
                if (state->power_used + b.power_usage > state->power_capacity) return false;
                b.online = true;
                state->power_used += b.power_usage;
            }
            return true;
        }
    }
    return false;
}

bool ColonyManagementSystem::addGoods(const std::string& entity_id,
    const std::string& good_type, float quantity, float max_quantity) {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    if (good_type.empty() || quantity < 0.0f) return false;
    // Check if good type already exists
    for (auto& g : state->stored_goods) {
        if (g.good_type == good_type) {
            g.quantity = std::min(g.max_quantity, g.quantity + quantity);
            return true;
        }
    }
    components::ColonyManagementState::StoredGood good;
    good.good_type = good_type;
    good.quantity = std::min(max_quantity, quantity);
    good.max_quantity = max_quantity;
    state->stored_goods.push_back(good);
    return true;
}

bool ColonyManagementSystem::exportGoods(const std::string& entity_id,
    const std::string& good_type, float quantity, float unit_price) {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    if (good_type.empty() || quantity <= 0.0f || unit_price <= 0.0f) return false;
    for (auto& g : state->stored_goods) {
        if (g.good_type == good_type) {
            if (g.quantity < quantity) return false;
            g.quantity -= quantity;
            state->total_exports += quantity;
            state->total_export_value += quantity * unit_price;
            return true;
        }
    }
    return false;
}

bool ColonyManagementSystem::setPowerCapacity(const std::string& entity_id, float capacity) {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    state->power_capacity = std::max(10.0f, std::min(10000.0f, capacity));
    return true;
}

int ColonyManagementSystem::getBuildingCount(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    return state ? static_cast<int>(state->buildings.size()) : 0;
}

int ColonyManagementSystem::getOnlineBuildingCount(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    if (!state) return 0;
    int count = 0;
    for (const auto& b : state->buildings) {
        if (b.online) count++;
    }
    return count;
}

float ColonyManagementSystem::getPowerUsed(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    return state ? state->power_used : 0.0f;
}

float ColonyManagementSystem::getRemainingPower(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    return state ? state->remainingPower() : 0.0f;
}

float ColonyManagementSystem::getGoodsQuantity(const std::string& entity_id,
    const std::string& good_type) const {
    auto* state = getComponentFor(entity_id);
    if (!state) return 0.0f;
    for (const auto& g : state->stored_goods) {
        if (g.good_type == good_type) return g.quantity;
    }
    return 0.0f;
}

float ColonyManagementSystem::getTotalExports(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    return state ? state->total_exports : 0.0f;
}

float ColonyManagementSystem::getTotalExportValue(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    return state ? state->total_export_value : 0.0f;
}

int ColonyManagementSystem::getTotalProductionCycles(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    return state ? state->total_production_cycles : 0;
}

std::string ColonyManagementSystem::getColonyName(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    return state ? state->colony_name : "";
}

} // namespace systems
} // namespace atlas
