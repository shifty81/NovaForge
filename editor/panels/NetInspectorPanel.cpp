#include "NetInspectorPanel.h"
#include <algorithm>
#include <numeric>

namespace atlas::editor {

void NetInspectorPanel::Draw() {
    auto stats = GetStats();

    // Auto-clear selection if the selected peer is no longer connected
    if (m_selectedPeer != 0) {
        auto& peers = m_net.Peers();
        auto it = std::find_if(peers.begin(), peers.end(),
            [this](const net::NetPeer& p) { return p.id == m_selectedPeer && p.connected; });
        if (it == peers.end()) {
            m_selectedPeer = 0;
        }
    }

    if (!GetContext()) return;

    auto& ctx = *GetContext();
    ApplyDockBounds(m_panelState);
    if (!atlas::panelBeginStateful(ctx, "Network", m_panelState)) {
        atlas::panelEnd(ctx);
        return;
    }

    const float pad     = ctx.theme().padding;
    const float rowH    = ctx.theme().rowHeight;
    const atlas::Rect& b = m_panelState.bounds;
    const float headerH = ctx.theme().headerHeight;
    float y = b.y + headerH + pad;

    // Network mode
    atlas::label(ctx, {b.x + pad, y}, "Mode: " + stats.modeString, ctx.theme().textPrimary);
    y += rowH + pad;

    // Connected peers
    atlas::label(ctx, {b.x + pad, y}, "Peers: " + std::to_string(stats.connectedPeerCount), ctx.theme().textPrimary);
    y += rowH + pad;

    // RTT stats
    atlas::label(ctx, {b.x + pad, y},
        "Avg RTT: " + std::to_string(static_cast<int>(stats.avgRTT)) + " ms"
        + "  Max RTT: " + std::to_string(static_cast<int>(stats.maxRTT)) + " ms",
        ctx.theme().textSecondary);
    y += rowH + pad;

    atlas::separator(ctx, {b.x + pad, y}, b.w - 2.0f * pad);
    y += pad;

    // Peer list
    auto& peers = m_net.Peers();
    for (auto& p : peers) {
        if (!p.connected) continue;
        std::string peerLabel = "Peer " + std::to_string(p.id)
            + " (RTT: " + std::to_string(static_cast<int>(p.rtt)) + " ms)";
        atlas::Rect row{b.x + pad, y, b.w - 2.0f * pad, rowH};
        if (atlas::button(ctx, peerLabel.c_str(), row)) {
            m_selectedPeer = p.id;
        }
        y += rowH + pad;
    }

    // Clear selection button
    if (m_selectedPeer != 0) {
        atlas::Rect clearBtn{b.x + pad, y, 120.0f, rowH + pad};
        if (atlas::button(ctx, "Clear Selection", clearBtn)) {
            m_selectedPeer = 0;
        }
    }

    atlas::panelEnd(ctx);
}

std::string NetInspectorPanel::ModeToString(net::NetMode mode) {
    switch (mode) {
        case net::NetMode::Standalone: return "Standalone";
        case net::NetMode::Client:     return "Client";
        case net::NetMode::Server:     return "Server";
        case net::NetMode::P2P_Host:   return "P2P_Host";
        case net::NetMode::P2P_Peer:   return "P2P_Peer";
        default:                       return "Unknown";
    }
}

void NetInspectorPanel::SelectPeer(uint32_t id) {
    auto& peers = m_net.Peers();
    auto it = std::find_if(peers.begin(), peers.end(),
        [id](const net::NetPeer& p) { return p.id == id && p.connected; });
    if (it != peers.end()) {
        m_selectedPeer = id;
    }
}

NetInspectorPanel::NetworkStats NetInspectorPanel::GetStats() const {
    NetworkStats stats;
    stats.modeString = ModeToString(m_net.Mode());

    auto& peers = m_net.Peers();
    float totalRTT = 0.0f;
    for (auto& p : peers) {
        if (p.connected) {
            ++stats.connectedPeerCount;
            totalRTT += p.rtt;
            if (p.rtt > stats.maxRTT) {
                stats.maxRTT = p.rtt;
            }
        }
    }
    if (stats.connectedPeerCount > 0) {
        stats.avgRTT = totalRTT / static_cast<float>(stats.connectedPeerCount);
    }
    return stats;
}

}
