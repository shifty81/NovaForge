#pragma once
#include "../ui/EditorPanel.h"
#include "../../cpp_client/include/ui/atlas/atlas_widgets.h"
#include <string>
#include <vector>
#include <cstdint>

namespace atlas::editor {

/**
 * @brief An insurance policy entry for the editor panel.
 */
struct InsurancePolicyEntry {
    uint32_t policyId = 0;
    std::string shipName;
    std::string tier;              ///< "basic", "standard", "platinum"
    float shipValue     = 0.0f;
    float premium       = 0.0f;
    float coverageRate  = 0.0f;   ///< 0.0–1.0
    bool  claimed       = false;
};

/**
 * InsuranceOverviewPanel — Insurance policy dashboard for balance tuning.
 *
 * Designers can:
 *   - Create and manage insurance policies at different tiers.
 *   - View premium/payout ratios for economy balancing.
 *   - File test claims to validate payout calculations.
 *   - Track aggregate premiums vs payouts (profit/loss).
 *   - Export insurance data to JSON for external analysis.
 *
 * Headless-safe: Draw() is a no-op when no AtlasContext is set.
 */
class InsuranceOverviewPanel : public EditorPanel {
public:
    InsuranceOverviewPanel();
    ~InsuranceOverviewPanel() override = default;

    const char* Name() const override { return "Insurance Overview"; }
    void Draw() override;

    // ── Policy management ────────────────────────────────────────

    int AddPolicy(const std::string& shipName, const std::string& tier,
                  float shipValue);
    bool RemovePolicy(uint32_t policyId);
    bool FileClaim(uint32_t policyId);
    int PolicyCount() const { return static_cast<int>(m_policies.size()); }
    const std::vector<InsurancePolicyEntry>& Policies() const { return m_policies; }

    // ── Policy queries ───────────────────────────────────────────

    float GetPolicyPayout(uint32_t policyId) const;
    float GetPolicyPremium(uint32_t policyId) const;
    int ActivePolicyCount() const;
    int ClaimedPolicyCount() const;

    // ── Aggregate stats ──────────────────────────────────────────

    float TotalPremiums() const;
    float TotalPayouts() const;
    float ProfitLoss() const;

    // ── Export ────────────────────────────────────────────────────

    std::string ExportJSON() const;

    // ── Log ──────────────────────────────────────────────────────

    const std::vector<std::string>& Log() const { return m_log; }

private:
    std::vector<InsurancePolicyEntry> m_policies;

    static constexpr int kMaxPolicies = 50;
    uint32_t m_nextPolicyId = 1;

    std::vector<std::string> m_log;
    atlas::PanelState m_panelState;
    float m_scrollOffset = 0.0f;

    void log(const std::string& msg);
    InsurancePolicyEntry* findPolicy(uint32_t policyId);
    const InsurancePolicyEntry* findPolicyConst(uint32_t policyId) const;

    float computePremium(const std::string& tier, float shipValue) const;
    float computeCoverage(const std::string& tier) const;
};

} // namespace atlas::editor
