// Tests for: ChatRouterSystem
#include "test_log.h"
#include "components/game_components.h"
#include "ecs/system.h"
#include "systems/chat_router_system.h"

using namespace atlas;

// ==================== ChatRouterSystem Tests ====================

static void testChatDefaultState() {
    std::cout << "\n=== ChatRouter: DefaultState ===" << std::endl;
    ecs::World world;
    systems::ChatRouterSystem sys(&world);
    auto* e = world.createEntity("player1");
    auto* state = addComp<components::ChatRouterState>(e);

    assertTrue(state->next_global_seq == 0, "Default global seq 0");
    assertTrue(state->next_local_seq == 0, "Default local seq 0");
    assertTrue(state->next_party_seq == 0, "Default party seq 0");
    assertTrue(state->next_guild_seq == 0, "Default guild seq 0");
    assertTrue(state->next_system_seq == 0, "Default system seq 0");
    assertTrue(state->total_messages_routed == 0, "Default routed 0");
    assertTrue(state->total_messages_rejected == 0, "Default rejected 0");
    assertTrue(state->rate_limit_violations == 0, "Default violations 0");
    assertTrue(state->messages_in_window == 0, "Default messages_in_window 0");
    assertTrue(state->max_messages_per_window == 3, "Default max 3");
    assertTrue(approxEqual(state->rate_window_seconds, 5.0f), "Default rate window 5s");
    assertTrue(state->max_message_length == 512, "Default max length 512");
    assertTrue(state->active, "Default active true");
}

static void testChatRouteIncrementsSeq() {
    std::cout << "\n=== ChatRouter: RouteIncrementsSeq ===" << std::endl;
    ecs::World world;
    systems::ChatRouterSystem sys(&world);
    auto* e = world.createEntity("p1");
    addComp<components::ChatRouterState>(e);

    assertTrue(sys.routeMessage("p1", "global", "hello"), "Route global OK");
    assertTrue(sys.getNextSeq("p1", "global") == 1, "Global seq now 1");
    assertTrue(sys.getTotalRouted("p1") == 1, "Total routed 1");

    assertTrue(sys.routeMessage("p1", "local", "hi"), "Route local OK");
    assertTrue(sys.getNextSeq("p1", "local") == 1, "Local seq now 1");
    assertTrue(sys.getTotalRouted("p1") == 2, "Total routed 2");

    assertTrue(sys.routeMessage("p1", "global", "world"), "Route 2nd global OK");
    assertTrue(sys.getNextSeq("p1", "global") == 2, "Global seq now 2");
}

static void testChatRateLimiting() {
    std::cout << "\n=== ChatRouter: RateLimiting ===" << std::endl;
    ecs::World world;
    systems::ChatRouterSystem sys(&world);
    auto* e = world.createEntity("p1");
    auto* state = addComp<components::ChatRouterState>(e);

    assertTrue(sys.routeMessage("p1", "global", "msg1"), "Route 1 OK");
    assertTrue(sys.routeMessage("p1", "global", "msg2"), "Route 2 OK");
    assertTrue(sys.routeMessage("p1", "global", "msg3"), "Route 3 OK");
    assertTrue(sys.isRateLimited("p1"), "Rate limited after 3");
    assertTrue(!sys.routeMessage("p1", "global", "msg4"), "Route 4 rejected");
    assertTrue(state->rate_limit_violations == 1, "1 rate violation");
    assertTrue(state->total_messages_rejected == 1, "1 rejected");
    assertTrue(state->total_messages_routed == 3, "3 routed");
}

static void testChatRateWindowReset() {
    std::cout << "\n=== ChatRouter: RateWindowReset ===" << std::endl;
    ecs::World world;
    systems::ChatRouterSystem sys(&world);
    auto* e = world.createEntity("p1");
    addComp<components::ChatRouterState>(e);

    sys.routeMessage("p1", "global", "msg1");
    sys.routeMessage("p1", "global", "msg2");
    sys.routeMessage("p1", "global", "msg3");
    assertTrue(sys.isRateLimited("p1"), "Rate limited");

    // Advance past rate window (5 seconds)
    sys.update(6.0f);
    assertTrue(!sys.isRateLimited("p1"), "No longer rate limited after window");
    assertTrue(sys.routeMessage("p1", "global", "msg4"), "Route OK after reset");
}

static void testChatRejectLongMessages() {
    std::cout << "\n=== ChatRouter: RejectLongMessages ===" << std::endl;
    ecs::World world;
    systems::ChatRouterSystem sys(&world);
    auto* e = world.createEntity("p1");
    auto* state = addComp<components::ChatRouterState>(e);

    std::string long_msg(513, 'A');
    assertTrue(!sys.routeMessage("p1", "global", long_msg), "Reject 513 chars");
    assertTrue(state->total_messages_rejected == 1, "1 rejected for length");
    assertTrue(state->total_messages_routed == 0, "0 routed");

    std::string ok_msg(512, 'B');
    assertTrue(sys.routeMessage("p1", "global", ok_msg), "Accept 512 chars");
}

static void testChatRejectEmptyMessages() {
    std::cout << "\n=== ChatRouter: RejectEmptyMessages ===" << std::endl;
    ecs::World world;
    systems::ChatRouterSystem sys(&world);
    auto* e = world.createEntity("p1");
    auto* state = addComp<components::ChatRouterState>(e);

    assertTrue(!sys.routeMessage("p1", "global", ""), "Reject empty");
    assertTrue(state->total_messages_rejected == 1, "1 rejected for empty");
}

static void testChatIndependentStreams() {
    std::cout << "\n=== ChatRouter: IndependentStreams ===" << std::endl;
    ecs::World world;
    systems::ChatRouterSystem sys(&world);
    auto* e = world.createEntity("p1");
    addComp<components::ChatRouterState>(e);

    // Reset rate window to allow many messages
    sys.routeMessage("p1", "global", "g1");
    sys.resetRateWindow("p1");
    sys.routeMessage("p1", "local", "l1");
    sys.resetRateWindow("p1");
    sys.routeMessage("p1", "party", "p1msg");
    sys.resetRateWindow("p1");
    sys.routeMessage("p1", "guild", "gu1");
    sys.resetRateWindow("p1");
    sys.routeMessage("p1", "system", "s1");

    assertTrue(sys.getNextSeq("p1", "global") == 1, "Global seq 1");
    assertTrue(sys.getNextSeq("p1", "local") == 1, "Local seq 1");
    assertTrue(sys.getNextSeq("p1", "party") == 1, "Party seq 1");
    assertTrue(sys.getNextSeq("p1", "guild") == 1, "Guild seq 1");
    assertTrue(sys.getNextSeq("p1", "system") == 1, "System seq 1");
}

static void testChatResetRateWindow() {
    std::cout << "\n=== ChatRouter: ManualResetRateWindow ===" << std::endl;
    ecs::World world;
    systems::ChatRouterSystem sys(&world);
    auto* e = world.createEntity("p1");
    addComp<components::ChatRouterState>(e);

    sys.routeMessage("p1", "global", "1");
    sys.routeMessage("p1", "global", "2");
    sys.routeMessage("p1", "global", "3");
    assertTrue(sys.isRateLimited("p1"), "Rate limited");
    sys.resetRateWindow("p1");
    assertTrue(!sys.isRateLimited("p1"), "Not limited after reset");
    assertTrue(sys.routeMessage("p1", "global", "4"), "Route after manual reset");
}

static void testChatMissingEntity() {
    std::cout << "\n=== ChatRouter: MissingEntity ===" << std::endl;
    ecs::World world;
    systems::ChatRouterSystem sys(&world);

    assertTrue(!sys.routeMessage("x", "global", "hi"), "Route on missing");
    assertTrue(sys.getNextSeq("x", "global") == 0, "Seq on missing");
    assertTrue(!sys.isRateLimited("x"), "Rate limited on missing");
    assertTrue(sys.getTotalRouted("x") == 0, "Routed on missing");
    assertTrue(sys.getTotalRejected("x") == 0, "Rejected on missing");
}

static void testChatInvalidStream() {
    std::cout << "\n=== ChatRouter: InvalidStream ===" << std::endl;
    ecs::World world;
    systems::ChatRouterSystem sys(&world);
    auto* e = world.createEntity("p1");
    auto* state = addComp<components::ChatRouterState>(e);

    assertTrue(!sys.routeMessage("p1", "invalid_stream", "hi"), "Reject invalid stream");
    assertTrue(state->total_messages_rejected == 1, "1 rejected for invalid stream");
}

void run_chat_router_system_tests() {
    testChatDefaultState();
    testChatRouteIncrementsSeq();
    testChatRateLimiting();
    testChatRateWindowReset();
    testChatRejectLongMessages();
    testChatRejectEmptyMessages();
    testChatIndependentStreams();
    testChatResetRateWindow();
    testChatMissingEntity();
    testChatInvalidStream();
}
