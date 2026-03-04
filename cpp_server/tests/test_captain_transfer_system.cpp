// Tests for: Captain Transfer System Tests
#include "test_log.h"
#include "components/fleet_components.h"
#include "ecs/system.h"
#include "systems/captain_transfer_system.h"

using namespace atlas;

// ==================== Captain Transfer System Tests ====================

static void testTransferHighMorale() {
    std::cout << "\n=== Transfer High Morale ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("xfer1");
    auto* req = addComp<components::CaptainTransferRequest>(e);
    auto* morale = addComp<components::FleetMorale>(e);
    morale->morale_score = 85.0f;

    systems::CaptainTransferSystem sys(&world);
    sys.update(1.0f);
    assertTrue(req->request_pending, "High morale triggers transfer request");
    assertTrue(req->request_type == components::CaptainTransferRequest::TransferType::BiggerShip,
               "High morale requests BiggerShip");
}

static void testTransferLowMorale() {
    std::cout << "\n=== Transfer Low Morale ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("xfer2");
    auto* req = addComp<components::CaptainTransferRequest>(e);
    auto* morale = addComp<components::FleetMorale>(e);
    morale->morale_score = 20.0f;

    systems::CaptainTransferSystem sys(&world);
    sys.update(1.0f);
    assertTrue(req->request_pending, "Low morale triggers transfer request");
    assertTrue(req->request_type == components::CaptainTransferRequest::TransferType::EscortOnly,
               "Low morale requests EscortOnly");
}

static void testTransferApprove() {
    std::cout << "\n=== Transfer Approve ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("xfer3");
    auto* req = addComp<components::CaptainTransferRequest>(e);
    auto* morale = addComp<components::FleetMorale>(e);
    morale->morale_score = 90.0f;

    systems::CaptainTransferSystem sys(&world);
    sys.update(1.0f);
    assertTrue(req->request_pending, "Request is pending");
    sys.approveTransfer("xfer3");
    assertTrue(!req->request_pending, "Approve clears pending");
}


void run_captain_transfer_system_tests() {
    testTransferHighMorale();
    testTransferLowMorale();
    testTransferApprove();
}
