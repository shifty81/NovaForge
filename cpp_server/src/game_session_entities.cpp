#include "game_session.h"
#include "game_session_internal.h"
#include "components/game_components.h"
#include "pcg/pcg_manager.h"
#include "pcg/pcg_context.h"
#include "pcg/capital_ship_generator.h"
#include "pcg/station_generator.h"
#include "pcg/salvage_system.h"
#include "pcg/snappable_grid.h"
#include "utils/logger.h"
#include "utils/entity_builder.h"
#include <sstream>
#include <chrono>

namespace atlas {

// ---------------------------------------------------------------------------
// State broadcast helpers
// ---------------------------------------------------------------------------

std::string GameSession::buildStateUpdate() const {
    // Increment snapshot sequence number for packet loss detection
    uint64_t seq = snapshot_sequence_++;
    
    std::ostringstream json;
    json << "{\"type\":\"state_update\",\"data\":{"
         << "\"sequence\":" << seq << ","
         << "\"timestamp\":" << std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now().time_since_epoch()).count() << ","
         << "\"entities\":[";

    auto entities = world_->getAllEntities();
    bool first = true;
    for (const auto* entity : entities) {
        if (!first) json << ",";
        first = false;

        auto* pos  = entity->getComponent<components::Position>();
        auto* vel  = entity->getComponent<components::Velocity>();
        auto* hp   = entity->getComponent<components::Health>();

        json << "{\"id\":\"" << entity->getId() << "\"";

        // Position
        if (pos) {
            json << ",\"pos\":{\"x\":" << pos->x
                 << ",\"y\":" << pos->y
                 << ",\"z\":" << pos->z
                 << ",\"rot\":" << pos->rotation << "}";
        }

        // Velocity
        if (vel) {
            json << ",\"vel\":{\"vx\":" << vel->vx
                 << ",\"vy\":" << vel->vy
                 << ",\"vz\":" << vel->vz << "}";
        }

        // Health
        if (hp) {
            json << ",\"health\":{"
                 << "\"shield\":" << hp->shield_hp
                 << ",\"armor\":" << hp->armor_hp
                 << ",\"hull\":" << hp->hull_hp
                 << ",\"max_shield\":" << hp->shield_max
                 << ",\"max_armor\":" << hp->armor_max
                 << ",\"max_hull\":" << hp->hull_max
                 << "}";
        }

        // Capacitor
        auto* cap = entity->getComponent<components::Capacitor>();
        if (cap) {
            json << ",\"capacitor\":{"
                 << "\"current\":" << cap->capacitor
                 << ",\"max\":" << cap->capacitor_max
                 << "}";
        }

        // Ship info (needed for correct model selection on the client)
        auto* ship = entity->getComponent<components::Ship>();
        if (ship) {
            json << ",\"ship_type\":\"" << ship->ship_type << "\"";
            json << ",\"ship_name\":\"" << ship->ship_name << "\"";
        }

        // Faction (needed for correct model coloring on the client)
        auto* fac = entity->getComponent<components::Faction>();
        if (fac) {
            json << ",\"faction\":\"" << fac->faction_name << "\"";
        }

        json << "}";
    }

    json << "]}}";
    return json.str();
}

std::string GameSession::buildSpawnEntity(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return "{}";

    auto* pos  = entity->getComponent<components::Position>();
    auto* hp   = entity->getComponent<components::Health>();
    auto* ship = entity->getComponent<components::Ship>();
    auto* fac  = entity->getComponent<components::Faction>();

    std::ostringstream json;
    json << "{\"type\":\"spawn_entity\",\"data\":{";
    json << "\"entity_id\":\"" << entity_id << "\"";

    if (pos) {
        json << ",\"position\":{\"x\":" << pos->x
             << ",\"y\":" << pos->y
             << ",\"z\":" << pos->z << "}";
    }

    if (hp) {
        json << ",\"health\":{"
             << "\"shield\":" << hp->shield_hp
             << ",\"armor\":" << hp->armor_hp
             << ",\"hull\":" << hp->hull_hp
             << ",\"max_shield\":" << hp->shield_max
             << ",\"max_armor\":" << hp->armor_max
             << ",\"max_hull\":" << hp->hull_max << "}";
    }

    auto* cap = entity->getComponent<components::Capacitor>();
    if (cap) {
        json << ",\"capacitor\":{"
             << "\"current\":" << cap->capacitor
             << ",\"max\":" << cap->capacitor_max
             << "}";
    }

    if (ship) {
        json << ",\"ship_type\":\"" << ship->ship_type << "\"";
        json << ",\"ship_name\":\"" << ship->ship_name << "\"";
    }

    if (fac) {
        json << ",\"faction\":\"" << fac->faction_name << "\"";
    }

    json << "}}";
    return json.str();
}

// ---------------------------------------------------------------------------
// Player entity creation
// ---------------------------------------------------------------------------

std::string GameSession::createPlayerEntity(const std::string& player_id,
                                            const std::string& character_name,
                                            const std::string& ship_type) {
    uint32_t id_num = next_entity_id_++;
    std::string entity_id = "player_" + std::to_string(id_num);

    auto* entity = world_->createEntity(entity_id);
    if (!entity) return entity_id;

    // Try to load ship stats from database; fall back to defaults
    const data::ShipTemplate* tmpl = ship_db_.getShip(ship_type);

    // Position – spawn near origin with spacing per player
    // Velocity
    // Faction
    // Target
    // Player tag
    // Default faction standings
    // Capacitor
    utils::EntityBuilder builder(entity);
    builder.withPosition(static_cast<float>(id_num) * PLAYER_SPAWN_SPACING_X,
                         0.0f,
                         static_cast<float>(id_num) * PLAYER_SPAWN_SPACING_Z)
           .withVelocity(tmpl ? tmpl->max_velocity : 300.0f)
           .withTarget()
           .withPlayer(player_id, character_name)
           .withFaction(tmpl ? tmpl->race : "Keldari")
           .withDefaultPlayerStandings()
           .withCapacitor(tmpl ? tmpl->capacitor : 250.0f,
                          tmpl ? (tmpl->capacitor / tmpl->capacitor_recharge_time) : 3.0f);

    // Health — requires per-layer resistance data from template
    if (tmpl) {
        builder.withHealthResists(
            tmpl->shield_hp, tmpl->armor_hp, tmpl->hull_hp,
            tmpl->shield_hp / tmpl->shield_recharge_time,
            tmpl->shield_resists.em, tmpl->shield_resists.thermal,
            tmpl->shield_resists.kinetic, tmpl->shield_resists.explosive,
            tmpl->armor_resists.em, tmpl->armor_resists.thermal,
            tmpl->armor_resists.kinetic, tmpl->armor_resists.explosive,
            tmpl->hull_resists.em, tmpl->hull_resists.thermal,
            tmpl->hull_resists.kinetic, tmpl->hull_resists.explosive);
    } else {
        builder.withHealth(450.0f, 350.0f, 300.0f, 3.5f);
    }

    // Ship info — full fitting stats
    builder.withShipFull(
        tmpl ? tmpl->name       : "Fang",
        tmpl ? tmpl->ship_class : "Frigate",
        tmpl ? tmpl->name       : "Fang",
        tmpl ? tmpl->race       : "Keldari",
        tmpl ? tmpl->cpu          : 125.0f,
        tmpl ? tmpl->powergrid    : 37.0f,
        tmpl ? tmpl->signature_radius    : 35.0f,
        tmpl ? tmpl->scan_resolution     : 400.0f,
        tmpl ? tmpl->max_locked_targets  : 4,
        tmpl ? tmpl->max_targeting_range : 18000.0f);

    builder.build();

    return entity_id;
}

// ---------------------------------------------------------------------------
// NPC spawning
// ---------------------------------------------------------------------------

void GameSession::spawnInitialNPCs() {
    spawnNPC("npc_venom_1", "Venom Syndicate Spy",       "Vipere",  "Venom Syndicate",
             1000.0f,  0.0f, -500.0f);
    spawnNPC("npc_corsairs_1", "Iron Corsairs Scout",      "Falk",    "Iron Corsairs",
             -800.0f,  0.0f,  600.0f);
    spawnNPC("npc_crimson_1",    "Crimson Order Seeker", "Sentinel",  "Crimson Order",
             500.0f,   0.0f,  1200.0f);

    // --- Procedural content generation ---
    if (pcg_manager_ && pcg_manager_->isInitialized()) {
        // Generate a procedural capital ship (derelict wreck to explore)
        {
            auto ctx = pcg_manager_->makeRootContext(
                pcg::PCGDomain::CapitalShip, 1, 1);
            auto capShip = pcg::CapitalShipGenerator::generate(ctx, 4);

            auto* entity = world_->createEntity("pcg_capital_wreck_1");
            if (entity) {
                int rooms = 0;
                for (const auto& d : capShip.decks) rooms += static_cast<int>(d.rooms.size());

                auto interior = std::make_unique<components::ProceduralInterior>();
                interior->shipClass = capShip.shipClass;
                interior->deckCount = static_cast<int>(capShip.decks.size());
                interior->roomCount = rooms;
                interior->elevatorCount = static_cast<int>(capShip.elevators.size());
                interior->pcgSeed = ctx.seed;

                utils::EntityBuilder(entity)
                    .withPosition(3000.0f, 200.0f, -2000.0f)
                    .withDestroyedHealth(20000.0f, 30000.0f, 50000.0f)
                    .withShip("Derelict Battleship", "Battleship", "Capital_Wreck")
                    .withComponent(std::move(interior))
                    .withFaction("Ancient Fleet")
                    .build();

                atlas::utils::Logger::instance().info(
                    "[PCG] Spawned capital wreck: " + std::to_string(capShip.decks.size()) +
                    " decks, " + std::to_string(rooms) + " rooms, " +
                    std::to_string(capShip.elevators.size()) + " elevators");
            }
        }

        // Generate a procedural station
        {
            auto ctx = pcg_manager_->makeRootContext(
                pcg::PCGDomain::Station, 1, 1);
            auto station = pcg::StationGenerator::generate(ctx, 8);

            auto* entity = world_->createEntity("pcg_station_1");
            if (entity) {
                auto stComp = std::make_unique<components::ProceduralStation>();
                stComp->moduleCount = static_cast<int>(station.modules.size());
                stComp->totalPowerConsumption = station.totalPowerConsumption;
                stComp->totalPowerProduction = station.totalPowerProduction;
                stComp->pcgSeed = ctx.seed;

                utils::EntityBuilder(entity)
                    .withPosition(-2000.0f, 0.0f, 1500.0f)
                    .withHealth(60000.0f, 80000.0f, 100000.0f)
                    .withComponent(std::move(stComp))
                    .withFaction("Independent")
                    .build();

                atlas::utils::Logger::instance().info(
                    "[PCG] Spawned station: " + std::to_string(station.modules.size()) +
                    " modules, power " + std::to_string(station.totalPowerProduction) +
                    "/" + std::to_string(station.totalPowerConsumption));
            }
        }

        // Generate a salvage field (debris from old battle)
        {
            auto ctx = pcg_manager_->makeRootContext(
                pcg::PCGDomain::Salvage, 1, 1);
            auto field = pcg::SalvageSystem::generateSalvageField(ctx, 30);

            auto* entity = world_->createEntity("pcg_salvage_field_1");
            if (entity) {
                auto sf = std::make_unique<components::SalvageFieldComponent>();
                sf->totalNodes = field.totalNodes;
                sf->hiddenNodes = field.hiddenNodes;
                sf->totalValue = pcg::SalvageSystem::calculateTotalValue(field);
                sf->pcgSeed = ctx.seed;

                utils::EntityBuilder(entity)
                    .withPosition(5000.0f, -100.0f, 4000.0f)
                    .withComponent(std::move(sf))
                    .withFaction("None")
                    .build();

                atlas::utils::Logger::instance().info(
                    "[PCG] Spawned salvage field: " + std::to_string(field.totalNodes) +
                    " nodes (" + std::to_string(field.hiddenNodes) + " hidden), value " +
                    std::to_string(entity->getComponent<components::SalvageFieldComponent>()->totalValue));
            }
        }

        // Generate sector grid (asteroid belt)
        {
            auto ctx = pcg_manager_->makeRootContext(
                pcg::PCGDomain::Sector, 1, 1);
            pcg::SnappableGrid grid(16, 8, 16, 50.0f);
            grid.generateSector(ctx);

            int occupied = 0;
            for (int x = 0; x < 16; ++x)
                for (int y = 0; y < 8; ++y)
                    for (int z = 0; z < 16; ++z) {
                        auto* c = grid.getCell(x, y, z);
                        if (c && c->occupied) ++occupied;
                    }

            auto* entity = world_->createEntity("pcg_asteroid_belt_1");
            if (entity) {
                auto sg = std::make_unique<components::SectorGrid>();
                sg->gridWidth = 16;
                sg->gridHeight = 8;
                sg->gridDepth = 16;
                sg->cellSize = 50.0f;
                sg->occupiedCells = occupied;
                sg->pcgSeed = ctx.seed;

                utils::EntityBuilder(entity)
                    .withPosition(-4000.0f, 0.0f, -3000.0f)
                    .withComponent(std::move(sg))
                    .build();

                atlas::utils::Logger::instance().info(
                    "[PCG] Spawned asteroid belt: 16x8x16 grid, " +
                    std::to_string(occupied) + " occupied cells");
            }
        }
    }
}

void GameSession::spawnNPC(const std::string& id, const std::string& name,
                           const std::string& ship_name,
                           const std::string& faction_name,
                           float x, float y, float z) {
    auto* entity = world_->createEntity(id);
    if (!entity) return;

    bool is_pirate = (faction_name == "Venom Syndicate" || faction_name == "Iron Corsairs" ||
                      faction_name == "Crimson Order"   || faction_name == "Hollow Collective");

    auto builder = utils::EntityBuilder(entity)
        .withPosition(x, y, z)
        .withVelocity(250.0f)
        .withHealth(300.0f, 250.0f, 200.0f)
        .withShip(ship_name, "Frigate", ship_name)
        .withFaction(faction_name);

    if (is_pirate) {
        builder.withPirateStandings();
    } else {
        builder.withEmpireStandings();
    }

    builder.withAI(components::AI::Behavior::Aggressive,
                   components::AI::State::Idle,
                   NPC_AWARENESS_RANGE)
           .withWeapon(12.0f, 5000.0f, 4.0f)
           .build();

    atlas::utils::Logger::instance().info(
        "[GameSession] Spawned NPC: " + name + " (" + faction_name + " " + ship_name + ")");
}

} // namespace atlas
