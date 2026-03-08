/**
 * Tests for InsuranceOverviewPanel:
 *   - Construction & defaults
 *   - Add/remove policies
 *   - Policy tiers & premiums
 *   - File claims
 *   - Profit/loss tracking
 *   - Max policies
 *   - Export JSON
 *   - Headless draw
 */
#include <cassert>
#include <string>
#include <cmath>
#include "tools/InsuranceOverviewPanel.h"

using atlas::editor::InsuranceOverviewPanel;

static bool approxEq(float a, float b, float eps = 0.01f) {
    return std::abs(a - b) < eps;
}

void test_insurance_overview_defaults() {
    InsuranceOverviewPanel panel;
    assert(panel.PolicyCount() == 0);
    assert(panel.ActivePolicyCount() == 0);
    assert(panel.ClaimedPolicyCount() == 0);
    assert(approxEq(panel.TotalPremiums(), 0.0f));
    assert(approxEq(panel.TotalPayouts(), 0.0f));
    assert(approxEq(panel.ProfitLoss(), 0.0f));
}

void test_insurance_overview_add_policy() {
    InsuranceOverviewPanel panel;
    int id = panel.AddPolicy("Rifter", "basic", 1000000.0f);
    assert(id > 0);
    assert(panel.PolicyCount() == 1);
    assert(panel.ActivePolicyCount() == 1);
    // Premium = 10% of 1M = 100K
    assert(approxEq(panel.GetPolicyPremium(id), 100000.0f));
    // Payout = 50% of 1M = 500K
    assert(approxEq(panel.GetPolicyPayout(id), 500000.0f));
}

void test_insurance_overview_policy_validation() {
    InsuranceOverviewPanel panel;
    assert(panel.AddPolicy("", "basic", 1000.0f) == -1);
    assert(panel.AddPolicy("Ship", "", 1000.0f) == -1);
    assert(panel.AddPolicy("Ship", "basic", 0.0f) == -1);
    assert(panel.AddPolicy("Ship", "basic", -100.0f) == -1);
    assert(panel.PolicyCount() == 0);
}

void test_insurance_overview_tiers() {
    InsuranceOverviewPanel panel;
    int id1 = panel.AddPolicy("Rifter", "basic", 100000.0f);
    int id2 = panel.AddPolicy("Hurricane", "standard", 100000.0f);
    int id3 = panel.AddPolicy("Megathron", "platinum", 100000.0f);
    assert(approxEq(panel.GetPolicyPayout(id1), 50000.0f));    // 50%
    assert(approxEq(panel.GetPolicyPayout(id2), 75000.0f));    // 75%
    assert(approxEq(panel.GetPolicyPayout(id3), 100000.0f));   // 100%
    // Premiums: 10K + 20K + 35K = 65K
    assert(approxEq(panel.TotalPremiums(), 65000.0f));
}

void test_insurance_overview_remove_policy() {
    InsuranceOverviewPanel panel;
    int id1 = panel.AddPolicy("Ship1", "basic", 1000.0f);
    panel.AddPolicy("Ship2", "standard", 2000.0f);
    assert(panel.RemovePolicy(id1));
    assert(panel.PolicyCount() == 1);
    assert(!panel.RemovePolicy(999));
}

void test_insurance_overview_file_claim() {
    InsuranceOverviewPanel panel;
    int id = panel.AddPolicy("Rifter", "basic", 1000000.0f);
    assert(panel.FileClaim(id));
    assert(panel.ClaimedPolicyCount() == 1);
    assert(panel.ActivePolicyCount() == 0);
    assert(approxEq(panel.TotalPayouts(), 500000.0f));
    assert(!panel.FileClaim(id));  // cannot claim twice
    assert(!panel.FileClaim(999)); // nonexistent
}

void test_insurance_overview_profit_loss() {
    InsuranceOverviewPanel panel;
    int id = panel.AddPolicy("Rifter", "basic", 100000.0f);
    // Premium = 10K, no claims -> P/L = 10K
    assert(panel.ProfitLoss() > 0.0f);
    panel.FileClaim(id);
    // Payout = 50K -> P/L = 10K - 50K = -40K
    assert(panel.ProfitLoss() < 0.0f);
}

void test_insurance_overview_max_policies() {
    InsuranceOverviewPanel panel;
    for (int i = 0; i < 50; ++i) {
        assert(panel.AddPolicy("Ship_" + std::to_string(i), "basic", 1000.0f) > 0);
    }
    assert(panel.AddPolicy("Overflow", "basic", 1000.0f) == -1);
    assert(panel.PolicyCount() == 50);
}

void test_insurance_overview_export_json() {
    InsuranceOverviewPanel panel;
    panel.AddPolicy("Rifter", "basic", 100000.0f);
    panel.AddPolicy("Hurricane", "standard", 200000.0f);
    std::string json = panel.ExportJSON();
    assert(json.find("Rifter") != std::string::npos);
    assert(json.find("Hurricane") != std::string::npos);
    assert(json.find("basic") != std::string::npos);
    assert(json.find("standard") != std::string::npos);
    assert(json.find("totalPremiums") != std::string::npos);
    assert(json.find("profitLoss") != std::string::npos);
}

void test_insurance_overview_headless_draw() {
    InsuranceOverviewPanel panel;
    panel.AddPolicy("Test", "basic", 1000.0f);
    panel.Draw(); // should not crash without context
}
