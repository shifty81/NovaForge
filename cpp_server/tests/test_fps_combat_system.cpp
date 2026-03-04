// Tests for: FPS Combat System Tests
#include "test_log.h"
#include "components/core_components.h"
#include "components/fps_components.h"
#include "ecs/system.h"
#include "systems/fps_character_controller_system.h"
#include "systems/fps_combat_system.h"
#include "systems/combat_system.h"

using namespace atlas;

// ==================== FPS Combat System Tests ====================

static void testFPSCombatCreateWeapon() {
    std::cout << "\n=== FPS Combat Create Weapon ===" << std::endl;
    ecs::World world;
    systems::FPSCombatSystem sys(&world);

    bool ok = sys.createWeapon("wpn_pistol", "p1",
                                components::FPSWeapon::WeaponCategory::Sidearm,
                                "kinetic", 15.0f, 50.0f, 0.5f, 12);
    assertTrue(ok, "Create weapon succeeds");
    assertTrue(!sys.createWeapon("wpn_pistol", "p1",
                                  components::FPSWeapon::WeaponCategory::Sidearm,
                                  "kinetic", 15.0f, 50.0f, 0.5f, 12),
               "Duplicate weapon fails");
    assertTrue(sys.getAmmo("wpn_pistol") == 12, "Ammo set correctly");
}

static void testFPSCombatEquipUnequip() {
    std::cout << "\n=== FPS Combat Equip/Unequip ===" << std::endl;
    ecs::World world;
    systems::FPSCombatSystem sys(&world);

    sys.createWeapon("wpn_1", "p1", components::FPSWeapon::WeaponCategory::Rifle,
                      "kinetic", 25.0f, 80.0f, 0.3f, 30);

    assertTrue(sys.equipWeapon("wpn_1"), "Equip succeeds");
    auto* e = world.getEntity("wpn_1");
    auto* w = e->getComponent<components::FPSWeapon>();
    assertTrue(w->is_equipped, "Weapon is equipped");

    assertTrue(sys.unequipWeapon("wpn_1"), "Unequip succeeds");
    assertTrue(!w->is_equipped, "Weapon is unequipped");
}

static void testFPSCombatReload() {
    std::cout << "\n=== FPS Combat Reload ===" << std::endl;
    ecs::World world;
    systems::FPSCombatSystem sys(&world);

    sys.createWeapon("wpn_1", "p1", components::FPSWeapon::WeaponCategory::Rifle,
                      "kinetic", 25.0f, 80.0f, 0.3f, 30, 1.0f, 2.0f);

    auto* e = world.getEntity("wpn_1");
    auto* w = e->getComponent<components::FPSWeapon>();
    w->ammo = 5;  // Partially depleted

    assertTrue(sys.startReload("wpn_1"), "Start reload succeeds");
    assertTrue(sys.isReloading("wpn_1"), "Weapon is reloading");
    assertTrue(!sys.startReload("wpn_1"), "Can't double-reload");

    // Tick through reload
    sys.update(2.5f);
    assertTrue(!sys.isReloading("wpn_1"), "Reload complete");
    assertTrue(sys.getAmmo("wpn_1") == 30, "Ammo refilled");
}

static void testFPSCombatReloadFullMag() {
    std::cout << "\n=== FPS Combat Reload Full Magazine ===" << std::endl;
    ecs::World world;
    systems::FPSCombatSystem sys(&world);

    sys.createWeapon("wpn_1", "p1", components::FPSWeapon::WeaponCategory::Rifle,
                      "kinetic", 25.0f, 80.0f, 0.3f, 30);

    assertTrue(!sys.startReload("wpn_1"), "Can't reload full magazine");
}

static void testFPSCombatCreateHealth() {
    std::cout << "\n=== FPS Combat Create Health ===" << std::endl;
    ecs::World world;
    systems::FPSCombatSystem sys(&world);

    auto* entity = world.createEntity("target_npc");
    assertTrue(sys.createHealth("target_npc", 100.0f, 50.0f, 5.0f), "Create health succeeds");
    assertTrue(!sys.createHealth("target_npc", 100.0f, 50.0f, 5.0f), "Duplicate health fails");
    assertTrue(approxEqual(sys.getHealth("target_npc"), 100.0f), "Health set");
    assertTrue(approxEqual(sys.getShield("target_npc"), 50.0f), "Shield set");
    assertTrue(sys.isAlive("target_npc"), "Alive");
}

static void testFPSCombatDamageShieldFirst() {
    std::cout << "\n=== FPS Combat Damage Shield First ===" << std::endl;
    ecs::World world;
    systems::FPSCombatSystem sys(&world);

    auto* entity = world.createEntity("target");
    sys.createHealth("target", 100.0f, 50.0f, 5.0f);

    sys.applyDamage("target", 30.0f, "kinetic");
    assertTrue(approxEqual(sys.getShield("target"), 20.0f), "Shield absorbed 30 damage");
    assertTrue(approxEqual(sys.getHealth("target"), 100.0f), "Health untouched");
}

static void testFPSCombatDamageOverflow() {
    std::cout << "\n=== FPS Combat Damage Shield Overflow ===" << std::endl;
    ecs::World world;
    systems::FPSCombatSystem sys(&world);

    auto* entity = world.createEntity("target");
    sys.createHealth("target", 100.0f, 30.0f, 5.0f);

    sys.applyDamage("target", 50.0f, "kinetic");
    assertTrue(approxEqual(sys.getShield("target"), 0.0f), "Shield depleted");
    assertTrue(approxEqual(sys.getHealth("target"), 80.0f), "Health took overflow (20)");
}

static void testFPSCombatDeath() {
    std::cout << "\n=== FPS Combat Death ===" << std::endl;
    ecs::World world;
    systems::FPSCombatSystem sys(&world);

    std::string dead_id;
    sys.setDeathCallback([&](const std::string& id, float, float, float) {
        dead_id = id;
    });

    auto* entity = world.createEntity("victim");
    sys.createHealth("victim", 50.0f, 0.0f, 0.0f);

    sys.applyDamage("victim", 999.0f, "kinetic");
    assertTrue(!sys.isAlive("victim"), "Victim is dead");
    assertTrue(dead_id == "victim", "Death callback fired");
}

static void testFPSCombatShieldRecharge() {
    std::cout << "\n=== FPS Combat Shield Recharge ===" << std::endl;
    ecs::World world;
    systems::FPSCombatSystem sys(&world);

    auto* entity = world.createEntity("target");
    sys.createHealth("target", 100.0f, 50.0f, 10.0f);

    // Damage shield
    sys.applyDamage("target", 30.0f, "kinetic");
    assertTrue(approxEqual(sys.getShield("target"), 20.0f), "Shield after damage");

    // Wait for recharge delay (default 3s)
    sys.update(4.0f);
    float shield_after = sys.getShield("target");
    assertTrue(shield_after > 20.0f, "Shield began recharging after delay");
}

static void testFPSCombatShieldRechargeDelay() {
    std::cout << "\n=== FPS Combat Shield Recharge Delay ===" << std::endl;
    ecs::World world;
    systems::FPSCombatSystem sys(&world);

    auto* entity = world.createEntity("target");
    sys.createHealth("target", 100.0f, 50.0f, 10.0f);

    sys.applyDamage("target", 30.0f, "kinetic");

    // Update with less than recharge delay — shield should not recharge
    sys.update(1.0f);
    assertTrue(approxEqual(sys.getShield("target"), 20.0f),
               "Shield not recharging during delay");
}

static void testFPSCombatFireWeapon() {
    std::cout << "\n=== FPS Combat Fire Weapon ===" << std::endl;
    ecs::World world;
    systems::FPSCombatSystem sys(&world);
    systems::FPSCharacterControllerSystem charSys(&world);

    // Shooter
    charSys.spawnCharacter("shooter", "interior_1", 0.0f, 0.0f, 0.0f, 0.0f);
    sys.createWeapon("wpn_s", "shooter", components::FPSWeapon::WeaponCategory::Sidearm,
                      "kinetic", 15.0f, 50.0f, 0.5f, 12);
    sys.equipWeapon("wpn_s");

    // Target (FPS character with health)
    charSys.spawnCharacter("target", "interior_1", 10.0f, 0.0f, 0.0f, 0.0f);
    auto* targetEnt = world.getEntity("fpschar_target");
    sys.createHealth("fpschar_target", 100.0f, 0.0f, 0.0f);

    bool ok = sys.fireWeapon("shooter", "fpschar_target");
    assertTrue(ok, "Fire weapon succeeds");
    assertTrue(sys.getAmmo("wpn_s") == 11, "Ammo decremented");
    assertTrue(approxEqual(sys.getHealth("fpschar_target"), 85.0f), "Target took 15 damage");
}

static void testFPSCombatFireOutOfRange() {
    std::cout << "\n=== FPS Combat Fire Out Of Range ===" << std::endl;
    ecs::World world;
    systems::FPSCombatSystem sys(&world);
    systems::FPSCharacterControllerSystem charSys(&world);

    charSys.spawnCharacter("shooter", "interior_1", 0.0f, 0.0f, 0.0f, 0.0f);
    sys.createWeapon("wpn_s", "shooter", components::FPSWeapon::WeaponCategory::Sidearm,
                      "kinetic", 15.0f, 50.0f, 0.5f, 12);
    sys.equipWeapon("wpn_s");

    charSys.spawnCharacter("target", "interior_1", 200.0f, 0.0f, 0.0f, 0.0f);
    sys.createHealth("fpschar_target", 100.0f, 0.0f, 0.0f);

    bool ok = sys.fireWeapon("shooter", "fpschar_target");
    assertTrue(!ok, "Fire out of range fails");
    assertTrue(sys.getAmmo("wpn_s") == 12, "Ammo not consumed");
}

static void testFPSCombatFireOnCooldown() {
    std::cout << "\n=== FPS Combat Fire On Cooldown ===" << std::endl;
    ecs::World world;
    systems::FPSCombatSystem sys(&world);
    systems::FPSCharacterControllerSystem charSys(&world);

    charSys.spawnCharacter("shooter", "interior_1", 0.0f, 0.0f, 0.0f, 0.0f);
    sys.createWeapon("wpn_s", "shooter", components::FPSWeapon::WeaponCategory::Sidearm,
                      "kinetic", 15.0f, 50.0f, 0.5f, 12);
    sys.equipWeapon("wpn_s");

    charSys.spawnCharacter("target", "interior_1", 10.0f, 0.0f, 0.0f, 0.0f);
    sys.createHealth("fpschar_target", 100.0f, 0.0f, 0.0f);

    assertTrue(sys.fireWeapon("shooter", "fpschar_target"), "First shot succeeds");
    assertTrue(!sys.fireWeapon("shooter", "fpschar_target"), "Second shot on cooldown fails");

    sys.update(1.0f);  // Clear cooldown
    assertTrue(sys.fireWeapon("shooter", "fpschar_target"), "Shot after cooldown succeeds");
}

static void testFPSCombatFireNoAmmo() {
    std::cout << "\n=== FPS Combat Fire No Ammo ===" << std::endl;
    ecs::World world;
    systems::FPSCombatSystem sys(&world);
    systems::FPSCharacterControllerSystem charSys(&world);

    charSys.spawnCharacter("shooter", "interior_1", 0.0f, 0.0f, 0.0f, 0.0f);
    sys.createWeapon("wpn_s", "shooter", components::FPSWeapon::WeaponCategory::Sidearm,
                      "kinetic", 15.0f, 50.0f, 0.5f, 1);
    sys.equipWeapon("wpn_s");

    charSys.spawnCharacter("target", "interior_1", 10.0f, 0.0f, 0.0f, 0.0f);
    sys.createHealth("fpschar_target", 100.0f, 0.0f, 0.0f);

    assertTrue(sys.fireWeapon("shooter", "fpschar_target"), "First shot uses last ammo");
    sys.update(1.0f);
    assertTrue(!sys.fireWeapon("shooter", "fpschar_target"), "No ammo to fire");
}

static void testFPSCombatCategoryNames() {
    std::cout << "\n=== FPS Combat Category Names ===" << std::endl;
    assertTrue(systems::FPSCombatSystem::categoryName(0) == "Sidearm", "Sidearm name");
    assertTrue(systems::FPSCombatSystem::categoryName(1) == "Rifle", "Rifle name");
    assertTrue(systems::FPSCombatSystem::categoryName(2) == "Shotgun", "Shotgun name");
    assertTrue(systems::FPSCombatSystem::categoryName(3) == "Tool", "Tool name");
    assertTrue(systems::FPSCombatSystem::categoryName(99) == "Unknown", "Unknown category");
}

static void testFPSCombatWeaponDefaults() {
    std::cout << "\n=== FPS Combat Weapon Defaults ===" << std::endl;
    components::FPSWeapon w;
    assertTrue(w.category == 0, "Default category Sidearm");
    assertTrue(w.damage_type == "kinetic", "Default damage type kinetic");
    assertTrue(approxEqual(w.damage, 15.0f), "Default damage 15");
    assertTrue(approxEqual(w.range, 50.0f), "Default range 50");
    assertTrue(w.ammo == 30, "Default ammo 30");
    assertTrue(!w.is_reloading, "Default not reloading");
    assertTrue(!w.is_equipped, "Default not equipped");
}

static void testFPSCombatHealthDefaults() {
    std::cout << "\n=== FPS Combat Health Defaults ===" << std::endl;
    components::FPSHealth h;
    assertTrue(approxEqual(h.health, 100.0f), "Default health 100");
    assertTrue(approxEqual(h.shield, 50.0f), "Default shield 50");
    assertTrue(h.is_alive, "Default alive");
    assertTrue(approxEqual(h.getHealthFraction(), 1.0f), "Health fraction 1.0");
    assertTrue(approxEqual(h.getShieldFraction(), 1.0f), "Shield fraction 1.0");
}


void run_fps_combat_system_tests() {
    testFPSCombatCreateWeapon();
    testFPSCombatEquipUnequip();
    testFPSCombatReload();
    testFPSCombatReloadFullMag();
    testFPSCombatCreateHealth();
    testFPSCombatDamageShieldFirst();
    testFPSCombatDamageOverflow();
    testFPSCombatDeath();
    testFPSCombatShieldRecharge();
    testFPSCombatShieldRechargeDelay();
    testFPSCombatFireWeapon();
    testFPSCombatFireOutOfRange();
    testFPSCombatFireOnCooldown();
    testFPSCombatFireNoAmmo();
    testFPSCombatCategoryNames();
    testFPSCombatWeaponDefaults();
    testFPSCombatHealthDefaults();
}
