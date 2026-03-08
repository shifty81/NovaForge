#include "systems/loot_table_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>
#include <cmath>

namespace atlas {
namespace systems {

LootTableSystem::LootTableSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void LootTableSystem::updateComponent(ecs::Entity& /*entity*/,
    components::LootTableState& state, float delta_time) {
    if (!state.active) return;
    state.elapsed += delta_time;
}

bool LootTableSystem::initialize(const std::string& entity_id,
    const std::string& table_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    if (table_id.empty()) return false;
    auto comp = std::make_unique<components::LootTableState>();
    comp->table_id = table_id;
    entity->addComponent(std::move(comp));
    return true;
}

bool LootTableSystem::addEntry(const std::string& entity_id, const std::string& item_id,
    const std::string& rarity, float weight, int min_qty, int max_qty) {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    if (item_id.empty() || rarity.empty()) return false;
    if (weight <= 0.0f) return false;
    if (min_qty < 1 || max_qty < min_qty) return false;
    if (static_cast<int>(state->entries.size()) >= state->max_entries) return false;
    // Check duplicate
    for (const auto& e : state->entries) {
        if (e.item_id == item_id) return false;
    }
    components::LootTableState::LootEntry entry;
    entry.item_id = item_id;
    entry.rarity = rarity;
    entry.weight = weight;
    entry.min_quantity = min_qty;
    entry.max_quantity = max_qty;
    state->entries.push_back(entry);
    return true;
}

bool LootTableSystem::removeEntry(const std::string& entity_id, const std::string& item_id) {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    auto it = std::find_if(state->entries.begin(), state->entries.end(),
        [&](const components::LootTableState::LootEntry& e) {
            return e.item_id == item_id;
        });
    if (it == state->entries.end()) return false;
    state->entries.erase(it);
    return true;
}

bool LootTableSystem::setLuckModifier(const std::string& entity_id, float luck) {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    state->luck_modifier = std::max(0.1f, std::min(10.0f, luck));
    return true;
}

std::string LootTableSystem::rollLoot(const std::string& entity_id, float seed_value) {
    auto* state = getComponentFor(entity_id);
    if (!state || state->entries.empty()) return "";

    // Compute adjusted weights (luck boosts rare+ items)
    float total_w = 0.0f;
    std::vector<float> weights;
    weights.reserve(state->entries.size());
    for (const auto& e : state->entries) {
        float w = e.weight;
        if (e.rarity != "common") {
            w *= state->luck_modifier;
        }
        weights.push_back(w);
        total_w += w;
    }
    if (total_w <= 0.0f) return "";

    // Deterministic selection using seed
    float roll = std::fmod(std::fabs(seed_value), total_w);
    float cumulative = 0.0f;
    for (size_t i = 0; i < state->entries.size(); ++i) {
        cumulative += weights[i];
        if (roll < cumulative) {
            state->total_rolls++;
            state->total_drops++;
            return state->entries[i].item_id;
        }
    }
    // Fallback: last entry
    state->total_rolls++;
    state->total_drops++;
    return state->entries.back().item_id;
}

std::vector<std::string> LootTableSystem::getEntriesByRarity(const std::string& entity_id,
    const std::string& rarity) const {
    std::vector<std::string> result;
    auto* state = getComponentFor(entity_id);
    if (!state) return result;
    for (const auto& e : state->entries) {
        if (e.rarity == rarity) result.push_back(e.item_id);
    }
    return result;
}

int LootTableSystem::getEntryCount(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    return state ? static_cast<int>(state->entries.size()) : 0;
}

float LootTableSystem::getTotalWeight(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    return state ? state->totalWeight() : 0.0f;
}

int LootTableSystem::getTotalRolls(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    return state ? state->total_rolls : 0;
}

int LootTableSystem::getTotalDrops(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    return state ? state->total_drops : 0;
}

float LootTableSystem::getLuckModifier(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    return state ? state->luck_modifier : 1.0f;
}

std::string LootTableSystem::getTableId(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    return state ? state->table_id : "";
}

} // namespace systems
} // namespace atlas
