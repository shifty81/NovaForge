// Tests for: WorldPersistence Tests
#include "test_log.h"
#include "components/core_components.h"
#include "components/navigation_components.h"
#include "components/social_components.h"
#include "ecs/component.h"
#include "ecs/system.h"
#include "systems/fleet_system.h"
#include "data/world_persistence.h"
#include <fstream>
#include <sys/stat.h>

using namespace atlas;

// ==================== WorldPersistence Tests ====================

static void testSerializeDeserializeBasicEntity() {
    std::cout << "\n=== Serialize/Deserialize Basic Entity ===" << std::endl;

    ecs::World world;
    auto* entity = world.createEntity("ship_1");

    auto pos = std::make_unique<components::Position>();
    pos->x = 100.0f; pos->y = 200.0f; pos->z = 300.0f; pos->rotation = 1.5f;
    entity->addComponent(std::move(pos));

    auto vel = std::make_unique<components::Velocity>();
    vel->vx = 10.0f; vel->vy = 20.0f; vel->vz = 30.0f; vel->max_speed = 500.0f;
    entity->addComponent(std::move(vel));

    data::WorldPersistence persistence;
    std::string json = persistence.serializeWorld(&world);

    assertTrue(!json.empty(), "Serialized JSON is not empty");
    assertTrue(json.find("ship_1") != std::string::npos, "JSON contains entity id");

    // Deserialize into a new world
    ecs::World world2;
    bool ok = persistence.deserializeWorld(&world2, json);
    assertTrue(ok, "Deserialize succeeds");
    assertTrue(world2.getEntityCount() == 1, "Loaded world has 1 entity");

    auto* loaded = world2.getEntity("ship_1");
    assertTrue(loaded != nullptr, "Loaded entity found by id");

    auto* lpos = loaded->getComponent<components::Position>();
    assertTrue(lpos != nullptr, "Loaded entity has Position");
    assertTrue(approxEqual(lpos->x, 100.0f), "Position.x preserved");
    assertTrue(approxEqual(lpos->y, 200.0f), "Position.y preserved");
    assertTrue(approxEqual(lpos->z, 300.0f), "Position.z preserved");
    assertTrue(approxEqual(lpos->rotation, 1.5f), "Position.rotation preserved");

    auto* lvel = loaded->getComponent<components::Velocity>();
    assertTrue(lvel != nullptr, "Loaded entity has Velocity");
    assertTrue(approxEqual(lvel->vx, 10.0f), "Velocity.vx preserved");
    assertTrue(approxEqual(lvel->max_speed, 500.0f), "Velocity.max_speed preserved");
}

static void testSerializeDeserializeHealthCapacitor() {
    std::cout << "\n=== Serialize/Deserialize Health & Capacitor ===" << std::endl;

    ecs::World world;
    auto* entity = world.createEntity("tanker");

    auto hp = std::make_unique<components::Health>();
    hp->shield_hp = 450.0f; hp->shield_max = 500.0f;
    hp->armor_hp = 300.0f; hp->armor_max = 400.0f;
    hp->hull_hp = 200.0f; hp->hull_max = 250.0f;
    hp->shield_recharge_rate = 5.0f;
    hp->shield_em_resist = 0.1f;
    hp->armor_thermal_resist = 0.35f;
    entity->addComponent(std::move(hp));

    auto cap = std::make_unique<components::Capacitor>();
    cap->capacitor = 180.0f; cap->capacitor_max = 250.0f; cap->recharge_rate = 4.0f;
    entity->addComponent(std::move(cap));

    data::WorldPersistence persistence;
    std::string json = persistence.serializeWorld(&world);

    ecs::World world2;
    persistence.deserializeWorld(&world2, json);

    auto* loaded = world2.getEntity("tanker");
    assertTrue(loaded != nullptr, "Entity loaded");

    auto* lhp = loaded->getComponent<components::Health>();
    assertTrue(lhp != nullptr, "Health component loaded");
    assertTrue(approxEqual(lhp->shield_hp, 450.0f), "Shield HP preserved");
    assertTrue(approxEqual(lhp->shield_max, 500.0f), "Shield max preserved");
    assertTrue(approxEqual(lhp->armor_hp, 300.0f), "Armor HP preserved");
    assertTrue(approxEqual(lhp->hull_hp, 200.0f), "Hull HP preserved");
    assertTrue(approxEqual(lhp->shield_recharge_rate, 5.0f), "Shield recharge rate preserved");
    assertTrue(approxEqual(lhp->shield_em_resist, 0.1f), "Shield EM resist preserved");
    assertTrue(approxEqual(lhp->armor_thermal_resist, 0.35f), "Armor thermal resist preserved");

    auto* lcap = loaded->getComponent<components::Capacitor>();
    assertTrue(lcap != nullptr, "Capacitor component loaded");
    assertTrue(approxEqual(lcap->capacitor, 180.0f), "Capacitor current preserved");
    assertTrue(approxEqual(lcap->capacitor_max, 250.0f), "Capacitor max preserved");
    assertTrue(approxEqual(lcap->recharge_rate, 4.0f), "Capacitor recharge rate preserved");
}

static void testSerializeDeserializeShipAndFaction() {
    std::cout << "\n=== Serialize/Deserialize Ship & Faction ===" << std::endl;

    ecs::World world;
    auto* entity = world.createEntity("player_ship");

    auto ship = std::make_unique<components::Ship>();
    ship->ship_type = "Cruiser";
    ship->ship_class = "Cruiser";
    ship->ship_name = "Caracal";
    ship->race = "Veyren";
    ship->cpu_max = 350.0f;
    ship->powergrid_max = 200.0f;
    ship->signature_radius = 140.0f;
    ship->scan_resolution = 250.0f;
    ship->max_locked_targets = 6;
    ship->max_targeting_range = 55000.0f;
    entity->addComponent(std::move(ship));

    auto fac = std::make_unique<components::Faction>();
    fac->faction_name = "Veyren";
    entity->addComponent(std::move(fac));

    data::WorldPersistence persistence;
    std::string json = persistence.serializeWorld(&world);

    ecs::World world2;
    persistence.deserializeWorld(&world2, json);

    auto* loaded = world2.getEntity("player_ship");
    assertTrue(loaded != nullptr, "Entity loaded");

    auto* lship = loaded->getComponent<components::Ship>();
    assertTrue(lship != nullptr, "Ship component loaded");
    assertTrue(lship->ship_name == "Caracal", "Ship name preserved");
    assertTrue(lship->race == "Veyren", "Ship race preserved");
    assertTrue(lship->ship_class == "Cruiser", "Ship class preserved");
    assertTrue(approxEqual(lship->cpu_max, 350.0f), "CPU max preserved");
    assertTrue(lship->max_locked_targets == 6, "Max locked targets preserved");
    assertTrue(approxEqual(lship->max_targeting_range, 55000.0f), "Max targeting range preserved");

    auto* lfac = loaded->getComponent<components::Faction>();
    assertTrue(lfac != nullptr, "Faction component loaded");
    assertTrue(lfac->faction_name == "Veyren", "Faction name preserved");
}

static void testSerializeDeserializeStandings() {
    std::cout << "\n=== Serialize/Deserialize Standings ===" << std::endl;

    ecs::World world;
    auto* entity = world.createEntity("player_1");

    // Add Standings component with test data
    auto standings = std::make_unique<components::Standings>();
    standings->personal_standings["npc_pirate_001"] = -5.0f;
    standings->personal_standings["player_friend"] = 8.5f;
    standings->corporation_standings["Republic Fleet"] = 3.0f;
    standings->corporation_standings["Venom Syndicate"] = -7.5f;
    standings->faction_standings["Keldari"] = 2.5f;
    standings->faction_standings["Solari"] = -1.5f;
    entity->addComponent(std::move(standings));

    data::WorldPersistence persistence;
    std::string json = persistence.serializeWorld(&world);

    ecs::World world2;
    persistence.deserializeWorld(&world2, json);

    auto* loaded = world2.getEntity("player_1");
    assertTrue(loaded != nullptr, "Entity loaded");

    auto* lstandings = loaded->getComponent<components::Standings>();
    assertTrue(lstandings != nullptr, "Standings component loaded");
    
    // Check personal standings
    assertTrue(lstandings->personal_standings.size() == 2, "Personal standings count preserved");
    assertTrue(approxEqual(lstandings->personal_standings["npc_pirate_001"], -5.0f), "Personal standing (pirate) preserved");
    assertTrue(approxEqual(lstandings->personal_standings["player_friend"], 8.5f), "Personal standing (friend) preserved");
    
    // Check corporation standings
    assertTrue(lstandings->corporation_standings.size() == 2, "Corporation standings count preserved");
    assertTrue(approxEqual(lstandings->corporation_standings["Republic Fleet"], 3.0f), "Corporation standing (Republic Fleet) preserved");
    assertTrue(approxEqual(lstandings->corporation_standings["Venom Syndicate"], -7.5f), "Corporation standing (Venom Syndicate) preserved");
    
    // Check faction standings
    assertTrue(lstandings->faction_standings.size() == 2, "Faction standings count preserved");
    assertTrue(approxEqual(lstandings->faction_standings["Keldari"], 2.5f), "Faction standing (Keldari) preserved");
    assertTrue(approxEqual(lstandings->faction_standings["Solari"], -1.5f), "Faction standing (Solari) preserved");
}

static void testStandingsGetStanding() {
    std::cout << "\n=== Standings getStandingWith ===" << std::endl;

    ecs::World world;
    auto* entity = world.createEntity("player_1");

    auto standings = std::make_unique<components::Standings>();
    standings->personal_standings["npc_001"] = -5.0f;
    standings->corporation_standings["TestCorp"] = 3.0f;
    standings->faction_standings["Veyren"] = 7.0f;
    entity->addComponent(std::move(standings));

    auto* comp = entity->getComponent<components::Standings>();
    
    // Personal standing has highest priority
    float standing1 = comp->getStandingWith("npc_001", "", "");
    assertTrue(approxEqual(standing1, -5.0f), "Personal standing returned");
    
    // Corporation standing used when no personal standing
    float standing2 = comp->getStandingWith("npc_002", "TestCorp", "");
    assertTrue(approxEqual(standing2, 3.0f), "Corporation standing returned");
    
    // Faction standing used when no personal or corp standing
    float standing3 = comp->getStandingWith("npc_003", "OtherCorp", "Veyren");
    assertTrue(approxEqual(standing3, 7.0f), "Faction standing returned");
    
    // Neutral (0) when no standing exists
    float standing4 = comp->getStandingWith("unknown", "UnknownCorp", "UnknownFaction");
    assertTrue(approxEqual(standing4, 0.0f), "Neutral standing for unknown entity");
    
    // Personal standing overrides corporation
    comp->personal_standings["npc_004"] = 9.0f;
    float standing5 = comp->getStandingWith("npc_004", "TestCorp", "");
    assertTrue(approxEqual(standing5, 9.0f), "Personal standing overrides corporation");
}

static void testStandingsModify() {
    std::cout << "\n=== Standings modifyStanding ===" << std::endl;

    std::map<std::string, float> test_standings;
    
    // Start with no standing (implicit 0)
    components::Standings::modifyStanding(test_standings, "entity1", 2.5f);
    assertTrue(approxEqual(test_standings["entity1"], 2.5f), "Standing increased from 0 to 2.5");
    
    // Increase existing standing
    components::Standings::modifyStanding(test_standings, "entity1", 3.0f);
    assertTrue(approxEqual(test_standings["entity1"], 5.5f), "Standing increased to 5.5");
    
    // Decrease standing
    components::Standings::modifyStanding(test_standings, "entity1", -2.0f);
    assertTrue(approxEqual(test_standings["entity1"], 3.5f), "Standing decreased to 3.5");
    
    // Clamp at maximum (10.0)
    components::Standings::modifyStanding(test_standings, "entity1", 15.0f);
    assertTrue(approxEqual(test_standings["entity1"], 10.0f), "Standing clamped at max (10.0)");
    
    // Clamp at minimum (-10.0)
    components::Standings::modifyStanding(test_standings, "entity2", -20.0f);
    assertTrue(approxEqual(test_standings["entity2"], -10.0f), "Standing clamped at min (-10.0)");
    
    // Negative adjustment from positive
    test_standings["entity3"] = 5.0f;
    components::Standings::modifyStanding(test_standings, "entity3", -8.0f);
    assertTrue(approxEqual(test_standings["entity3"], -3.0f), "Standing went from +5 to -3");
}

static void testSerializeDeserializeAIAndWeapon() {
    std::cout << "\n=== Serialize/Deserialize AI & Weapon ===" << std::endl;

    ecs::World world;
    auto* entity = world.createEntity("npc_1");

    auto ai = std::make_unique<components::AI>();
    ai->behavior = components::AI::Behavior::Aggressive;
    ai->state = components::AI::State::Attacking;
    ai->target_entity_id = "player_1";
    ai->orbit_distance = 2500.0f;
    ai->awareness_range = 60000.0f;
    entity->addComponent(std::move(ai));

    auto weapon = std::make_unique<components::Weapon>();
    weapon->weapon_type = "Missile";
    weapon->damage_type = "kinetic";
    weapon->damage = 75.0f;
    weapon->optimal_range = 20000.0f;
    weapon->rate_of_fire = 8.0f;
    weapon->capacitor_cost = 15.0f;
    weapon->ammo_type = "Scourge";
    weapon->ammo_count = 50;
    entity->addComponent(std::move(weapon));

    data::WorldPersistence persistence;
    std::string json = persistence.serializeWorld(&world);

    ecs::World world2;
    persistence.deserializeWorld(&world2, json);

    auto* loaded = world2.getEntity("npc_1");
    assertTrue(loaded != nullptr, "NPC entity loaded");

    auto* lai = loaded->getComponent<components::AI>();
    assertTrue(lai != nullptr, "AI component loaded");
    assertTrue(lai->behavior == components::AI::Behavior::Aggressive, "AI behavior preserved");
    assertTrue(lai->state == components::AI::State::Attacking, "AI state preserved");
    assertTrue(lai->target_entity_id == "player_1", "AI target preserved");
    assertTrue(approxEqual(lai->orbit_distance, 2500.0f), "AI orbit distance preserved");

    auto* lwep = loaded->getComponent<components::Weapon>();
    assertTrue(lwep != nullptr, "Weapon component loaded");
    assertTrue(lwep->weapon_type == "Missile", "Weapon type preserved");
    assertTrue(lwep->damage_type == "kinetic", "Damage type preserved");
    assertTrue(approxEqual(lwep->damage, 75.0f), "Weapon damage preserved");
    assertTrue(lwep->ammo_type == "Scourge", "Ammo type preserved");
    assertTrue(lwep->ammo_count == 50, "Ammo count preserved");
}

static void testSerializeDeserializePlayerComponent() {
    std::cout << "\n=== Serialize/Deserialize Player Component ===" << std::endl;

    ecs::World world;
    auto* entity = world.createEntity("player_42");

    auto player = std::make_unique<components::Player>();
    player->player_id = "steam_12345";
    player->character_name = "TestPilot";
    player->credits = 5000000.0;
    player->corporation = "Test Corp";
    entity->addComponent(std::move(player));

    data::WorldPersistence persistence;
    std::string json = persistence.serializeWorld(&world);

    ecs::World world2;
    persistence.deserializeWorld(&world2, json);

    auto* loaded = world2.getEntity("player_42");
    assertTrue(loaded != nullptr, "Player entity loaded");

    auto* lp = loaded->getComponent<components::Player>();
    assertTrue(lp != nullptr, "Player component loaded");
    assertTrue(lp->player_id == "steam_12345", "Player ID preserved");
    assertTrue(lp->character_name == "TestPilot", "Character name preserved");
    assertTrue(lp->credits > 4999999.0 && lp->credits < 5000001.0, "Credits preserved");
    assertTrue(lp->corporation == "Test Corp", "Corporation preserved");
}

static void testSerializeDeserializeMultipleEntities() {
    std::cout << "\n=== Serialize/Deserialize Multiple Entities ===" << std::endl;

    ecs::World world;

    // Create 3 entities with different component combinations
    auto* e1 = world.createEntity("ship_a");
    auto p1 = std::make_unique<components::Position>();
    p1->x = 10.0f;
    e1->addComponent(std::move(p1));

    auto* e2 = world.createEntity("ship_b");
    auto p2 = std::make_unique<components::Position>();
    p2->x = 20.0f;
    e2->addComponent(std::move(p2));
    auto h2 = std::make_unique<components::Health>();
    h2->shield_hp = 999.0f;
    e2->addComponent(std::move(h2));

    auto* e3 = world.createEntity("ship_c");
    auto p3 = std::make_unique<components::Position>();
    p3->x = 30.0f;
    e3->addComponent(std::move(p3));

    data::WorldPersistence persistence;
    std::string json = persistence.serializeWorld(&world);

    ecs::World world2;
    persistence.deserializeWorld(&world2, json);

    assertTrue(world2.getEntityCount() == 3, "All 3 entities loaded");
    assertTrue(world2.getEntity("ship_a") != nullptr, "ship_a loaded");
    assertTrue(world2.getEntity("ship_b") != nullptr, "ship_b loaded");
    assertTrue(world2.getEntity("ship_c") != nullptr, "ship_c loaded");

    auto* lb = world2.getEntity("ship_b");
    auto* lhp = lb->getComponent<components::Health>();
    assertTrue(lhp != nullptr, "ship_b has Health component");
    assertTrue(approxEqual(lhp->shield_hp, 999.0f), "ship_b shield HP preserved");
}

static void testSaveLoadFile() {
    std::cout << "\n=== Save/Load World File ===" << std::endl;

    ecs::World world;
    auto* entity = world.createEntity("file_test");
    auto pos = std::make_unique<components::Position>();
    pos->x = 42.0f; pos->y = 84.0f;
    entity->addComponent(std::move(pos));

    data::WorldPersistence persistence;
    std::string filepath = "/tmp/eve_test_world.json";

    bool saved = persistence.saveWorld(&world, filepath);
    assertTrue(saved, "World saved to file");

    // Verify file exists
    std::ifstream check(filepath);
    assertTrue(check.good(), "Save file exists on disk");
    check.close();

    ecs::World world2;
    bool loaded = persistence.loadWorld(&world2, filepath);
    assertTrue(loaded, "World loaded from file");
    assertTrue(world2.getEntityCount() == 1, "Loaded world has 1 entity");

    auto* le = world2.getEntity("file_test");
    assertTrue(le != nullptr, "Entity loaded from file");
    auto* lpos = le->getComponent<components::Position>();
    assertTrue(lpos != nullptr, "Position loaded from file");
    assertTrue(approxEqual(lpos->x, 42.0f), "Position.x loaded from file");
    assertTrue(approxEqual(lpos->y, 84.0f), "Position.y loaded from file");

    // Clean up
    std::remove(filepath.c_str());
}

static void testLoadNonexistentFile() {
    std::cout << "\n=== Load Nonexistent File ===" << std::endl;

    ecs::World world;
    data::WorldPersistence persistence;
    bool loaded = persistence.loadWorld(&world, "/tmp/does_not_exist_12345.json");
    assertTrue(!loaded, "Loading nonexistent file returns false");
    assertTrue(world.getEntityCount() == 0, "World unchanged on failed load");
}

static void testSerializeDeserializeWormholeAndSolarSystem() {
    std::cout << "\n=== Serialize/Deserialize Wormhole & SolarSystem ===" << std::endl;

    ecs::World world;
    auto* entity = world.createEntity("wh_j123456");

    auto ss = std::make_unique<components::SolarSystem>();
    ss->system_id = "j123456";
    ss->system_name = "J123456";
    ss->wormhole_class = 3;
    ss->effect_name = "magnetar";
    ss->dormants_spawned = true;
    entity->addComponent(std::move(ss));

    auto* wh_entity = world.createEntity("wh_conn_1");
    auto wh = std::make_unique<components::WormholeConnection>();
    wh->wormhole_id = "wh_001";
    wh->source_system = "j123456";
    wh->destination_system = "jita";
    wh->max_mass = 1000000000.0;
    wh->remaining_mass = 750000000.0;
    wh->max_jump_mass = 300000000.0;
    wh->max_lifetime_hours = 16.0f;
    wh->elapsed_hours = 4.5f;
    wh->collapsed = false;
    wh_entity->addComponent(std::move(wh));

    data::WorldPersistence persistence;
    std::string json = persistence.serializeWorld(&world);

    ecs::World world2;
    persistence.deserializeWorld(&world2, json);

    auto* lss_entity = world2.getEntity("wh_j123456");
    assertTrue(lss_entity != nullptr, "SolarSystem entity loaded");
    auto* lss = lss_entity->getComponent<components::SolarSystem>();
    assertTrue(lss != nullptr, "SolarSystem component loaded");
    assertTrue(lss->system_id == "j123456", "System ID preserved");
    assertTrue(lss->wormhole_class == 3, "Wormhole class preserved");
    assertTrue(lss->effect_name == "magnetar", "Effect name preserved");
    assertTrue(lss->dormants_spawned == true, "Dormants spawned preserved");

    auto* lwh_entity = world2.getEntity("wh_conn_1");
    assertTrue(lwh_entity != nullptr, "WormholeConnection entity loaded");
    auto* lwh = lwh_entity->getComponent<components::WormholeConnection>();
    assertTrue(lwh != nullptr, "WormholeConnection component loaded");
    assertTrue(lwh->wormhole_id == "wh_001", "Wormhole ID preserved");
    assertTrue(lwh->remaining_mass > 749999999.0 && lwh->remaining_mass < 750000001.0,
               "Remaining mass preserved");
    assertTrue(approxEqual(lwh->elapsed_hours, 4.5f), "Elapsed hours preserved");
    assertTrue(lwh->collapsed == false, "Collapsed state preserved");
}

static void testEmptyWorldSerialize() {
    std::cout << "\n=== Empty World Serialize ===" << std::endl;

    ecs::World world;
    data::WorldPersistence persistence;
    std::string json = persistence.serializeWorld(&world);

    assertTrue(!json.empty(), "Empty world produces valid JSON");
    assertTrue(json.find("entities") != std::string::npos, "JSON has entities key");

    ecs::World world2;
    bool ok = persistence.deserializeWorld(&world2, json);
    assertTrue(ok, "Deserialize empty world succeeds");
    assertTrue(world2.getEntityCount() == 0, "Empty world has 0 entities");
}


void run_world_persistence_tests() {
    testSerializeDeserializeBasicEntity();
    testSerializeDeserializeHealthCapacitor();
    testSerializeDeserializeShipAndFaction();
    testSerializeDeserializeStandings();
    testStandingsGetStanding();
    testStandingsModify();
    testSerializeDeserializeAIAndWeapon();
    testSerializeDeserializePlayerComponent();
    testSerializeDeserializeMultipleEntities();
    testSaveLoadFile();
    testLoadNonexistentFile();
    testSerializeDeserializeWormholeAndSolarSystem();
    testEmptyWorldSerialize();
}
