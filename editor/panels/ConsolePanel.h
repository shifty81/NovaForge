#pragma once
#include "../ui/EditorPanel.h"
#include "../../engine/ecs/ECS.h"
#include "../../engine/net/NetContext.h"
#include "../../engine/sim/TickScheduler.h"
#include "../../cpp_client/include/ui/atlas/atlas_widgets.h"
#include "../ai/AIAggregator.h"
#include <string>
#include <vector>
#include <sstream>

namespace atlas::editor {

class ConsolePanel : public EditorPanel {
public:
    ConsolePanel(ecs::World& world, net::NetContext& net, sim::TickScheduler& scheduler)
        : m_world(world), m_net(net), m_scheduler(scheduler) {}

    const char* Name() const override { return "Console"; }
    void Draw() override;

    void AddLine(const std::string& line);
    void Execute(const std::string& command);

    const std::vector<std::string>& History() const { return m_history; }

    /** Attach an AI aggregator so that `ai.query` commands work. */
    void SetAIAggregator(atlas::ai::AIAggregator* agg) { m_ai = agg; }

private:
    std::vector<std::string> m_history;
    std::string m_inputBuffer;
    ecs::World& m_world;
    net::NetContext& m_net;
    sim::TickScheduler& m_scheduler;
    atlas::ai::AIAggregator* m_ai = nullptr;

    atlas::PanelState     m_panelState;
    atlas::TextInputState m_inputState;
    float                 m_scrollOffset = 0.0f;
};

}
