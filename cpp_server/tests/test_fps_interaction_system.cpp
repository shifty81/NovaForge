// Tests for: FPS Interaction System Tests
#include "test_log.h"
#include "components/crew_components.h"
#include "components/fps_components.h"
#include "components/navigation_components.h"
#include "ecs/component.h"
#include "ecs/system.h"
#include "systems/eva_airlock_system.h"
#include "systems/fps_character_controller_system.h"
#include "systems/fps_interaction_system.h"
#include "systems/movement_system.h"
#include "systems/interior_door_system.h"
#include <sys/stat.h>

using namespace atlas;

// ==================== FPS Interaction System Tests ====================

static void testFPSInteractionCreate() {
    std::cout << "\n=== FPS Interaction Create ===" << std::endl;
    ecs::World world;
    systems::FPSInteractionSystem sys(&world);

    bool ok = sys.createInteractable("ia_door1", "interior_1", "door_1",
                                      components::FPSInteractable::InteractionType::Door,
                                      5.0f, 0.0f, 3.0f, 2.0f, "Open Door");
    assertTrue(ok, "Create interactable succeeds");
    assertTrue(!sys.createInteractable("ia_door1", "interior_1", "door_1",
                                        components::FPSInteractable::InteractionType::Door,
                                        5.0f, 0.0f, 3.0f),
               "Duplicate create fails");
}

static void testFPSInteractionFindNearest() {
    std::cout << "\n=== FPS Interaction Find Nearest ===" << std::endl;
    ecs::World world;
    systems::FPSInteractionSystem sys(&world);
    systems::FPSCharacterControllerSystem charSys(&world);

    charSys.spawnCharacter("p1", "interior_1", 5.0f, 0.0f, 3.0f, 0.0f);

    sys.createInteractable("ia_near", "interior_1", "door_near",
                            components::FPSInteractable::InteractionType::Door,
                            5.5f, 0.0f, 3.0f, 2.0f);
    sys.createInteractable("ia_far", "interior_1", "door_far",
                            components::FPSInteractable::InteractionType::Door,
                            50.0f, 0.0f, 3.0f, 2.0f);

    std::string nearest = sys.findNearestInteractable("p1");
    assertTrue(nearest == "ia_near", "Nearest interactable found");
}

static void testFPSInteractionOutOfRange() {
    std::cout << "\n=== FPS Interaction Out Of Range ===" << std::endl;
    ecs::World world;
    systems::FPSInteractionSystem sys(&world);
    systems::FPSCharacterControllerSystem charSys(&world);

    charSys.spawnCharacter("p1", "interior_1", 0.0f, 0.0f, 0.0f, 0.0f);

    sys.createInteractable("ia_far", "interior_1", "door_far",
                            components::FPSInteractable::InteractionType::Door,
                            100.0f, 0.0f, 0.0f, 2.0f);

    assertTrue(!sys.isInRange("p1", "ia_far"), "Out of range detected");
    assertTrue(sys.findNearestInteractable("p1").empty(), "No nearest when all out of range");
}

static void testFPSInteractionDoorToggle() {
    std::cout << "\n=== FPS Interaction Door Toggle ===" << std::endl;
    ecs::World world;
    systems::FPSInteractionSystem sys(&world);
    systems::FPSCharacterControllerSystem charSys(&world);
    systems::InteriorDoorSystem doorSys(&world);

    charSys.spawnCharacter("p1", "interior_1", 5.0f, 0.0f, 3.0f, 0.0f);
    doorSys.createDoor("door_1", "interior_1", "roomA", "roomB",
                        static_cast<int>(components::InteriorDoor::DoorType::Standard));

    sys.createInteractable("ia_door1", "interior_1", "door_1",
                            components::FPSInteractable::InteractionType::Door,
                            5.5f, 0.0f, 3.0f, 2.0f);

    bool ok = sys.interact("p1", "ia_door1");
    assertTrue(ok, "Door opened via interaction");
    assertTrue(doorSys.getDoorState("door_1") ==
               static_cast<int>(components::InteriorDoor::DoorState::Opening),
               "Door is now Opening");

    // Fully open the door
    doorSys.update(10.0f);
    assertTrue(doorSys.getDoorState("door_1") ==
               static_cast<int>(components::InteriorDoor::DoorState::Open),
               "Door fully opened");

    // Close via interaction
    bool ok2 = sys.interact("p1", "ia_door1");
    assertTrue(ok2, "Door closed via interaction");
    assertTrue(doorSys.getDoorState("door_1") ==
               static_cast<int>(components::InteriorDoor::DoorState::Closing),
               "Door is now Closing");
}

static void testFPSInteractionAirlock() {
    std::cout << "\n=== FPS Interaction Airlock ===" << std::endl;
    ecs::World world;
    systems::FPSInteractionSystem sys(&world);
    systems::FPSCharacterControllerSystem charSys(&world);
    systems::EVAAirlockSystem alSys(&world);

    charSys.spawnCharacter("p1", "interior_1", 5.0f, 0.0f, 3.0f, 0.0f);
    // Add survival needs for oxygen check
    auto* charEnt = world.getEntity("fpschar_p1");
    auto needs = std::make_unique<components::SurvivalNeeds>();
    needs->oxygen = 80.0f;
    charEnt->addComponent(std::move(needs));

    alSys.createAirlock("airlock_1", "ship_1", 2.0f);

    sys.createInteractable("ia_airlock1", "interior_1", "airlock_1",
                            components::FPSInteractable::InteractionType::Airlock,
                            5.5f, 0.0f, 3.0f, 2.0f);

    bool ok = sys.interact("p1", "ia_airlock1");
    assertTrue(ok, "Airlock EVA initiated via interaction");
    assertTrue(alSys.getPhase("airlock_1") ==
               static_cast<int>(components::EVAAirlockState::Phase::EnterChamber),
               "Airlock in EnterChamber phase");
}

static void testFPSInteractionMedicalBay() {
    std::cout << "\n=== FPS Interaction Medical Bay ===" << std::endl;
    ecs::World world;
    systems::FPSInteractionSystem sys(&world);
    systems::FPSCharacterControllerSystem charSys(&world);

    charSys.spawnCharacter("p1", "interior_1", 5.0f, 0.0f, 3.0f, 0.0f);
    auto* charEnt = world.getEntity("fpschar_p1");
    auto needs = std::make_unique<components::SurvivalNeeds>();
    needs->oxygen = 30.0f;
    needs->hunger = 80.0f;
    needs->fatigue = 70.0f;
    charEnt->addComponent(std::move(needs));

    sys.createInteractable("ia_med1", "interior_1", "",
                            components::FPSInteractable::InteractionType::MedicalBay,
                            5.5f, 0.0f, 3.0f, 2.0f);

    bool ok = sys.interact("p1", "ia_med1");
    assertTrue(ok, "Medical bay used");

    auto* n = charEnt->getComponent<components::SurvivalNeeds>();
    assertTrue(approxEqual(n->oxygen, 100.0f), "Oxygen refilled");
    assertTrue(n->hunger < 80.0f, "Hunger reduced");
    assertTrue(n->fatigue < 70.0f, "Fatigue reduced");
}

static void testFPSInteractionAccessControl() {
    std::cout << "\n=== FPS Interaction Access Control ===" << std::endl;
    ecs::World world;
    systems::FPSInteractionSystem sys(&world);
    systems::FPSCharacterControllerSystem charSys(&world);

    charSys.spawnCharacter("p1", "interior_1", 5.0f, 0.0f, 3.0f, 0.0f);

    sys.createInteractable("ia_sec", "interior_1", "terminal_sec",
                            components::FPSInteractable::InteractionType::Terminal,
                            5.5f, 0.0f, 3.0f, 2.0f);

    // Set access requirement
    auto* iaEnt = world.getEntity("ia_sec");
    auto* ia = iaEnt->getComponent<components::FPSInteractable>();
    ia->required_access = "captain";

    assertTrue(!sys.interact("p1", "ia_sec", "crew"), "Wrong access denied");
    assertTrue(sys.interact("p1", "ia_sec", "captain"), "Correct access granted");
}

static void testFPSInteractionEnable() {
    std::cout << "\n=== FPS Interaction Enable/Disable ===" << std::endl;
    ecs::World world;
    systems::FPSInteractionSystem sys(&world);
    systems::FPSCharacterControllerSystem charSys(&world);

    charSys.spawnCharacter("p1", "interior_1", 5.0f, 0.0f, 3.0f, 0.0f);

    sys.createInteractable("ia_t", "interior_1", "term_1",
                            components::FPSInteractable::InteractionType::Terminal,
                            5.5f, 0.0f, 3.0f, 2.0f);

    assertTrue(sys.interact("p1", "ia_t"), "Interact when enabled");
    sys.setEnabled("ia_t", false);
    assertTrue(!sys.interact("p1", "ia_t"), "Interact fails when disabled");
    sys.setEnabled("ia_t", true);
    assertTrue(sys.interact("p1", "ia_t"), "Interact succeeds after re-enable");
}

static void testFPSInteractionTypeNames() {
    std::cout << "\n=== FPS Interaction Type Names ===" << std::endl;
    assertTrue(systems::FPSInteractionSystem::typeName(0) == "Door", "Door name");
    assertTrue(systems::FPSInteractionSystem::typeName(1) == "Airlock", "Airlock name");
    assertTrue(systems::FPSInteractionSystem::typeName(2) == "Terminal", "Terminal name");
    assertTrue(systems::FPSInteractionSystem::typeName(3) == "LootContainer", "LootContainer name");
    assertTrue(systems::FPSInteractionSystem::typeName(4) == "Fabricator", "Fabricator name");
    assertTrue(systems::FPSInteractionSystem::typeName(5) == "MedicalBay", "MedicalBay name");
    assertTrue(systems::FPSInteractionSystem::typeName(99) == "Unknown", "Unknown type");
}

static void testFPSInteractionComponentDefaults() {
    std::cout << "\n=== FPS Interaction Component Defaults ===" << std::endl;
    components::FPSInteractable ia;
    assertTrue(ia.interaction_type == 0, "Default type Door");
    assertTrue(approxEqual(ia.interact_range, 2.0f), "Default range 2.0");
    assertTrue(ia.is_enabled, "Default enabled");
    assertTrue(ia.required_access.empty(), "Default no access required");
}

static void testFPSInteractionDifferentInteriors() {
    std::cout << "\n=== FPS Interaction Different Interiors ===" << std::endl;
    ecs::World world;
    systems::FPSInteractionSystem sys(&world);
    systems::FPSCharacterControllerSystem charSys(&world);

    charSys.spawnCharacter("p1", "interior_A", 5.0f, 0.0f, 3.0f, 0.0f);

    sys.createInteractable("ia_other", "interior_B", "door_other",
                            components::FPSInteractable::InteractionType::Door,
                            5.5f, 0.0f, 3.0f, 2.0f);

    assertTrue(sys.findNearestInteractable("p1").empty(),
               "Cannot find interactable in different interior");
}


void run_fps_interaction_system_tests() {
    testFPSInteractionCreate();
    testFPSInteractionFindNearest();
    testFPSInteractionOutOfRange();
    testFPSInteractionDoorToggle();
    testFPSInteractionAirlock();
    testFPSInteractionMedicalBay();
    testFPSInteractionAccessControl();
    testFPSInteractionEnable();
    testFPSInteractionTypeNames();
    testFPSInteractionComponentDefaults();
    testFPSInteractionDifferentInteriors();
}
