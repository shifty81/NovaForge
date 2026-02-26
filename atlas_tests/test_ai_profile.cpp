/**
 * Tests for AIProfileLoader — data-driven AI behavior configuration
 * for modding support.  Profiles define per-archetype combat, economy,
 * and lifecycle parameters.
 */

#include <cassert>
#include <cmath>
#include "../engine/sim/AIProfileLoader.h"

using namespace atlas::sim;

static bool approxEq(float a, float b, float eps = 0.01f) {
    return std::fabs(a - b) < eps;
}

// ══════════════════════════════════════════════════════════════════
// Defaults and construction
// ══════════════════════════════════════════════════════════════════

void test_ai_profile_defaults() {
    AIProfileLoader loader;
    assert(loader.ProfileCount() == 0);
    assert(loader.GetProfile("miner_default") == nullptr);
}

void test_ai_profile_install_defaults() {
    AIProfileLoader loader;
    loader.InstallDefaults();
    assert(loader.ProfileCount() == 6);

    const AIProfile* miner = loader.GetProfile("miner_default");
    assert(miner != nullptr);
    assert(miner->archetype == AIArchetype::Miner);
    assert(approxEq(miner->cargoCapacity, 400.0f));

    const AIProfile* pirate = loader.GetProfile("pirate_default");
    assert(pirate != nullptr);
    assert(pirate->archetype == AIArchetype::Pirate);
    assert(pirate->focusFire == true);
    assert(approxEq(pirate->travelSpeed, 120.0f));
}

// ══════════════════════════════════════════════════════════════════
// Add / remove
// ══════════════════════════════════════════════════════════════════

void test_ai_profile_add() {
    AIProfileLoader loader;
    AIProfile p;
    p.name = "Elite Pirate";
    p.archetype = AIArchetype::Pirate;
    p.hitPoints = 250.0f;
    assert(loader.AddProfile("pirate_elite", p));
    assert(loader.ProfileCount() == 1);

    const AIProfile* got = loader.GetProfile("pirate_elite");
    assert(got != nullptr);
    assert(got->name == "Elite Pirate");
    assert(approxEq(got->hitPoints, 250.0f));
}

void test_ai_profile_add_overwrites() {
    AIProfileLoader loader;
    AIProfile p1;
    p1.name = "V1";
    p1.hitPoints = 100.0f;
    loader.AddProfile("test", p1);

    AIProfile p2;
    p2.name = "V2";
    p2.hitPoints = 200.0f;
    loader.AddProfile("test", p2);

    assert(loader.ProfileCount() == 1);
    assert(loader.GetProfile("test")->name == "V2");
    assert(approxEq(loader.GetProfile("test")->hitPoints, 200.0f));
}

void test_ai_profile_remove() {
    AIProfileLoader loader;
    AIProfile p;
    loader.AddProfile("x", p);
    assert(loader.RemoveProfile("x"));
    assert(loader.ProfileCount() == 0);
}

void test_ai_profile_remove_nonexistent() {
    AIProfileLoader loader;
    assert(!loader.RemoveProfile("nope"));
}

void test_ai_profile_get_nonexistent() {
    AIProfileLoader loader;
    assert(loader.GetProfile("nope") == nullptr);
}

// ══════════════════════════════════════════════════════════════════
// Archetype queries
// ══════════════════════════════════════════════════════════════════

void test_ai_profile_get_by_archetype() {
    AIProfileLoader loader;
    loader.InstallDefaults();

    auto miners = loader.GetByArchetype(AIArchetype::Miner);
    assert(miners.size() == 1);
    assert(miners[0]->archetype == AIArchetype::Miner);

    auto pirates = loader.GetByArchetype(AIArchetype::Pirate);
    assert(pirates.size() == 1);
    assert(pirates[0]->archetype == AIArchetype::Pirate);
}

void test_ai_profile_get_by_archetype_multiple() {
    AIProfileLoader loader;
    loader.InstallDefaults();
    AIProfile p;
    p.archetype = AIArchetype::Pirate;
    p.name = "Elite Pirate";
    loader.AddProfile("pirate_elite", p);

    auto pirates = loader.GetByArchetype(AIArchetype::Pirate);
    assert(pirates.size() == 2);
}

// ══════════════════════════════════════════════════════════════════
// Profile IDs
// ══════════════════════════════════════════════════════════════════

void test_ai_profile_ids() {
    AIProfileLoader loader;
    loader.InstallDefaults();
    auto ids = loader.ProfileIds();
    assert(ids.size() == 6);
}

// ══════════════════════════════════════════════════════════════════
// Text loading
// ══════════════════════════════════════════════════════════════════

void test_ai_profile_load_empty() {
    AIProfileLoader loader;
    assert(loader.LoadFromText("") == 0);
    assert(loader.ProfileCount() == 0);
}

void test_ai_profile_load_single() {
    AIProfileLoader loader;
    std::string text =
        "[pirate_custom]\n"
        "name = Custom Pirate\n"
        "archetype = Pirate\n"
        "orbitRange = 8000\n"
        "retreatThreshold = 0.15\n"
        "focusFire = true\n"
        "hitPoints = 200\n";

    assert(loader.LoadFromText(text) == 1);
    assert(loader.ProfileCount() == 1);

    const AIProfile* p = loader.GetProfile("pirate_custom");
    assert(p != nullptr);
    assert(p->name == "Custom Pirate");
    assert(p->archetype == AIArchetype::Pirate);
    assert(approxEq(p->orbitRange, 8000.0f));
    assert(approxEq(p->retreatThreshold, 0.15f));
    assert(p->focusFire == true);
    assert(approxEq(p->hitPoints, 200.0f));
}

void test_ai_profile_load_multiple() {
    AIProfileLoader loader;
    std::string text =
        "[miner_fast]\n"
        "name = Fast Miner\n"
        "archetype = Miner\n"
        "travelSpeed = 200\n"
        "\n"
        "[trader_rich]\n"
        "name = Rich Trader\n"
        "archetype = Trader\n"
        "startingCredits = 50000\n";

    assert(loader.LoadFromText(text) == 2);
    assert(loader.ProfileCount() == 2);

    const AIProfile* m = loader.GetProfile("miner_fast");
    assert(m != nullptr);
    assert(approxEq(m->travelSpeed, 200.0f));

    const AIProfile* t = loader.GetProfile("trader_rich");
    assert(t != nullptr);
    assert(approxEq(t->startingCredits, 50000.0f));
}

void test_ai_profile_load_overwrites_defaults() {
    AIProfileLoader loader;
    loader.InstallDefaults();

    // Override the default pirate
    std::string text =
        "[pirate_default]\n"
        "name = Modded Pirate\n"
        "archetype = Pirate\n"
        "hitPoints = 500\n";

    loader.LoadFromText(text);
    const AIProfile* p = loader.GetProfile("pirate_default");
    assert(p != nullptr);
    assert(p->name == "Modded Pirate");
    assert(approxEq(p->hitPoints, 500.0f));
    // Defaults loaded count unchanged, just overwritten
    assert(loader.ProfileCount() == 6);
}

void test_ai_profile_all_archetypes() {
    AIProfileLoader loader;
    loader.InstallDefaults();

    assert(loader.GetProfile("miner_default")->archetype         == AIArchetype::Miner);
    assert(loader.GetProfile("hauler_default")->archetype        == AIArchetype::Hauler);
    assert(loader.GetProfile("trader_default")->archetype        == AIArchetype::Trader);
    assert(loader.GetProfile("pirate_default")->archetype        == AIArchetype::Pirate);
    assert(loader.GetProfile("security_default")->archetype      == AIArchetype::Security);
    assert(loader.GetProfile("industrialist_default")->archetype == AIArchetype::Industrialist);
}
