#include "systems/fps_inventory_system.h"
#include "ecs/world.h"
#include <algorithm>
#include <memory>

namespace atlas {
namespace systems {

FPSInventorySystem::FPSInventorySystem(ecs::World* world)
    : System(world) {
}

void FPSInventorySystem::update(float /*delta_time*/) {
    // Inventory is event-driven; no per-tick work required.
}

// ---------------------------------------------------------------------------
// Inventory management
// ---------------------------------------------------------------------------

bool FPSInventorySystem::createInventory(const std::string& player_id,
                                          int max_slots) {
    std::string eid = std::string(INV_PREFIX) + player_id;
    if (world_->getEntity(eid)) return false;

    auto* entity = world_->createEntity(eid);
    if (!entity) return false;

    auto comp = std::make_unique<components::FPSInventoryComponent>();
    comp->owner_id  = player_id;
    comp->max_slots = max_slots;
    entity->addComponent(std::move(comp));
    return true;
}

bool FPSInventorySystem::addItem(const std::string& player_id,
                                  const std::string& item_id,
                                  const std::string& item_name,
                                  int quantity) {
    std::string eid = std::string(INV_PREFIX) + player_id;
    auto* entity = world_->getEntity(eid);
    if (!entity) return false;
    auto* inv = entity->getComponent<components::FPSInventoryComponent>();
    if (!inv) return false;

    // Check for existing stack
    for (auto& slot : inv->slots) {
        if (slot.item_id == item_id) {
            slot.quantity += quantity;
            return true;
        }
    }

    // New slot
    if (inv->isFull()) return false;

    components::FPSInventoryComponent::InventorySlot slot;
    slot.item_id   = item_id;
    slot.item_name = item_name;
    slot.quantity  = quantity;
    inv->slots.push_back(slot);
    return true;
}

bool FPSInventorySystem::removeItem(const std::string& player_id,
                                     const std::string& item_id) {
    std::string eid = std::string(INV_PREFIX) + player_id;
    auto* entity = world_->getEntity(eid);
    if (!entity) return false;
    auto* inv = entity->getComponent<components::FPSInventoryComponent>();
    if (!inv) return false;

    auto it = std::find_if(inv->slots.begin(), inv->slots.end(),
        [&](const components::FPSInventoryComponent::InventorySlot& s) {
            return s.item_id == item_id;
        });
    if (it == inv->slots.end()) return false;

    it->quantity--;
    if (it->quantity <= 0) {
        // Unequip if this was the equipped weapon/tool
        if (inv->equipped_weapon_id == item_id) inv->equipped_weapon_id.clear();
        if (inv->equipped_tool_id == item_id)   inv->equipped_tool_id.clear();
        inv->slots.erase(it);
    }
    return true;
}

bool FPSInventorySystem::hasItem(const std::string& player_id,
                                  const std::string& item_id) const {
    std::string eid = std::string(INV_PREFIX) + player_id;
    auto* entity = world_->getEntity(eid);
    if (!entity) return false;
    auto* inv = entity->getComponent<components::FPSInventoryComponent>();
    return inv ? inv->hasItem(item_id) : false;
}

// ---------------------------------------------------------------------------
// Equipment
// ---------------------------------------------------------------------------

bool FPSInventorySystem::equipWeapon(const std::string& player_id,
                                      const std::string& weapon_item_id) {
    std::string eid = std::string(INV_PREFIX) + player_id;
    auto* entity = world_->getEntity(eid);
    if (!entity) return false;
    auto* inv = entity->getComponent<components::FPSInventoryComponent>();
    if (!inv) return false;
    if (!inv->hasItem(weapon_item_id)) return false;

    inv->equipped_weapon_id = weapon_item_id;
    return true;
}

bool FPSInventorySystem::unequipWeapon(const std::string& player_id) {
    std::string eid = std::string(INV_PREFIX) + player_id;
    auto* entity = world_->getEntity(eid);
    if (!entity) return false;
    auto* inv = entity->getComponent<components::FPSInventoryComponent>();
    if (!inv) return false;
    if (inv->equipped_weapon_id.empty()) return false;

    inv->equipped_weapon_id.clear();
    return true;
}

bool FPSInventorySystem::equipTool(const std::string& player_id,
                                    const std::string& tool_item_id) {
    std::string eid = std::string(INV_PREFIX) + player_id;
    auto* entity = world_->getEntity(eid);
    if (!entity) return false;
    auto* inv = entity->getComponent<components::FPSInventoryComponent>();
    if (!inv) return false;
    if (!inv->hasItem(tool_item_id)) return false;

    inv->equipped_tool_id = tool_item_id;
    return true;
}

bool FPSInventorySystem::unequipTool(const std::string& player_id) {
    std::string eid = std::string(INV_PREFIX) + player_id;
    auto* entity = world_->getEntity(eid);
    if (!entity) return false;
    auto* inv = entity->getComponent<components::FPSInventoryComponent>();
    if (!inv) return false;
    if (inv->equipped_tool_id.empty()) return false;

    inv->equipped_tool_id.clear();
    return true;
}

// ---------------------------------------------------------------------------
// Consumables
// ---------------------------------------------------------------------------

bool FPSInventorySystem::useConsumable(const std::string& player_id,
                                        const std::string& item_id) {
    // Must have the item
    if (!hasItem(player_id, item_id)) return false;

    // Find the player's FPS character to apply survival effects
    std::string char_eid = "fpschar_" + player_id;
    auto* char_entity = world_->getEntity(char_eid);

    // Apply effect based on item_id convention
    bool consumed = false;

    if (char_entity) {
        auto* needs = char_entity->getComponent<components::SurvivalNeeds>();
        if (needs) {
            if (item_id.find("oxygen") != std::string::npos) {
                needs->oxygen = std::min(100.0f, needs->oxygen + 50.0f);
                consumed = true;
            } else if (item_id.find("food") != std::string::npos ||
                       item_id.find("ration") != std::string::npos) {
                needs->hunger = std::max(0.0f, needs->hunger - 40.0f);
                consumed = true;
            } else if (item_id.find("stim") != std::string::npos) {
                needs->fatigue = std::max(0.0f, needs->fatigue - 40.0f);
                consumed = true;
            }
        }

        auto* health = char_entity->getComponent<components::FPSHealth>();
        if (health) {
            if (item_id.find("medkit") != std::string::npos) {
                health->health = std::min(health->health_max, health->health + 50.0f);
                consumed = true;
            }
        }
    }

    if (consumed) {
        removeItem(player_id, item_id);
    }

    return consumed;
}

// ---------------------------------------------------------------------------
// Queries
// ---------------------------------------------------------------------------

int FPSInventorySystem::getItemCount(const std::string& player_id) const {
    std::string eid = std::string(INV_PREFIX) + player_id;
    auto* entity = world_->getEntity(eid);
    if (!entity) return 0;
    auto* inv = entity->getComponent<components::FPSInventoryComponent>();
    return inv ? inv->itemCount() : 0;
}

int FPSInventorySystem::getMaxSlots(const std::string& player_id) const {
    std::string eid = std::string(INV_PREFIX) + player_id;
    auto* entity = world_->getEntity(eid);
    if (!entity) return 0;
    auto* inv = entity->getComponent<components::FPSInventoryComponent>();
    return inv ? inv->max_slots : 0;
}

bool FPSInventorySystem::isInventoryFull(const std::string& player_id) const {
    std::string eid = std::string(INV_PREFIX) + player_id;
    auto* entity = world_->getEntity(eid);
    if (!entity) return false;
    auto* inv = entity->getComponent<components::FPSInventoryComponent>();
    return inv ? inv->isFull() : false;
}

std::string FPSInventorySystem::getEquippedWeapon(const std::string& player_id) const {
    std::string eid = std::string(INV_PREFIX) + player_id;
    auto* entity = world_->getEntity(eid);
    if (!entity) return "";
    auto* inv = entity->getComponent<components::FPSInventoryComponent>();
    return inv ? inv->equipped_weapon_id : "";
}

std::string FPSInventorySystem::getEquippedTool(const std::string& player_id) const {
    std::string eid = std::string(INV_PREFIX) + player_id;
    auto* entity = world_->getEntity(eid);
    if (!entity) return "";
    auto* inv = entity->getComponent<components::FPSInventoryComponent>();
    return inv ? inv->equipped_tool_id : "";
}

} // namespace systems
} // namespace atlas
