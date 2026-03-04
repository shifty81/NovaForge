// Tests for: FPS Character Controller System Tests
#include "test_log.h"
#include "components/fps_components.h"
#include "ecs/component.h"
#include "ecs/system.h"
#include "systems/fps_character_controller_system.h"
#include <sys/stat.h>

using namespace atlas;

// ==================== FPS Character Controller System Tests ====================

static void testFPSCharControllerSpawn() {
    std::cout << "\n=== FPS Character Controller Spawn ===" << std::endl;
    ecs::World world;
    systems::FPSCharacterControllerSystem sys(&world);

    assertTrue(sys.spawnCharacter("player1", "ship_interior_1", 5.0f, 0.0f, 3.0f, 90.0f),
               "Spawn character");
    assertTrue(!sys.spawnCharacter("player1", "ship_interior_1", 0, 0, 0),
               "Duplicate spawn fails");

    auto [x, y, z] = sys.getPosition("player1");
    assertTrue(approxEqual(x, 5.0f), "Spawn X");
    assertTrue(approxEqual(y, 0.0f), "Spawn Y");
    assertTrue(approxEqual(z, 3.0f), "Spawn Z");
}

static void testFPSCharControllerMovement() {
    std::cout << "\n=== FPS Character Controller Movement ===" << std::endl;
    ecs::World world;
    systems::FPSCharacterControllerSystem sys(&world);
    sys.spawnCharacter("player1", "interior1", 0.0f, 0.0f, 0.0f);

    // Walk forward (positive Z)
    sys.setMoveInput("player1", 0.0f, 1.0f);
    sys.update(1.0f);  // 1 second at walk speed 4 m/s

    auto [x, y, z] = sys.getPosition("player1");
    assertTrue(approxEqual(z, 4.0f), "Walked forward 4m in 1s");
    assertTrue(approxEqual(x, 0.0f), "No lateral movement");
}

static void testFPSCharControllerSprint() {
    std::cout << "\n=== FPS Character Controller Sprint ===" << std::endl;
    ecs::World world;
    systems::FPSCharacterControllerSystem sys(&world);
    sys.spawnCharacter("player1", "interior1", 0.0f, 0.0f, 0.0f);

    sys.setStance("player1", static_cast<int>(components::FPSCharacterState::Stance::Sprinting));
    sys.setMoveInput("player1", 0.0f, 1.0f);
    sys.update(1.0f);  // 1 second at sprint speed 7 m/s

    auto [x, y, z] = sys.getPosition("player1");
    assertTrue(approxEqual(z, 7.0f), "Sprinted forward 7m in 1s");
    assertTrue(sys.getStance("player1") ==
               static_cast<int>(components::FPSCharacterState::Stance::Sprinting),
               "Still sprinting");
}

static void testFPSCharControllerCrouch() {
    std::cout << "\n=== FPS Character Controller Crouch ===" << std::endl;
    ecs::World world;
    systems::FPSCharacterControllerSystem sys(&world);
    sys.spawnCharacter("player1", "interior1", 0.0f, 0.0f, 0.0f);

    sys.setStance("player1", static_cast<int>(components::FPSCharacterState::Stance::Crouching));
    sys.setMoveInput("player1", 1.0f, 0.0f);
    sys.update(1.0f);  // 1 second at crouch speed 2 m/s

    auto [x, y, z] = sys.getPosition("player1");
    assertTrue(approxEqual(x, 2.0f), "Crouch-walked sideways 2m in 1s");
}

static void testFPSCharControllerJump() {
    std::cout << "\n=== FPS Character Controller Jump ===" << std::endl;
    ecs::World world;
    systems::FPSCharacterControllerSystem sys(&world);
    sys.spawnCharacter("player1", "interior1", 0.0f, 0.0f, 0.0f);

    assertTrue(sys.isGrounded("player1"), "Starts grounded");
    sys.requestJump("player1");
    sys.update(0.1f);  // Short tick

    auto [x, y, z] = sys.getPosition("player1");
    assertTrue(y > 0.0f, "Character is above ground after jump");
    assertTrue(!sys.isGrounded("player1"), "Not grounded during jump");
}

static void testFPSCharControllerGravityLanding() {
    std::cout << "\n=== FPS Character Controller Gravity Landing ===" << std::endl;
    ecs::World world;
    systems::FPSCharacterControllerSystem sys(&world);
    sys.spawnCharacter("player1", "interior1", 0.0f, 0.0f, 0.0f);

    sys.requestJump("player1");
    // Simulate enough time for jump arc to complete
    for (int i = 0; i < 30; ++i) {
        sys.update(0.1f);
    }

    auto [x, y, z] = sys.getPosition("player1");
    assertTrue(approxEqual(y, 0.0f), "Landed back on ground");
    assertTrue(sys.isGrounded("player1"), "Grounded after landing");
}

static void testFPSCharControllerZeroG() {
    std::cout << "\n=== FPS Character Controller Zero-G ===" << std::endl;
    ecs::World world;
    systems::FPSCharacterControllerSystem sys(&world);
    sys.spawnCharacter("player1", "interior1", 0.0f, 5.0f, 0.0f);

    sys.setGravity("player1", 0.0f);  // Zero gravity
    sys.update(1.0f);

    auto [x, y, z] = sys.getPosition("player1");
    assertTrue(approxEqual(y, 5.0f), "No falling in zero-g");
    assertTrue(!sys.isGrounded("player1"), "Never grounded in zero-g");
}

static void testFPSCharControllerStaminaDrain() {
    std::cout << "\n=== FPS Character Controller Stamina Drain ===" << std::endl;
    ecs::World world;
    systems::FPSCharacterControllerSystem sys(&world);
    sys.spawnCharacter("player1", "interior1", 0.0f, 0.0f, 0.0f);

    assertTrue(approxEqual(sys.getStaminaFraction("player1"), 1.0f), "Full stamina initially");

    sys.setStance("player1", static_cast<int>(components::FPSCharacterState::Stance::Sprinting));
    sys.setMoveInput("player1", 0.0f, 1.0f);
    sys.update(3.0f);  // 3 seconds of sprinting, drains 60 stamina

    assertTrue(sys.getStaminaFraction("player1") < 1.0f, "Stamina drained by sprinting");
    assertTrue(sys.getStaminaFraction("player1") > 0.0f, "Stamina not fully depleted");
}

static void testFPSCharControllerStaminaExhaustion() {
    std::cout << "\n=== FPS Character Controller Stamina Exhaustion ===" << std::endl;
    ecs::World world;
    systems::FPSCharacterControllerSystem sys(&world);
    sys.spawnCharacter("player1", "interior1", 0.0f, 0.0f, 0.0f);

    sys.setStance("player1", static_cast<int>(components::FPSCharacterState::Stance::Sprinting));
    sys.setMoveInput("player1", 0.0f, 1.0f);
    sys.update(6.0f);  // 6 seconds, drains 120 stamina (max 100) → exhausted

    assertTrue(sys.getStance("player1") ==
               static_cast<int>(components::FPSCharacterState::Stance::Standing),
               "Reverts to standing when exhausted");
}

static void testFPSCharControllerLookDirection() {
    std::cout << "\n=== FPS Character Controller Look Direction ===" << std::endl;
    ecs::World world;
    systems::FPSCharacterControllerSystem sys(&world);
    sys.spawnCharacter("player1", "interior1", 0.0f, 0.0f, 0.0f);

    sys.setLookDirection("player1", 180.0f, 45.0f);
    auto [yaw, pitch] = sys.getLookDirection("player1");
    assertTrue(approxEqual(yaw, 180.0f), "Yaw set");
    assertTrue(approxEqual(pitch, 45.0f), "Pitch set");

    // Pitch clamp
    sys.setLookDirection("player1", 0.0f, 95.0f);
    auto [yaw2, pitch2] = sys.getLookDirection("player1");
    assertTrue(approxEqual(pitch2, 89.0f), "Pitch clamped to 89");
}

static void testFPSCharControllerRemove() {
    std::cout << "\n=== FPS Character Controller Remove ===" << std::endl;
    ecs::World world;
    systems::FPSCharacterControllerSystem sys(&world);
    sys.spawnCharacter("player1", "interior1", 0.0f, 0.0f, 0.0f);

    assertTrue(sys.removeCharacter("player1"), "Remove succeeds");
    assertTrue(!sys.removeCharacter("player1"), "Double remove fails");
    assertTrue(!sys.removeCharacter("nonexistent"), "Remove nonexistent fails");
}

static void testFPSCharControllerStanceNames() {
    std::cout << "\n=== FPS Character Controller Stance Names ===" << std::endl;
    assertTrue(systems::FPSCharacterControllerSystem::stanceName(0) == "Standing", "Standing name");
    assertTrue(systems::FPSCharacterControllerSystem::stanceName(1) == "Crouching", "Crouching name");
    assertTrue(systems::FPSCharacterControllerSystem::stanceName(2) == "Sprinting", "Sprinting name");
    assertTrue(systems::FPSCharacterControllerSystem::stanceName(99) == "Unknown", "Unknown stance");
}

static void testFPSCharControllerComponentDefaults() {
    std::cout << "\n=== FPS Character Controller Component Defaults ===" << std::endl;
    components::FPSCharacterState state;
    assertTrue(state.stance == 0, "Default stance standing");
    assertTrue(state.grounded, "Default grounded");
    assertTrue(approxEqual(state.walk_speed, 4.0f), "Default walk speed 4");
    assertTrue(approxEqual(state.sprint_speed, 7.0f), "Default sprint speed 7");
    assertTrue(approxEqual(state.gravity, 9.81f), "Default gravity 9.81");
    assertTrue(approxEqual(state.stamina, 100.0f), "Default stamina 100");
}


void run_fps_character_controller_system_tests() {
    testFPSCharControllerSpawn();
    testFPSCharControllerMovement();
    testFPSCharControllerSprint();
    testFPSCharControllerCrouch();
    testFPSCharControllerJump();
    testFPSCharControllerGravityLanding();
    testFPSCharControllerZeroG();
    testFPSCharControllerStaminaDrain();
    testFPSCharControllerStaminaExhaustion();
    testFPSCharControllerLookDirection();
    testFPSCharControllerRemove();
    testFPSCharControllerStanceNames();
    testFPSCharControllerComponentDefaults();
}
