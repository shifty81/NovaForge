// Tests for: ContentValidation System Tests
#include "test_log.h"
#include "components/core_components.h"
#include "ecs/component.h"
#include "ecs/system.h"
#include "systems/content_validation_system.h"
#include <sys/stat.h>

using namespace atlas;

// ==================== ContentValidation System Tests ====================

static void testContentValidationCreate() {
    std::cout << "\n=== ContentValidation: Create ===" << std::endl;
    ecs::World world;
    systems::ContentValidationSystem sys(&world);
    auto* e = world.createEntity("val1");
    assertTrue(sys.createValidator("val1"), "Create validator succeeds");
    auto* cv = e->getComponent<components::ContentValidation>();
    assertTrue(cv != nullptr, "Component exists");
    assertTrue(cv->active, "Validator is active by default");
    assertTrue(cv->total_validations == 0, "No validations yet");
}

static void testContentValidationSubmit() {
    std::cout << "\n=== ContentValidation: Submit ===" << std::endl;
    ecs::World world;
    systems::ContentValidationSystem sys(&world);
    world.createEntity("val1");
    sys.createValidator("val1");
    assertTrue(sys.submitContent("val1", "ship_001", 0, "Custom Frigate"), "Submit succeeds");
    assertTrue(sys.getContentState("val1", "ship_001") == 0, "State is Pending (0)");
    assertTrue(sys.getPendingCount("val1") == 1, "1 pending");
}

static void testContentValidationDuplicate() {
    std::cout << "\n=== ContentValidation: Duplicate ===" << std::endl;
    ecs::World world;
    systems::ContentValidationSystem sys(&world);
    world.createEntity("val1");
    sys.createValidator("val1");
    sys.submitContent("val1", "ship_001", 0, "Custom Frigate");
    assertTrue(!sys.submitContent("val1", "ship_001", 0, "Duplicate"), "Duplicate rejected");
    assertTrue(sys.getPendingCount("val1") == 1, "Still 1 pending");
}

static void testContentValidationRun() {
    std::cout << "\n=== ContentValidation: RunValidation ===" << std::endl;
    ecs::World world;
    systems::ContentValidationSystem sys(&world);
    world.createEntity("val1");
    sys.createValidator("val1");
    sys.submitContent("val1", "ship_001", 0, "Custom Frigate");
    assertTrue(sys.runValidation("val1", "ship_001"), "Validation starts");
    assertTrue(sys.getContentState("val1", "ship_001") == 1, "State is Validating (1)");
    assertTrue(sys.getTotalValidations("val1") == 1, "1 total validation");
}

static void testContentValidationApprove() {
    std::cout << "\n=== ContentValidation: Approve ===" << std::endl;
    ecs::World world;
    systems::ContentValidationSystem sys(&world);
    world.createEntity("val1");
    sys.createValidator("val1");
    sys.submitContent("val1", "ship_001", 0, "Custom Frigate");
    sys.runValidation("val1", "ship_001");
    assertTrue(sys.approveContent("val1", "ship_001"), "Approve succeeds");
    assertTrue(sys.getContentState("val1", "ship_001") == 2, "State is Approved (2)");
    assertTrue(sys.getApprovedCount("val1") == 1, "1 approved");
}

static void testContentValidationReject() {
    std::cout << "\n=== ContentValidation: Reject ===" << std::endl;
    ecs::World world;
    systems::ContentValidationSystem sys(&world);
    world.createEntity("val1");
    sys.createValidator("val1");
    sys.submitContent("val1", "mod_001", 1, "OP Module");
    sys.runValidation("val1", "mod_001");
    assertTrue(sys.rejectContent("val1", "mod_001", "Overpowered stats"), "Reject succeeds");
    assertTrue(sys.getContentState("val1", "mod_001") == 3, "State is Rejected (3)");
    assertTrue(sys.getRejectedCount("val1") == 1, "1 rejected");
    assertTrue(sys.getRejectionReason("val1", "mod_001") == "Overpowered stats", "Reason stored");
}

static void testContentValidationWorkflow() {
    std::cout << "\n=== ContentValidation: Workflow ===" << std::endl;
    ecs::World world;
    systems::ContentValidationSystem sys(&world);
    world.createEntity("val1");
    sys.createValidator("val1");
    sys.submitContent("val1", "s1", 0, "Ship A");
    sys.submitContent("val1", "s2", 0, "Ship B");
    sys.submitContent("val1", "m1", 2, "Mission X");
    assertTrue(sys.getPendingCount("val1") == 3, "3 pending");
    sys.runValidation("val1", "s1");
    sys.runValidation("val1", "s2");
    sys.approveContent("val1", "s1");
    sys.rejectContent("val1", "s2", "Bad stats");
    assertTrue(sys.getApprovedCount("val1") == 1, "1 approved");
    assertTrue(sys.getRejectedCount("val1") == 1, "1 rejected");
    assertTrue(sys.getPendingCount("val1") == 1, "1 still pending");
    assertTrue(sys.getTotalValidations("val1") == 2, "2 total validations");
}

static void testContentValidationStateTransition() {
    std::cout << "\n=== ContentValidation: StateTransition ===" << std::endl;
    ecs::World world;
    systems::ContentValidationSystem sys(&world);
    world.createEntity("val1");
    sys.createValidator("val1");
    sys.submitContent("val1", "s1", 0, "Ship");
    assertTrue(!sys.approveContent("val1", "s1"), "Cannot approve from Pending");
    assertTrue(!sys.rejectContent("val1", "s1", "reason"), "Cannot reject from Pending");
    sys.runValidation("val1", "s1");
    assertTrue(!sys.runValidation("val1", "s1"), "Cannot re-validate from Validating");
}

static void testContentValidationMissing() {
    std::cout << "\n=== ContentValidation: Missing ===" << std::endl;
    ecs::World world;
    systems::ContentValidationSystem sys(&world);
    assertTrue(!sys.createValidator("nonexistent"), "Create fails on missing");
    assertTrue(!sys.submitContent("nonexistent", "x", 0, "X"), "Submit fails on missing");
    assertTrue(!sys.runValidation("nonexistent", "x"), "Validate fails on missing");
    assertTrue(!sys.approveContent("nonexistent", "x"), "Approve fails on missing");
    assertTrue(!sys.rejectContent("nonexistent", "x", "r"), "Reject fails on missing");
    assertTrue(sys.getContentState("nonexistent", "x") == -1, "-1 state on missing");
    assertTrue(sys.getPendingCount("nonexistent") == 0, "0 pending on missing");
    assertTrue(sys.getApprovedCount("nonexistent") == 0, "0 approved on missing");
    assertTrue(sys.getRejectedCount("nonexistent") == 0, "0 rejected on missing");
    assertTrue(sys.getTotalValidations("nonexistent") == 0, "0 validations on missing");
}


void run_content_validation_system_tests() {
    testContentValidationCreate();
    testContentValidationSubmit();
    testContentValidationDuplicate();
    testContentValidationRun();
    testContentValidationApprove();
    testContentValidationReject();
    testContentValidationWorkflow();
    testContentValidationStateTransition();
    testContentValidationMissing();
}
