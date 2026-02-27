#include "ui/atlas/atlas_context.h"
#include <fstream>
#include <sstream>
#include <filesystem>
#include <iostream>

namespace atlas {

AtlasContext::AtlasContext() = default;
AtlasContext::~AtlasContext() { shutdown(); }

bool AtlasContext::init() {
    return m_renderer.init();
}

void AtlasContext::shutdown() {
    m_renderer.shutdown();
}

void AtlasContext::beginFrame(const InputState& input) {
    m_prevMousePos = m_input.mousePos;  // save previous frame's position
    m_input = input;
    m_hotID = 0;  // reset hot each frame; widgets re-claim it
    m_mouseConsumed = false;  // reset consumed flag each frame
    m_renderer.begin(input.windowW, input.windowH);
}

void AtlasContext::endFrame() {
    m_renderer.end();
    // If mouse was released this frame, clear active widget
    if (m_input.mouseReleased[0]) {
        m_activeID = 0;
    }
}

// ── Interaction ─────────────────────────────────────────────────────

bool AtlasContext::isHovered(const Rect& r) const {
    return r.contains(m_input.mousePos);
}

void AtlasContext::setHot(WidgetID id) {
    // Only allow hot if no other widget is active (or this one is)
    if (m_activeID == 0 || m_activeID == id) {
        m_hotID = id;
    }
}

void AtlasContext::setActive(WidgetID id)  { m_activeID = id; }
void AtlasContext::clearActive()           { m_activeID = 0; }

bool AtlasContext::buttonBehavior(const Rect& r, WidgetID id) {
    bool hovered = isHovered(r);
    bool clicked = false;

    // If mouse is already consumed by a higher-priority widget, skip interaction
    if (m_mouseConsumed) {
        return false;
    }

    if (hovered) {
        setHot(id);
        if (m_input.mouseClicked[0]) {
            setActive(id);
        }
    }

    if (isActive(id) && m_input.mouseReleased[0]) {
        if (hovered) clicked = true;
        clearActive();
    }

    return clicked;
}

// ── ID stack ────────────────────────────────────────────────────────

void AtlasContext::pushID(const char* label) {
    WidgetID parent = m_idStack.empty() ? 0u : m_idStack.back();
    // Combine parent hash with label hash
    WidgetID h = hashID(label);
    h ^= parent * 2654435761u;  // Knuth multiplicative hash mix
    m_idStack.push_back(h);
}

void AtlasContext::popID() {
    if (!m_idStack.empty()) m_idStack.pop_back();
}

WidgetID AtlasContext::currentID(const char* label) const {
    WidgetID parent = m_idStack.empty() ? 0u : m_idStack.back();
    WidgetID h = hashID(label);
    h ^= parent * 2654435761u;
    return h;
}

Vec2 AtlasContext::getDragDelta() const {
    return m_input.mousePos - m_prevMousePos;
}

// ── Theme loading ───────────────────────────────────────────────────

static Color parseHexColor(const std::string& hex) {
    if (hex.size() < 7 || hex[0] != '#') return {1, 1, 1, 1};
    auto hexVal = [](char c) -> int {
        if (c >= '0' && c <= '9') return c - '0';
        if (c >= 'a' && c <= 'f') return 10 + c - 'a';
        if (c >= 'A' && c <= 'F') return 10 + c - 'A';
        return 0;
    };
    int r = hexVal(hex[1]) * 16 + hexVal(hex[2]);
    int g = hexVal(hex[3]) * 16 + hexVal(hex[4]);
    int b = hexVal(hex[5]) * 16 + hexVal(hex[6]);
    int a = (hex.size() >= 9) ? (hexVal(hex[7]) * 16 + hexVal(hex[8])) : 255;
    return Color::fromRGBA(r, g, b, a);
}

static std::string extractJsonString(const std::string& json, const std::string& key,
                                      size_t searchStart, size_t searchEnd) {
    std::string needle = "\"" + key + "\"";
    size_t pos = json.find(needle, searchStart);
    if (pos == std::string::npos || pos >= searchEnd) return "";
    size_t colon = json.find(':', pos + needle.size());
    if (colon == std::string::npos || colon >= searchEnd) return "";
    size_t q1 = json.find('\"', colon + 1);
    if (q1 == std::string::npos || q1 >= searchEnd) return "";
    size_t q2 = json.find('\"', q1 + 1);
    if (q2 == std::string::npos || q2 >= searchEnd) return "";
    return json.substr(q1 + 1, q2 - q1 - 1);
}

static float extractJsonFloat(const std::string& json, const std::string& key,
                               size_t searchStart, size_t searchEnd, float defaultVal) {
    std::string needle = "\"" + key + "\"";
    size_t pos = json.find(needle, searchStart);
    if (pos == std::string::npos || pos >= searchEnd) return defaultVal;
    size_t colon = json.find(':', pos + needle.size());
    if (colon == std::string::npos || colon >= searchEnd) return defaultVal;
    try { return std::stof(json.substr(colon + 1)); }
    catch (...) { return defaultVal; }
}

bool AtlasContext::loadThemeFromFile(const std::string& path) {
    if (!std::filesystem::exists(path)) return false;
    std::ifstream in(path);
    if (!in.is_open()) return false;
    std::string json((std::istreambuf_iterator<char>(in)),
                      std::istreambuf_iterator<char>());
    in.close();

    // Parse color sections
    auto colorHex = [&](const std::string& key) -> Color {
        std::string val = extractJsonString(json, key, 0, json.size());
        if (val.empty()) return {0, 0, 0, 0};
        return parseHexColor(val);
    };

    // Background colors — JSON uses #RRGGBB without alpha, so we apply
    // translucency alphas matching the Photon Dark design guidelines
    Color bg0 = colorHex("bg_0");
    Color bg1 = colorHex("bg_1");
    Color bg2 = colorHex("bg_2");
    if (bg0.a > 0) { m_theme.bgPrimary = bg0; m_theme.bgPrimary.a = 0.96f; }
    if (bg1.a > 0) { m_theme.bgSecondary = bg1; m_theme.bgSecondary.a = 0.94f; }
    if (bg2.a > 0) { m_theme.bgPanel = bg2; m_theme.bgPanel.a = 0.96f; }

    // Text colors
    Color textPri = colorHex("primary");
    // "primary" key is ambiguous — look specifically in the "text" section
    size_t textSection = json.find("\"text\"");
    if (textSection != std::string::npos) {
        size_t sectionEnd = json.find('}', textSection);
        std::string tp = extractJsonString(json, "primary", textSection, sectionEnd);
        if (!tp.empty()) m_theme.textPrimary = parseHexColor(tp);
        std::string ts = extractJsonString(json, "secondary", textSection, sectionEnd);
        if (!ts.empty()) m_theme.textSecondary = parseHexColor(ts);
        std::string tm = extractJsonString(json, "muted", textSection, sectionEnd);
        if (!tm.empty()) m_theme.textMuted = parseHexColor(tm);
        std::string td = extractJsonString(json, "disabled", textSection, sectionEnd);
        if (!td.empty()) m_theme.textDisabled = parseHexColor(td);
    }

    // Accent colors
    size_t accentSection = json.find("\"accent\"");
    if (accentSection != std::string::npos) {
        size_t sectionEnd = json.find('}', accentSection);
        std::string ad = extractJsonString(json, "default", accentSection, sectionEnd);
        if (!ad.empty()) m_theme.accentPrimary = parseHexColor(ad);
        std::string ah = extractJsonString(json, "highlight", accentSection, sectionEnd);
        if (!ah.empty()) m_theme.accentSecondary = parseHexColor(ah);
    }

    // Status colors
    Color success = colorHex("positive");
    Color warning = colorHex("warning");
    Color danger = colorHex("negative");
    if (success.a > 0) m_theme.success = success;
    if (warning.a > 0) m_theme.warning = warning;
    if (danger.a > 0) m_theme.danger = danger;

    // Ship HUD
    Color shield = colorHex("shield");
    Color armor = colorHex("armor");
    Color hull = colorHex("hull");
    if (shield.a > 0) m_theme.shield = shield;
    if (armor.a > 0) m_theme.armor = armor;
    if (hull.a > 0) m_theme.hull = hull;

    // Targeting / standings
    Color hostile = colorHex("hostile");
    Color friendly = colorHex("friendly");
    if (hostile.a > 0) m_theme.hostile = hostile;
    if (friendly.a > 0) m_theme.friendly = friendly;

    // Window/border colors
    Color border = colorHex("border");
    Color borderFocused = colorHex("border_focused");
    if (border.a > 0) m_theme.borderNormal = border;
    if (borderFocused.a > 0) m_theme.borderHighlight = borderFocused;

    // Selection / hover
    Color rowHover = colorHex("row_hover");
    Color rowSelected = colorHex("row_selected");
    if (rowHover.a > 0) m_theme.hover = rowHover;
    if (rowSelected.a > 0) m_theme.selection = rowSelected;

    // Header
    Color titlebarBg = colorHex("titlebar_bg");
    if (titlebarBg.a > 0) m_theme.bgHeader = titlebarBg;

    // Spacing values
    size_t spacingSection = json.find("\"spacing\"");
    if (spacingSection != std::string::npos) {
        size_t sEnd = json.find('}', spacingSection);
        m_theme.borderWidth   = extractJsonFloat(json, "border_width", spacingSection, sEnd, m_theme.borderWidth);
        m_theme.rowHeight     = extractJsonFloat(json, "row_height", spacingSection, sEnd, m_theme.rowHeight);
        m_theme.headerHeight  = extractJsonFloat(json, "titlebar_height", spacingSection, sEnd, m_theme.headerHeight);
        m_theme.padding       = extractJsonFloat(json, "padding_sm", spacingSection, sEnd, m_theme.padding);
        m_theme.itemSpacing   = extractJsonFloat(json, "grid_unit", spacingSection, sEnd, m_theme.itemSpacing);
        m_theme.panelCornerRadius = extractJsonFloat(json, "border_radius", spacingSection, sEnd, m_theme.panelCornerRadius);
    }

    return true;
}

} // namespace atlas
