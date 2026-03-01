#include "systems/fps_spawn_system.h"
#include "ecs/world.h"

namespace atlas {
namespace systems {

FPSSpawnSystem::FPSSpawnSystem(ecs::World* world)
    : System(world) {
}

void FPSSpawnSystem::update(float /*delta_time*/) {
    // Spawn points are static — queries happen on demand.
}

// ---------------------------------------------------------------------------
// Spawn point creation
// ---------------------------------------------------------------------------

bool FPSSpawnSystem::createSpawnPoint(
        const std::string& spawn_id,
        const std::string& parent_id,
        components::FPSSpawnPoint::SpawnContext context,
        float x, float y, float z,
        float yaw) {

    if (world_->getEntity(spawn_id)) return false;

    auto* entity = world_->createEntity(spawn_id);
    if (!entity) return false;

    auto sp = std::make_unique<components::FPSSpawnPoint>();
    sp->spawn_id         = spawn_id;
    sp->parent_entity_id = parent_id;
    sp->context          = context;
    sp->pos_x            = x;
    sp->pos_y            = y;
    sp->pos_z            = z;
    sp->yaw              = yaw;
    sp->is_active        = true;
    entity->addComponent(std::move(sp));
    return true;
}

// ---------------------------------------------------------------------------
// Spawn resolution
// ---------------------------------------------------------------------------

std::string FPSSpawnSystem::findSpawnForPlayer(const std::string& player_id) const {
    auto* player_entity = world_->getEntity(player_id);
    if (!player_entity) return "";

    // Check if player is docked at a station.
    auto* docked = player_entity->getComponent<components::Docked>();

    // Determine desired context.
    components::FPSSpawnPoint::SpawnContext desired =
        components::FPSSpawnPoint::SpawnContext::ShipBridge;

    if (docked) {
        // Check if there's a hangar owned by this player at the station.
        bool has_hangar = false;
        for (auto* ent : world_->getEntities<components::StationHangar>()) {
            auto* hangar = ent->getComponent<components::StationHangar>();
            if (hangar && hangar->station_id == docked->station_id &&
                hangar->owner_id == player_id) {
                has_hangar = true;
                break;
            }
        }

        // Check if there's a tether arm with this player's ship.
        bool has_tether = false;
        for (auto* ent : world_->getEntities<components::TetherDockingArm>()) {
            auto* arm = ent->getComponent<components::TetherDockingArm>();
            if (arm && arm->station_id == docked->station_id &&
                arm->tethered_ship_id == player_id &&
                arm->crew_transfer_enabled) {
                has_tether = true;
                break;
            }
        }

        if (has_hangar) {
            desired = components::FPSSpawnPoint::SpawnContext::Hangar;
        } else if (has_tether) {
            desired = components::FPSSpawnPoint::SpawnContext::TetherAirlock;
        } else {
            desired = components::FPSSpawnPoint::SpawnContext::StationLobby;
        }
    }

    // Find a matching active spawn point.
    std::string fallback;
    for (auto* ent : world_->getEntities<components::FPSSpawnPoint>()) {
        auto* sp = ent->getComponent<components::FPSSpawnPoint>();
        if (!sp || !sp->is_active) continue;

        if (fallback.empty()) fallback = sp->spawn_id;

        if (sp->context == desired) {
            return sp->spawn_id;
        }
    }

    return fallback;
}

// ---------------------------------------------------------------------------
// Queries
// ---------------------------------------------------------------------------

std::tuple<float, float, float, float> FPSSpawnSystem::getSpawnTransform(
        const std::string& spawn_id) const {
    auto* entity = world_->getEntity(spawn_id);
    if (!entity) return {0.0f, 0.0f, 0.0f, 0.0f};

    auto* sp = entity->getComponent<components::FPSSpawnPoint>();
    if (!sp) return {0.0f, 0.0f, 0.0f, 0.0f};

    return {sp->pos_x, sp->pos_y, sp->pos_z, sp->yaw};
}

components::FPSSpawnPoint::SpawnContext
FPSSpawnSystem::getSpawnContext(const std::string& spawn_id) const {
    auto* entity = world_->getEntity(spawn_id);
    if (!entity) return components::FPSSpawnPoint::SpawnContext::Hangar;

    auto* sp = entity->getComponent<components::FPSSpawnPoint>();
    if (!sp) return components::FPSSpawnPoint::SpawnContext::Hangar;

    return sp->context;
}

bool FPSSpawnSystem::setSpawnActive(const std::string& spawn_id, bool active) {
    auto* entity = world_->getEntity(spawn_id);
    if (!entity) return false;

    auto* sp = entity->getComponent<components::FPSSpawnPoint>();
    if (!sp) return false;

    sp->is_active = active;
    return true;
}

std::string FPSSpawnSystem::contextName(components::FPSSpawnPoint::SpawnContext ctx) {
    switch (ctx) {
        case components::FPSSpawnPoint::SpawnContext::Hangar:       return "Hangar";
        case components::FPSSpawnPoint::SpawnContext::StationLobby: return "StationLobby";
        case components::FPSSpawnPoint::SpawnContext::ShipBridge:   return "ShipBridge";
        case components::FPSSpawnPoint::SpawnContext::TetherAirlock: return "TetherAirlock";
        case components::FPSSpawnPoint::SpawnContext::EVAHatch:     return "EVAHatch";
    }
    return "Unknown";
}

} // namespace systems
} // namespace atlas
