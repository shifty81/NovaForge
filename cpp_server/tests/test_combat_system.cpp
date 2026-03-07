// Tests for: Damage Event Tests, Combat Death → Wreck Integration Tests, Combat Loop Integration Tests
#include "test_log.h"
#include "components/combat_components.h"
#include "components/core_components.h"
#include "components/narrative_components.h"
#include "components/ship_components.h"
#include "ecs/system.h"
#include "pcg/salvage_system.h"
#include "systems/combat_system.h"
#include "systems/loot_system.h"
#include "systems/wreck_salvage_system.h"

using namespace atlas;

// ==================== Damage Event Tests ====================

static void testDamageEventOnShieldHit() {
    std::cout << "\n=== Damage Event On Shield Hit ===" << std::endl;

    ecs::World world;
    systems::CombatSystem combatSys(&world);

    auto* target = world.createEntity("target1");
    auto* health = addComp<components::Health>(target);
    health->shield_hp = 500.0f;
    health->shield_max = 500.0f;
    health->armor_hp = 300.0f;
    health->armor_max = 300.0f;
    health->hull_hp = 200.0f;
    health->hull_max = 200.0f;

    combatSys.applyDamage("target1", 50.0f, "kinetic");

    auto* dmgEvent = target->getComponent<components::DamageEvent>();
    assertTrue(dmgEvent != nullptr, "DamageEvent created on damage");
    assertTrue(dmgEvent->recent_hits.size() == 1, "One hit recorded");
    assertTrue(dmgEvent->recent_hits[0].layer_hit == "shield", "Hit registered on shield layer");
    assertTrue(approxEqual(dmgEvent->recent_hits[0].damage_amount, 50.0f), "Damage amount recorded");
    assertTrue(dmgEvent->recent_hits[0].damage_type == "kinetic", "Damage type recorded");
    assertTrue(!dmgEvent->recent_hits[0].shield_depleted, "Shield not depleted");
}

static void testDamageEventShieldDepleted() {
    std::cout << "\n=== Damage Event Shield Depleted ===" << std::endl;

    ecs::World world;
    systems::CombatSystem combatSys(&world);

    auto* target = world.createEntity("target1");
    auto* health = addComp<components::Health>(target);
    health->shield_hp = 20.0f;
    health->shield_max = 500.0f;
    health->armor_hp = 300.0f;
    health->armor_max = 300.0f;
    health->hull_hp = 200.0f;
    health->hull_max = 200.0f;

    // Apply 50 damage; 20 to shield (depletes) + 30 overflows to armor
    combatSys.applyDamage("target1", 50.0f, "kinetic");

    auto* dmgEvent = target->getComponent<components::DamageEvent>();
    assertTrue(dmgEvent != nullptr, "DamageEvent created");
    assertTrue(dmgEvent->recent_hits.size() == 1, "One hit recorded");
    assertTrue(dmgEvent->recent_hits[0].shield_depleted, "Shield depleted flag set");
    assertTrue(approxEqual(health->shield_hp, 0.0f), "Shield HP is 0");
}

static void testDamageEventHullCritical() {
    std::cout << "\n=== Damage Event Hull Critical ===" << std::endl;

    ecs::World world;
    systems::CombatSystem combatSys(&world);

    auto* target = world.createEntity("target1");
    auto* health = addComp<components::Health>(target);
    health->shield_hp = 0.0f;
    health->shield_max = 100.0f;
    health->armor_hp = 0.0f;
    health->armor_max = 100.0f;
    health->hull_hp = 100.0f;
    health->hull_max = 100.0f;

    // Hit hull for 80 damage, leaving 20 HP (20% < 25% threshold)
    combatSys.applyDamage("target1", 80.0f, "explosive");

    auto* dmgEvent = target->getComponent<components::DamageEvent>();
    assertTrue(dmgEvent != nullptr, "DamageEvent created");
    assertTrue(dmgEvent->recent_hits[0].hull_critical, "Hull critical flag set (below 25%)");
    assertTrue(dmgEvent->recent_hits[0].layer_hit == "hull", "Hit on hull layer");
}

static void testDamageEventMultipleHits() {
    std::cout << "\n=== Damage Event Multiple Hits ===" << std::endl;

    ecs::World world;
    systems::CombatSystem combatSys(&world);

    auto* target = world.createEntity("target1");
    auto* health = addComp<components::Health>(target);
    health->shield_hp = 500.0f;
    health->shield_max = 500.0f;
    health->armor_hp = 300.0f;
    health->armor_max = 300.0f;

    combatSys.applyDamage("target1", 10.0f, "em");
    combatSys.applyDamage("target1", 20.0f, "thermal");
    combatSys.applyDamage("target1", 30.0f, "kinetic");

    auto* dmgEvent = target->getComponent<components::DamageEvent>();
    assertTrue(dmgEvent != nullptr, "DamageEvent exists");
    assertTrue(dmgEvent->recent_hits.size() == 3, "Three hits recorded");
    assertTrue(approxEqual(dmgEvent->total_damage_taken, 60.0f), "Total damage tracked");
}

static void testDamageEventClearOldHits() {
    std::cout << "\n=== Damage Event Clear Old Hits ===" << std::endl;

    ecs::World world;
    systems::CombatSystem combatSys(&world);

    auto* target = world.createEntity("target1");
    auto* health = addComp<components::Health>(target);
    health->shield_hp = 500.0f;
    health->shield_max = 500.0f;

    combatSys.applyDamage("target1", 10.0f, "em");

    auto* dmgEvent = target->getComponent<components::DamageEvent>();
    assertTrue(dmgEvent != nullptr, "DamageEvent exists");
    assertTrue(dmgEvent->recent_hits.size() == 1, "One hit before clear");

    // Clear with a future timestamp beyond max_age
    dmgEvent->clearOldHits(100.0f, 5.0f);
    assertTrue(dmgEvent->recent_hits.size() == 0, "Old hits cleared");
}


// ==================== Combat Death → Wreck Integration Tests ====================

static void testCombatDeathCallbackFires() {
    std::cout << "\n=== Combat: Death Callback Fires ===" << std::endl;

    ecs::World world;
    systems::CombatSystem combatSys(&world);
    systems::WreckSalvageSystem wreckSys(&world);

    std::string wreck_created;
    combatSys.setDeathCallback([&](const std::string& entity_id, float x, float y, float z) {
        wreck_created = wreckSys.createWreck(entity_id, x, y, z);
    });

    auto* npc = world.createEntity("npc_1");
    auto* hp = addComp<components::Health>(npc);
    hp->shield_hp = 0.0f;
    hp->armor_hp = 0.0f;
    hp->hull_hp = 10.0f;
    hp->hull_max = 100.0f;
    auto* pos = addComp<components::Position>(npc);
    pos->x = 500.0f;
    pos->y = 200.0f;
    pos->z = 100.0f;

    // Apply lethal damage
    combatSys.applyDamage("npc_1", 50.0f, "kinetic");

    assertTrue(!wreck_created.empty(), "Wreck created on death");
    assertTrue(wreckSys.getActiveWreckCount() == 1, "One active wreck");

    auto* wreck_entity = world.getEntity(wreck_created);
    assertTrue(wreck_entity != nullptr, "Wreck entity exists");
    auto* wreck_pos = wreck_entity->getComponent<components::Position>();
    assertTrue(wreck_pos != nullptr, "Wreck has position");
    assertTrue(approxEqual(wreck_pos->x, 500.0f), "Wreck at correct x");
    assertTrue(approxEqual(wreck_pos->y, 200.0f), "Wreck at correct y");
}

static void testCombatDeathCallbackWithLoot() {
    std::cout << "\n=== Combat: Death Callback with Loot ===" << std::endl;

    ecs::World world;
    systems::CombatSystem combatSys(&world);
    systems::LootSystem lootSys(&world);
    lootSys.setRandomSeed(999);

    std::string wreck_id;
    combatSys.setDeathCallback([&](const std::string& entity_id, float, float, float) {
        wreck_id = lootSys.generateLoot(entity_id);
    });

    auto* npc = world.createEntity("npc_loot");
    auto* hp = addComp<components::Health>(npc);
    hp->shield_hp = 0.0f;
    hp->armor_hp = 0.0f;
    hp->hull_hp = 5.0f;
    hp->hull_max = 100.0f;
    addComp<components::Position>(npc);

    auto* lt = addComp<components::LootTable>(npc);
    components::LootTable::LootEntry entry;
    entry.item_id = "module_afterburner";
    entry.name = "Afterburner I";
    entry.type = "module";
    entry.drop_chance = 1.0f; // always drops
    entry.min_quantity = 1;
    entry.max_quantity = 1;
    entry.volume = 5.0f;
    lt->entries.push_back(entry);
    lt->isc_drop = 5000.0;

    combatSys.applyDamage("npc_loot", 100.0f, "em");

    assertTrue(!wreck_id.empty(), "Loot wreck created on death");
    auto* wreck = world.getEntity(wreck_id);
    assertTrue(wreck != nullptr, "Wreck entity exists");
    auto* wreck_inv = wreck->getComponent<components::Inventory>();
    assertTrue(wreck_inv != nullptr, "Wreck has inventory");
    assertTrue(wreck_inv->items.size() >= 1, "Wreck contains loot");
}

static void testCombatNoCallbackOnSurvival() {
    std::cout << "\n=== Combat: No Callback On Survival ===" << std::endl;

    ecs::World world;
    systems::CombatSystem combatSys(&world);

    bool callback_fired = false;
    combatSys.setDeathCallback([&](const std::string&, float, float, float) {
        callback_fired = true;
    });

    auto* npc = world.createEntity("npc_alive");
    auto* hp = addComp<components::Health>(npc);
    hp->shield_hp = 100.0f;
    hp->armor_hp = 100.0f;
    hp->hull_hp = 100.0f;
    hp->hull_max = 100.0f;

    combatSys.applyDamage("npc_alive", 10.0f, "kinetic");
    assertTrue(!callback_fired, "Death callback does NOT fire when entity survives");
}


// ==================== Combat Loop Integration Tests ====================

static void testCombatFireWeaponAppliesDamage() {
    std::cout << "\n=== Combat Loop: Fire Weapon Applies Damage ===" << std::endl;

    ecs::World world;
    systems::CombatSystem combatSys(&world);

    // Create shooter with weapon
    auto* shooter = world.createEntity("player_1");
    auto* wep = addComp<components::Weapon>(shooter);
    wep->damage = 50.0f;
    wep->damage_type = "kinetic";
    wep->optimal_range = 10000.0f;
    wep->falloff_range = 5000.0f;
    wep->rate_of_fire = 3.0f;
    wep->cooldown = 0.0f;
    wep->ammo_count = 100;
    wep->capacitor_cost = 10.0f;
    auto* spos = addComp<components::Position>(shooter);
    spos->x = 0.0f; spos->y = 0.0f; spos->z = 0.0f;

    // Create target with health
    auto* target = world.createEntity("npc_target");
    auto* hp = addComp<components::Health>(target);
    hp->shield_hp = 100.0f; hp->shield_max = 100.0f;
    hp->armor_hp = 100.0f; hp->armor_max = 100.0f;
    hp->hull_hp = 100.0f; hp->hull_max = 100.0f;
    auto* tpos = addComp<components::Position>(target);
    tpos->x = 5000.0f; tpos->y = 0.0f; tpos->z = 0.0f;

    float shield_before = hp->shield_hp;
    bool fired = combatSys.fireWeapon("player_1", "npc_target");

    assertTrue(fired, "Weapon fired successfully");
    assertTrue(hp->shield_hp < shield_before, "Target shield took damage");
    assertTrue(wep->cooldown > 0.0f, "Weapon now on cooldown");
    assertTrue(wep->ammo_count == 99, "Ammo consumed");
}

static void testCombatFireWeaponCooldownPrevents() {
    std::cout << "\n=== Combat Loop: Cooldown Prevents Firing ===" << std::endl;

    ecs::World world;
    systems::CombatSystem combatSys(&world);

    auto* shooter = world.createEntity("player_cd");
    auto* wep = addComp<components::Weapon>(shooter);
    wep->damage = 50.0f;
    wep->damage_type = "em";
    wep->optimal_range = 10000.0f;
    wep->falloff_range = 5000.0f;
    wep->rate_of_fire = 3.0f;
    wep->cooldown = 2.0f;  // On cooldown
    wep->ammo_count = 100;
    addComp<components::Position>(shooter);

    auto* target = world.createEntity("npc_cd_target");
    auto* hp = addComp<components::Health>(target);
    hp->shield_hp = 100.0f; hp->shield_max = 100.0f;
    hp->armor_hp = 100.0f; hp->armor_max = 100.0f;
    hp->hull_hp = 100.0f; hp->hull_max = 100.0f;
    addComp<components::Position>(target);

    bool fired = combatSys.fireWeapon("player_cd", "npc_cd_target");

    assertTrue(!fired, "Weapon blocked by cooldown");
    assertTrue(hp->shield_hp == 100.0f, "Target took no damage");
}

static void testCombatFireWeaponOutOfRange() {
    std::cout << "\n=== Combat Loop: Out of Range Prevents Firing ===" << std::endl;

    ecs::World world;
    systems::CombatSystem combatSys(&world);

    auto* shooter = world.createEntity("player_range");
    auto* wep = addComp<components::Weapon>(shooter);
    wep->damage = 50.0f;
    wep->damage_type = "thermal";
    wep->optimal_range = 5000.0f;
    wep->falloff_range = 2000.0f;
    wep->rate_of_fire = 3.0f;
    wep->cooldown = 0.0f;
    wep->ammo_count = 100;
    auto* spos = addComp<components::Position>(shooter);
    spos->x = 0.0f; spos->y = 0.0f; spos->z = 0.0f;

    auto* target = world.createEntity("npc_far");
    auto* hp = addComp<components::Health>(target);
    hp->shield_hp = 100.0f; hp->shield_max = 100.0f;
    hp->armor_hp = 100.0f; hp->armor_max = 100.0f;
    hp->hull_hp = 100.0f; hp->hull_max = 100.0f;
    auto* tpos = addComp<components::Position>(target);
    tpos->x = 50000.0f; tpos->y = 0.0f; tpos->z = 0.0f;  // Way beyond range

    bool fired = combatSys.fireWeapon("player_range", "npc_far");

    assertTrue(!fired, "Weapon blocked by range");
    assertTrue(hp->shield_hp == 100.0f, "Target took no damage when out of range");
}

static void testCombatFireWeaponDamageLayerCascade() {
    std::cout << "\n=== Combat Loop: Damage Cascades Shield → Armor → Hull ===" << std::endl;

    ecs::World world;
    systems::CombatSystem combatSys(&world);

    auto* shooter = world.createEntity("player_cascade");
    auto* wep = addComp<components::Weapon>(shooter);
    wep->damage = 200.0f;
    wep->damage_type = "em";
    wep->optimal_range = 20000.0f;
    wep->falloff_range = 5000.0f;
    wep->rate_of_fire = 5.0f;
    wep->cooldown = 0.0f;
    wep->ammo_count = 100;
    auto* spos = addComp<components::Position>(shooter);
    spos->x = 0.0f; spos->y = 0.0f; spos->z = 0.0f;

    auto* target = world.createEntity("npc_cascade");
    auto* hp = addComp<components::Health>(target);
    hp->shield_hp = 50.0f; hp->shield_max = 100.0f;
    hp->armor_hp = 50.0f; hp->armor_max = 100.0f;
    hp->hull_hp = 100.0f; hp->hull_max = 100.0f;
    auto* tpos = addComp<components::Position>(target);
    tpos->x = 1000.0f; tpos->y = 0.0f; tpos->z = 0.0f;

    combatSys.fireWeapon("player_cascade", "npc_cascade");

    assertTrue(hp->shield_hp == 0.0f, "Shield depleted");
    assertTrue(hp->armor_hp == 0.0f, "Armor depleted");
    assertTrue(hp->hull_hp < 100.0f, "Hull took overflow damage");
}


void run_combat_system_tests() {
    testDamageEventOnShieldHit();
    testDamageEventShieldDepleted();
    testDamageEventHullCritical();
    testDamageEventMultipleHits();
    testDamageEventClearOldHits();
    testCombatDeathCallbackFires();
    testCombatDeathCallbackWithLoot();
    testCombatNoCallbackOnSurvival();
    testCombatFireWeaponAppliesDamage();
    testCombatFireWeaponCooldownPrevents();
    testCombatFireWeaponOutOfRange();
    testCombatFireWeaponDamageLayerCascade();
}
