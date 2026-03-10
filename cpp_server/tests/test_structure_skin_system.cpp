// Tests for: StructureSkinSystem
#include "test_log.h"
#include "components/game_components.h"
#include "ecs/system.h"
#include "systems/structure_skin_system.h"

using namespace atlas;
using ST  = components::StructureSkinCollection::StructureType;
using Rar = components::StructureSkinCollection::Rarity;

// ==================== StructureSkinSystem Tests ====================

static void testSkinInit() {
    std::cout << "\n=== StructureSkin: Init ===" << std::endl;
    ecs::World world;
    systems::StructureSkinSystem sys(&world);
    world.createEntity("corp1");
    assertTrue(sys.initialize("corp1", "owner1"), "Init succeeds");
    assertTrue(sys.getSkinCount("corp1") == 0, "Zero skins initially");
    assertTrue(sys.getAppliedSkinId("corp1").empty(), "No skin applied initially");
    assertTrue(sys.getTotalAcquired("corp1") == 0, "Zero acquired initially");
}

static void testSkinInitFails() {
    std::cout << "\n=== StructureSkin: InitFails ===" << std::endl;
    ecs::World world;
    systems::StructureSkinSystem sys(&world);
    assertTrue(!sys.initialize("nonexistent", "owner1"), "Fails on missing entity");
    world.createEntity("corp1");
    assertTrue(!sys.initialize("corp1", ""), "Fails with empty owner_id");
}

static void testSkinAdd() {
    std::cout << "\n=== StructureSkin: Add ===" << std::endl;
    ecs::World world;
    systems::StructureSkinSystem sys(&world);
    world.createEntity("corp1");
    sys.initialize("corp1", "owner1");

    assertTrue(sys.addSkin("corp1", "s1", "Ember Skin", ST::Astrahus,
               Rar::Common, "#FF4400", "#220000"), "Add Astrahus skin");
    assertTrue(sys.addSkin("corp1", "s2", "Glacial Skin", ST::Fortizar,
               Rar::Rare, "#0088FF", "#002244"), "Add Fortizar skin");
    assertTrue(sys.getSkinCount("corp1") == 2, "Two skins stored");
    assertTrue(sys.getTotalAcquired("corp1") == 2, "Two acquired counted");

    assertTrue(!sys.addSkin("corp1", "s1", "Dup", ST::Astrahus,
               Rar::Common, "", ""), "Duplicate skin_id rejected");
    assertTrue(!sys.addSkin("corp1", "", "Empty ID", ST::Astrahus,
               Rar::Common, "", ""), "Empty skin_id rejected");
    assertTrue(!sys.addSkin("corp1", "s3", "", ST::Astrahus,
               Rar::Common, "", ""), "Empty name rejected");
    assertTrue(sys.getSkinCount("corp1") == 2, "Count unchanged after rejections");
}

static void testSkinRemove() {
    std::cout << "\n=== StructureSkin: Remove ===" << std::endl;
    ecs::World world;
    systems::StructureSkinSystem sys(&world);
    world.createEntity("corp1");
    sys.initialize("corp1", "owner1");
    sys.addSkin("corp1", "s1", "Ember Skin", ST::Astrahus, Rar::Common, "#F00", "#200");
    sys.addSkin("corp1", "s2", "Glacial Skin", ST::Fortizar, Rar::Rare, "#00F", "#002");

    assertTrue(sys.removeSkin("corp1", "s1"), "Remove existing skin");
    assertTrue(sys.getSkinCount("corp1") == 1, "Count decremented");
    assertTrue(!sys.hasSkin("corp1", "s1"), "Removed skin gone");
    assertTrue(!sys.removeSkin("corp1", "s1"), "Remove nonexistent fails");
    assertTrue(!sys.removeSkin("corp1", "nonexistent"), "Remove unknown fails");
}

static void testSkinApply() {
    std::cout << "\n=== StructureSkin: Apply ===" << std::endl;
    ecs::World world;
    systems::StructureSkinSystem sys(&world);
    world.createEntity("corp1");
    sys.initialize("corp1", "owner1");
    sys.addSkin("corp1", "s1", "Ember Skin", ST::Astrahus, Rar::Common, "#F00", "#200");
    sys.addSkin("corp1", "s2", "Glacial Skin", ST::Fortizar, Rar::Rare, "#00F", "#002");

    assertTrue(sys.getAppliedSkinId("corp1").empty(), "No skin applied initially");
    assertTrue(sys.applySkin("corp1", "s1"), "Apply s1");
    assertTrue(sys.getAppliedSkinId("corp1") == "s1", "s1 is applied");

    // Applying a second skin unequips the first
    assertTrue(sys.applySkin("corp1", "s2"), "Apply s2 switches from s1");
    assertTrue(sys.getAppliedSkinId("corp1") == "s2", "s2 is now applied");

    assertTrue(!sys.applySkin("corp1", "nonexistent"), "Apply unknown skin fails");
    assertTrue(sys.getAppliedSkinId("corp1") == "s2", "s2 still applied after failed switch");
}

static void testSkinUnapply() {
    std::cout << "\n=== StructureSkin: Unapply ===" << std::endl;
    ecs::World world;
    systems::StructureSkinSystem sys(&world);
    world.createEntity("corp1");
    sys.initialize("corp1", "owner1");
    sys.addSkin("corp1", "s1", "Ember Skin", ST::Astrahus, Rar::Common, "#F00", "#200");
    sys.applySkin("corp1", "s1");

    assertTrue(sys.getAppliedSkinId("corp1") == "s1", "s1 applied");
    assertTrue(sys.unapplySkin("corp1"), "Unapply succeeds when skin applied");
    assertTrue(sys.getAppliedSkinId("corp1").empty(), "No skin applied after unapply");
    assertTrue(!sys.unapplySkin("corp1"), "Unapply returns false when nothing applied");
}

static void testSkinCountByType() {
    std::cout << "\n=== StructureSkin: CountByType ===" << std::endl;
    ecs::World world;
    systems::StructureSkinSystem sys(&world);
    world.createEntity("corp1");
    sys.initialize("corp1", "owner1");
    sys.addSkin("corp1", "s1", "A1", ST::Astrahus, Rar::Common, "", "");
    sys.addSkin("corp1", "s2", "A2", ST::Astrahus, Rar::Uncommon, "", "");
    sys.addSkin("corp1", "s3", "F1", ST::Fortizar, Rar::Rare, "", "");
    sys.addSkin("corp1", "s4", "K1", ST::Keepstar, Rar::Legendary, "", "");

    assertTrue(sys.getSkinCountByType("corp1", ST::Astrahus) == 2, "2 Astrahus skins");
    assertTrue(sys.getSkinCountByType("corp1", ST::Fortizar) == 1, "1 Fortizar skin");
    assertTrue(sys.getSkinCountByType("corp1", ST::Keepstar) == 1, "1 Keepstar skin");
    assertTrue(sys.getSkinCountByType("corp1", ST::Athanor) == 0, "0 Athanor skins");
}

static void testSkinMaxCapacity() {
    std::cout << "\n=== StructureSkin: MaxCapacity ===" << std::endl;
    ecs::World world;
    systems::StructureSkinSystem sys(&world);
    world.createEntity("corp1");
    sys.initialize("corp1", "owner1");
    auto* comp = world.getEntity("corp1")
                     ->getComponent<components::StructureSkinCollection>();
    comp->max_skins = 3;

    sys.addSkin("corp1", "s1", "A", ST::Astrahus, Rar::Common, "", "");
    sys.addSkin("corp1", "s2", "B", ST::Fortizar, Rar::Common, "", "");
    sys.addSkin("corp1", "s3", "C", ST::Keepstar, Rar::Common, "", "");
    assertTrue(sys.getSkinCount("corp1") == 3, "At capacity");
    assertTrue(!sys.addSkin("corp1", "s4", "D", ST::Athanor, Rar::Common, "", ""),
               "Fourth skin rejected at capacity");
    assertTrue(sys.getSkinCount("corp1") == 3, "Count still 3");
}

static void testSkinTotalAcquired() {
    std::cout << "\n=== StructureSkin: TotalAcquired ===" << std::endl;
    ecs::World world;
    systems::StructureSkinSystem sys(&world);
    world.createEntity("corp1");
    sys.initialize("corp1", "owner1");
    sys.addSkin("corp1", "s1", "A", ST::Astrahus, Rar::Common, "", "");
    sys.addSkin("corp1", "s2", "B", ST::Fortizar, Rar::Common, "", "");
    assertTrue(sys.getTotalAcquired("corp1") == 2, "Two acquired");
    sys.removeSkin("corp1", "s1");
    assertTrue(sys.getTotalAcquired("corp1") == 2, "Acquired unchanged after remove");
}

static void testSkinHas() {
    std::cout << "\n=== StructureSkin: Has ===" << std::endl;
    ecs::World world;
    systems::StructureSkinSystem sys(&world);
    world.createEntity("corp1");
    sys.initialize("corp1", "owner1");
    sys.addSkin("corp1", "s1", "Ember Skin", ST::Astrahus, Rar::Common, "#F00", "#200");

    assertTrue(sys.hasSkin("corp1", "s1"), "Has s1");
    assertTrue(!sys.hasSkin("corp1", "nonexistent"), "Does not have nonexistent");
    assertTrue(!sys.hasSkin("nonexistent", "s1"), "Missing entity returns false");
}

static void testSkinMissing() {
    std::cout << "\n=== StructureSkin: Missing ===" << std::endl;
    ecs::World world;
    systems::StructureSkinSystem sys(&world);

    assertTrue(!sys.addSkin("nonexistent", "s1", "A", ST::Astrahus,
               Rar::Common, "", ""), "AddSkin fails on missing");
    assertTrue(!sys.removeSkin("nonexistent", "s1"), "RemoveSkin fails on missing");
    assertTrue(!sys.applySkin("nonexistent", "s1"), "Apply fails on missing");
    assertTrue(!sys.unapplySkin("nonexistent"), "Unapply fails on missing");
    assertTrue(sys.getSkinCount("nonexistent") == 0, "Zero skins on missing");
    assertTrue(sys.getAppliedSkinId("nonexistent").empty(), "Empty applied on missing");
    assertTrue(sys.getSkinCountByType("nonexistent", ST::Astrahus) == 0,
               "Zero by type on missing");
    assertTrue(sys.getTotalAcquired("nonexistent") == 0, "Zero acquired on missing");
    assertTrue(!sys.hasSkin("nonexistent", "s1"), "HasSkin false on missing");
}

void run_structure_skin_system_tests() {
    testSkinInit();
    testSkinInitFails();
    testSkinAdd();
    testSkinRemove();
    testSkinApply();
    testSkinUnapply();
    testSkinCountByType();
    testSkinMaxCapacity();
    testSkinTotalAcquired();
    testSkinHas();
    testSkinMissing();
}
