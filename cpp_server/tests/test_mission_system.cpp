// Tests for: Mission System Tests, Mission Economy Effects Tests, Mission Protocol Tests
#include "test_log.h"
#include "components/combat_components.h"
#include "components/core_components.h"
#include "components/mission_components.h"
#include "components/ship_components.h"
#include "ecs/system.h"
#include "systems/ai_system.h"
#include "systems/movement_system.h"
#include "network/protocol_handler.h"
#include "systems/mission_generator_system.h"
#include "systems/mission_system.h"
#include "systems/mission_template_system.h"
#include <sys/stat.h>

using namespace atlas;

// ==================== Mission System Tests ====================

static void testMissionAcceptAndComplete() {
    std::cout << "\n=== Mission Accept & Complete ===" << std::endl;

    ecs::World world;
    systems::MissionSystem missionSys(&world);

    auto* player = world.createEntity("player1");
    addComp<components::MissionTracker>(player);
    auto* playerComp = addComp<components::Player>(player);
    playerComp->credits = 0.0;
    auto* standings = addComp<components::Standings>(player);

    // Accept a mission
    bool accepted = missionSys.acceptMission("player1", "mission_001",
        "Destroy Pirates", 1, "combat", "Veyren", 100000.0, 0.5f);
    assertTrue(accepted, "Mission accepted successfully");

    auto* tracker = player->getComponent<components::MissionTracker>();
    assertTrue(tracker->active_missions.size() == 1, "One active mission");

    // Add objective
    components::MissionTracker::Objective obj;
    obj.type = "destroy";
    obj.target = "pirate_frigate";
    obj.required = 3;
    obj.completed = 0;
    tracker->active_missions[0].objectives.push_back(obj);

    // Record partial progress
    missionSys.recordProgress("player1", "mission_001", "destroy", "pirate_frigate", 2);
    assertTrue(tracker->active_missions[0].objectives[0].completed == 2,
               "Partial progress recorded (2/3)");

    // Complete the objective
    missionSys.recordProgress("player1", "mission_001", "destroy", "pirate_frigate", 1);
    assertTrue(tracker->active_missions[0].objectives[0].done(),
               "Objective completed (3/3)");

    // Update should process completion
    missionSys.update(0.0f);
    assertTrue(approxEqual(static_cast<float>(playerComp->credits), 100000.0f, 1.0f),
               "Credits reward applied");
    assertTrue(tracker->completed_mission_ids.size() == 1,
               "Mission recorded as completed");
    assertTrue(tracker->active_missions.empty(),
               "Active missions cleared after completion");

    // Check standing was applied
    float standing = standings->faction_standings["Veyren"];
    assertTrue(approxEqual(standing, 0.5f), "Standing reward applied");
}

static void testMissionTimeout() {
    std::cout << "\n=== Mission Timeout ===" << std::endl;

    ecs::World world;
    systems::MissionSystem missionSys(&world);

    auto* player = world.createEntity("player1");
    addComp<components::MissionTracker>(player);
    addComp<components::Player>(player);

    // Accept a timed mission (30 second limit)
    missionSys.acceptMission("player1", "timed_001",
        "Timed Mission", 1, "combat", "Veyren", 50000.0, 0.1f, 30.0f);

    auto* tracker = player->getComponent<components::MissionTracker>();

    // Add an incomplete objective
    components::MissionTracker::Objective obj;
    obj.type = "destroy";
    obj.target = "enemy";
    obj.required = 5;
    tracker->active_missions[0].objectives.push_back(obj);

    // Update for 25 seconds (should still be active)
    missionSys.update(25.0f);
    assertTrue(tracker->active_missions.size() == 1, "Mission still active at 25s");

    // Update past the time limit
    missionSys.update(10.0f);
    assertTrue(tracker->active_missions.empty(), "Timed-out mission removed");
}

static void testMissionAbandon() {
    std::cout << "\n=== Mission Abandon ===" << std::endl;

    ecs::World world;
    systems::MissionSystem missionSys(&world);

    auto* player = world.createEntity("player1");
    addComp<components::MissionTracker>(player);

    missionSys.acceptMission("player1", "abandon_001",
        "Will Abandon", 1, "combat", "Faction", 10000.0, 0.1f);

    auto* tracker = player->getComponent<components::MissionTracker>();
    assertTrue(tracker->active_missions.size() == 1, "Mission active before abandon");

    missionSys.abandonMission("player1", "abandon_001");
    assertTrue(tracker->active_missions.empty(), "Mission removed after abandon");
}

static void testMissionDuplicatePrevention() {
    std::cout << "\n=== Mission Duplicate Prevention ===" << std::endl;

    ecs::World world;
    systems::MissionSystem missionSys(&world);

    auto* player = world.createEntity("player1");
    addComp<components::MissionTracker>(player);

    bool first = missionSys.acceptMission("player1", "dup_001",
        "First", 1, "combat", "Faction", 10000.0, 0.1f);
    bool second = missionSys.acceptMission("player1", "dup_001",
        "Duplicate", 1, "combat", "Faction", 10000.0, 0.1f);

    assertTrue(first, "First accept succeeds");
    assertTrue(!second, "Duplicate accept rejected");
}


// ==================== Mission Economy Effects Tests ====================

static void testMissionEconomyCombatReducesSpawnRate() {
    ecs::World world;
    systems::MissionSystem sys(&world);

    // Create system entity with DifficultyZone
    auto* sys_entity = world.createEntity("system1");
    auto* zone = addComp<components::DifficultyZone>(sys_entity);
    zone->spawn_rate_multiplier = 1.5f;

    // Create player with mission
    auto* player = world.createEntity("player1");
    auto* tracker = addComp<components::MissionTracker>(player);
    addComp<components::Player>(player);

    sys.setEconomySystemId("system1");
    sys.acceptMission("player1", "m1", "Clear Pirates", 3, "combat", "Solari",
                      100000.0, 0.1f, -1.0f);

    // Add objective and complete it
    tracker->active_missions[0].objectives.push_back({"destroy", "pirate", 1, 1});
    sys.update(0.1f);

    assertTrue(zone->spawn_rate_multiplier < 1.5f,
               "Combat mission completion reduces spawn rate");
}

static void testMissionEconomyMiningReducesOre() {
    ecs::World world;
    systems::MissionSystem sys(&world);

    auto* sys_entity = world.createEntity("system1");
    auto* resources = addComp<components::SystemResources>(sys_entity);
    components::SystemResources::ResourceEntry entry;
    entry.mineral_type = "Ferrite";
    entry.total_quantity = 1000.0f;
    entry.remaining_quantity = 1000.0f;
    resources->resources.push_back(entry);

    auto* player = world.createEntity("player1");
    auto* tracker = addComp<components::MissionTracker>(player);
    addComp<components::Player>(player);

    sys.setEconomySystemId("system1");
    sys.acceptMission("player1", "m1", "Mine Ore", 2, "mining", "Solari",
                      50000.0, 0.05f, -1.0f);
    tracker->active_missions[0].objectives.push_back({"mine", "Ferrite", 1, 1});
    sys.update(0.1f);

    assertTrue(resources->resources[0].remaining_quantity < 1000.0f,
               "Mining mission completion depletes ore reserves");
}

static void testMissionEconomyCompletedCount() {
    ecs::World world;
    systems::MissionSystem sys(&world);

    auto* player = world.createEntity("player1");
    auto* tracker = addComp<components::MissionTracker>(player);
    addComp<components::Player>(player);

    sys.acceptMission("player1", "m1", "Test", 1, "combat", "Solari",
                      10000.0, 0.1f, -1.0f);
    tracker->active_missions[0].objectives.push_back({"destroy", "pirate", 1, 1});
    sys.update(0.1f);

    assertTrue(sys.getCompletedMissionCount() == 1,
               "Completed mission count increments");
}


// ==================== Mission Protocol Tests ====================

static void testMissionProtocolRoundTrip() {
    std::cout << "\n=== Mission Protocol Round Trip ===" << std::endl;

    atlas::network::ProtocolHandler proto;

    // Test mission_list message parses correctly
    std::string list_msg = "{\"message_type\":\"mission_list\",\"data\":{\"system_id\":\"sys_01\"}}";
    atlas::network::MessageType type;
    std::string data;
    bool ok = proto.parseMessage(list_msg, type, data);
    assertTrue(ok, "Mission list message parses");
    assertTrue(type == atlas::network::MessageType::MISSION_LIST, "Type is MISSION_LIST");

    // Test accept_mission message parses correctly
    std::string accept_msg = "{\"message_type\":\"accept_mission\",\"data\":{\"system_id\":\"sys_01\",\"mission_index\":0}}";
    ok = proto.parseMessage(accept_msg, type, data);
    assertTrue(ok, "Accept mission message parses");
    assertTrue(type == atlas::network::MessageType::ACCEPT_MISSION, "Type is ACCEPT_MISSION");

    // Test abandon_mission message parses correctly
    std::string abandon_msg = "{\"message_type\":\"abandon_mission\",\"data\":{\"mission_id\":\"m_001\"}}";
    ok = proto.parseMessage(abandon_msg, type, data);
    assertTrue(ok, "Abandon mission message parses");
    assertTrue(type == atlas::network::MessageType::ABANDON_MISSION, "Type is ABANDON_MISSION");

    // Test mission_progress message parses correctly
    std::string progress_msg = "{\"message_type\":\"mission_progress\",\"data\":{\"mission_id\":\"m_001\",\"objective_type\":\"destroy\"}}";
    ok = proto.parseMessage(progress_msg, type, data);
    assertTrue(ok, "Mission progress message parses");
    assertTrue(type == atlas::network::MessageType::MISSION_PROGRESS, "Type is MISSION_PROGRESS");

    // Test createMissionList output
    std::string ml = proto.createMissionList("sys_01", 2, "[{\"name\":\"Patrol\"},{\"name\":\"Mining\"}]");
    assertTrue(ml.find("mission_list") != std::string::npos, "MissionList contains type");
    assertTrue(ml.find("sys_01") != std::string::npos, "MissionList contains system_id");
    assertTrue(ml.find("Patrol") != std::string::npos, "MissionList contains mission data");

    // Test createMissionResult output
    std::string mr = proto.createMissionResult(true, "m_001", "accept", "Mission accepted");
    assertTrue(mr.find("mission_result") != std::string::npos, "MissionResult contains type");
    assertTrue(mr.find("m_001") != std::string::npos, "MissionResult contains mission_id");
    assertTrue(mr.find("accept") != std::string::npos, "MissionResult contains action");
}

static void testMissionGeneratorNetworkFlow() {
    std::cout << "\n=== Mission Generator Network Flow ===" << std::endl;

    ecs::World world;
    systems::MissionTemplateSystem templates(&world);
    systems::MissionGeneratorSystem generator(&world, &templates);

    templates.installDefaultTemplates();

    // Create a system entity
    auto* sys = world.createEntity("system_01");
    auto* dz = addComp<components::DifficultyZone>(sys);
    dz->security_status = 0.5f;

    // Generate missions for the system
    int count = generator.generateMissionsForSystem("system_01", 42);
    assertTrue(count > 0, "Missions generated for system");

    auto available = generator.getAvailableMissions("system_01");
    assertTrue(!available.empty(), "Available missions list is not empty");

    // Create a player and offer a mission
    auto* player = world.createEntity("player_01");
    addComp<components::MissionTracker>(player);
    addComp<components::Player>(player);

    bool offered = generator.offerMissionToPlayer("player_01", "system_01", 0);
    assertTrue(offered, "Mission offered to player successfully");

    auto* tracker = player->getComponent<components::MissionTracker>();
    assertTrue(tracker->active_missions.size() == 1, "Player has one active mission");
    assertTrue(!tracker->active_missions[0].mission_id.empty(), "Mission has a valid ID");
}

static void testMissionAcceptAbandonNetworkFlow() {
    std::cout << "\n=== Mission Accept/Abandon Network Flow ===" << std::endl;

    ecs::World world;
    systems::MissionSystem missions(&world);
    systems::MissionTemplateSystem templates(&world);
    systems::MissionGeneratorSystem generator(&world, &templates);

    templates.installDefaultTemplates();

    auto* sys_entity = world.createEntity("system_01");
    auto* dz = addComp<components::DifficultyZone>(sys_entity);
    dz->security_status = 0.5f;

    generator.generateMissionsForSystem("system_01", 42);

    auto* player = world.createEntity("player_01");
    addComp<components::MissionTracker>(player);
    addComp<components::Player>(player);

    // Accept a mission
    generator.offerMissionToPlayer("player_01", "system_01", 0);
    auto* tracker = player->getComponent<components::MissionTracker>();
    std::string mid = tracker->active_missions[0].mission_id;
    assertTrue(!mid.empty(), "Accepted mission has valid ID");

    // Abandon the mission
    missions.abandonMission("player_01", mid);
    assertTrue(tracker->active_missions.empty(), "Mission abandoned — active list empty");
}

static void testAIDefensiveBehavior() {
    std::cout << "\n=== AI Defensive Behavior ===" << std::endl;

    ecs::World world;
    systems::AISystem aiSys(&world);

    // Create a defensive NPC (patrol)
    auto* patrol = world.createEntity("patrol_01");
    auto* ai = addComp<components::AI>(patrol);
    ai->behavior = components::AI::Behavior::Defensive;
    ai->awareness_range = 100000.0f;
    addComp<components::Position>(patrol);
    addComp<components::Velocity>(patrol);
    auto* patrolFaction = addComp<components::Faction>(patrol);
    patrolFaction->faction_name = "Solari";
    patrolFaction->standings["Veyren"] = -5.0f;  // hostile to Veyren

    // Create a friendly player (positive standing with Solari)
    auto* player = world.createEntity("player_01");
    addComp<components::Player>(player);
    auto* playerPos = addComp<components::Position>(player);
    playerPos->x = 200.0f;
    auto* standings = addComp<components::Standings>(player);
    standings->faction_standings["Solari"] = 3.0f;
    // Player has damage events (being attacked)
    auto* dmg = addComp<components::DamageEvent>(player);
    components::DamageEvent::HitRecord hit;
    hit.damage_amount = 50.0f;
    hit.layer_hit = "shield";
    hit.damage_type = "em";
    dmg->recent_hits.push_back(hit);

    // Create a hostile NPC attacking the player
    auto* pirate = world.createEntity("pirate_01");
    auto* pirateAI = addComp<components::AI>(pirate);
    pirateAI->behavior = components::AI::Behavior::Aggressive;
    pirateAI->target_entity_id = "player_01";  // targeting the player
    auto* piratePos = addComp<components::Position>(pirate);
    piratePos->x = 300.0f;
    addComp<components::Velocity>(pirate);
    auto* pirateFaction = addComp<components::Faction>(pirate);
    pirateFaction->faction_name = "Veyren";

    // The patrol should find the pirate attacking its friendly player
    ecs::Entity* attacker = aiSys.findAttackerOfFriendly(patrol);
    assertTrue(attacker == pirate, "Defensive NPC finds attacker of friendly player");
}

static void testAIDefensiveNoFriendly() {
    std::cout << "\n=== AI Defensive No Friendly ===" << std::endl;

    ecs::World world;
    systems::AISystem aiSys(&world);

    auto* patrol = world.createEntity("patrol_01");
    auto* ai = addComp<components::AI>(patrol);
    ai->behavior = components::AI::Behavior::Defensive;
    ai->awareness_range = 100000.0f;
    addComp<components::Position>(patrol);
    addComp<components::Velocity>(patrol);
    auto* patrolFaction = addComp<components::Faction>(patrol);
    patrolFaction->faction_name = "Solari";

    // No friendly entities in range → should find no attacker
    ecs::Entity* attacker = aiSys.findAttackerOfFriendly(patrol);
    assertTrue(attacker == nullptr, "No attacker found when no friendlies present");
}

static void testAIDefensiveIdleTransition() {
    std::cout << "\n=== AI Defensive Idle Transition ===" << std::endl;

    ecs::World world;
    systems::AISystem aiSys(&world);

    // Create a defensive NPC
    auto* patrol = world.createEntity("patrol_01");
    auto* ai = addComp<components::AI>(patrol);
    ai->behavior = components::AI::Behavior::Defensive;
    ai->awareness_range = 100000.0f;
    ai->state = components::AI::State::Idle;
    addComp<components::Position>(patrol);
    addComp<components::Velocity>(patrol);
    auto* patrolFaction = addComp<components::Faction>(patrol);
    patrolFaction->faction_name = "Solari";
    patrolFaction->standings["Veyren"] = -5.0f;

    // Create friendly player under attack
    auto* player = world.createEntity("player_01");
    addComp<components::Player>(player);
    auto* playerPos = addComp<components::Position>(player);
    playerPos->x = 200.0f;
    auto* standings = addComp<components::Standings>(player);
    standings->faction_standings["Solari"] = 3.0f;
    auto* dmg = addComp<components::DamageEvent>(player);
    components::DamageEvent::HitRecord hit;
    hit.damage_amount = 50.0f;
    hit.layer_hit = "shield";
    hit.damage_type = "em";
    dmg->recent_hits.push_back(hit);

    auto* pirate = world.createEntity("pirate_01");
    auto* pirateAI = addComp<components::AI>(pirate);
    pirateAI->behavior = components::AI::Behavior::Aggressive;
    pirateAI->target_entity_id = "player_01";
    auto* piratePos = addComp<components::Position>(pirate);
    piratePos->x = 300.0f;
    addComp<components::Velocity>(pirate);
    auto* pirateFaction = addComp<components::Faction>(pirate);
    pirateFaction->faction_name = "Veyren";

    // Run AI update — patrol should transition to Approaching the pirate
    aiSys.update(0.1f);

    assertTrue(ai->state == components::AI::State::Approaching,
               "Defensive NPC transitions from Idle to Approaching when friendly attacked");
    assertTrue(ai->target_entity_id == "pirate_01",
               "Defensive NPC targets the attacker");
}


void run_mission_system_tests() {
    testMissionAcceptAndComplete();
    testMissionTimeout();
    testMissionAbandon();
    testMissionDuplicatePrevention();
    testMissionEconomyCombatReducesSpawnRate();
    testMissionEconomyMiningReducesOre();
    testMissionEconomyCompletedCount();
    testMissionProtocolRoundTrip();
    testMissionGeneratorNetworkFlow();
    testMissionAcceptAbandonNetworkFlow();
    testAIDefensiveBehavior();
    testAIDefensiveNoFriendly();
    testAIDefensiveIdleTransition();
}
