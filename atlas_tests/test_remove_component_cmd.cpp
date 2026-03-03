/**
 * Tests for RemoveComponentCommand<T>.
 *
 * Validates:
 * - Removes an existing component and captures previous value
 * - No-op when component does not exist
 * - Description string
 * - Integration with EditorCommandBus
 * - TargetID accessor
 */

#include "../engine/ecs/EntityCommands.h"
#include "../engine/ecs/ECS.h"
#include "../cpp_client/include/editor/editor_command_bus.h"
#include <cassert>
#include <string>
#include <memory>

using namespace atlas::ecs;
using namespace atlas::editor;

// ── Test components (local to this TU) ──────────────────────────────

namespace {
struct Armor { float value = 50.0f; };
struct Shield { float value = 200.0f; };
} // namespace

// ── Tests ───────────────────────────────────────────────────────────

void test_remove_component_cmd_removes() {
    World world;
    EntityID eid = world.CreateEntity();
    world.AddComponent<Armor>(eid, Armor{75.0f});
    assert(world.HasComponent<Armor>(eid));

    RemoveComponentCommand<Armor> cmd(world, eid);
    cmd.Execute();

    assert(!world.HasComponent<Armor>(eid));
    assert(cmd.HadPrevious());
    assert(cmd.PreviousValue().value > 74.9f && cmd.PreviousValue().value < 75.1f);
}

void test_remove_component_cmd_no_component() {
    World world;
    EntityID eid = world.CreateEntity();

    RemoveComponentCommand<Armor> cmd(world, eid);
    cmd.Execute();

    assert(!cmd.HadPrevious());
}

void test_remove_component_cmd_description() {
    World world;
    RemoveComponentCommand<Shield> cmd(world, 1);
    assert(std::string(cmd.Description()) == "Remove Component");
}

void test_remove_component_cmd_via_bus() {
    World world;
    EntityID eid = world.CreateEntity();
    world.AddComponent<Shield>(eid, Shield{150.0f});

    EditorCommandBus bus;
    bus.PostCommand(std::make_unique<RemoveComponentCommand<Shield>>(world, eid));
    bus.ProcessCommands();

    assert(!world.HasComponent<Shield>(eid));
}

void test_remove_component_cmd_target_id() {
    World world;
    EntityID eid = world.CreateEntity();
    RemoveComponentCommand<Armor> cmd(world, eid);
    assert(cmd.TargetID() == eid);
}
