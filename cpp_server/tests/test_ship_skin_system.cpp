// Tests for: ShipSkinSystem
#include "test_log.h"
#include "components/game_components.h"
#include "ecs/system.h"
#include "systems/ship_skin_system.h"

using namespace atlas;

// ==================== ShipSkinSystem Tests ====================

static void testShipSkinInit() {
    std::cout << "\n=== ShipSkin: Init ===" << std::endl;
    ecs::World world;
    systems::ShipSkinSystem sys(&world);
    world.createEntity("p1");
    assertTrue(sys.initialize("p1", "player_alice"), "Init succeeds");
    assertTrue(sys.getSkinCount("p1") == 0, "Zero skins initially");
    assertTrue(sys.getEquippedSkinId("p1").empty(), "No skin equipped initially");
    assertTrue(sys.getTotalAcquired("p1") == 0, "Zero total acquired initially");
}

static void testShipSkinInitFails() {
    std::cout << "\n=== ShipSkin: InitFails ===" << std::endl;
    ecs::World world;
    systems::ShipSkinSystem sys(&world);
    assertTrue(!sys.initialize("nonexistent", "owner"), "Init fails on missing entity");
    world.createEntity("p1");
    assertTrue(!sys.initialize("p1", ""), "Init fails with empty owner_id");
}

static void testShipSkinAddSkin() {
    std::cout << "\n=== ShipSkin: AddSkin ===" << std::endl;
    ecs::World world;
    systems::ShipSkinSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1", "alice");

    assertTrue(sys.addSkin("p1", "skin_1", "Crimson Rifter", "Rifter",
               components::ShipSkinCollection::Rarity::Rare, "#FF0000", "#000000"),
               "Add first skin");
    assertTrue(sys.addSkin("p1", "skin_2", "Azure Raven", "Raven",
               components::ShipSkinCollection::Rarity::Epic, "#0000FF", "#FFFFFF"),
               "Add second skin");
    assertTrue(sys.getSkinCount("p1") == 2, "Two skins stored");
    assertTrue(sys.getTotalAcquired("p1") == 2, "Total acquired = 2");
    assertTrue(!sys.addSkin("p1", "skin_1", "Duplicate", "Rifter",
               components::ShipSkinCollection::Rarity::Common, "#000", "#000"),
               "Duplicate skin_id rejected");
    assertTrue(!sys.addSkin("p1", "", "Empty ID", "Rifter",
               components::ShipSkinCollection::Rarity::Common, "#000", "#000"),
               "Empty skin_id rejected");
    assertTrue(!sys.addSkin("p1", "skin_3", "", "Rifter",
               components::ShipSkinCollection::Rarity::Common, "#000", "#000"),
               "Empty name rejected");
    assertTrue(sys.getSkinCount("p1") == 2, "Count unchanged after rejections");
}

static void testShipSkinRemoveSkin() {
    std::cout << "\n=== ShipSkin: RemoveSkin ===" << std::endl;
    ecs::World world;
    systems::ShipSkinSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1", "alice");
    sys.addSkin("p1", "skin_1", "Crimson Rifter", "Rifter",
                components::ShipSkinCollection::Rarity::Rare, "#FF0000", "#000000");
    sys.addSkin("p1", "skin_2", "Azure Raven", "Raven",
                components::ShipSkinCollection::Rarity::Epic, "#0000FF", "#FFFFFF");

    assertTrue(sys.removeSkin("p1", "skin_1"), "Remove existing skin");
    assertTrue(sys.getSkinCount("p1") == 1, "Count decremented");
    assertTrue(!sys.hasSkin("p1", "skin_1"), "Removed skin no longer found");
    assertTrue(!sys.removeSkin("p1", "skin_1"), "Remove nonexistent fails");
    assertTrue(!sys.removeSkin("p1", "nonexistent"), "Remove unknown fails");
    // total_acquired stays 2 (lifetime counter)
    assertTrue(sys.getTotalAcquired("p1") == 2, "Total acquired unchanged after removal");
}

static void testShipSkinEquipSkin() {
    std::cout << "\n=== ShipSkin: EquipSkin ===" << std::endl;
    ecs::World world;
    systems::ShipSkinSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1", "alice");
    sys.addSkin("p1", "skin_1", "Crimson Rifter", "Rifter",
                components::ShipSkinCollection::Rarity::Rare, "#FF0000", "#000000");
    sys.addSkin("p1", "skin_2", "Azure Raven", "Raven",
                components::ShipSkinCollection::Rarity::Epic, "#0000FF", "#FFFFFF");

    assertTrue(sys.equipSkin("p1", "skin_1"), "Equip skin_1 succeeds");
    assertTrue(sys.getEquippedSkinId("p1") == "skin_1", "skin_1 is equipped");
    assertTrue(sys.equipSkin("p1", "skin_2"), "Equip skin_2 replaces skin_1");
    assertTrue(sys.getEquippedSkinId("p1") == "skin_2", "skin_2 is now equipped");
    assertTrue(!sys.equipSkin("p1", "nonexistent"), "Cannot equip nonexistent skin");
    assertTrue(sys.getEquippedSkinId("p1") == "skin_2", "Still skin_2 after failed equip");
}

static void testShipSkinUnequipSkin() {
    std::cout << "\n=== ShipSkin: UnequipSkin ===" << std::endl;
    ecs::World world;
    systems::ShipSkinSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1", "alice");
    sys.addSkin("p1", "skin_1", "Crimson Rifter", "Rifter",
                components::ShipSkinCollection::Rarity::Rare, "#FF0000", "#000000");

    assertTrue(!sys.unequipSkin("p1"), "Unequip returns false when none equipped");
    sys.equipSkin("p1", "skin_1");
    assertTrue(sys.unequipSkin("p1"), "Unequip returns true when skin was equipped");
    assertTrue(sys.getEquippedSkinId("p1").empty(), "No skin equipped after unequip");
    assertTrue(!sys.unequipSkin("p1"), "Unequip again returns false");
}

static void testShipSkinRarityCount() {
    std::cout << "\n=== ShipSkin: RarityCount ===" << std::endl;
    ecs::World world;
    systems::ShipSkinSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1", "alice");

    sys.addSkin("p1", "s1", "Skin A", "Rifter",
                components::ShipSkinCollection::Rarity::Common, "#FFF", "#000");
    sys.addSkin("p1", "s2", "Skin B", "Rifter",
                components::ShipSkinCollection::Rarity::Common, "#FFF", "#000");
    sys.addSkin("p1", "s3", "Skin C", "Raven",
                components::ShipSkinCollection::Rarity::Legendary, "#FFD700", "#000");

    assertTrue(sys.getSkinCountByRarity("p1", components::ShipSkinCollection::Rarity::Common) == 2,
               "Two Common skins");
    assertTrue(sys.getSkinCountByRarity("p1", components::ShipSkinCollection::Rarity::Legendary) == 1,
               "One Legendary skin");
    assertTrue(sys.getSkinCountByRarity("p1", components::ShipSkinCollection::Rarity::Epic) == 0,
               "Zero Epic skins");
}

static void testShipSkinHasSkin() {
    std::cout << "\n=== ShipSkin: HasSkin ===" << std::endl;
    ecs::World world;
    systems::ShipSkinSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1", "alice");
    sys.addSkin("p1", "skin_1", "Crimson Rifter", "Rifter",
                components::ShipSkinCollection::Rarity::Rare, "#FF0000", "#000000");

    assertTrue(sys.hasSkin("p1", "skin_1"), "Has skin_1");
    assertTrue(!sys.hasSkin("p1", "nonexistent"), "Does not have nonexistent");
    assertTrue(!sys.hasSkin("nonexistent", "skin_1"), "Missing entity returns false");
}

static void testShipSkinMaxCapacity() {
    std::cout << "\n=== ShipSkin: MaxCapacity ===" << std::endl;
    ecs::World world;
    systems::ShipSkinSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1", "alice");

    // Set a small max for testing
    auto* comp = world.getEntity("p1")->getComponent<components::ShipSkinCollection>();
    comp->max_skins = 3;

    sys.addSkin("p1", "s1", "Skin1", "", components::ShipSkinCollection::Rarity::Common, "#F", "#0");
    sys.addSkin("p1", "s2", "Skin2", "", components::ShipSkinCollection::Rarity::Common, "#F", "#0");
    sys.addSkin("p1", "s3", "Skin3", "", components::ShipSkinCollection::Rarity::Common, "#F", "#0");
    assertTrue(sys.getSkinCount("p1") == 3, "Three skins at capacity");
    assertTrue(!sys.addSkin("p1", "s4", "Skin4", "", components::ShipSkinCollection::Rarity::Common, "#F", "#0"),
               "Fourth skin rejected at capacity");
    assertTrue(sys.getSkinCount("p1") == 3, "Count still 3");
}

static void testShipSkinRemoveEquipped() {
    std::cout << "\n=== ShipSkin: RemoveEquipped ===" << std::endl;
    ecs::World world;
    systems::ShipSkinSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1", "alice");
    sys.addSkin("p1", "skin_1", "Crimson Rifter", "Rifter",
                components::ShipSkinCollection::Rarity::Rare, "#FF0000", "#000000");
    sys.equipSkin("p1", "skin_1");
    assertTrue(sys.getEquippedSkinId("p1") == "skin_1", "skin_1 equipped");

    assertTrue(sys.removeSkin("p1", "skin_1"), "Remove equipped skin succeeds");
    assertTrue(sys.getEquippedSkinId("p1").empty(), "No skin equipped after removal");
    assertTrue(sys.getSkinCount("p1") == 0, "Zero skins after removal");
}

static void testShipSkinMissing() {
    std::cout << "\n=== ShipSkin: Missing ===" << std::endl;
    ecs::World world;
    systems::ShipSkinSystem sys(&world);

    assertTrue(!sys.addSkin("nonexistent", "s", "S", "",
               components::ShipSkinCollection::Rarity::Common, "#", "#"),
               "AddSkin fails on missing");
    assertTrue(!sys.removeSkin("nonexistent", "s"), "RemoveSkin fails on missing");
    assertTrue(!sys.equipSkin("nonexistent", "s"), "EquipSkin fails on missing");
    assertTrue(!sys.unequipSkin("nonexistent"), "UnequipSkin fails on missing");
    assertTrue(sys.getSkinCount("nonexistent") == 0, "Zero skins on missing");
    assertTrue(sys.getEquippedSkinId("nonexistent").empty(), "Empty equipped on missing");
    assertTrue(sys.getSkinCountByRarity("nonexistent",
               components::ShipSkinCollection::Rarity::Common) == 0,
               "Zero rarity count on missing");
    assertTrue(sys.getTotalAcquired("nonexistent") == 0, "Zero acquired on missing");
    assertTrue(!sys.hasSkin("nonexistent", "s"), "HasSkin false on missing");
}

void run_ship_skin_system_tests() {
    testShipSkinInit();
    testShipSkinInitFails();
    testShipSkinAddSkin();
    testShipSkinRemoveSkin();
    testShipSkinEquipSkin();
    testShipSkinUnequipSkin();
    testShipSkinRarityCount();
    testShipSkinHasSkin();
    testShipSkinMaxCapacity();
    testShipSkinRemoveEquipped();
    testShipSkinMissing();
}
