// Tests for: EntityBuilder utility
#include "test_log.h"
#include "utils/entity_builder.h"
#include "components/core_components.h"
#include "components/game_components.h"

using namespace atlas;

// ==================== EntityBuilder Tests ====================

static void testEntityBuilderPosition() {
    std::cout << "\n=== EntityBuilder: Position ===" << std::endl;
    ecs::World world;
    auto* entity = world.createEntity("test1");
    utils::EntityBuilder(entity)
        .withPosition(10.0f, 20.0f, 30.0f, 1.5f)
        .build();

    auto* pos = entity->getComponent<components::Position>();
    assertTrue(pos != nullptr, "Position component added");
    assertTrue(approxEqual(pos->x, 10.0f), "Position x = 10");
    assertTrue(approxEqual(pos->y, 20.0f), "Position y = 20");
    assertTrue(approxEqual(pos->z, 30.0f), "Position z = 30");
    assertTrue(approxEqual(pos->rotation, 1.5f), "Rotation = 1.5");
}

static void testEntityBuilderVelocity() {
    std::cout << "\n=== EntityBuilder: Velocity ===" << std::endl;
    ecs::World world;
    auto* entity = world.createEntity("test1");
    utils::EntityBuilder(entity)
        .withVelocity(300.0f)
        .build();

    auto* vel = entity->getComponent<components::Velocity>();
    assertTrue(vel != nullptr, "Velocity component added");
    assertTrue(approxEqual(vel->max_speed, 300.0f), "max_speed = 300");
}

static void testEntityBuilderHealth() {
    std::cout << "\n=== EntityBuilder: Health ===" << std::endl;
    ecs::World world;
    auto* entity = world.createEntity("test1");
    utils::EntityBuilder(entity)
        .withHealth(450.0f, 350.0f, 300.0f, 3.5f)
        .build();

    auto* hp = entity->getComponent<components::Health>();
    assertTrue(hp != nullptr, "Health component added");
    assertTrue(approxEqual(hp->shield_hp, 450.0f), "shield_hp = 450");
    assertTrue(approxEqual(hp->shield_max, 450.0f), "shield_max = 450");
    assertTrue(approxEqual(hp->armor_hp, 350.0f), "armor_hp = 350");
    assertTrue(approxEqual(hp->armor_max, 350.0f), "armor_max = 350");
    assertTrue(approxEqual(hp->hull_hp, 300.0f), "hull_hp = 300");
    assertTrue(approxEqual(hp->hull_max, 300.0f), "hull_max = 300");
    assertTrue(approxEqual(hp->shield_recharge_rate, 3.5f), "recharge rate = 3.5");
}

static void testEntityBuilderDestroyedHealth() {
    std::cout << "\n=== EntityBuilder: Destroyed Health ===" << std::endl;
    ecs::World world;
    auto* entity = world.createEntity("wreck1");
    utils::EntityBuilder(entity)
        .withDestroyedHealth(20000.0f, 30000.0f, 50000.0f)
        .build();

    auto* hp = entity->getComponent<components::Health>();
    assertTrue(hp != nullptr, "Health component added");
    assertTrue(approxEqual(hp->shield_hp, 0.0f), "shield_hp = 0 (destroyed)");
    assertTrue(approxEqual(hp->shield_max, 20000.0f), "shield_max = 20000");
    assertTrue(approxEqual(hp->hull_hp, 0.0f), "hull_hp = 0 (destroyed)");
    assertTrue(approxEqual(hp->hull_max, 50000.0f), "hull_max = 50000");
}

static void testEntityBuilderShip() {
    std::cout << "\n=== EntityBuilder: Ship ===" << std::endl;
    ecs::World world;
    auto* entity = world.createEntity("test1");
    utils::EntityBuilder(entity)
        .withShip("Fang", "Frigate", "Fang", "Keldari")
        .build();

    auto* ship = entity->getComponent<components::Ship>();
    assertTrue(ship != nullptr, "Ship component added");
    assertTrue(ship->ship_name == "Fang", "ship_name = Fang");
    assertTrue(ship->ship_class == "Frigate", "ship_class = Frigate");
    assertTrue(ship->ship_type == "Fang", "ship_type = Fang");
    assertTrue(ship->race == "Keldari", "race = Keldari");
}

static void testEntityBuilderPlayer() {
    std::cout << "\n=== EntityBuilder: Player ===" << std::endl;
    ecs::World world;
    auto* entity = world.createEntity("test1");
    utils::EntityBuilder(entity)
        .withPlayer("player_123", "Commander Alpha")
        .build();

    auto* p = entity->getComponent<components::Player>();
    assertTrue(p != nullptr, "Player component added");
    assertTrue(p->player_id == "player_123", "player_id");
    assertTrue(p->character_name == "Commander Alpha", "character_name");
}

static void testEntityBuilderFactionStandings() {
    std::cout << "\n=== EntityBuilder: Faction + Standings ===" << std::endl;
    ecs::World world;
    auto* entity = world.createEntity("test1");
    utils::EntityBuilder(entity)
        .withFaction("Keldari")
        .withDefaultPlayerStandings()
        .build();

    auto* fac = entity->getComponent<components::Faction>();
    assertTrue(fac != nullptr, "Faction component added");
    assertTrue(fac->faction_name == "Keldari", "faction = Keldari");

    auto* st = entity->getComponent<components::Standings>();
    assertTrue(st != nullptr, "Standings component added");
    assertTrue(approxEqual(st->faction_standings["Veyren"], 0.0f), "Veyren neutral");
    assertTrue(approxEqual(st->faction_standings["Venom Syndicate"], -5.0f), "Venom hostile");
}

static void testEntityBuilderPirateStandings() {
    std::cout << "\n=== EntityBuilder: Pirate Standings ===" << std::endl;
    ecs::World world;
    auto* entity = world.createEntity("pirate1");
    utils::EntityBuilder(entity)
        .withPirateStandings()
        .build();

    auto* st = entity->getComponent<components::Standings>();
    assertTrue(st != nullptr, "Standings component added");
    assertTrue(approxEqual(st->faction_standings["Veyren"], -5.0f), "Hostile to Veyren");
    assertTrue(approxEqual(st->faction_standings["Keldari"], -5.0f), "Hostile to Keldari");
}

static void testEntityBuilderCapacitor() {
    std::cout << "\n=== EntityBuilder: Capacitor ===" << std::endl;
    ecs::World world;
    auto* entity = world.createEntity("test1");
    utils::EntityBuilder(entity)
        .withCapacitor(250.0f, 3.0f)
        .build();

    auto* cap = entity->getComponent<components::Capacitor>();
    assertTrue(cap != nullptr, "Capacitor component added");
    assertTrue(approxEqual(cap->capacitor_max, 250.0f), "cap_max = 250");
    assertTrue(approxEqual(cap->capacitor, 250.0f), "cap = 250 (full)");
    assertTrue(approxEqual(cap->recharge_rate, 3.0f), "recharge = 3");
}

static void testEntityBuilderAI() {
    std::cout << "\n=== EntityBuilder: AI ===" << std::endl;
    ecs::World world;
    auto* entity = world.createEntity("npc1");
    utils::EntityBuilder(entity)
        .withAI(components::AI::Behavior::Aggressive,
                components::AI::State::Idle,
                15000.0f)
        .build();

    auto* ai = entity->getComponent<components::AI>();
    assertTrue(ai != nullptr, "AI component added");
    assertTrue(ai->behavior == components::AI::Behavior::Aggressive, "AI aggressive");
    assertTrue(ai->state == components::AI::State::Idle, "AI idle");
    assertTrue(approxEqual(ai->awareness_range, 15000.0f), "awareness = 15000");
}

static void testEntityBuilderWeapon() {
    std::cout << "\n=== EntityBuilder: Weapon ===" << std::endl;
    ecs::World world;
    auto* entity = world.createEntity("npc1");
    utils::EntityBuilder(entity)
        .withWeapon(12.0f, 5000.0f, 4.0f)
        .build();

    auto* w = entity->getComponent<components::Weapon>();
    assertTrue(w != nullptr, "Weapon component added");
    assertTrue(approxEqual(w->damage, 12.0f), "damage = 12");
    assertTrue(approxEqual(w->optimal_range, 5000.0f), "range = 5000");
    assertTrue(approxEqual(w->rate_of_fire, 4.0f), "rof = 4");
}

static void testEntityBuilderChaining() {
    std::cout << "\n=== EntityBuilder: Full Chaining ===" << std::endl;
    ecs::World world;
    auto* entity = world.createEntity("full_player");
    utils::EntityBuilder(entity)
        .withPosition(100.0f, 0.0f, -50.0f)
        .withVelocity(300.0f)
        .withHealth(450.0f, 350.0f, 300.0f, 3.5f)
        .withShip("Fang", "Frigate", "Fang", "Keldari")
        .withTarget()
        .withPlayer("p1", "TestPilot")
        .withFaction("Keldari")
        .withDefaultPlayerStandings()
        .withCapacitor(250.0f, 3.0f)
        .build();

    assertTrue(entity->hasComponent<components::Position>(), "Has Position");
    assertTrue(entity->hasComponent<components::Velocity>(), "Has Velocity");
    assertTrue(entity->hasComponent<components::Health>(), "Has Health");
    assertTrue(entity->hasComponent<components::Ship>(), "Has Ship");
    assertTrue(entity->hasComponent<components::Target>(), "Has Target");
    assertTrue(entity->hasComponent<components::Player>(), "Has Player");
    assertTrue(entity->hasComponent<components::Faction>(), "Has Faction");
    assertTrue(entity->hasComponent<components::Standings>(), "Has Standings");
    assertTrue(entity->hasComponent<components::Capacitor>(), "Has Capacitor");
}

static void testEntityBuilderGenericComponent() {
    std::cout << "\n=== EntityBuilder: Generic Component ===" << std::endl;
    ecs::World world;
    auto* entity = world.createEntity("test1");
    utils::EntityBuilder(entity)
        .withComponent<components::Target>()
        .build();

    assertTrue(entity->hasComponent<components::Target>(), "Generic component added");
}

static void testEntityBuilderBuildReturnsEntity() {
    std::cout << "\n=== EntityBuilder: Build Returns Entity ===" << std::endl;
    ecs::World world;
    auto* entity = world.createEntity("test1");
    auto* result = utils::EntityBuilder(entity)
        .withPosition(0.0f, 0.0f, 0.0f)
        .build();

    assertTrue(result == entity, "build() returns the entity");
    assertTrue(result->getId() == "test1", "Entity ID preserved");
}

static void testEntityBuilderNPCPattern() {
    std::cout << "\n=== EntityBuilder: NPC Pattern ===" << std::endl;
    ecs::World world;
    auto* entity = world.createEntity("npc_1");
    utils::EntityBuilder(entity)
        .withPosition(1000.0f, 0.0f, -500.0f)
        .withVelocity(250.0f)
        .withHealth(300.0f, 250.0f, 200.0f)
        .withShip("Vipere", "Frigate", "Vipere")
        .withFaction("Venom Syndicate")
        .withPirateStandings()
        .withAI()
        .withWeapon(12.0f, 5000.0f, 4.0f)
        .build();

    assertTrue(entity->hasComponent<components::Position>(), "NPC has Position");
    assertTrue(entity->hasComponent<components::AI>(), "NPC has AI");
    assertTrue(entity->hasComponent<components::Weapon>(), "NPC has Weapon");

    auto* fac = entity->getComponent<components::Faction>();
    assertTrue(fac->faction_name == "Venom Syndicate", "NPC faction");

    auto* st = entity->getComponent<components::Standings>();
    assertTrue(approxEqual(st->faction_standings["Veyren"], -5.0f), "NPC hostile to Veyren");
}

void run_entity_builder_tests() {
    testEntityBuilderPosition();
    testEntityBuilderVelocity();
    testEntityBuilderHealth();
    testEntityBuilderDestroyedHealth();
    testEntityBuilderShip();
    testEntityBuilderPlayer();
    testEntityBuilderFactionStandings();
    testEntityBuilderPirateStandings();
    testEntityBuilderCapacitor();
    testEntityBuilderAI();
    testEntityBuilderWeapon();
    testEntityBuilderChaining();
    testEntityBuilderGenericComponent();
    testEntityBuilderBuildReturnsEntity();
    testEntityBuilderNPCPattern();
}
