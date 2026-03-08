#pragma once
#include "../ui/EditorPanel.h"
#include "../../cpp_client/include/ui/atlas/atlas_widgets.h"
#include <string>
#include <vector>
#include <cstdint>

namespace atlas::editor {

/**
 * @brief A colony building entry for the editor panel.
 */
struct ColonyBuildingEntry {
    uint32_t buildingId = 0;
    std::string buildingName;
    std::string buildingType;     ///< "extractor", "processor", "storage", "launchpad"
    float productionRate = 1.0f;
    float powerUsage     = 10.0f;
    bool  online         = true;
};

/**
 * @brief A stored good entry for colony inventory display.
 */
struct ColonyGoodEntry {
    std::string goodType;
    float quantity    = 0.0f;
    float maxQuantity = 1000.0f;
};

/**
 * ColonyManagerPanel — Colony production and export management panel.
 *
 * Designers can:
 *   - Create buildings with power budgets for planetary colonies.
 *   - Toggle buildings on/off and monitor power usage.
 *   - Add goods to colony storage and simulate exports.
 *   - Track total production output and export revenue.
 *   - Export colony data to JSON for game data pipelines.
 *
 * Headless-safe: Draw() is a no-op when no AtlasContext is set.
 */
class ColonyManagerPanel : public EditorPanel {
public:
    ColonyManagerPanel();
    ~ColonyManagerPanel() override = default;

    const char* Name() const override { return "Colony Manager"; }
    void Draw() override;

    // ── Building management ──────────────────────────────────────

    int AddBuilding(const std::string& name, const std::string& type,
                    float productionRate, float powerUsage);
    bool RemoveBuilding(uint32_t buildingId);
    bool ToggleBuilding(uint32_t buildingId);
    int BuildingCount() const { return static_cast<int>(m_buildings.size()); }
    int OnlineCount() const;
    const std::vector<ColonyBuildingEntry>& Buildings() const { return m_buildings; }

    // ── Power management ─────────────────────────────────────────

    bool SetPowerCapacity(float capacity);
    float PowerCapacity() const { return m_powerCapacity; }
    float PowerUsed() const;
    float RemainingPower() const { return m_powerCapacity - PowerUsed(); }

    // ── Goods management ─────────────────────────────────────────

    bool AddGoods(const std::string& goodType, float quantity, float maxQuantity);
    bool ExportGoods(const std::string& goodType, float quantity, float unitPrice);
    float GetGoodsQuantity(const std::string& goodType) const;
    int GoodTypeCount() const { return static_cast<int>(m_goods.size()); }

    // ── Aggregate stats ──────────────────────────────────────────

    float TotalExports() const { return m_totalExports; }
    float TotalExportValue() const { return m_totalExportValue; }

    // ── Export ────────────────────────────────────────────────────

    std::string ExportJSON() const;

    // ── Log ──────────────────────────────────────────────────────

    const std::vector<std::string>& Log() const { return m_log; }

private:
    std::vector<ColonyBuildingEntry> m_buildings;
    std::vector<ColonyGoodEntry>     m_goods;

    static constexpr int kMaxBuildings = 30;
    float m_powerCapacity   = 100.0f;
    float m_totalExports    = 0.0f;
    float m_totalExportValue = 0.0f;

    uint32_t m_nextBuildingId = 1;

    std::vector<std::string> m_log;
    atlas::PanelState m_panelState;
    float m_scrollOffset = 0.0f;

    void log(const std::string& msg);
    ColonyBuildingEntry* findBuilding(uint32_t buildingId);
    const ColonyBuildingEntry* findBuildingConst(uint32_t buildingId) const;
};

} // namespace atlas::editor
