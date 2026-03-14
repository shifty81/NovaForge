#include "systems/fighter_squadron_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

FighterSquadronSystem::FighterSquadronSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

// ---------------------------------------------------------------------------
// Tick
// ---------------------------------------------------------------------------

void FighterSquadronSystem::updateComponent(ecs::Entity& /*entity*/,
                                             components::FighterSquadronState& comp,
                                             float delta_time) {
    if (!comp.active) return;
    comp.elapsed += delta_time;
}

// ---------------------------------------------------------------------------
// Lifecycle
// ---------------------------------------------------------------------------

bool FighterSquadronSystem::initialize(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::FighterSquadronState>();
    entity->addComponent(std::move(comp));
    return true;
}

// ---------------------------------------------------------------------------
// Squadron management
// ---------------------------------------------------------------------------

bool FighterSquadronSystem::addSquadron(
        const std::string& entity_id,
        const std::string& squadron_id,
        const std::string& name,
        components::FighterSquadronState::SquadronType type,
        int max_health,
        int max_ammo) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (squadron_id.empty()) return false;
    if (name.empty()) return false;
    if (max_health <= 0) return false;
    if (max_ammo < 0) return false;

    for (const auto& s : comp->squadrons) {
        if (s.squadron_id == squadron_id) return false;
    }

    if (static_cast<int>(comp->squadrons.size()) >= comp->max_squadrons) return false;

    components::FighterSquadronState::Squadron sq;
    sq.squadron_id    = squadron_id;
    sq.name           = name;
    sq.type           = type;
    sq.max_health     = max_health;
    sq.current_health = max_health;
    sq.max_ammo       = max_ammo;
    sq.current_ammo   = max_ammo;
    sq.launched       = false;
    sq.kills          = 0;
    comp->squadrons.push_back(sq);
    return true;
}

bool FighterSquadronSystem::removeSquadron(const std::string& entity_id,
                                            const std::string& squadron_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    auto it = std::find_if(comp->squadrons.begin(), comp->squadrons.end(),
        [&](const components::FighterSquadronState::Squadron& s) {
            return s.squadron_id == squadron_id;
        });
    if (it == comp->squadrons.end()) return false;
    comp->squadrons.erase(it);
    return true;
}

bool FighterSquadronSystem::clearSquadrons(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->squadrons.clear();
    return true;
}

// ---------------------------------------------------------------------------
// Launch / Recall
// ---------------------------------------------------------------------------

bool FighterSquadronSystem::launchSquadron(const std::string& entity_id,
                                            const std::string& squadron_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;

    auto it = std::find_if(comp->squadrons.begin(), comp->squadrons.end(),
        [&](const components::FighterSquadronState::Squadron& s) {
            return s.squadron_id == squadron_id;
        });
    if (it == comp->squadrons.end()) return false;
    if (it->launched) return false;
    if (it->current_health <= 0) return false;

    // Check launched cap
    int launched_count = 0;
    for (const auto& s : comp->squadrons) {
        if (s.launched) launched_count++;
    }
    if (launched_count >= comp->max_squadrons) return false;

    it->launched = true;
    comp->total_launched++;
    return true;
}

bool FighterSquadronSystem::recallSquadron(const std::string& entity_id,
                                            const std::string& squadron_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;

    auto it = std::find_if(comp->squadrons.begin(), comp->squadrons.end(),
        [&](const components::FighterSquadronState::Squadron& s) {
            return s.squadron_id == squadron_id;
        });
    if (it == comp->squadrons.end()) return false;
    if (!it->launched) return false;

    it->launched = false;
    comp->total_recalled++;
    return true;
}

bool FighterSquadronSystem::recallAll(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (auto& s : comp->squadrons) {
        if (s.launched) {
            s.launched = false;
            comp->total_recalled++;
        }
    }
    return true;
}

// ---------------------------------------------------------------------------
// Combat
// ---------------------------------------------------------------------------

bool FighterSquadronSystem::recordKill(const std::string& entity_id,
                                        const std::string& squadron_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;

    auto it = std::find_if(comp->squadrons.begin(), comp->squadrons.end(),
        [&](const components::FighterSquadronState::Squadron& s) {
            return s.squadron_id == squadron_id;
        });
    if (it == comp->squadrons.end()) return false;
    if (!it->launched) return false;

    it->kills++;
    comp->total_kills++;
    return true;
}

bool FighterSquadronSystem::applyDamage(const std::string& entity_id,
                                         const std::string& squadron_id,
                                         int damage) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (damage <= 0) return false;

    auto it = std::find_if(comp->squadrons.begin(), comp->squadrons.end(),
        [&](const components::FighterSquadronState::Squadron& s) {
            return s.squadron_id == squadron_id;
        });
    if (it == comp->squadrons.end()) return false;
    if (!it->launched) return false;

    it->current_health -= damage;
    if (it->current_health <= 0) {
        it->current_health = 0;
        it->launched = false;
        comp->total_recalled++;
    }
    return true;
}

// ---------------------------------------------------------------------------
// Maintenance
// ---------------------------------------------------------------------------

bool FighterSquadronSystem::resupplyAmmo(const std::string& entity_id,
                                          const std::string& squadron_id,
                                          int amount) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (amount <= 0) return false;

    auto it = std::find_if(comp->squadrons.begin(), comp->squadrons.end(),
        [&](const components::FighterSquadronState::Squadron& s) {
            return s.squadron_id == squadron_id;
        });
    if (it == comp->squadrons.end()) return false;
    if (it->launched) return false;  // must be docked to resupply

    it->current_ammo = std::min(it->current_ammo + amount, it->max_ammo);
    return true;
}

bool FighterSquadronSystem::repairSquadron(const std::string& entity_id,
                                            const std::string& squadron_id,
                                            int amount) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (amount <= 0) return false;

    auto it = std::find_if(comp->squadrons.begin(), comp->squadrons.end(),
        [&](const components::FighterSquadronState::Squadron& s) {
            return s.squadron_id == squadron_id;
        });
    if (it == comp->squadrons.end()) return false;
    if (it->launched) return false;  // must be docked to repair

    it->current_health = std::min(it->current_health + amount, it->max_health);
    return true;
}

// ---------------------------------------------------------------------------
// Queries
// ---------------------------------------------------------------------------

int FighterSquadronSystem::getSquadronCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? static_cast<int>(comp->squadrons.size()) : 0;
}

int FighterSquadronSystem::getLaunchedCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    int count = 0;
    for (const auto& s : comp->squadrons) {
        if (s.launched) count++;
    }
    return count;
}

bool FighterSquadronSystem::isLaunched(const std::string& entity_id,
                                        const std::string& squadron_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (const auto& s : comp->squadrons) {
        if (s.squadron_id == squadron_id) return s.launched;
    }
    return false;
}

int FighterSquadronSystem::getHealth(const std::string& entity_id,
                                      const std::string& squadron_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    for (const auto& s : comp->squadrons) {
        if (s.squadron_id == squadron_id) return s.current_health;
    }
    return 0;
}

int FighterSquadronSystem::getAmmo(const std::string& entity_id,
                                    const std::string& squadron_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    for (const auto& s : comp->squadrons) {
        if (s.squadron_id == squadron_id) return s.current_ammo;
    }
    return 0;
}

int FighterSquadronSystem::getTotalLaunched(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_launched : 0;
}

int FighterSquadronSystem::getTotalRecalled(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_recalled : 0;
}

int FighterSquadronSystem::getTotalKills(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_kills : 0;
}

bool FighterSquadronSystem::hasSquadron(const std::string& entity_id,
                                         const std::string& squadron_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (const auto& s : comp->squadrons) {
        if (s.squadron_id == squadron_id) return true;
    }
    return false;
}

int FighterSquadronSystem::getCountByType(
        const std::string& entity_id,
        components::FighterSquadronState::SquadronType type) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    int count = 0;
    for (const auto& s : comp->squadrons) {
        if (s.type == type) count++;
    }
    return count;
}

} // namespace systems
} // namespace atlas
