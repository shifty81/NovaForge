/**
 * Tests for the ISystem / SystemRegistry abstraction.
 *
 * Validates:
 * - Registration, counting, and lookup by name
 * - Priority-ordered execution
 * - Enable/disable
 * - Init and Shutdown lifecycle
 * - Unregister
 * - Null safety
 */

#include "../engine/ecs/System.h"
#include "../engine/ecs/ECS.h"
#include <cassert>
#include <string>
#include <vector>

using namespace atlas::ecs;

// ── Helper systems ──────────────────────────────────────────────────

class RecordingSystem : public ISystem {
public:
    RecordingSystem(const char* name, int priority, std::vector<std::string>& log)
        : m_name(name), m_priority(priority), m_log(log) {}

    const char* Name() const override { return m_name; }
    int Priority() const override { return m_priority; }

    void Init(World&) override { m_log.push_back(std::string(m_name) + ":init"); }
    void Execute(World&, float dt) override {
        m_log.push_back(std::string(m_name) + ":exec");
        m_totalDt += dt;
    }
    void Shutdown(World&) override { m_log.push_back(std::string(m_name) + ":shutdown"); }

    float m_totalDt = 0.0f;
private:
    const char* m_name;
    int m_priority;
    std::vector<std::string>& m_log;
};

// ── Tests ───────────────────────────────────────────────────────────

void test_sysreg_empty_by_default() {
    SystemRegistry reg;
    assert(reg.Count() == 0);
}

void test_sysreg_register_increments_count() {
    SystemRegistry reg;
    std::vector<std::string> log;
    reg.Register(std::make_unique<RecordingSystem>("A", 100, log));
    assert(reg.Count() == 1);
    reg.Register(std::make_unique<RecordingSystem>("B", 100, log));
    assert(reg.Count() == 2);
}

void test_sysreg_register_null_ignored() {
    SystemRegistry reg;
    reg.Register(nullptr);
    assert(reg.Count() == 0);
}

void test_sysreg_find_by_name() {
    SystemRegistry reg;
    std::vector<std::string> log;
    reg.Register(std::make_unique<RecordingSystem>("Physics", 10, log));
    reg.Register(std::make_unique<RecordingSystem>("AI", 50, log));

    assert(reg.FindByName("Physics") != nullptr);
    assert(reg.FindByName("AI") != nullptr);
    assert(reg.FindByName("Nonexistent") == nullptr);
    assert(reg.FindByName(nullptr) == nullptr);
}

void test_sysreg_priority_order() {
    SystemRegistry reg;
    std::vector<std::string> log;
    // Register out of priority order.
    reg.Register(std::make_unique<RecordingSystem>("C", 300, log));
    reg.Register(std::make_unique<RecordingSystem>("A", 100, log));
    reg.Register(std::make_unique<RecordingSystem>("B", 200, log));

    World world;
    reg.UpdateAll(world, 0.016f);

    assert(log.size() == 3);
    assert(log[0] == "A:exec");
    assert(log[1] == "B:exec");
    assert(log[2] == "C:exec");
}

void test_sysreg_init_all() {
    SystemRegistry reg;
    std::vector<std::string> log;
    reg.Register(std::make_unique<RecordingSystem>("X", 50, log));
    reg.Register(std::make_unique<RecordingSystem>("Y", 10, log));

    World world;
    reg.InitAll(world);

    // Init should follow priority order (Y before X).
    assert(log.size() == 2);
    assert(log[0] == "Y:init");
    assert(log[1] == "X:init");
}

void test_sysreg_shutdown_all() {
    SystemRegistry reg;
    std::vector<std::string> log;
    reg.Register(std::make_unique<RecordingSystem>("A", 10, log));
    reg.Register(std::make_unique<RecordingSystem>("B", 20, log));

    World world;
    reg.InitAll(world);
    log.clear();

    reg.ShutdownAll(world);

    // Shutdown in reverse priority order (B before A).
    assert(log.size() == 2);
    assert(log[0] == "B:shutdown");
    assert(log[1] == "A:shutdown");
}

void test_sysreg_disabled_system_skipped() {
    SystemRegistry reg;
    std::vector<std::string> log;
    reg.Register(std::make_unique<RecordingSystem>("Enabled", 100, log));
    reg.Register(std::make_unique<RecordingSystem>("Disabled", 200, log));

    // Disable second system
    auto* sys = reg.FindByName("Disabled");
    sys->SetEnabled(false);

    World world;
    reg.UpdateAll(world, 0.016f);

    assert(log.size() == 1);
    assert(log[0] == "Enabled:exec");
}

void test_sysreg_unregister() {
    SystemRegistry reg;
    std::vector<std::string> log;
    reg.Register(std::make_unique<RecordingSystem>("Keep", 100, log));
    reg.Register(std::make_unique<RecordingSystem>("Remove", 200, log));

    auto* toRemove = reg.FindByName("Remove");
    assert(reg.Unregister(toRemove));
    assert(reg.Count() == 1);
    assert(reg.FindByName("Remove") == nullptr);

    // Unregister non-existent returns false.
    assert(!reg.Unregister(nullptr));
    assert(!reg.Unregister(toRemove));  // already removed
}

void test_sysreg_update_passes_dt() {
    SystemRegistry reg;
    std::vector<std::string> log;
    auto sys = std::make_unique<RecordingSystem>("Ticker", 100, log);
    auto* ptr = sys.get();
    reg.Register(std::move(sys));

    World world;
    reg.UpdateAll(world, 0.5f);
    reg.UpdateAll(world, 0.25f);

    assert(ptr->m_totalDt > 0.74f && ptr->m_totalDt < 0.76f);
}

void test_sysreg_init_idempotent() {
    SystemRegistry reg;
    std::vector<std::string> log;
    reg.Register(std::make_unique<RecordingSystem>("Once", 100, log));

    World world;
    reg.InitAll(world);
    reg.InitAll(world);  // Second call should not re-init.

    assert(log.size() == 1);
    assert(log[0] == "Once:init");
}
