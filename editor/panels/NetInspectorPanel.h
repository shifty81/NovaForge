#pragma once
#include "../ui/EditorPanel.h"
#include "../../engine/net/NetContext.h"
#include <string>

namespace atlas::editor {

class NetInspectorPanel : public EditorPanel {
public:
    struct NetworkStats {
        size_t connectedPeerCount = 0;
        float avgRTT = 0.0f;
        float maxRTT = 0.0f;
        std::string modeString;
    };

    explicit NetInspectorPanel(net::NetContext& net) : m_net(net) {}

    const char* Name() const override { return "Network"; }
    void Draw() override;

    uint32_t SelectedPeer() const { return m_selectedPeer; }
    void SelectPeer(uint32_t id);
    void ClearPeerSelection() { m_selectedPeer = 0; }
    NetworkStats GetStats() const;

    static std::string ModeToString(net::NetMode mode);

private:
    net::NetContext& m_net;
    uint32_t m_selectedPeer = 0;
};

}
