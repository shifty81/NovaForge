#include "EditorLayout.h"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <filesystem>

namespace atlas::editor {

void EditorLayout::RegisterPanel(EditorPanel* panel) {
    m_panels.push_back(panel);
    m_menuBar.RegisterPanel(panel);
}

void EditorLayout::SetContext(atlas::AtlasContext* ctx) {
    m_ctx = ctx;
    for (auto* panel : m_panels) {
        if (panel) panel->SetContext(ctx);
    }
}

void EditorLayout::Draw() {
    // Draw the menu bar at the top of the editor window.
    // The menu bar consumes MenuBarHeight pixels from the top; dock panels
    // render below it.  In headless mode (no context) this still returns
    // the height so tests can verify the offset.
    float windowW = 1600.0f;
    if (m_ctx) {
        windowW = static_cast<float>(m_ctx->input().windowW);
    }
    m_menuBar.Draw(m_ctx, windowW);

    DrawNode(m_root);
}

void EditorLayout::BroadcastAssetReload(const std::string& assetId,
                                         const std::string& path) {
    for (auto* panel : m_panels) {
        if (panel) panel->OnAssetReloaded(assetId, path);
    }
}

void EditorLayout::DrawNode(DockNode& node) {
    if (node.split == DockSplit::Tab) {
        if (!node.tabs.empty()) {
            int idx = std::clamp(node.activeTab, 0, static_cast<int>(node.tabs.size()) - 1);
            if (node.tabs[idx] && node.tabs[idx]->IsVisible()) {
                node.tabs[idx]->Draw();
            }
        }
        return;
    }

    if (node.split == DockSplit::None) {
        if (node.panel && node.panel->IsVisible()) {
            node.panel->Draw();
        }
        return;
    }

    if (node.a) DrawNode(*node.a);
    if (node.b) DrawNode(*node.b);
}

// ── Serialisation helpers ──────────────────────────────────────────

static const char* splitName(DockSplit s) {
    switch (s) {
        case DockSplit::None:       return "None";
        case DockSplit::Horizontal: return "Horizontal";
        case DockSplit::Vertical:   return "Vertical";
        case DockSplit::Tab:        return "Tab";
    }
    return "None";
}

static DockSplit splitFromName(const std::string& name) {
    if (name == "Horizontal") return DockSplit::Horizontal;
    if (name == "Vertical")   return DockSplit::Vertical;
    if (name == "Tab")        return DockSplit::Tab;
    return DockSplit::None;
}

static std::string indent(int level) {
    return std::string(static_cast<size_t>(level) * 2, ' ');
}

std::string EditorLayout::serializeNode(const DockNode& node, int ind) const {
    std::ostringstream os;
    os << indent(ind) << "{\n";
    os << indent(ind + 1) << "\"split\": \"" << splitName(node.split) << "\",\n";
    os << indent(ind + 1) << "\"splitRatio\": " << node.splitRatio << ",\n";
    os << indent(ind + 1) << "\"activeTab\": " << node.activeTab;

    if (node.panel) {
        os << ",\n" << indent(ind + 1) << "\"panel\": \"" << node.panel->Name() << "\"";
    }

    if (!node.tabs.empty()) {
        os << ",\n" << indent(ind + 1) << "\"tabs\": [";
        for (size_t i = 0; i < node.tabs.size(); ++i) {
            os << "\"" << (node.tabs[i] ? node.tabs[i]->Name() : "") << "\"";
            if (i + 1 < node.tabs.size()) os << ", ";
        }
        os << "]";
    }

    if (node.a) {
        os << ",\n" << indent(ind + 1) << "\"a\":\n" << serializeNode(*node.a, ind + 1);
    }
    if (node.b) {
        os << ",\n" << indent(ind + 1) << "\"b\":\n" << serializeNode(*node.b, ind + 1);
    }

    os << "\n" << indent(ind) << "}";
    return os.str();
}

EditorPanel* EditorLayout::findPanelByName(const std::string& name) const {
    for (auto* p : m_panels) {
        if (p && name == p->Name()) return p;
    }
    return nullptr;
}

std::string EditorLayout::extractString(const std::string& json, const std::string& key,
                                         size_t searchStart, size_t searchEnd) {
    std::string needle = "\"" + key + "\"";
    size_t pos = json.find(needle, searchStart);
    if (pos == std::string::npos || pos >= searchEnd) return "";
    size_t colon = json.find(':', pos + needle.size());
    if (colon == std::string::npos || colon >= searchEnd) return "";
    size_t q1 = json.find('\"', colon + 1);
    if (q1 == std::string::npos || q1 >= searchEnd) return "";
    size_t q2 = json.find('\"', q1 + 1);
    if (q2 == std::string::npos || q2 > searchEnd) return "";
    return json.substr(q1 + 1, q2 - q1 - 1);
}

float EditorLayout::extractFloat(const std::string& json, const std::string& key,
                                  size_t searchStart, size_t searchEnd, float defaultVal) {
    std::string needle = "\"" + key + "\"";
    size_t pos = json.find(needle, searchStart);
    if (pos == std::string::npos || pos >= searchEnd) return defaultVal;
    size_t colon = json.find(':', pos + needle.size());
    if (colon == std::string::npos || colon >= searchEnd) return defaultVal;
    try { return std::stof(json.substr(colon + 1)); }
    catch (const std::exception& e) {
        std::cerr << "[EditorLayout] Failed to parse float for key '" << key << "': " << e.what() << std::endl;
        return defaultVal;
    }
}

int EditorLayout::extractInt(const std::string& json, const std::string& key,
                              size_t searchStart, size_t searchEnd, int defaultVal) {
    std::string needle = "\"" + key + "\"";
    size_t pos = json.find(needle, searchStart);
    if (pos == std::string::npos || pos >= searchEnd) return defaultVal;
    size_t colon = json.find(':', pos + needle.size());
    if (colon == std::string::npos || colon >= searchEnd) return defaultVal;
    try { return std::stoi(json.substr(colon + 1)); }
    catch (const std::exception& e) {
        std::cerr << "[EditorLayout] Failed to parse int for key '" << key << "': " << e.what() << std::endl;
        return defaultVal;
    }
}

// Find matching brace from a given opening brace position
static size_t findMatchingBrace(const std::string& json, size_t openPos) {
    if (openPos >= json.size() || json[openPos] != '{') return std::string::npos;
    int depth = 1;
    for (size_t i = openPos + 1; i < json.size(); ++i) {
        if (json[i] == '{') ++depth;
        else if (json[i] == '}') { --depth; if (depth == 0) return i; }
    }
    return std::string::npos;
}

void EditorLayout::deserializeNode(DockNode& node, const std::string& json, size_t& pos) {
    // Find the opening brace of this node object
    size_t openBrace = json.find('{', pos);
    if (openBrace == std::string::npos) return;
    size_t closeBrace = findMatchingBrace(json, openBrace);
    if (closeBrace == std::string::npos) return;

    // Extract fields within this brace range
    std::string splitStr = extractString(json, "split", openBrace, closeBrace);
    node.split     = splitFromName(splitStr);
    node.splitRatio = extractFloat(json, "splitRatio", openBrace, closeBrace, 0.5f);
    node.activeTab  = extractInt(json, "activeTab", openBrace, closeBrace, 0);

    // Panel
    std::string panelName = extractString(json, "panel", openBrace, closeBrace);
    if (!panelName.empty()) {
        node.panel = findPanelByName(panelName);
    }

    // Tabs array
    std::string tabsKey = "\"tabs\"";
    size_t tabsPos = json.find(tabsKey, openBrace);
    if (tabsPos != std::string::npos && tabsPos < closeBrace) {
        size_t arrOpen = json.find('[', tabsPos);
        size_t arrClose = json.find(']', arrOpen);
        if (arrOpen != std::string::npos && arrClose != std::string::npos && arrClose < closeBrace) {
            node.tabs.clear();
            size_t tPos = arrOpen + 1;
            while (tPos < arrClose) {
                size_t tq1 = json.find('\"', tPos);
                if (tq1 == std::string::npos || tq1 >= arrClose) break;
                size_t tq2 = json.find('\"', tq1 + 1);
                if (tq2 == std::string::npos || tq2 > arrClose) break;
                std::string tabName = json.substr(tq1 + 1, tq2 - tq1 - 1);
                if (!tabName.empty()) {
                    EditorPanel* tabPanel = findPanelByName(tabName);
                    if (tabPanel) node.tabs.push_back(tabPanel);
                }
                tPos = tq2 + 1;
            }
        }
    }

    // Child "a"
    std::string aKey = "\"a\"";
    size_t aPos = json.find(aKey, openBrace);
    if (aPos != std::string::npos && aPos < closeBrace) {
        size_t aOpen = json.find('{', aPos + aKey.size());
        if (aOpen != std::string::npos && aOpen < closeBrace) {
            node.a = std::make_unique<DockNode>();
            size_t aStart = aOpen;
            deserializeNode(*node.a, json, aStart);
        }
    }

    // Child "b"
    std::string bKey = "\"b\"";
    // Search for "b" after the "a" subtree to avoid matching inside "a"
    size_t bSearchStart = openBrace;
    if (node.a) {
        size_t aOpen2 = json.find('{', json.find(aKey, openBrace) + aKey.size());
        if (aOpen2 != std::string::npos) {
            size_t aClose2 = findMatchingBrace(json, aOpen2);
            if (aClose2 != std::string::npos) bSearchStart = aClose2 + 1;
        }
    }
    size_t bPos = json.find(bKey, bSearchStart);
    if (bPos != std::string::npos && bPos < closeBrace) {
        size_t bOpen = json.find('{', bPos + bKey.size());
        if (bOpen != std::string::npos && bOpen < closeBrace) {
            node.b = std::make_unique<DockNode>();
            size_t bStart = bOpen;
            deserializeNode(*node.b, json, bStart);
        }
    }

    pos = closeBrace + 1;
}

// ── Public serialisation API ──────────────────────────────────────

std::string EditorLayout::SerializeToJSON() const {
    std::ostringstream os;
    os << "{\n  \"layout\":\n" << serializeNode(m_root, 1) << "\n}\n";
    return os.str();
}

bool EditorLayout::DeserializeFromJSON(const std::string& json) {
    size_t layoutKey = json.find("\"layout\"");
    if (layoutKey == std::string::npos) return false;

    size_t braceStart = json.find('{', layoutKey + 8);
    if (braceStart == std::string::npos) return false;

    // Reset root
    m_root = DockNode{};
    size_t pos = braceStart;
    deserializeNode(m_root, json, pos);
    return true;
}

bool EditorLayout::SaveToFile(const std::string& path) const {
    std::filesystem::path fspath(path);
    if (fspath.has_parent_path()) {
        std::filesystem::create_directories(fspath.parent_path());
    }
    std::ofstream out(path);
    if (!out.is_open()) return false;
    out << SerializeToJSON();
    out.close();
    return true;
}

bool EditorLayout::LoadFromFile(const std::string& path) {
    if (!std::filesystem::exists(path)) return false;
    std::ifstream in(path);
    if (!in.is_open()) return false;
    std::string json((std::istreambuf_iterator<char>(in)),
                      std::istreambuf_iterator<char>());
    in.close();
    return DeserializeFromJSON(json);
}

}
