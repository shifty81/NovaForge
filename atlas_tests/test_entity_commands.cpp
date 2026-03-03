/**
 * Tests for concrete ECS editor commands:
 *   - CreateEntityCommand
 *   - DestroyEntityCommand
 *   - SetComponentCommand
 *
 * Validates that commands execute correctly against the ECS World and
 * integrate with the EditorCommandBus.
 */

#include "../engine/ecs/EntityCommands.h"
#include "../engine/ecs/ECS.h"
#include "../cpp_client/include/editor/editor_command_bus.h"
#include <cassert>
#include <string>
#include <memory>

using namespace atlas::ecs;
using namespace atlas::editor;

// ── Test component ──────────────────────────────────────────────────

struct Health {
    float hp = 100.0f;
};

struct Name {
    std::string value;
};

// ── CreateEntityCommand tests ───────────────────────────────────────

void test_create_entity_cmd_executes() {
    World world;
    assert(world.EntityCount() == 0);

    CreateEntityCommand cmd(world);
    cmd.Execute();

    assert(world.EntityCount() == 1);
    assert(cmd.CreatedID() != 0);
    assert(world.IsAlive(cmd.CreatedID()));
}

void test_create_entity_cmd_callback() {
    World world;
    EntityID received = 0;
    CreateEntityCommand cmd(world, [&](EntityID id) { received = id; });
    cmd.Execute();

    assert(received != 0);
    assert(received == cmd.CreatedID());
}

void test_create_entity_cmd_description() {
    World world;
    CreateEntityCommand cmd(world);
    assert(std::string(cmd.Description()) == "Create Entity");
}

void test_create_entity_cmd_via_bus() {
    World world;
    EditorCommandBus bus;

    EntityID createdID = 0;
    bus.PostCommand(std::make_unique<CreateEntityCommand>(
        world, [&](EntityID id) { createdID = id; }));
    bus.ProcessCommands();

    assert(world.EntityCount() == 1);
    assert(createdID != 0);
}

// ── DestroyEntityCommand tests ──────────────────────────────────────

void test_destroy_entity_cmd_executes() {
    World world;
    EntityID eid = world.CreateEntity();
    assert(world.IsAlive(eid));

    DestroyEntityCommand cmd(world, eid);
    cmd.Execute();

    assert(!world.IsAlive(eid));
    assert(cmd.WasAlive());
}

void test_destroy_entity_cmd_nonexistent() {
    World world;
    DestroyEntityCommand cmd(world, 999);
    cmd.Execute();
    assert(!cmd.WasAlive());
}

void test_destroy_entity_cmd_description() {
    World world;
    DestroyEntityCommand cmd(world, 1);
    assert(std::string(cmd.Description()) == "Destroy Entity");
}

void test_destroy_entity_cmd_via_bus() {
    World world;
    EntityID eid = world.CreateEntity();
    EditorCommandBus bus;
    bus.PostCommand(std::make_unique<DestroyEntityCommand>(world, eid));
    bus.ProcessCommands();
    assert(!world.IsAlive(eid));
}

// ── SetComponentCommand tests ───────────────────────────────────────

void test_set_component_cmd_adds_new() {
    World world;
    EntityID eid = world.CreateEntity();

    SetComponentCommand<Health> cmd(world, eid, Health{75.0f});
    cmd.Execute();

    Health* h = world.GetComponent<Health>(eid);
    assert(h != nullptr);
    assert(h->hp > 74.9f && h->hp < 75.1f);
    assert(!cmd.HadPrevious());
}

void test_set_component_cmd_replaces_existing() {
    World world;
    EntityID eid = world.CreateEntity();
    world.AddComponent<Health>(eid, Health{100.0f});

    SetComponentCommand<Health> cmd(world, eid, Health{50.0f});
    cmd.Execute();

    Health* h = world.GetComponent<Health>(eid);
    assert(h != nullptr);
    assert(h->hp > 49.9f && h->hp < 50.1f);
    assert(cmd.HadPrevious());
    assert(cmd.PreviousValue().hp > 99.9f && cmd.PreviousValue().hp < 100.1f);
}

void test_set_component_cmd_description() {
    World world;
    SetComponentCommand<Health> cmd(world, 1, Health{});
    assert(std::string(cmd.Description()) == "Set Component");
}

void test_set_component_cmd_via_bus() {
    World world;
    EntityID eid = world.CreateEntity();
    EditorCommandBus bus;
    bus.PostCommand(std::make_unique<SetComponentCommand<Name>>(world, eid, Name{"Ship01"}));
    bus.ProcessCommands();

    Name* n = world.GetComponent<Name>(eid);
    assert(n != nullptr);
    assert(n->value == "Ship01");
}

void test_set_component_cmd_target_id() {
    World world;
    EntityID eid = world.CreateEntity();
    SetComponentCommand<Health> cmd(world, eid, Health{});
    assert(cmd.TargetID() == eid);
}

// ── RemoveComponentCommand tests ────────────────────────────────────

void test_remove_component_cmd_removes() {
    World world;
    EntityID eid = world.CreateEntity();
    world.AddComponent<Health>(eid, Health{80.0f});

    RemoveComponentCommand<Health> cmd(world, eid);
    cmd.Execute();

    assert(!world.HasComponent<Health>(eid));
    assert(cmd.HadComponent());
    assert(cmd.PreviousValue().hp > 79.9f && cmd.PreviousValue().hp < 80.1f);
}

void test_remove_component_cmd_nonexistent() {
    World world;
    EntityID eid = world.CreateEntity();

    RemoveComponentCommand<Health> cmd(world, eid);
    cmd.Execute();

    assert(!cmd.HadComponent());
}

void test_remove_component_cmd_description() {
    World world;
    RemoveComponentCommand<Health> cmd(world, 1);
    assert(std::string(cmd.Description()) == "Remove Component");
}

void test_remove_component_cmd_via_bus() {
    World world;
    EntityID eid = world.CreateEntity();
    world.AddComponent<Name>(eid, Name{"TestShip"});

    EditorCommandBus bus;
    bus.PostCommand(std::make_unique<RemoveComponentCommand<Name>>(world, eid));
    bus.ProcessCommands();

    assert(!world.HasComponent<Name>(eid));
}

void test_remove_component_cmd_target_id() {
    World world;
    EntityID eid = world.CreateEntity();
    RemoveComponentCommand<Health> cmd(world, eid);
    assert(cmd.TargetID() == eid);
}
