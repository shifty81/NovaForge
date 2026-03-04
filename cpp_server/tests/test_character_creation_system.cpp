// Tests for: CharacterCreationSystem Tests
#include "test_log.h"
#include "components/fps_components.h"
#include "components/social_components.h"
#include "ecs/system.h"
#include "systems/character_creation_system.h"
#include <sys/stat.h>

using namespace atlas;

// ==================== CharacterCreationSystem Tests ====================

static void testCharacterCreate() {
    std::cout << "\n=== Character Create ===" << std::endl;

    ecs::World world;
    systems::CharacterCreationSystem charSys(&world);

    auto* entity = world.createEntity("pilot_1");
    addComp<components::CharacterSheet>(entity);

    bool result = charSys.createCharacter("pilot_1", "TestPilot", "Caldari", "Deteis", "Scientist", "male");
    assertTrue(result, "createCharacter returns true for valid race");

    auto* sheet = entity->getComponent<components::CharacterSheet>();
    assertTrue(sheet->character_name == "TestPilot", "Character name is set correctly");
    assertTrue(sheet->intelligence == 23 && sheet->memory == 21, "Caldari starting attributes are correct");
}

static void testCharacterInvalidRace() {
    std::cout << "\n=== Character Invalid Race ===" << std::endl;

    ecs::World world;
    systems::CharacterCreationSystem charSys(&world);

    auto* entity = world.createEntity("pilot_1");
    addComp<components::CharacterSheet>(entity);

    bool result = charSys.createCharacter("pilot_1", "TestPilot", "Jove", "Unknown", "Unknown", "male");
    assertTrue(!result, "createCharacter returns false for invalid race Jove");
}

static void testCharacterInstallImplant() {
    std::cout << "\n=== Character Install Implant ===" << std::endl;

    ecs::World world;
    systems::CharacterCreationSystem charSys(&world);

    auto* entity = world.createEntity("pilot_1");
    addComp<components::CharacterSheet>(entity);
    charSys.createCharacter("pilot_1", "TestPilot", "Caldari", "Deteis", "Scientist", "male");

    bool result = charSys.installImplant("pilot_1", "imp_1", "Neural Boost", 1, "intelligence", 3);
    auto* sheet = entity->getComponent<components::CharacterSheet>();
    assertTrue(sheet->implants.size() == 1, "Implant added to implants vector");
    assertTrue(charSys.getEffectiveAttribute("pilot_1", "intelligence") == 23 + 3, "Effective attribute includes implant bonus");
}

static void testCharacterImplantSlotOccupied() {
    std::cout << "\n=== Character Implant Slot Occupied ===" << std::endl;

    ecs::World world;
    systems::CharacterCreationSystem charSys(&world);

    auto* entity = world.createEntity("pilot_1");
    addComp<components::CharacterSheet>(entity);
    charSys.createCharacter("pilot_1", "TestPilot", "Caldari", "Deteis", "Scientist", "male");

    bool first_install_result = charSys.installImplant("pilot_1", "imp_1", "Neural Boost", 1, "intelligence", 3);
    assertTrue(first_install_result, "First implant in slot 1 succeeds");

    bool second_install_result = charSys.installImplant("pilot_1", "imp_2", "Another Boost", 1, "perception", 2);
    assertTrue(!second_install_result, "Second implant in same slot 1 fails");
}

static void testCharacterRemoveImplant() {
    std::cout << "\n=== Character Remove Implant ===" << std::endl;

    ecs::World world;
    systems::CharacterCreationSystem charSys(&world);

    auto* entity = world.createEntity("pilot_1");
    addComp<components::CharacterSheet>(entity);
    charSys.createCharacter("pilot_1", "TestPilot", "Caldari", "Deteis", "Scientist", "male");
    charSys.installImplant("pilot_1", "imp_1", "Neural Boost", 1, "intelligence", 3);

    bool result = charSys.removeImplant("pilot_1", 1);
    assertTrue(result, "removeImplant returns true for occupied slot");

    auto* sheet = entity->getComponent<components::CharacterSheet>();
    assertTrue(sheet->implants.empty(), "Implants vector is empty after removal");
}

static void testCharacterCloneGrade() {
    std::cout << "\n=== Character Clone Grade ===" << std::endl;

    ecs::World world;
    systems::CharacterCreationSystem charSys(&world);

    auto* entity = world.createEntity("pilot_1");
    addComp<components::CharacterSheet>(entity);
    charSys.createCharacter("pilot_1", "TestPilot", "Caldari", "Deteis", "Scientist", "male");

    bool result = charSys.setCloneGrade("pilot_1", "apex");
    auto* sheet = entity->getComponent<components::CharacterSheet>();
    assertTrue(result && sheet->clone_grade == "apex", "Clone grade set to omega");

    bool gamma_result = charSys.setCloneGrade("pilot_1", "gamma");
    assertTrue(!gamma_result, "Invalid clone grade gamma returns false");
}

static void testCharacterRelayClone() {
    std::cout << "\n=== Character Relay Clone ===" << std::endl;

    ecs::World world;
    systems::CharacterCreationSystem charSys(&world);

    auto* entity = world.createEntity("pilot_1");
    addComp<components::CharacterSheet>(entity);
    charSys.createCharacter("pilot_1", "TestPilot", "Caldari", "Deteis", "Scientist", "male");

    bool result = charSys.jumpClone("pilot_1");
    auto* sheet = entity->getComponent<components::CharacterSheet>();
    assertTrue(result && sheet->clone_jump_cooldown > 0, "Relay clone sets cooldown");

    bool second = charSys.jumpClone("pilot_1");
    assertTrue(!second, "Cannot relay clone while on cooldown");
}

static void testCharacterCloneCooldownDecay() {
    std::cout << "\n=== Character Clone Cooldown Decay ===" << std::endl;

    ecs::World world;
    systems::CharacterCreationSystem charSys(&world);

    auto* entity = world.createEntity("pilot_1");
    addComp<components::CharacterSheet>(entity);
    charSys.createCharacter("pilot_1", "TestPilot", "Caldari", "Deteis", "Scientist", "male");

    charSys.jumpClone("pilot_1");
    charSys.update(86400.0f);

    auto* sheet = entity->getComponent<components::CharacterSheet>();
    assertTrue(sheet->clone_jump_cooldown == 0, "Cooldown decays to 0 after 86400 seconds");

    bool result = charSys.jumpClone("pilot_1");
    assertTrue(result, "Can relay clone again after cooldown expires");
}

static void testCharacterSecurityStatus() {
    std::cout << "\n=== Character Security Status ===" << std::endl;

    ecs::World world;
    systems::CharacterCreationSystem charSys(&world);

    auto* entity = world.createEntity("pilot_1");
    addComp<components::CharacterSheet>(entity);
    charSys.createCharacter("pilot_1", "TestPilot", "Caldari", "Deteis", "Scientist", "male");

    charSys.modifySecurityStatus("pilot_1", 5.0f);
    auto* sheet = entity->getComponent<components::CharacterSheet>();
    assertTrue(approxEqual(sheet->security_status, 5.0f), "Security status increased to 5.0");

    charSys.modifySecurityStatus("pilot_1", 8.0f);
    assertTrue(approxEqual(sheet->security_status, 10.0f), "Security status clamped to 10.0");
}

static void testCharacterEmploymentHistory() {
    std::cout << "\n=== Character Employment History ===" << std::endl;

    ecs::World world;
    systems::CharacterCreationSystem charSys(&world);

    auto* entity = world.createEntity("pilot_1");
    addComp<components::CharacterSheet>(entity);
    charSys.createCharacter("pilot_1", "TestPilot", "Caldari", "Deteis", "Scientist", "male");

    charSys.addEmploymentRecord("pilot_1", "corp_1", "Test Corp", 1000.0f);
    charSys.addEmploymentRecord("pilot_1", "corp_2", "Another Corp", 2000.0f);

    auto* sheet = entity->getComponent<components::CharacterSheet>();
    assertTrue(sheet->employment_history.size() == 2, "Two employment records added");
}

static void testCharacterRaceAttributes() {
    std::cout << "\n=== Character Race Attributes ===" << std::endl;

    ecs::World world;
    systems::CharacterCreationSystem charSys(&world);

    auto* e1 = world.createEntity("amarr_pilot");
    addComp<components::CharacterSheet>(e1);
    charSys.createCharacter("amarr_pilot", "AmarrPilot", "Amarr", "Khanid", "Cyber Knight", "male");

    auto* e2 = world.createEntity("gallente_pilot");
    addComp<components::CharacterSheet>(e2);
    charSys.createCharacter("gallente_pilot", "GallentePilot", "Gallente", "Intaki", "Diplomat", "female");

    auto* e3 = world.createEntity("minmatar_pilot");
    addComp<components::CharacterSheet>(e3);
    charSys.createCharacter("minmatar_pilot", "MinmatarPilot", "Minmatar", "Brutor", "Warrior", "male");

    auto* e4 = world.createEntity("caldari_pilot");
    addComp<components::CharacterSheet>(e4);
    charSys.createCharacter("caldari_pilot", "CaldariPilot", "Caldari", "Deteis", "Scientist", "male");

    auto* s1 = e1->getComponent<components::CharacterSheet>();
    assertTrue(s1->willpower == 22, "Amarr willpower is 22");

    auto* s2 = e2->getComponent<components::CharacterSheet>();
    assertTrue(s2->charisma == 22, "Gallente charisma is 22");

    auto* s3 = e3->getComponent<components::CharacterSheet>();
    assertTrue(s3->perception == 22, "Minmatar perception is 22");

    auto* s4 = e4->getComponent<components::CharacterSheet>();
    assertTrue(s4->intelligence == 23, "Caldari intelligence is 23");
}


void run_character_creation_system_tests() {
    testCharacterCreate();
    testCharacterInvalidRace();
    testCharacterInstallImplant();
    testCharacterImplantSlotOccupied();
    testCharacterRemoveImplant();
    testCharacterCloneGrade();
    testCharacterRelayClone();
    testCharacterCloneCooldownDecay();
    testCharacterSecurityStatus();
    testCharacterEmploymentHistory();
    testCharacterRaceAttributes();
}
