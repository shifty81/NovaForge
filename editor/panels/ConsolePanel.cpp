#include "ConsolePanel.h"
#include <sstream>

namespace atlas::editor {

void ConsolePanel::Draw() {
    if (!GetContext()) return;

    auto& ctx = *GetContext();
    if (!panelBeginStateful(ctx, "Console", m_panelState))
    {
        panelEnd(ctx);
        return;
    }

    const float pad    = ctx.theme().padding;
    const float rowH   = ctx.theme().rowHeight;
    const Rect& b      = m_panelState.bounds;
    const float headerH = ctx.theme().headerHeight;

    // Layout constants
    const float inputH   = rowH + pad;
    const float buttonW  = 60.0f;
    const float bottomY  = b.y + b.h - pad - inputH;

    // History area (below header, above input row)
    Rect logRect{b.x + pad, b.y + headerH + pad,
                 b.w - 2.0f * pad,
                 bottomY - (b.y + headerH + pad) - pad};
    combatLogWidget(ctx, logRect, m_history, m_scrollOffset);

    // Text input field (bottom-left, leaving room for button)
    Rect inputRect{b.x + pad, bottomY,
                   b.w - 3.0f * pad - buttonW, inputH};
    bool submitted = textInput(ctx, "##console_input", inputRect,
                               m_inputState, "Enter command...");

    // Execute button (bottom-right)
    Rect btnRect{inputRect.x + inputRect.w + pad, bottomY,
                 buttonW, inputH};
    bool clicked = button(ctx, "Execute", btnRect);

    if ((submitted || clicked) && !m_inputState.text.empty()) {
        Execute(m_inputState.text);
        m_inputState.text.clear();
        m_inputState.cursorPos = 0;
    }

    panelEnd(ctx);
}

void ConsolePanel::AddLine(const std::string& line) {
    m_history.push_back(line);
}

void ConsolePanel::Execute(const std::string& command) {
    m_history.push_back("> " + command);

    std::istringstream iss(command);
    std::string cmd;
    iss >> cmd;

    if (cmd == "spawn_entity") {
        auto id = m_world.CreateEntity();
        m_history.push_back("Created entity " + std::to_string(id));
    } else if (cmd == "ecs.dump") {
        auto entities = m_world.GetEntities();
        m_history.push_back("Entities: " + std::to_string(entities.size()));
        for (auto e : entities) {
            auto types = m_world.GetComponentTypes(e);
            m_history.push_back("  Entity " + std::to_string(e) + " (" + std::to_string(types.size()) + " components)");
        }
    } else if (cmd == "ecs.count") {
        m_history.push_back("Entity count: " + std::to_string(m_world.EntityCount()));
    } else if (cmd == "ecs.destroy") {
        uint64_t id = 0;
        iss >> id;
        if (id > 0 && m_world.IsAlive(id)) {
            m_world.DestroyEntity(id);
            m_history.push_back("Destroyed entity " + std::to_string(id));
        } else {
            m_history.push_back("Invalid or dead entity: " + std::to_string(id));
        }
    } else if (cmd == "set") {
        std::string key;
        iss >> key;
        if (key == "tickrate") {
            uint32_t rate = 0;
            iss >> rate;
            if (rate > 0) {
                m_scheduler.SetTickRate(rate);
                m_history.push_back("Tick rate set to " + std::to_string(rate));
            } else {
                m_history.push_back("Invalid tick rate");
            }
        } else {
            m_history.push_back("Unknown setting: " + key);
        }
    } else if (cmd == "net.mode") {
        auto mode = m_net.Mode();
        std::string modeStr;
        switch (mode) {
            case net::NetMode::Standalone: modeStr = "Standalone"; break;
            case net::NetMode::Client: modeStr = "Client"; break;
            case net::NetMode::Server: modeStr = "Server"; break;
            case net::NetMode::P2P_Host: modeStr = "P2P_Host"; break;
            case net::NetMode::P2P_Peer: modeStr = "P2P_Peer"; break;
        }
        m_history.push_back("Net mode: " + modeStr);
    } else if (cmd == "net.stats") {
        auto& peers = m_net.Peers();
        size_t connected = 0;
        for (const auto& p : peers) {
            if (p.connected) ++connected;
        }
        m_history.push_back("Peers total:     " + std::to_string(peers.size()));
        m_history.push_back("Peers connected: " + std::to_string(connected));
        m_history.push_back("Authority:       " + std::string(m_net.IsAuthority() ? "yes" : "no"));
    } else if (cmd == "net.peers") {
        auto& peers = m_net.Peers();
        m_history.push_back("Connected peers: " + std::to_string(peers.size()));
        for (const auto& p : peers) {
            std::ostringstream oss;
            oss << "  Peer " << p.id
                << (p.connected ? " [connected]" : " [disconnected]")
                << " rtt=" << p.rtt << "ms";
            m_history.push_back(oss.str());
        }
    } else if (cmd == "clear") {
        m_history.clear();
    } else if (cmd == "ai.query") {
        // Collect the rest of the line as the prompt
        std::string prompt;
        std::getline(iss, prompt);
        // Trim leading whitespace
        size_t start = prompt.find_first_not_of(" \t");
        if (start != std::string::npos) prompt = prompt.substr(start);

        if (prompt.empty()) {
            m_history.push_back("Usage: ai.query <prompt>");
        } else if (!m_ai) {
            m_history.push_back("AI backend not connected");
        } else {
            atlas::ai::AIContext ctx;
            auto resp = m_ai->Execute(atlas::ai::AIRequestType::Analysis,
                                       prompt, ctx);
            if (resp.confidence > 0.0f) {
                m_history.push_back("[AI] " + resp.content);
                std::ostringstream oss;
                oss << "[AI] Confidence: " << static_cast<int>(resp.confidence * 100.0f) << "%";
                m_history.push_back(oss.str());
            } else {
                m_history.push_back("[AI] No relevant response found");
            }
        }
    } else if (cmd == "help") {
        m_history.push_back("Commands: spawn_entity, ecs.dump, ecs.count, "
                            "ecs.destroy <id>, set tickrate <N>, net.mode, "
                            "net.stats, net.peers, ai.query <prompt>, clear, help");
    } else {
        m_history.push_back("Unknown command: " + cmd);
    }
}

}
