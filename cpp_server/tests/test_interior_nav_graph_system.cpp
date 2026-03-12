// Tests for: InteriorNavGraphSystem
#include "test_log.h"
#include "components/navigation_components.h"
#include "ecs/system.h"
#include "systems/interior_nav_graph_system.h"

using namespace atlas;

// ==================== InteriorNavGraphSystem Tests ====================

static void testNavGraphInit() {
    std::cout << "\n=== NavGraph: Init ===" << std::endl;
    ecs::World world;
    systems::InteriorNavGraphSystem sys(&world);
    world.createEntity("e1");
    assertTrue(sys.initialize("e1"), "Init succeeds");
    assertTrue(sys.get_node_count("e1") == 0, "No nodes initially");
    assertTrue(sys.get_total_path_queries("e1") == 0, "Zero queries");
    assertTrue(!sys.initialize("nonexistent"), "Init fails on missing entity");
}

static void testNavGraphAddNode() {
    std::cout << "\n=== NavGraph: AddNode ===" << std::endl;
    ecs::World world;
    systems::InteriorNavGraphSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    assertTrue(sys.add_node("e1", "door1", 0, 0.0f, 0.0f, 0.0f, 1.0f), "Add door1");
    assertTrue(sys.add_node("e1", "ramp1", 1, 5.0f, 0.0f, 0.0f, 1.5f), "Add ramp1");
    assertTrue(sys.add_node("e1", "ladder1", 2, 10.0f, 0.0f, 5.0f, 2.0f), "Add ladder1");
    assertTrue(sys.get_node_count("e1") == 3, "Three nodes");
    assertTrue(sys.has_node("e1", "door1"), "Has door1");
    assertTrue(sys.has_node("e1", "ramp1"), "Has ramp1");
    assertTrue(!sys.has_node("e1", "ghost"), "No ghost node");

    assertTrue(!sys.add_node("e1", "door1", 0, 0.0f, 0.0f, 0.0f, 1.0f), "Duplicate rejected");
    assertTrue(!sys.add_node("e1", "", 0, 0.0f, 0.0f, 0.0f, 1.0f), "Empty id rejected");
}

static void testNavGraphRemoveNode() {
    std::cout << "\n=== NavGraph: RemoveNode ===" << std::endl;
    ecs::World world;
    systems::InteriorNavGraphSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.add_node("e1", "n1", 0, 0, 0, 0, 1.0f);
    sys.add_node("e1", "n2", 1, 5, 0, 0, 1.0f);
    sys.connect_nodes("e1", "n1", "n2");

    assertTrue(sys.remove_node("e1", "n1"), "Remove n1");
    assertTrue(sys.get_node_count("e1") == 1, "One node left");
    assertTrue(!sys.has_node("e1", "n1"), "n1 removed");
    // n2 should no longer have n1 in its connections
    assertTrue(sys.get_connection_count("e1", "n2") == 0, "n2 connections cleaned");
    assertTrue(!sys.remove_node("e1", "n1"), "Remove nonexistent fails");
}

static void testNavGraphConnect() {
    std::cout << "\n=== NavGraph: Connect ===" << std::endl;
    ecs::World world;
    systems::InteriorNavGraphSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.add_node("e1", "a", 0, 0, 0, 0, 1.0f);
    sys.add_node("e1", "b", 0, 5, 0, 0, 1.0f);
    sys.add_node("e1", "c", 0, 10, 0, 0, 1.0f);

    assertTrue(sys.connect_nodes("e1", "a", "b"), "Connect a-b");
    assertTrue(sys.is_connected("e1", "a", "b"), "a connected to b");
    assertTrue(sys.is_connected("e1", "b", "a"), "b connected to a (bidirectional)");
    assertTrue(sys.get_connection_count("e1", "a") == 1, "a has 1 connection");
    assertTrue(sys.get_connection_count("e1", "b") == 1, "b has 1 connection");

    // Duplicate
    assertTrue(!sys.connect_nodes("e1", "a", "b"), "Duplicate connection rejected");

    // Self-connect
    assertTrue(!sys.connect_nodes("e1", "a", "a"), "Self-connection rejected");

    // Connect to nonexistent
    assertTrue(!sys.connect_nodes("e1", "a", "z"), "Connect to nonexistent fails");
}

static void testNavGraphDisconnect() {
    std::cout << "\n=== NavGraph: Disconnect ===" << std::endl;
    ecs::World world;
    systems::InteriorNavGraphSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.add_node("e1", "a", 0, 0, 0, 0, 1.0f);
    sys.add_node("e1", "b", 0, 5, 0, 0, 1.0f);
    sys.connect_nodes("e1", "a", "b");

    assertTrue(sys.disconnect_nodes("e1", "a", "b"), "Disconnect a-b");
    assertTrue(!sys.is_connected("e1", "a", "b"), "a no longer connected to b");
    assertTrue(!sys.is_connected("e1", "b", "a"), "b no longer connected to a");
    assertTrue(!sys.disconnect_nodes("e1", "a", "b"), "Disconnect again fails");
}

static void testNavGraphNodeType() {
    std::cout << "\n=== NavGraph: NodeType ===" << std::endl;
    ecs::World world;
    systems::InteriorNavGraphSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    using NT = components::InteriorNavGraphState::NodeType;
    sys.add_node("e1", "n1", static_cast<int>(NT::Elevator), 0, 0, 0, 1.0f);
    sys.add_node("e1", "n2", static_cast<int>(NT::Airlock), 5, 0, 0, 1.0f);

    assertTrue(sys.get_node_type("e1", "n1") == static_cast<int>(NT::Elevator), "n1 is Elevator");
    assertTrue(sys.get_node_type("e1", "n2") == static_cast<int>(NT::Airlock), "n2 is Airlock");
    assertTrue(sys.get_node_type("e1", "ghost") == 0, "Unknown node returns 0");
}

static void testNavGraphClear() {
    std::cout << "\n=== NavGraph: Clear ===" << std::endl;
    ecs::World world;
    systems::InteriorNavGraphSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.add_node("e1", "a", 0, 0, 0, 0, 1.0f);
    sys.add_node("e1", "b", 0, 5, 0, 0, 1.0f);
    sys.connect_nodes("e1", "a", "b");

    assertTrue(sys.clear_graph("e1"), "Clear graph");
    assertTrue(sys.get_node_count("e1") == 0, "Zero nodes after clear");
}

static void testNavGraphConnectivity() {
    std::cout << "\n=== NavGraph: Connectivity ===" << std::endl;
    ecs::World world;
    systems::InteriorNavGraphSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    // Build a connected graph: a - b - c
    sys.add_node("e1", "a", 0, 0, 0, 0, 1.0f);
    sys.add_node("e1", "b", 0, 5, 0, 0, 1.0f);
    sys.add_node("e1", "c", 0, 10, 0, 0, 1.0f);
    sys.connect_nodes("e1", "a", "b");
    sys.connect_nodes("e1", "b", "c");

    assertTrue(sys.validate_connectivity("e1", "a"), "All reachable from a");
    assertTrue(sys.get_total_path_queries("e1") == 1, "One query");

    // Add disconnected node
    sys.add_node("e1", "d", 0, 20, 0, 0, 1.0f);
    assertTrue(!sys.validate_connectivity("e1", "a"), "d is unreachable");
    assertTrue(sys.get_total_path_queries("e1") == 2, "Two queries");

    // Connect d
    sys.connect_nodes("e1", "c", "d");
    assertTrue(sys.validate_connectivity("e1", "a"), "Now all reachable");
}

static void testNavGraphConnectivityEmpty() {
    std::cout << "\n=== NavGraph: ConnectivityEmpty ===" << std::endl;
    ecs::World world;
    systems::InteriorNavGraphSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    // Empty graph
    assertTrue(!sys.validate_connectivity("e1", "ghost"), "Start node not found");
}

static void testNavGraphUpdate() {
    std::cout << "\n=== NavGraph: Update ===" << std::endl;
    ecs::World world;
    systems::InteriorNavGraphSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.add_node("e1", "a", 0, 0, 0, 0, 1.0f);
    sys.update(0.016f);
    assertTrue(sys.get_node_count("e1") == 1, "Node persists after update");
}

static void testNavGraphMissing() {
    std::cout << "\n=== NavGraph: Missing ===" << std::endl;
    ecs::World world;
    systems::InteriorNavGraphSystem sys(&world);

    assertTrue(!sys.add_node("no", "n", 0, 0, 0, 0, 1), "add_node fails");
    assertTrue(!sys.remove_node("no", "n"), "remove_node fails");
    assertTrue(!sys.connect_nodes("no", "a", "b"), "connect_nodes fails");
    assertTrue(!sys.disconnect_nodes("no", "a", "b"), "disconnect_nodes fails");
    assertTrue(!sys.clear_graph("no"), "clear_graph fails");
    assertTrue(!sys.validate_connectivity("no", "a"), "validate fails");
    assertTrue(sys.get_node_count("no") == 0, "get_node_count default");
    assertTrue(!sys.has_node("no", "n"), "has_node default");
    assertTrue(sys.get_connection_count("no", "n") == 0, "get_connection_count default");
    assertTrue(!sys.is_connected("no", "a", "b"), "is_connected default");
    assertTrue(sys.get_node_type("no", "n") == 0, "get_node_type default");
    assertTrue(sys.get_total_path_queries("no") == 0, "get_total_path_queries default");
}

void run_interior_nav_graph_system_tests() {
    testNavGraphInit();
    testNavGraphAddNode();
    testNavGraphRemoveNode();
    testNavGraphConnect();
    testNavGraphDisconnect();
    testNavGraphNodeType();
    testNavGraphClear();
    testNavGraphConnectivity();
    testNavGraphConnectivityEmpty();
    testNavGraphUpdate();
    testNavGraphMissing();
}
