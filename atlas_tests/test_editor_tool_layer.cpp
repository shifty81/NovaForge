/**
 * Tests for the editor tool layer infrastructure:
 *   - EditorCommandBus: post, process, pending count
 *   - EditorToolLayer: init, shutdown, toggle, panel count (headless)
 */

#include <cassert>
#include <string>
#include <memory>
#include "../cpp_client/include/editor/editor_command_bus.h"
#include "../cpp_client/include/editor/itool.h"

using namespace atlas::editor;

// ══════════════════════════════════════════════════════════════════
// EditorCommandBus tests
// ══════════════════════════════════════════════════════════════════

void test_cmdbus_empty_by_default() {
    EditorCommandBus bus;
    assert(bus.PendingCount() == 0);
}

void test_cmdbus_post_increments_count() {
    EditorCommandBus bus;
    struct NoopCmd : ICommand {
        void Execute() override {}
        const char* Description() const override { return "noop"; }
    };
    bus.PostCommand(std::make_unique<NoopCmd>());
    assert(bus.PendingCount() == 1);
    bus.PostCommand(std::make_unique<NoopCmd>());
    assert(bus.PendingCount() == 2);
}

void test_cmdbus_process_drains_queue() {
    EditorCommandBus bus;
    int counter = 0;
    struct CountCmd : ICommand {
        int* c;
        explicit CountCmd(int* c) : c(c) {}
        void Execute() override { ++(*c); }
        const char* Description() const override { return "count"; }
    };
    bus.PostCommand(std::make_unique<CountCmd>(&counter));
    bus.PostCommand(std::make_unique<CountCmd>(&counter));
    bus.PostCommand(std::make_unique<CountCmd>(&counter));
    assert(bus.PendingCount() == 3);
    bus.ProcessCommands();
    assert(bus.PendingCount() == 0);
    assert(counter == 3);
}

void test_cmdbus_process_empty_is_safe() {
    EditorCommandBus bus;
    bus.ProcessCommands(); // should not crash
    assert(bus.PendingCount() == 0);
}

void test_cmdbus_post_null_ignored() {
    EditorCommandBus bus;
    bus.PostCommand(nullptr);
    assert(bus.PendingCount() == 0);
}

void test_cmdbus_fifo_order() {
    EditorCommandBus bus;
    std::string order;
    struct AppendCmd : ICommand {
        std::string* s;
        char ch;
        AppendCmd(std::string* s, char ch) : s(s), ch(ch) {}
        void Execute() override { s->push_back(ch); }
        const char* Description() const override { return "append"; }
    };
    bus.PostCommand(std::make_unique<AppendCmd>(&order, 'A'));
    bus.PostCommand(std::make_unique<AppendCmd>(&order, 'B'));
    bus.PostCommand(std::make_unique<AppendCmd>(&order, 'C'));
    bus.ProcessCommands();
    assert(order == "ABC");
}

void test_cmdbus_description() {
    struct DescCmd : ICommand {
        void Execute() override {}
        const char* Description() const override { return "test description"; }
    };
    DescCmd cmd;
    assert(std::string(cmd.Description()) == "test description");
}

// ══════════════════════════════════════════════════════════════════
// ITool interface tests (concrete mock)
// ══════════════════════════════════════════════════════════════════

class MockTool : public ITool {
public:
    const char* Name() const override { return "MockTool"; }
    void Activate() override { m_active = true; ++activateCount; }
    void Deactivate() override { m_active = false; ++deactivateCount; }
    void Update(float dt) override { totalDt += dt; ++updateCount; }
    bool IsActive() const override { return m_active; }

    int activateCount = 0;
    int deactivateCount = 0;
    int updateCount = 0;
    float totalDt = 0.0f;

private:
    bool m_active = false;
};

void test_itool_defaults() {
    MockTool tool;
    assert(!tool.IsActive());
    assert(std::string(tool.Name()) == "MockTool");
    assert(tool.activateCount == 0);
}

void test_itool_activate_deactivate() {
    MockTool tool;
    tool.Activate();
    assert(tool.IsActive());
    assert(tool.activateCount == 1);
    tool.Deactivate();
    assert(!tool.IsActive());
    assert(tool.deactivateCount == 1);
}

void test_itool_update() {
    MockTool tool;
    tool.Activate();
    tool.Update(0.016f);
    tool.Update(0.016f);
    assert(tool.updateCount == 2);
    assert(tool.totalDt > 0.03f);
}
