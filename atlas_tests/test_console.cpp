#include "../editor/panels/ConsolePanel.h"
#include "../editor/ai/AIAggregator.h"
#include "../editor/ai/TemplateAIBackend.h"
#include "../engine/ecs/ECS.h"
#include "../engine/net/NetContext.h"
#include "../engine/sim/TickScheduler.h"
#include <iostream>
#include <cassert>
#include <string>

using namespace atlas::editor;
using namespace atlas::ecs;
using namespace atlas::net;
using namespace atlas::sim;

void test_console_spawn_entity() {
    World world;
    NetContext net;
    net.Init(NetMode::Standalone);
    TickScheduler scheduler;

    ConsolePanel console(world, net, scheduler);

    assert(world.EntityCount() == 0);
    console.Execute("spawn_entity");
    assert(world.EntityCount() == 1);

    // Check history
    auto& history = console.History();
    assert(history.size() == 2); // "> spawn_entity" and "Created entity N"
    assert(history[0] == "> spawn_entity");

}

void test_console_ecs_dump() {
    World world;
    NetContext net;
    net.Init(NetMode::Standalone);
    TickScheduler scheduler;

    ConsolePanel console(world, net, scheduler);

    world.CreateEntity();
    world.CreateEntity();

    console.Execute("ecs.dump");

    auto& history = console.History();
    // "> ecs.dump", "Entities: 2", "  Entity 1 (0 components)", "  Entity 2 (0 components)"
    assert(history.size() == 4);
    assert(history[1] == "Entities: 2");

}

void test_console_set_tickrate() {
    World world;
    NetContext net;
    net.Init(NetMode::Standalone);
    TickScheduler scheduler;
    scheduler.SetTickRate(30);

    ConsolePanel console(world, net, scheduler);

    console.Execute("set tickrate 60");

    assert(scheduler.TickRate() == 60);

    auto& history = console.History();
    assert(history.size() == 2);
    assert(history[1] == "Tick rate set to 60");

}

void test_console_net_mode() {
    World world;
    NetContext net;
    net.Init(NetMode::Server);
    TickScheduler scheduler;

    ConsolePanel console(world, net, scheduler);

    console.Execute("net.mode");

    auto& history = console.History();
    assert(history.size() == 2);
    assert(history[1] == "Net mode: Server");

}

void test_console_help() {
    World world;
    NetContext net;
    net.Init(NetMode::Standalone);
    TickScheduler scheduler;

    ConsolePanel console(world, net, scheduler);

    console.Execute("help");

    auto& history = console.History();
    assert(history.size() == 2);
    assert(history[0] == "> help");

}

void test_console_unknown_command() {
    World world;
    NetContext net;
    net.Init(NetMode::Standalone);
    TickScheduler scheduler;

    ConsolePanel console(world, net, scheduler);

    console.Execute("foobar");

    auto& history = console.History();
    assert(history.size() == 2);
    assert(history[1] == "Unknown command: foobar");

}

void test_console_ecs_count() {
    World world;
    NetContext net;
    net.Init(NetMode::Standalone);
    TickScheduler scheduler;

    ConsolePanel console(world, net, scheduler);
    world.CreateEntity();
    world.CreateEntity();
    world.CreateEntity();

    console.Execute("ecs.count");

    auto& history = console.History();
    assert(history.size() == 2);
    assert(history[1] == "Entity count: 3");
}

void test_console_ecs_destroy() {
    World world;
    NetContext net;
    net.Init(NetMode::Standalone);
    TickScheduler scheduler;

    ConsolePanel console(world, net, scheduler);
    auto e1 = world.CreateEntity();
    world.CreateEntity();

    assert(world.EntityCount() == 2);

    console.Execute("ecs.destroy " + std::to_string(e1));

    assert(world.EntityCount() == 1);
    auto& history = console.History();
    assert(history[1].find("Destroyed entity") != std::string::npos);
}

void test_console_ecs_destroy_invalid() {
    World world;
    NetContext net;
    net.Init(NetMode::Standalone);
    TickScheduler scheduler;

    ConsolePanel console(world, net, scheduler);

    console.Execute("ecs.destroy 999");

    auto& history = console.History();
    assert(history[1].find("Invalid or dead entity") != std::string::npos);
}

void test_console_net_stats() {
    World world;
    NetContext net;
    net.Init(NetMode::Server);
    net.AddPeer();
    TickScheduler scheduler;

    ConsolePanel console(world, net, scheduler);
    console.Execute("net.stats");

    auto& history = console.History();
    // Should have: "> net.stats", stats lines
    assert(history.size() >= 2);
    assert(history[1].find("Peers total:") != std::string::npos);
}

void test_console_net_peers() {
    World world;
    NetContext net;
    net.Init(NetMode::Server);
    net.AddPeer();
    net.AddPeer();
    TickScheduler scheduler;

    ConsolePanel console(world, net, scheduler);
    console.Execute("net.peers");

    auto& history = console.History();
    assert(history.size() >= 2);
    assert(history[1].find("Connected peers: 2") != std::string::npos);
}

void test_console_clear() {
    World world;
    NetContext net;
    net.Init(NetMode::Standalone);
    TickScheduler scheduler;

    ConsolePanel console(world, net, scheduler);
    console.AddLine("line1");
    console.AddLine("line2");
    assert(console.History().size() == 2);

    console.Execute("clear");
    assert(console.History().empty());
}

void test_console_ai_query() {
    World world;
    NetContext net;
    net.Init(NetMode::Standalone);
    TickScheduler scheduler;

    ConsolePanel console(world, net, scheduler);

    // Without AI backend connected
    console.Execute("ai.query frigate suggestions");
    auto& h1 = console.History();
    assert(h1.back() == "AI backend not connected");

    // Connect AI backend
    atlas::ai::AIAggregator agg;
    atlas::ai::TemplateAIBackend templateAI;
    agg.RegisterBackend(&templateAI);
    console.SetAIAggregator(&agg);

    console.Execute("ai.query frigate");
    auto& h2 = console.History();
    // Should have [AI] response and [AI] Confidence lines
    bool foundResponse = false;
    for (auto& line : h2) {
        if (line.find("[AI]") != std::string::npos && line.find("frigate") != std::string::npos) {
            foundResponse = true;
            break;
        }
    }
    assert(foundResponse);
}

void test_console_ai_query_empty() {
    World world;
    NetContext net;
    net.Init(NetMode::Standalone);
    TickScheduler scheduler;

    ConsolePanel console(world, net, scheduler);
    console.Execute("ai.query");

    auto& history = console.History();
    assert(history.back() == "Usage: ai.query <prompt>");
}

void test_console_list_command() {
    World world;
    NetContext net;
    net.Init(NetMode::Standalone);
    TickScheduler scheduler;

    ConsolePanel console(world, net, scheduler);
    console.Execute("list");

    auto& history = console.History();
    assert(history.size() >= 2);
    assert(history[1].find("Available panels") != std::string::npos);
}

void test_console_status_command() {
    World world;
    NetContext net;
    net.Init(NetMode::Standalone);
    TickScheduler scheduler;

    ConsolePanel console(world, net, scheduler);
    world.CreateEntity();
    world.CreateEntity();

    console.Execute("status");

    auto& history = console.History();
    bool foundEntities = false;
    bool foundNetMode = false;
    for (auto& line : history) {
        if (line.find("Entities: 2") != std::string::npos) foundEntities = true;
        if (line.find("Net Mode: Standalone") != std::string::npos) foundNetMode = true;
    }
    assert(foundEntities);
    assert(foundNetMode);
}
