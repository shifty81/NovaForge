// Tests for: ClientPredictionSystem Tests
#include "test_log.h"
#include "components/core_components.h"
#include "ecs/component.h"
#include "ecs/system.h"
#include "ui/server_console.h"
#include "systems/client_prediction_system.h"
#include <sys/stat.h>

using namespace atlas;

// ==================== ClientPredictionSystem Tests ====================

static void testClientPredictionInit() {
    std::cout << "\n=== ClientPrediction: Init ===" << std::endl;
    ecs::World world;
    systems::ClientPredictionSystem sys(&world);
    auto* e = world.createEntity("ship1");
    assertTrue(sys.initPrediction("ship1", "client_abc"), "Init prediction succeeds");
    auto* cp = e->getComponent<components::ClientPrediction>();
    assertTrue(cp != nullptr, "Component exists");
    assertTrue(cp->client_id == "client_abc", "Client ID set");
    assertTrue(cp->active == true, "Active by default");
}

static void testClientPredictionServerState() {
    std::cout << "\n=== ClientPrediction: Server State ===" << std::endl;
    ecs::World world;
    systems::ClientPredictionSystem sys(&world);
    world.createEntity("ship1");
    sys.initPrediction("ship1", "c1");
    assertTrue(sys.setServerState("ship1", 10.0f, 20.0f, 30.0f, 5), "Set server state");
    assertTrue(sys.setServerState("nonexistent", 0, 0, 0, 0) == false, "Fails on missing");
}

static void testClientPredictionApplyInput() {
    std::cout << "\n=== ClientPrediction: Apply Input ===" << std::endl;
    ecs::World world;
    systems::ClientPredictionSystem sys(&world);
    world.createEntity("ship1");
    sys.initPrediction("ship1", "c1");
    assertTrue(sys.applyInput("ship1", 1.0f, 0.0f, 0.0f, 10), "Apply input succeeds");
    assertTrue(sys.getPredictionFrame("ship1") == 10, "Frame set to 10");
}

static void testClientPredictionUpdate() {
    std::cout << "\n=== ClientPrediction: Update ===" << std::endl;
    ecs::World world;
    systems::ClientPredictionSystem sys(&world);
    auto* e = world.createEntity("ship1");
    sys.initPrediction("ship1", "c1");
    sys.applyInput("ship1", 10.0f, 0.0f, 0.0f, 1);
    sys.update(0.1f);
    auto* cp = e->getComponent<components::ClientPrediction>();
    assertTrue(cp->predicted_x > 0.0f, "Predicted X advanced by velocity");
}

static void testClientPredictionError() {
    std::cout << "\n=== ClientPrediction: Error Calculation ===" << std::endl;
    ecs::World world;
    systems::ClientPredictionSystem sys(&world);
    auto* e = world.createEntity("ship1");
    sys.initPrediction("ship1", "c1");
    auto* cp = e->getComponent<components::ClientPrediction>();
    cp->predicted_x = 10.0f;
    cp->server_x = 0.0f;
    sys.update(0.0f);
    assertTrue(sys.getPredictionError("ship1") > 0.0f, "Error is non-zero");
}

static void testClientPredictionReconciliation() {
    std::cout << "\n=== ClientPrediction: Reconciliation ===" << std::endl;
    ecs::World world;
    systems::ClientPredictionSystem sys(&world);
    auto* e = world.createEntity("ship1");
    sys.initPrediction("ship1", "c1");
    auto* cp = e->getComponent<components::ClientPrediction>();
    cp->predicted_x = 100.0f;
    cp->server_x = 0.0f;
    cp->correction_speed = 5.0f;
    sys.update(0.1f);
    assertTrue(sys.isReconciling("ship1"), "Is reconciling after error");
    assertTrue(sys.getCorrectionBlend("ship1") > 0.0f, "Blend advanced");
}

static void testClientPredictionSnap() {
    std::cout << "\n=== ClientPrediction: Snap to Server ===" << std::endl;
    ecs::World world;
    systems::ClientPredictionSystem sys(&world);
    auto* e = world.createEntity("ship1");
    sys.initPrediction("ship1", "c1");
    auto* cp = e->getComponent<components::ClientPrediction>();
    cp->predicted_x = 10.0f;
    cp->server_x = 0.0f;
    cp->correction_speed = 100.0f;
    sys.update(1.0f);
    assertTrue(approxEqual(cp->predicted_x, cp->server_x, 0.1f), "Snapped to server after full blend");
    assertTrue(approxEqual(cp->correction_blend, 0.0f), "Blend reset after snap");
}

static void testClientPredictionInactive() {
    std::cout << "\n=== ClientPrediction: Inactive ===" << std::endl;
    ecs::World world;
    systems::ClientPredictionSystem sys(&world);
    auto* e = world.createEntity("ship1");
    sys.initPrediction("ship1", "c1");
    auto* cp = e->getComponent<components::ClientPrediction>();
    cp->active = false;
    cp->velocity_x = 100.0f;
    sys.update(1.0f);
    assertTrue(approxEqual(cp->predicted_x, 0.0f), "Inactive entity not updated");
}

static void testClientPredictionFrame() {
    std::cout << "\n=== ClientPrediction: Frame Tracking ===" << std::endl;
    ecs::World world;
    systems::ClientPredictionSystem sys(&world);
    world.createEntity("ship1");
    sys.initPrediction("ship1", "c1");
    sys.applyInput("ship1", 0, 0, 0, 42);
    assertTrue(sys.getPredictionFrame("ship1") == 42, "Prediction frame is 42");
    sys.setServerState("ship1", 0, 0, 0, 40);
    assertTrue(sys.getPredictionFrame("ship1") == 42, "Frame unchanged after server state");
}

static void testClientPredictionMissing() {
    std::cout << "\n=== ClientPrediction: Missing Entity ===" << std::endl;
    ecs::World world;
    systems::ClientPredictionSystem sys(&world);
    assertTrue(!sys.initPrediction("nonexistent", "c1"), "Init fails on missing");
    assertTrue(!sys.setServerState("nonexistent", 0, 0, 0, 0), "SetServer fails on missing");
    assertTrue(!sys.applyInput("nonexistent", 0, 0, 0, 0), "ApplyInput fails on missing");
    assertTrue(sys.getPredictionError("nonexistent") == 0.0f, "Error 0 on missing");
    assertTrue(!sys.isReconciling("nonexistent"), "Not reconciling on missing");
    assertTrue(sys.getCorrectionBlend("nonexistent") == 0.0f, "Blend 0 on missing");
    assertTrue(sys.getPredictionFrame("nonexistent") == 0, "Frame 0 on missing");
}


void run_client_prediction_system_tests() {
    testClientPredictionInit();
    testClientPredictionServerState();
    testClientPredictionApplyInput();
    testClientPredictionUpdate();
    testClientPredictionError();
    testClientPredictionReconciliation();
    testClientPredictionSnap();
    testClientPredictionInactive();
    testClientPredictionFrame();
    testClientPredictionMissing();
}
