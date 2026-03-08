#include "InsuranceOverviewPanel.h"

#include <algorithm>
#include <sstream>

namespace atlas::editor {

// ── Construction ─────────────────────────────────────────────────────

InsuranceOverviewPanel::InsuranceOverviewPanel() = default;

// ── Internal helpers ─────────────────────────────────────────────────

InsurancePolicyEntry* InsuranceOverviewPanel::findPolicy(uint32_t policyId) {
    for (auto& p : m_policies) {
        if (p.policyId == policyId) return &p;
    }
    return nullptr;
}

const InsurancePolicyEntry* InsuranceOverviewPanel::findPolicyConst(uint32_t policyId) const {
    for (const auto& p : m_policies) {
        if (p.policyId == policyId) return &p;
    }
    return nullptr;
}

float InsuranceOverviewPanel::computePremium(const std::string& tier, float shipValue) const {
    float rate = 0.10f; // basic: 10%
    if (tier == "standard") rate = 0.20f;
    else if (tier == "platinum") rate = 0.35f;
    return shipValue * rate;
}

float InsuranceOverviewPanel::computeCoverage(const std::string& tier) const {
    if (tier == "basic") return 0.50f;
    if (tier == "standard") return 0.75f;
    if (tier == "platinum") return 1.00f;
    return 0.50f;
}

// ── Policy management ────────────────────────────────────────────────

int InsuranceOverviewPanel::AddPolicy(const std::string& shipName,
                                       const std::string& tier,
                                       float shipValue) {
    if (static_cast<int>(m_policies.size()) >= kMaxPolicies) return -1;
    if (shipName.empty() || tier.empty()) return -1;
    if (shipValue <= 0.0f) return -1;

    InsurancePolicyEntry entry;
    entry.policyId    = m_nextPolicyId++;
    entry.shipName    = shipName;
    entry.tier        = tier;
    entry.shipValue   = shipValue;
    entry.premium     = computePremium(tier, shipValue);
    entry.coverageRate = computeCoverage(tier);
    m_policies.push_back(entry);

    log("Added policy #" + std::to_string(entry.policyId)
        + " \"" + shipName + "\" tier=" + tier
        + " value=" + std::to_string(static_cast<int>(shipValue)));
    return static_cast<int>(entry.policyId);
}

bool InsuranceOverviewPanel::RemovePolicy(uint32_t policyId) {
    auto it = std::find_if(m_policies.begin(), m_policies.end(),
        [policyId](const InsurancePolicyEntry& p) { return p.policyId == policyId; });
    if (it == m_policies.end()) return false;
    log("Removed policy #" + std::to_string(policyId));
    m_policies.erase(it);
    return true;
}

bool InsuranceOverviewPanel::FileClaim(uint32_t policyId) {
    auto* p = findPolicy(policyId);
    if (!p) return false;
    if (p->claimed) return false;
    p->claimed = true;
    log("Filed claim on policy #" + std::to_string(policyId)
        + " payout=" + std::to_string(static_cast<int>(p->shipValue * p->coverageRate)));
    return true;
}

// ── Policy queries ───────────────────────────────────────────────────

float InsuranceOverviewPanel::GetPolicyPayout(uint32_t policyId) const {
    const auto* p = findPolicyConst(policyId);
    return p ? p->shipValue * p->coverageRate : 0.0f;
}

float InsuranceOverviewPanel::GetPolicyPremium(uint32_t policyId) const {
    const auto* p = findPolicyConst(policyId);
    return p ? p->premium : 0.0f;
}

int InsuranceOverviewPanel::ActivePolicyCount() const {
    int count = 0;
    for (const auto& p : m_policies) {
        if (!p.claimed) ++count;
    }
    return count;
}

int InsuranceOverviewPanel::ClaimedPolicyCount() const {
    int count = 0;
    for (const auto& p : m_policies) {
        if (p.claimed) ++count;
    }
    return count;
}

// ── Aggregate stats ──────────────────────────────────────────────────

float InsuranceOverviewPanel::TotalPremiums() const {
    float total = 0.0f;
    for (const auto& p : m_policies) total += p.premium;
    return total;
}

float InsuranceOverviewPanel::TotalPayouts() const {
    float total = 0.0f;
    for (const auto& p : m_policies) {
        if (p.claimed) total += p.shipValue * p.coverageRate;
    }
    return total;
}

float InsuranceOverviewPanel::ProfitLoss() const {
    return TotalPremiums() - TotalPayouts();
}

// ── Export ────────────────────────────────────────────────────────────

std::string InsuranceOverviewPanel::ExportJSON() const {
    std::ostringstream os;
    os << "{\n";
    os << "  \"policies\": [\n";
    for (size_t i = 0; i < m_policies.size(); ++i) {
        const auto& p = m_policies[i];
        os << "    {\"id\": " << p.policyId
           << ", \"ship\": \"" << p.shipName << "\""
           << ", \"tier\": \"" << p.tier << "\""
           << ", \"value\": " << p.shipValue
           << ", \"premium\": " << p.premium
           << ", \"coverage\": " << p.coverageRate
           << ", \"claimed\": " << (p.claimed ? "true" : "false") << "}";
        if (i + 1 < m_policies.size()) os << ",";
        os << "\n";
    }
    os << "  ],\n";
    os << "  \"totalPremiums\": " << TotalPremiums() << ",\n";
    os << "  \"totalPayouts\": " << TotalPayouts() << ",\n";
    os << "  \"profitLoss\": " << ProfitLoss() << "\n";
    os << "}";
    return os.str();
}

// ── Draw ──────────────────────────────────────────────────────────────

void InsuranceOverviewPanel::Draw() {
    if (!GetContext()) return;

    auto& ctx = *GetContext();
    ApplyDockBounds(m_panelState);
    if (!atlas::panelBeginStateful(ctx, "Insurance Overview", m_panelState)) {
        atlas::panelEnd(ctx);
        return;
    }

    const float pad    = ctx.theme().padding;
    const float rowH   = ctx.theme().rowHeight;
    const atlas::Rect& b = m_panelState.bounds;
    const float headerH  = ctx.theme().headerHeight;
    float y = b.y + headerH + pad;

    // Summary
    std::string summary = "Policies: " + std::to_string(PolicyCount())
        + "  Active: " + std::to_string(ActivePolicyCount())
        + "  Claimed: " + std::to_string(ClaimedPolicyCount())
        + "  P/L: " + std::to_string(static_cast<int>(ProfitLoss()));
    atlas::label(ctx, {b.x + pad, y}, summary, ctx.theme().textPrimary);
    y += rowH + pad;

    // Policy listing
    for (int i = 0; i < PolicyCount() && i < 12; ++i) {
        const auto& p = m_policies[i];
        std::string info = "#" + std::to_string(p.policyId) + " " + p.shipName
            + " [" + p.tier + "] value=" + std::to_string(static_cast<int>(p.shipValue))
            + (p.claimed ? " CLAIMED" : " active");
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

void InsuranceOverviewPanel::log(const std::string& msg) {
    m_log.push_back(msg);
}

} // namespace atlas::editor
