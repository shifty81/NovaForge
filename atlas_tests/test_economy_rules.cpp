/**
 * Tests for EconomyRulesLoader — data-driven economy rules configuration
 * for modding support.  Rule-sets define per-security-band NPC spawn rates,
 * market fees, resource drift rates, and pricing bounds.
 */

#include <cassert>
#include <cmath>
#include "../engine/sim/EconomyRulesLoader.h"

using namespace atlas::sim;

static bool approxEq(float a, float b, float eps = 0.01f) {
    return std::fabs(a - b) < eps;
}

// ══════════════════════════════════════════════════════════════════
// Defaults and construction
// ══════════════════════════════════════════════════════════════════

void test_economy_rules_defaults() {
    EconomyRulesLoader loader;
    assert(loader.RulesCount() == 0);
    assert(loader.GetRules("global") == nullptr);
}

void test_economy_rules_install_defaults() {
    EconomyRulesLoader loader;
    loader.InstallDefaults();
    assert(loader.RulesCount() == 4);

    const EconomyRules* global = loader.GetRules("global");
    assert(global != nullptr);
    assert(approxEq(global->pirateSpawnRate, 0.05f));
    assert(approxEq(global->brokerFeePercent, 3.0f));

    const EconomyRules* hs = loader.GetRules("highsec");
    assert(hs != nullptr);
    assert(approxEq(hs->pirateSpawnRate, 0.01f));
    assert(approxEq(hs->miningYieldMultiplier, 0.8f));

    const EconomyRules* ls = loader.GetRules("lowsec");
    assert(ls != nullptr);
    assert(approxEq(ls->pirateSpawnRate, 0.10f));
    assert(approxEq(ls->miningYieldMultiplier, 1.5f));

    const EconomyRules* ns = loader.GetRules("nullsec");
    assert(ns != nullptr);
    assert(approxEq(ns->pirateSpawnRate, 0.15f));
    assert(approxEq(ns->miningYieldMultiplier, 2.0f));
}

// ══════════════════════════════════════════════════════════════════
// Add / remove
// ══════════════════════════════════════════════════════════════════

void test_economy_rules_add() {
    EconomyRulesLoader loader;
    EconomyRules r;
    r.name = "Custom Rules";
    r.pirateSpawnRate = 0.20f;
    assert(loader.AddRules("custom", r));
    assert(loader.RulesCount() == 1);

    const EconomyRules* got = loader.GetRules("custom");
    assert(got != nullptr);
    assert(got->name == "Custom Rules");
    assert(approxEq(got->pirateSpawnRate, 0.20f));
}

void test_economy_rules_add_overwrites() {
    EconomyRulesLoader loader;
    EconomyRules r1;
    r1.name = "V1";
    r1.salesTaxPercent = 1.0f;
    loader.AddRules("test", r1);

    EconomyRules r2;
    r2.name = "V2";
    r2.salesTaxPercent = 5.0f;
    loader.AddRules("test", r2);

    assert(loader.RulesCount() == 1);
    assert(loader.GetRules("test")->name == "V2");
    assert(approxEq(loader.GetRules("test")->salesTaxPercent, 5.0f));
}

void test_economy_rules_remove() {
    EconomyRulesLoader loader;
    EconomyRules r;
    loader.AddRules("x", r);
    assert(loader.RemoveRules("x"));
    assert(loader.RulesCount() == 0);
}

void test_economy_rules_remove_nonexistent() {
    EconomyRulesLoader loader;
    assert(!loader.RemoveRules("nope"));
}

void test_economy_rules_get_nonexistent() {
    EconomyRulesLoader loader;
    assert(loader.GetRules("nope") == nullptr);
}

// ══════════════════════════════════════════════════════════════════
// Security-based lookup
// ══════════════════════════════════════════════════════════════════

void test_economy_rules_security_highsec() {
    EconomyRulesLoader loader;
    loader.InstallDefaults();

    const EconomyRules* r = loader.GetRulesForSecurity(0.8f);
    assert(r != nullptr);
    assert(r->name == "High Security");
}

void test_economy_rules_security_lowsec() {
    EconomyRulesLoader loader;
    loader.InstallDefaults();

    const EconomyRules* r = loader.GetRulesForSecurity(0.3f);
    assert(r != nullptr);
    assert(r->name == "Low Security");
}

void test_economy_rules_security_nullsec() {
    EconomyRulesLoader loader;
    loader.InstallDefaults();

    const EconomyRules* r = loader.GetRulesForSecurity(0.05f);
    assert(r != nullptr);
    assert(r->name == "Null Security");
}

void test_economy_rules_security_boundary() {
    EconomyRulesLoader loader;
    loader.InstallDefaults();

    // Exactly at threshold → highsec
    const EconomyRules* r = loader.GetRulesForSecurity(0.5f);
    assert(r != nullptr);
    assert(r->name == "High Security");

    // Just below → lowsec
    r = loader.GetRulesForSecurity(0.49f);
    assert(r != nullptr);
    assert(r->name == "Low Security");

    // Exactly at low threshold → lowsec
    r = loader.GetRulesForSecurity(0.1f);
    assert(r != nullptr);
    assert(r->name == "Low Security");

    // Just below → nullsec
    r = loader.GetRulesForSecurity(0.09f);
    assert(r != nullptr);
    assert(r->name == "Null Security");
}

// ══════════════════════════════════════════════════════════════════
// Rule IDs
// ══════════════════════════════════════════════════════════════════

void test_economy_rules_ids() {
    EconomyRulesLoader loader;
    loader.InstallDefaults();
    auto ids = loader.RulesIds();
    assert(ids.size() == 4);
}

// ══════════════════════════════════════════════════════════════════
// Text loading
// ══════════════════════════════════════════════════════════════════

void test_economy_rules_load_empty() {
    EconomyRulesLoader loader;
    assert(loader.LoadFromText("") == 0);
    assert(loader.RulesCount() == 0);
}

void test_economy_rules_load_single() {
    EconomyRulesLoader loader;
    std::string text =
        "[wormhole]\n"
        "name = Wormhole Space\n"
        "pirateSpawnRate = 0.0\n"
        "minerSpawnRate = 0.02\n"
        "miningYieldMultiplier = 3.0\n"
        "brokerFeePercent = 0.0\n"
        "salesTaxPercent = 0.0\n";

    assert(loader.LoadFromText(text) == 1);
    assert(loader.RulesCount() == 1);

    const EconomyRules* r = loader.GetRules("wormhole");
    assert(r != nullptr);
    assert(r->name == "Wormhole Space");
    assert(approxEq(r->pirateSpawnRate, 0.0f));
    assert(approxEq(r->miningYieldMultiplier, 3.0f));
    assert(approxEq(r->brokerFeePercent, 0.0f));
}

void test_economy_rules_load_multiple() {
    EconomyRulesLoader loader;
    std::string text =
        "[band_a]\n"
        "name = Band A\n"
        "pirateSpawnRate = 0.1\n"
        "\n"
        "[band_b]\n"
        "name = Band B\n"
        "pirateSpawnRate = 0.2\n";

    assert(loader.LoadFromText(text) == 2);
    assert(loader.RulesCount() == 2);

    assert(approxEq(loader.GetRules("band_a")->pirateSpawnRate, 0.1f));
    assert(approxEq(loader.GetRules("band_b")->pirateSpawnRate, 0.2f));
}

void test_economy_rules_load_overwrites_defaults() {
    EconomyRulesLoader loader;
    loader.InstallDefaults();

    std::string text =
        "[global]\n"
        "name = Modded Global\n"
        "brokerFeePercent = 5.0\n";

    loader.LoadFromText(text);
    const EconomyRules* g = loader.GetRules("global");
    assert(g != nullptr);
    assert(g->name == "Modded Global");
    assert(approxEq(g->brokerFeePercent, 5.0f));
    assert(loader.RulesCount() == 4);
}

void test_economy_rules_all_bands() {
    EconomyRulesLoader loader;
    loader.InstallDefaults();

    assert(loader.GetRules("global") != nullptr);
    assert(loader.GetRules("highsec") != nullptr);
    assert(loader.GetRules("lowsec") != nullptr);
    assert(loader.GetRules("nullsec") != nullptr);
}
