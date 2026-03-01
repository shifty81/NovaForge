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
    auto pos = std::make_unique<components::Position>();
    pos->x = static_cast<float>(id_num) * PLAYER_SPAWN_SPACING_X;
    pos->y = 0.0f;
    pos->z = static_cast<float>(id_num) * PLAYER_SPAWN_SPACING_Z;
    entity->addComponent(std::move(pos));

    // Velocity
    auto vel = std::make_unique<components::Velocity>();
    vel->max_speed = tmpl ? tmpl->max_velocity : 300.0f;
    entity->addComponent(std::move(vel));

    // Health
    auto hp = std::make_unique<components::Health>();
    hp->shield_hp  = hp->shield_max  = tmpl ? tmpl->shield_hp : 450.0f;
    hp->armor_hp   = hp->armor_max   = tmpl ? tmpl->armor_hp  : 350.0f;
    hp->hull_hp    = hp->hull_max    = tmpl ? tmpl->hull_hp   : 300.0f;
    hp->shield_recharge_rate = tmpl ? (tmpl->shield_hp / tmpl->shield_recharge_time) : 3.5f;
    if (tmpl) {
        hp->shield_em_resist        = tmpl->shield_resists.em;
        hp->shield_thermal_resist   = tmpl->shield_resists.thermal;
        hp->shield_kinetic_resist   = tmpl->shield_resists.kinetic;
        hp->shield_explosive_resist = tmpl->shield_resists.explosive;
        hp->armor_em_resist         = tmpl->armor_resists.em;
        hp->armor_thermal_resist    = tmpl->armor_resists.thermal;
        hp->armor_kinetic_resist    = tmpl->armor_resists.kinetic;
        hp->armor_explosive_resist  = tmpl->armor_resists.explosive;
        hp->hull_em_resist          = tmpl->hull_resists.em;
        hp->hull_thermal_resist     = tmpl->hull_resists.thermal;
        hp->hull_kinetic_resist     = tmpl->hull_resists.kinetic;
        hp->hull_explosive_resist   = tmpl->hull_resists.explosive;
    }
    entity->addComponent(std::move(hp));

    // Ship info
    auto ship = std::make_unique<components::Ship>();
    ship->ship_name  = tmpl ? tmpl->name       : "Fang";
    ship->ship_class = tmpl ? tmpl->ship_class  : "Frigate";
    ship->ship_type  = tmpl ? tmpl->name        : "Fang";
    ship->race       = tmpl ? tmpl->race        : "Keldari";
    ship->cpu_max    = tmpl ? tmpl->cpu          : 125.0f;
    ship->powergrid_max = tmpl ? tmpl->powergrid : 37.0f;
    ship->signature_radius    = tmpl ? tmpl->signature_radius    : 35.0f;
    ship->scan_resolution     = tmpl ? tmpl->scan_resolution     : 400.0f;
    ship->max_locked_targets  = tmpl ? tmpl->max_locked_targets  : 4;
    ship->max_targeting_range = tmpl ? tmpl->max_targeting_range : 18000.0f;
    entity->addComponent(std::move(ship));

    // Target component (for target locking)
    entity->addComponent(std::make_unique<components::Target>());

    // Player tag
    auto player = std::make_unique<components::Player>();
    player->player_id      = player_id;
    player->character_name = character_name;
    entity->addComponent(std::move(player));

    // Faction
    auto faction = std::make_unique<components::Faction>();
    faction->faction_name = tmpl ? tmpl->race : "Keldari";
    entity->addComponent(std::move(faction));

    // Standings - Initialize with default faction standings
    auto standings = std::make_unique<components::Standings>();
    // Set default standings for major factions (-10 to +10)
    // Player starts neutral with most factions
    standings->faction_standings["Veyren"] = 0.0f;
    standings->faction_standings["Aurelian"] = 0.0f;
    standings->faction_standings["Solari"] = 0.0f;
    standings->faction_standings["Keldari"] = 0.0f;
    // Pirate factions are hostile by default
    standings->faction_standings["Venom Syndicate"] = -5.0f;
    standings->faction_standings["Iron Corsairs"] = -5.0f;
    standings->faction_standings["Crimson Order"] = -5.0f;
    standings->faction_standings["Hollow Collective"] = -5.0f;
    entity->addComponent(std::move(standings));

    // Capacitor
    auto cap = std::make_unique<components::Capacitor>();
    cap->capacitor_max = tmpl ? tmpl->capacitor : 250.0f;
    cap->capacitor     = cap->capacitor_max;
    cap->recharge_rate = tmpl ? (tmpl->capacitor / tmpl->capacitor_recharge_time) : 3.0f;
    entity->addComponent(std::move(cap));

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
                auto pos = std::make_unique<components::Position>();
                pos->x = 3000.0f; pos->y = 200.0f; pos->z = -2000.0f;
                entity->addComponent(std::move(pos));

                auto hp = std::make_unique<components::Health>();
                hp->hull_hp = 0.0f; hp->hull_max = 50000.0f;
                hp->armor_hp = 0.0f; hp->armor_max = 30000.0f;
                hp->shield_hp = 0.0f; hp->shield_max = 20000.0f;
                entity->addComponent(std::move(hp));

                auto ship = std::make_unique<components::Ship>();
                ship->ship_name = "Derelict Battleship";
                ship->ship_class = "Battleship";
                ship->ship_type = "Capital_Wreck";
                entity->addComponent(std::move(ship));

                auto interior = std::make_unique<components::ProceduralInterior>();
                interior->shipClass = capShip.shipClass;
                interior->deckCount = static_cast<int>(capShip.decks.size());
                int rooms = 0;
                for (const auto& d : capShip.decks) rooms += static_cast<int>(d.rooms.size());
                interior->roomCount = rooms;
                interior->elevatorCount = static_cast<int>(capShip.elevators.size());
                interior->pcgSeed = ctx.seed;
                entity->addComponent(std::move(interior));

                auto fac = std::make_unique<components::Faction>();
                fac->faction_name = "Ancient Fleet";
                entity->addComponent(std::move(fac));

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
                auto pos = std::make_unique<components::Position>();
                pos->x = -2000.0f; pos->y = 0.0f; pos->z = 1500.0f;
                entity->addComponent(std::move(pos));

                auto hp = std::make_unique<components::Health>();
                hp->hull_hp = hp->hull_max = 100000.0f;
                hp->armor_hp = hp->armor_max = 80000.0f;
                hp->shield_hp = hp->shield_max = 60000.0f;
                entity->addComponent(std::move(hp));

                auto stComp = std::make_unique<components::ProceduralStation>();
                stComp->moduleCount = static_cast<int>(station.modules.size());
                stComp->totalPowerConsumption = station.totalPowerConsumption;
                stComp->totalPowerProduction = station.totalPowerProduction;
                stComp->pcgSeed = ctx.seed;
                entity->addComponent(std::move(stComp));

                auto fac = std::make_unique<components::Faction>();
                fac->faction_name = "Independent";
                entity->addComponent(std::move(fac));

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
                auto pos = std::make_unique<components::Position>();
                pos->x = 5000.0f; pos->y = -100.0f; pos->z = 4000.0f;
                entity->addComponent(std::move(pos));

                auto sf = std::make_unique<components::SalvageFieldComponent>();
                sf->totalNodes = field.totalNodes;
                sf->hiddenNodes = field.hiddenNodes;
                sf->totalValue = pcg::SalvageSystem::calculateTotalValue(field);
                sf->pcgSeed = ctx.seed;
                entity->addComponent(std::move(sf));

                auto fac = std::make_unique<components::Faction>();
                fac->faction_name = "None";
                entity->addComponent(std::move(fac));

                atlas::utils::Logger::instance().info(
                    "[PCG] Spawned salvage field: " + std::to_string(field.totalNodes) +
                    " nodes (" + std::to_string(field.hiddenNodes) + " hidden), value " +
                    std::to_string(sf->totalValue));
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
                auto pos = std::make_unique<components::Position>();
                pos->x = -4000.0f; pos->y = 0.0f; pos->z = -3000.0f;
                entity->addComponent(std::move(pos));

                auto sg = std::make_unique<components::SectorGrid>();
                sg->gridWidth = 16;
                sg->gridHeight = 8;
                sg->gridDepth = 16;
                sg->cellSize = 50.0f;
                sg->occupiedCells = occupied;
                sg->pcgSeed = ctx.seed;
                entity->addComponent(std::move(sg));

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

    auto pos = std::make_unique<components::Position>();
    pos->x = x;  pos->y = y;  pos->z = z;
    entity->addComponent(std::move(pos));

    auto vel = std::make_unique<components::Velocity>();
    vel->max_speed = 250.0f;
    entity->addComponent(std::move(vel));

    auto hp = std::make_unique<components::Health>();
    hp->shield_hp = hp->shield_max = 300.0f;
    hp->armor_hp  = hp->armor_max  = 250.0f;
    hp->hull_hp   = hp->hull_max   = 200.0f;
    entity->addComponent(std::move(hp));

    auto ship = std::make_unique<components::Ship>();
    ship->ship_name  = ship_name;
    ship->ship_class = "Frigate";
    ship->ship_type  = ship_name;
    entity->addComponent(std::move(ship));

    auto fac = std::make_unique<components::Faction>();
    fac->faction_name = faction_name;
    entity->addComponent(std::move(fac));

    // Standings - NPCs have faction standings based on their faction
    auto standings = std::make_unique<components::Standings>();
    // Pirate NPCs are hostile to players
    if (faction_name == "Venom Syndicate" || faction_name == "Iron Corsairs" || 
        faction_name == "Crimson Order" || faction_name == "Hollow Collective") {
        // Hostile to all empire factions
        standings->faction_standings["Veyren"] = -5.0f;
        standings->faction_standings["Aurelian"] = -5.0f;
        standings->faction_standings["Solari"] = -5.0f;
        standings->faction_standings["Keldari"] = -5.0f;
    } else {
        // Empire NPCs are neutral to players by default
        standings->faction_standings["Veyren"] = 0.0f;
        standings->faction_standings["Aurelian"] = 0.0f;
        standings->faction_standings["Solari"] = 0.0f;
        standings->faction_standings["Keldari"] = 0.0f;
    }
    entity->addComponent(std::move(standings));

    auto ai = std::make_unique<components::AI>();
    ai->behavior = components::AI::Behavior::Aggressive;
    ai->state    = components::AI::State::Idle;
    ai->awareness_range = NPC_AWARENESS_RANGE;
    entity->addComponent(std::move(ai));

    auto weapon = std::make_unique<components::Weapon>();
    weapon->damage       = 12.0f;
    weapon->optimal_range = 5000.0f;
    weapon->rate_of_fire  = 4.0f;
    entity->addComponent(std::move(weapon));

    atlas::utils::Logger::instance().info(
        "[GameSession] Spawned NPC: " + name + " (" + faction_name + " " + ship_name + ")");
}

} // namespace atlas
