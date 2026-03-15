// Tests for: AccessListSystem
#include "test_log.h"
#include "components/social_components.h"
#include "ecs/system.h"
#include "systems/access_list_system.h"

using namespace atlas;

// ==================== AccessListSystem Tests ====================

static void testAccessListInit() {
    std::cout << "\n=== AccessList: Init ===" << std::endl;
    ecs::World world;
    systems::AccessListSystem sys(&world);
    world.createEntity("e1");
    assertTrue(sys.initialize("e1"), "Init succeeds");
    assertTrue(sys.getEntryCount("e1") == 0, "Zero entries initially");
    assertTrue(sys.getOwnerId("e1") == "", "No owner initially");
    assertTrue(sys.getStructureId("e1") == "", "No structure initially");
    assertTrue(!sys.isDefaultAllow("e1"), "Default policy is Block");
    assertTrue(sys.getTotalEntriesAdded("e1") == 0, "Zero added");
    assertTrue(sys.getTotalEntriesExpired("e1") == 0, "Zero expired");
    assertTrue(sys.getTotalAccessChecks("e1") == 0, "Zero checks");
    assertTrue(!sys.initialize("nonexistent"), "Init fails on missing entity");
}

static void testAccessListAddEntry() {
    std::cout << "\n=== AccessList: AddEntry ===" << std::endl;
    ecs::World world;
    systems::AccessListSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    using P = components::AccessListState::Permission;
    assertTrue(sys.addEntry("e1", "acl1", "player_a", P::Allow),
               "Add allow entry");
    assertTrue(sys.getEntryCount("e1") == 1, "1 entry");
    assertTrue(sys.hasEntry("e1", "acl1"), "Has acl1");
    assertTrue(sys.hasMember("e1", "player_a"), "Has player_a");
    assertTrue(sys.getTotalEntriesAdded("e1") == 1, "1 added");

    assertTrue(sys.addEntry("e1", "acl2", "player_b", P::Block, 60.0f),
               "Add block entry with TTL");
    assertTrue(sys.getEntryCount("e1") == 2, "2 entries");
    assertTrue(approxEqual(sys.getEntryTtl("e1", "acl2"), 60.0f), "TTL is 60");
}

static void testAccessListAddEntryValidation() {
    std::cout << "\n=== AccessList: AddEntryValidation ===" << std::endl;
    ecs::World world;
    systems::AccessListSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    using P = components::AccessListState::Permission;
    assertTrue(!sys.addEntry("e1", "", "player_a", P::Allow),
               "Empty entry_id rejected");
    assertTrue(!sys.addEntry("e1", "acl1", "", P::Allow),
               "Empty member_id rejected");
    assertTrue(!sys.addEntry("e1", "acl1", "player_a", P::Allow, -1.0f),
               "Negative TTL rejected");

    assertTrue(sys.addEntry("e1", "acl1", "player_a", P::Allow),
               "Valid entry succeeds");
    assertTrue(!sys.addEntry("e1", "acl1", "player_b", P::Block),
               "Duplicate entry_id rejected");
    assertTrue(!sys.addEntry("missing", "acl9", "player_a", P::Allow),
               "Missing entity rejected");
}

static void testAccessListCapacity() {
    std::cout << "\n=== AccessList: Capacity ===" << std::endl;
    ecs::World world;
    systems::AccessListSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.setMaxEntries("e1", 3);

    using P = components::AccessListState::Permission;
    assertTrue(sys.addEntry("e1", "a1", "p1", P::Allow), "Add 1");
    assertTrue(sys.addEntry("e1", "a2", "p2", P::Allow), "Add 2");
    assertTrue(sys.addEntry("e1", "a3", "p3", P::Allow), "Add 3");
    assertTrue(!sys.addEntry("e1", "a4", "p4", P::Allow),
               "Add 4 rejected at capacity");
    assertTrue(sys.getEntryCount("e1") == 3, "Still 3");
}

static void testAccessListCheckAccess() {
    std::cout << "\n=== AccessList: CheckAccess ===" << std::endl;
    ecs::World world;
    systems::AccessListSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    using P = components::AccessListState::Permission;
    // Default policy is Block
    assertTrue(!sys.checkAccess("e1", "stranger"), "Stranger blocked by default");
    assertTrue(sys.getTotalAccessChecks("e1") == 1, "1 check");

    // Add allow entry
    sys.addEntry("e1", "acl1", "friend", P::Allow);
    assertTrue(sys.checkAccess("e1", "friend"), "Friend allowed");
    assertTrue(!sys.checkAccess("e1", "stranger"), "Stranger still blocked");
    assertTrue(sys.getTotalAccessChecks("e1") == 3, "3 checks");

    // Add block entry
    sys.addEntry("e1", "acl2", "enemy", P::Block);
    assertTrue(!sys.checkAccess("e1", "enemy"), "Enemy blocked explicitly");
}

static void testAccessListOwnerAlwaysAllowed() {
    std::cout << "\n=== AccessList: OwnerAlwaysAllowed ===" << std::endl;
    ecs::World world;
    systems::AccessListSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    using P = components::AccessListState::Permission;
    sys.setOwnerId("e1", "owner_01");

    // Owner allowed even with Block default policy
    assertTrue(sys.checkAccess("e1", "owner_01"), "Owner always allowed");

    // Add explicit block for owner — owner still gets in
    sys.addEntry("e1", "acl_block_owner", "owner_01", P::Block);
    assertTrue(sys.checkAccess("e1", "owner_01"), "Owner bypasses block entry");

    // Non-owner still blocked
    assertTrue(!sys.checkAccess("e1", "intruder"), "Non-owner blocked");
}

static void testAccessListDefaultPolicyAllow() {
    std::cout << "\n=== AccessList: DefaultPolicyAllow ===" << std::endl;
    ecs::World world;
    systems::AccessListSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    using P = components::AccessListState::Permission;
    sys.setDefaultPolicy("e1", P::Allow);
    assertTrue(sys.isDefaultAllow("e1"), "Default is now Allow");
    assertTrue(sys.checkAccess("e1", "anyone"), "Anyone allowed");

    // Explicit block overrides default allow
    sys.addEntry("e1", "acl1", "banned", P::Block);
    assertTrue(!sys.checkAccess("e1", "banned"), "Banned member blocked");
    assertTrue(sys.checkAccess("e1", "anyone_else"), "Others still allowed");
}

static void testAccessListSetPermission() {
    std::cout << "\n=== AccessList: SetPermission ===" << std::endl;
    ecs::World world;
    systems::AccessListSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    using P = components::AccessListState::Permission;
    sys.addEntry("e1", "acl1", "player_a", P::Allow);
    assertTrue(sys.checkAccess("e1", "player_a"), "Initially allowed");

    assertTrue(sys.setPermission("e1", "acl1", P::Block), "Change to Block");
    assertTrue(!sys.checkAccess("e1", "player_a"), "Now blocked");

    assertTrue(sys.setPermission("e1", "acl1", P::Allow), "Change back to Allow");
    assertTrue(sys.checkAccess("e1", "player_a"), "Allowed again");

    assertTrue(!sys.setPermission("e1", "nonexistent", P::Block),
               "Unknown entry fails");
}

static void testAccessListRemoveEntry() {
    std::cout << "\n=== AccessList: RemoveEntry ===" << std::endl;
    ecs::World world;
    systems::AccessListSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    using P = components::AccessListState::Permission;
    sys.addEntry("e1", "acl1", "player_a", P::Allow);
    sys.addEntry("e1", "acl2", "player_b", P::Block);

    assertTrue(sys.removeEntry("e1", "acl1"), "Remove acl1");
    assertTrue(sys.getEntryCount("e1") == 1, "1 left");
    assertTrue(!sys.hasEntry("e1", "acl1"), "acl1 gone");
    assertTrue(sys.hasEntry("e1", "acl2"), "acl2 present");
    assertTrue(!sys.removeEntry("e1", "acl1"), "Remove already removed fails");
    assertTrue(!sys.removeEntry("e1", "unknown"), "Remove unknown fails");
}

static void testAccessListClearEntries() {
    std::cout << "\n=== AccessList: ClearEntries ===" << std::endl;
    ecs::World world;
    systems::AccessListSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    using P = components::AccessListState::Permission;
    sys.addEntry("e1", "acl1", "p1", P::Allow);
    sys.addEntry("e1", "acl2", "p2", P::Block);

    assertTrue(sys.clearEntries("e1"), "ClearEntries succeeds");
    assertTrue(sys.getEntryCount("e1") == 0, "0 after clear");
}

static void testAccessListTtlExpiry() {
    std::cout << "\n=== AccessList: TtlExpiry ===" << std::endl;
    ecs::World world;
    systems::AccessListSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    using P = components::AccessListState::Permission;
    // Permanent entry (TTL = 0)
    sys.addEntry("e1", "perm", "player_a", P::Allow, 0.0f);
    // Temporary entry (TTL = 10s)
    sys.addEntry("e1", "temp", "player_b", P::Allow, 10.0f);

    assertTrue(sys.getEntryCount("e1") == 2, "2 entries");

    sys.update(5.0f);
    assertTrue(sys.getEntryCount("e1") == 2, "Still 2 after 5s");
    assertTrue(approxEqual(sys.getEntryTtl("e1", "temp"), 5.0f), "TTL decreased to 5");
    assertTrue(approxEqual(sys.getEntryTtl("e1", "perm"), 0.0f), "Permanent TTL unchanged");

    sys.update(6.0f);
    assertTrue(sys.getEntryCount("e1") == 1, "1 after 11s (temp expired)");
    assertTrue(sys.hasEntry("e1", "perm"), "Permanent remains");
    assertTrue(!sys.hasEntry("e1", "temp"), "Temp expired");
    assertTrue(sys.getTotalEntriesExpired("e1") == 1, "1 expired");
}

static void testAccessListConfiguration() {
    std::cout << "\n=== AccessList: Configuration ===" << std::endl;
    ecs::World world;
    systems::AccessListSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    assertTrue(sys.setOwnerId("e1", "owner_01"), "Set owner");
    assertTrue(sys.getOwnerId("e1") == "owner_01", "Owner matches");

    assertTrue(sys.setStructureId("e1", "citadel_x"), "Set structure");
    assertTrue(sys.getStructureId("e1") == "citadel_x", "Structure matches");

    assertTrue(sys.setMaxEntries("e1", 100), "Set max entries");
    assertTrue(!sys.setMaxEntries("e1", 0), "Zero max rejected");
    assertTrue(!sys.setMaxEntries("e1", -1), "Negative max rejected");
}

static void testAccessListMissing() {
    std::cout << "\n=== AccessList: Missing ===" << std::endl;
    ecs::World world;
    systems::AccessListSystem sys(&world);

    using P = components::AccessListState::Permission;
    assertTrue(!sys.addEntry("none", "a1", "p1", P::Allow),
               "Add fails on missing");
    assertTrue(!sys.removeEntry("none", "a1"), "Remove fails on missing");
    assertTrue(!sys.clearEntries("none"), "Clear fails on missing");
    assertTrue(!sys.checkAccess("none", "p1"), "CheckAccess fails on missing");
    assertTrue(!sys.setPermission("none", "a1", P::Block),
               "SetPermission fails on missing");
    assertTrue(!sys.setOwnerId("none", "o1"), "SetOwner fails on missing");
    assertTrue(!sys.setStructureId("none", "s1"), "SetStructure fails on missing");
    assertTrue(!sys.setDefaultPolicy("none", P::Allow),
               "SetDefaultPolicy fails on missing");
    assertTrue(!sys.setMaxEntries("none", 10), "SetMax fails on missing");
    assertTrue(sys.getEntryCount("none") == 0, "0 count on missing");
    assertTrue(!sys.hasEntry("none", "a1"), "No entry on missing");
    assertTrue(!sys.hasMember("none", "p1"), "No member on missing");
    assertTrue(sys.getOwnerId("none") == "", "Empty owner on missing");
    assertTrue(sys.getStructureId("none") == "", "Empty structure on missing");
    assertTrue(sys.getTotalEntriesAdded("none") == 0, "0 added on missing");
    assertTrue(sys.getTotalEntriesExpired("none") == 0, "0 expired on missing");
    assertTrue(sys.getTotalAccessChecks("none") == 0, "0 checks on missing");
    assertTrue(approxEqual(sys.getEntryTtl("none", "a1"), 0.0f), "0 TTL on missing");
    assertTrue(!sys.isDefaultAllow("none"), "Not default-allow on missing");
}

void run_access_list_system_tests() {
    testAccessListInit();
    testAccessListAddEntry();
    testAccessListAddEntryValidation();
    testAccessListCapacity();
    testAccessListCheckAccess();
    testAccessListOwnerAlwaysAllowed();
    testAccessListDefaultPolicyAllow();
    testAccessListSetPermission();
    testAccessListRemoveEntry();
    testAccessListClearEntries();
    testAccessListTtlExpiry();
    testAccessListConfiguration();
    testAccessListMissing();
}
