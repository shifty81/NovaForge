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
