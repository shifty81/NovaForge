#include "ColonyManagerPanel.h"

#include <algorithm>
#include <sstream>

namespace atlas::editor {

// ── Construction ─────────────────────────────────────────────────────

ColonyManagerPanel::ColonyManagerPanel() = default;

// ── Internal helpers ─────────────────────────────────────────────────

ColonyBuildingEntry* ColonyManagerPanel::findBuilding(uint32_t buildingId) {
    for (auto& b : m_buildings) {
        if (b.buildingId == buildingId) return &b;
    }
    return nullptr;
}

const ColonyBuildingEntry* ColonyManagerPanel::findBuildingConst(uint32_t buildingId) const {
    for (const auto& b : m_buildings) {
        if (b.buildingId == buildingId) return &b;
    }
    return nullptr;
}

// ── Building management ──────────────────────────────────────────────

int ColonyManagerPanel::AddBuilding(const std::string& name,
                                     const std::string& type,
                                     float productionRate,
                                     float powerUsage) {
    if (static_cast<int>(m_buildings.size()) >= kMaxBuildings) return -1;
    if (name.empty() || type.empty()) return -1;
    if (powerUsage < 0.0f) powerUsage = 0.0f;
    if (PowerUsed() + powerUsage > m_powerCapacity) return -1;

    ColonyBuildingEntry entry;
    entry.buildingId    = m_nextBuildingId++;
    entry.buildingName  = name;
    entry.buildingType  = type;
    entry.productionRate = std::max(0.1f, productionRate);
    entry.powerUsage    = powerUsage;
    m_buildings.push_back(entry);

    log("Added building #" + std::to_string(entry.buildingId)
        + " \"" + name + "\" type=" + type
        + " power=" + std::to_string(static_cast<int>(powerUsage)) + "W");
    return static_cast<int>(entry.buildingId);
}

bool ColonyManagerPanel::RemoveBuilding(uint32_t buildingId) {
    auto it = std::find_if(m_buildings.begin(), m_buildings.end(),
        [buildingId](const ColonyBuildingEntry& b) { return b.buildingId == buildingId; });
    if (it == m_buildings.end()) return false;
    log("Removed building #" + std::to_string(buildingId));
    m_buildings.erase(it);
    return true;
}

bool ColonyManagerPanel::ToggleBuilding(uint32_t buildingId) {
    auto* b = findBuilding(buildingId);
    if (!b) return false;
    if (b->online) {
        b->online = false;
        log("Building #" + std::to_string(buildingId) + " offline");
    } else {
        if (PowerUsed() + b->powerUsage > m_powerCapacity) return false;
        b->online = true;
        log("Building #" + std::to_string(buildingId) + " online");
    }
    return true;
}

int ColonyManagerPanel::OnlineCount() const {
    int count = 0;
    for (const auto& b : m_buildings) {
        if (b.online) ++count;
    }
    return count;
}

// ── Power management ─────────────────────────────────────────────────

bool ColonyManagerPanel::SetPowerCapacity(float capacity) {
    if (capacity < 10.0f) capacity = 10.0f;
    m_powerCapacity = capacity;
    return true;
}

float ColonyManagerPanel::PowerUsed() const {
    float total = 0.0f;
    for (const auto& b : m_buildings) {
        if (b.online) total += b.powerUsage;
    }
    return total;
}

// ── Goods management ─────────────────────────────────────────────────

bool ColonyManagerPanel::AddGoods(const std::string& goodType,
                                   float quantity, float maxQuantity) {
    if (goodType.empty() || quantity < 0.0f) return false;
    for (auto& g : m_goods) {
        if (g.goodType == goodType) {
            g.quantity = std::min(g.maxQuantity, g.quantity + quantity);
            return true;
        }
    }
    ColonyGoodEntry g;
    g.goodType    = goodType;
    g.quantity    = std::min(maxQuantity, quantity);
    g.maxQuantity = maxQuantity;
    m_goods.push_back(g);
    return true;
}

bool ColonyManagerPanel::ExportGoods(const std::string& goodType,
                                      float quantity, float unitPrice) {
    if (goodType.empty() || quantity <= 0.0f || unitPrice <= 0.0f) return false;
    for (auto& g : m_goods) {
        if (g.goodType == goodType) {
            if (g.quantity < quantity) return false;
            g.quantity -= quantity;
            m_totalExports += quantity;
            m_totalExportValue += quantity * unitPrice;
            log("Exported " + std::to_string(static_cast<int>(quantity))
                + " " + goodType + " @ " + std::to_string(static_cast<int>(unitPrice)) + " ISC");
            return true;
        }
    }
    return false;
}

float ColonyManagerPanel::GetGoodsQuantity(const std::string& goodType) const {
    for (const auto& g : m_goods) {
        if (g.goodType == goodType) return g.quantity;
    }
    return 0.0f;
}

// ── Export ────────────────────────────────────────────────────────────

std::string ColonyManagerPanel::ExportJSON() const {
    std::ostringstream os;
    os << "{\n";
    os << "  \"buildings\": [\n";
    for (size_t i = 0; i < m_buildings.size(); ++i) {
        const auto& b = m_buildings[i];
        os << "    {\"id\": " << b.buildingId
           << ", \"name\": \"" << b.buildingName << "\""
           << ", \"type\": \"" << b.buildingType << "\""
           << ", \"production\": " << b.productionRate
           << ", \"power\": " << b.powerUsage
           << ", \"online\": " << (b.online ? "true" : "false") << "}";
        if (i + 1 < m_buildings.size()) os << ",";
        os << "\n";
    }
    os << "  ],\n";
    os << "  \"goods\": [\n";
    for (size_t i = 0; i < m_goods.size(); ++i) {
        const auto& g = m_goods[i];
        os << "    {\"type\": \"" << g.goodType << "\""
           << ", \"quantity\": " << g.quantity
           << ", \"max\": " << g.maxQuantity << "}";
        if (i + 1 < m_goods.size()) os << ",";
        os << "\n";
    }
    os << "  ],\n";
    os << "  \"totalExports\": " << m_totalExports << ",\n";
    os << "  \"totalExportValue\": " << m_totalExportValue << ",\n";
    os << "  \"powerCapacity\": " << m_powerCapacity << ",\n";
    os << "  \"powerUsed\": " << PowerUsed() << "\n";
    os << "}";
    return os.str();
}

// ── Draw ──────────────────────────────────────────────────────────────

void ColonyManagerPanel::Draw() {
    if (!GetContext()) return;

    auto& ctx = *GetContext();
    ApplyDockBounds(m_panelState);
    if (!atlas::panelBeginStateful(ctx, "Colony Manager", m_panelState)) {
        atlas::panelEnd(ctx);
        return;
    }

    const float pad    = ctx.theme().padding;
    const float rowH   = ctx.theme().rowHeight;
    const atlas::Rect& b = m_panelState.bounds;
    const float headerH  = ctx.theme().headerHeight;
    float y = b.y + headerH + pad;

    // Summary
    std::string summary = "Buildings: " + std::to_string(BuildingCount())
        + "  Online: " + std::to_string(OnlineCount())
        + "  Power: " + std::to_string(static_cast<int>(PowerUsed()))
        + "/" + std::to_string(static_cast<int>(m_powerCapacity)) + "W"
        + "  Exports: " + std::to_string(static_cast<int>(m_totalExportValue)) + " ISC";
    atlas::label(ctx, {b.x + pad, y}, summary, ctx.theme().textPrimary);
    y += rowH + pad;

    // Building listing
    for (int i = 0; i < BuildingCount() && i < 12; ++i) {
        const auto& b2 = m_buildings[i];
        std::string info = "#" + std::to_string(b2.buildingId) + " " + b2.buildingName
            + " [" + b2.buildingType + "] "
            + std::to_string(static_cast<int>(b2.powerUsage)) + "W"
            + (b2.online ? "" : " (offline)");
        atlas::label(ctx, {b.x + pad, y}, info, ctx.theme().textPrimary);
        y += rowH;
    }
    y += pad;

    atlas::separator(ctx, {b.x + pad, y}, b.w - 2.0f * pad);
    y += pad;

    // Log area
    atlas::Rect logRect{b.x + pad, y, b.w - 2.0f * pad, b.y + b.h - y - pad};
    atlas::combatLogWidget(ctx, logRect, m_log, m_scrollOffset);

    atlas::panelEnd(ctx);
}

// ── Helpers ───────────────────────────────────────────────────────────

void ColonyManagerPanel::log(const std::string& msg) {
    m_log.push_back(msg);
}

} // namespace atlas::editor
