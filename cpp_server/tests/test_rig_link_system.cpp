// Tests for: RigLinkSystem
#include "test_log.h"
#include "components/fps_components.h"
#include "ecs/system.h"
#include "systems/rig_link_system.h"

using namespace atlas;

// ==================== RigLinkSystem Tests ====================

static void testRigLinkInit() {
    std::cout << "\n=== RigLink: Init ===" << std::endl;
    ecs::World world;
    systems::RigLinkSystem sys(&world);
    world.createEntity("e1");
    assertTrue(sys.initialize("e1"), "Init succeeds");
    assertTrue(!sys.is_linked("e1"), "Not linked initially");
    assertTrue(sys.get_linked_ship("e1") == "", "No linked ship");
    assertTrue(sys.get_linked_port("e1") == "", "No linked port");
    assertTrue(sys.get_stat_count("e1") == 0, "No stats");
    assertTrue(sys.get_interface_level("e1") == 1, "Default interface level 1");
    assertTrue(approxEqual(sys.get_link_quality("e1"), 1.0f), "Default quality 1.0");
    assertTrue(sys.get_total_links("e1") == 0, "Zero links");
    assertTrue(!sys.initialize("nonexistent"), "Init fails on missing entity");
}

static void testRigLinkToShip() {
    std::cout << "\n=== RigLink: LinkToShip ===" << std::endl;
    ecs::World world;
    systems::RigLinkSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    assertTrue(sys.link_to_ship("e1", "ship_42", "port_a"), "Link succeeds");
    assertTrue(sys.is_linked("e1"), "Now linked");
    assertTrue(sys.get_linked_ship("e1") == "ship_42", "Ship is ship_42");
    assertTrue(sys.get_linked_port("e1") == "port_a", "Port is port_a");
    assertTrue(sys.get_total_links("e1") == 1, "One link");

    // Can't double-link
    assertTrue(!sys.link_to_ship("e1", "ship_99", "port_b"), "Double-link rejected");
    assertTrue(sys.get_linked_ship("e1") == "ship_42", "Still ship_42");
}

static void testRigLinkUnlink() {
    std::cout << "\n=== RigLink: Unlink ===" << std::endl;
    ecs::World world;
    systems::RigLinkSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    assertTrue(!sys.unlink("e1"), "Unlink when not linked fails");
    sys.link_to_ship("e1", "ship_42", "port_a");
    assertTrue(sys.unlink("e1"), "Unlink succeeds");
    assertTrue(!sys.is_linked("e1"), "No longer linked");
    assertTrue(sys.get_linked_ship("e1") == "", "Ship cleared");
    assertTrue(sys.get_linked_port("e1") == "", "Port cleared");

    // Re-link
    assertTrue(sys.link_to_ship("e1", "ship_99", "port_b"), "Re-link succeeds");
    assertTrue(sys.get_total_links("e1") == 2, "Two total links");
}

static void testRigLinkStats() {
    std::cout << "\n=== RigLink: Stats ===" << std::endl;
    ecs::World world;
    systems::RigLinkSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    assertTrue(sys.add_stat("e1", "speed", 100.0f), "Add speed stat");
    assertTrue(sys.add_stat("e1", "armor", 500.0f), "Add armor stat");
    assertTrue(sys.get_stat_count("e1") == 2, "Two stats");
    assertTrue(approxEqual(sys.get_stat_value("e1", "speed"), 100.0f), "Speed is 100");
    assertTrue(approxEqual(sys.get_stat_value("e1", "armor"), 500.0f), "Armor is 500");

    // Duplicate
    assertTrue(!sys.add_stat("e1", "speed", 200.0f), "Duplicate stat rejected");
    // Empty name
    assertTrue(!sys.add_stat("e1", "", 100.0f), "Empty name rejected");
}

static void testRigLinkStatBonus() {
    std::cout << "\n=== RigLink: StatBonus ===" << std::endl;
    ecs::World world;
    systems::RigLinkSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.add_stat("e1", "speed", 100.0f);

    assertTrue(sys.set_stat_bonus("e1", "speed", 25.0f), "Set bonus");
    assertTrue(approxEqual(sys.get_stat_value("e1", "speed"), 125.0f), "Speed = 100+25");

    assertTrue(sys.set_stat_bonus("e1", "speed", -10.0f), "Set negative bonus");
    assertTrue(approxEqual(sys.get_stat_value("e1", "speed"), 90.0f), "Speed = 100-10");

    assertTrue(!sys.set_stat_bonus("e1", "ghost", 5.0f), "Bonus on nonexistent stat fails");
}

static void testRigLinkRemoveStat() {
    std::cout << "\n=== RigLink: RemoveStat ===" << std::endl;
    ecs::World world;
    systems::RigLinkSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.add_stat("e1", "speed", 100.0f);

    assertTrue(sys.remove_stat("e1", "speed"), "Remove speed");
    assertTrue(sys.get_stat_count("e1") == 0, "Zero stats");
    assertTrue(approxEqual(sys.get_stat_value("e1", "speed"), 0.0f), "Removed stat returns 0");
    assertTrue(!sys.remove_stat("e1", "speed"), "Remove again fails");
}

static void testRigLinkClearStats() {
    std::cout << "\n=== RigLink: ClearStats ===" << std::endl;
    ecs::World world;
    systems::RigLinkSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.add_stat("e1", "speed", 100.0f);
    sys.add_stat("e1", "armor", 500.0f);

    assertTrue(sys.clear_stats("e1"), "Clear stats");
    assertTrue(sys.get_stat_count("e1") == 0, "Zero stats after clear");
}

static void testRigLinkInterfaceLevel() {
    std::cout << "\n=== RigLink: InterfaceLevel ===" << std::endl;
    ecs::World world;
    systems::RigLinkSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    assertTrue(sys.set_interface_level("e1", 5), "Set level 5");
    assertTrue(sys.get_interface_level("e1") == 5, "Level is 5");
    assertTrue(!sys.set_interface_level("e1", 0), "Level 0 rejected");
    assertTrue(!sys.set_interface_level("e1", -1), "Negative level rejected");
    assertTrue(sys.get_interface_level("e1") == 5, "Level unchanged");
}

static void testRigLinkQuality() {
    std::cout << "\n=== RigLink: Quality ===" << std::endl;
    ecs::World world;
    systems::RigLinkSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    assertTrue(sys.set_link_quality("e1", 0.75f), "Set quality 0.75");
    assertTrue(approxEqual(sys.get_link_quality("e1"), 0.75f), "Quality is 0.75");
}

static void testRigLinkMaxStats() {
    std::cout << "\n=== RigLink: MaxStats ===" << std::endl;
    ecs::World world;
    systems::RigLinkSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    for (int i = 0; i < 20; i++) {
        std::string name = "stat_" + std::to_string(i);
        assertTrue(sys.add_stat("e1", name, static_cast<float>(i)), "Add stat within limit");
    }
    assertTrue(!sys.add_stat("e1", "stat_20", 20.0f), "Blocked at max");
    assertTrue(sys.get_stat_count("e1") == 20, "At max capacity");
}

static void testRigLinkUpdate() {
    std::cout << "\n=== RigLink: Update ===" << std::endl;
    ecs::World world;
    systems::RigLinkSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.link_to_ship("e1", "ship", "port");
    sys.update(0.016f);
    assertTrue(sys.is_linked("e1"), "Still linked after update");
}

static void testRigLinkMissing() {
    std::cout << "\n=== RigLink: Missing ===" << std::endl;
    ecs::World world;
    systems::RigLinkSystem sys(&world);

    assertTrue(!sys.link_to_ship("no", "s", "p"), "link fails");
    assertTrue(!sys.unlink("no"), "unlink fails");
    assertTrue(!sys.add_stat("no", "s", 1.0f), "add_stat fails");
    assertTrue(!sys.remove_stat("no", "s"), "remove_stat fails");
    assertTrue(!sys.set_stat_bonus("no", "s", 1.0f), "set_stat_bonus fails");
    assertTrue(!sys.clear_stats("no"), "clear_stats fails");
    assertTrue(!sys.set_interface_level("no", 1), "set_interface_level fails");
    assertTrue(!sys.set_link_quality("no", 1.0f), "set_link_quality fails");
    assertTrue(!sys.is_linked("no"), "is_linked default");
    assertTrue(sys.get_linked_ship("no") == "", "linked_ship default");
    assertTrue(sys.get_linked_port("no") == "", "linked_port default");
    assertTrue(approxEqual(sys.get_stat_value("no", "s"), 0.0f), "stat_value default");
    assertTrue(sys.get_stat_count("no") == 0, "stat_count default");
    assertTrue(sys.get_interface_level("no") == 0, "interface_level default");
    assertTrue(approxEqual(sys.get_link_quality("no"), 0.0f), "link_quality default");
    assertTrue(sys.get_total_links("no") == 0, "total_links default");
}

void run_rig_link_system_tests() {
    testRigLinkInit();
    testRigLinkToShip();
    testRigLinkUnlink();
    testRigLinkStats();
    testRigLinkStatBonus();
    testRigLinkRemoveStat();
    testRigLinkClearStats();
    testRigLinkInterfaceLevel();
    testRigLinkQuality();
    testRigLinkMaxStats();
    testRigLinkUpdate();
    testRigLinkMissing();
}
