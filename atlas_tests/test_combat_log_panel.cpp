/**
 * Tests for CombatLogPanel:
 *   - Construction & defaults
 *   - Add/remove entries
 *   - Entry damage types
 *   - Add/remove engagements
 *   - Engagement DPS and accuracy
 *   - Aggregate stats
 *   - Max entries/engagements limit
 *   - Export JSON
 *   - Headless draw
 */

#include <cassert>
#include <string>
#include "../editor/tools/CombatLogPanel.h"

using namespace atlas::editor;

void test_combat_log_panel_defaults() {
    CombatLogPanel panel;
    assert(std::string(panel.Name()) == "Combat Log Viewer");
    assert(panel.EntryCount() == 0);
    assert(panel.EngagementCount() == 0);
    assert(panel.TotalDamage() < 0.01f);
    assert(panel.TotalHits() == 0);
    assert(panel.TotalMisses() == 0);
}

void test_combat_log_panel_add_entry() {
    CombatLogPanel panel;
    int id = panel.AddEntry("ship1", "ship2", "Kinetic", 500.0f, "Railgun", true);
    assert(id > 0);
    assert(panel.EntryCount() == 1);
    int id2 = panel.AddEntry("ship1", "ship2", "EM", 300.0f, "Pulse Laser", false);
    assert(id2 > 0);
    assert(panel.EntryCount() == 2);
}

void test_combat_log_panel_entry_validation() {
    CombatLogPanel panel;
    assert(panel.AddEntry("", "ship2", "Kinetic", 100.0f, "Gun", true) == -1);
    assert(panel.AddEntry("ship1", "", "Kinetic", 100.0f, "Gun", true) == -1);
    assert(panel.AddEntry("ship1", "ship2", "Invalid", 100.0f, "Gun", true) == -1);
    assert(panel.EntryCount() == 0);
}

void test_combat_log_panel_damage_types() {
    CombatLogPanel panel;
    assert(panel.AddEntry("s1", "s2", "EM", 100.0f, "w", true) > 0);
    assert(panel.AddEntry("s1", "s2", "Thermal", 200.0f, "w", true) > 0);
    assert(panel.AddEntry("s1", "s2", "Kinetic", 300.0f, "w", true) > 0);
    assert(panel.AddEntry("s1", "s2", "Explosive", 400.0f, "w", true) > 0);
    assert(panel.EntryCount() == 4);
    float total = panel.TotalDamage();
    assert(total > 999.0f && total < 1001.0f);
}

void test_combat_log_panel_remove_entry() {
    CombatLogPanel panel;
    int id = panel.AddEntry("s1", "s2", "Kinetic", 100.0f, "Gun", true);
    assert(panel.RemoveEntry(static_cast<uint32_t>(id)));
    assert(panel.EntryCount() == 0);
    assert(!panel.RemoveEntry(static_cast<uint32_t>(id))); // double remove
}

void test_combat_log_panel_add_engagement() {
    CombatLogPanel panel;
    int id = panel.AddEngagement("Battle of Jita", "Victory");
    assert(id > 0);
    assert(panel.EngagementCount() == 1);
    int id2 = panel.AddEngagement("Pirate Ambush", "Defeat");
    assert(id2 > 0);
    assert(panel.EngagementCount() == 2);
}

void test_combat_log_panel_engagement_validation() {
    CombatLogPanel panel;
    assert(panel.AddEngagement("", "Victory") == -1);
    assert(panel.AddEngagement("Battle", "Invalid") == -1);
    assert(panel.EngagementCount() == 0);
}

void test_combat_log_panel_remove_engagement() {
    CombatLogPanel panel;
    int id = panel.AddEngagement("Battle", "Victory");
    assert(panel.RemoveEngagement(static_cast<uint32_t>(id)));
    assert(panel.EngagementCount() == 0);
    assert(!panel.RemoveEngagement(static_cast<uint32_t>(id)));
}

void test_combat_log_panel_engagement_dps() {
    CombatLogPanel panel;
    int id = panel.AddEngagement("Battle", "Victory");
    panel.SetEngagementDuration(static_cast<uint32_t>(id), 10.0f);
    panel.SetEngagementDamage(static_cast<uint32_t>(id), 5000.0f);
    float dps = panel.GetEngagementDPS(static_cast<uint32_t>(id));
    assert(dps > 499.0f && dps < 501.0f); // 5000/10 = 500
    // Zero duration should return 0
    int id2 = panel.AddEngagement("Quick", "Draw");
    assert(panel.GetEngagementDPS(static_cast<uint32_t>(id2)) < 0.01f);
}

void test_combat_log_panel_engagement_accuracy() {
    CombatLogPanel panel;
    int id = panel.AddEngagement("Battle", "Victory");
    panel.SetEngagementHits(static_cast<uint32_t>(id), 80, 20);
    float acc = panel.GetEngagementAccuracy(static_cast<uint32_t>(id));
    assert(acc > 0.79f && acc < 0.81f); // 80/100 = 0.8
    // Zero hits+misses should return 0
    int id2 = panel.AddEngagement("Empty", "Ongoing");
    assert(panel.GetEngagementAccuracy(static_cast<uint32_t>(id2)) < 0.01f);
}

void test_combat_log_panel_hit_miss_stats() {
    CombatLogPanel panel;
    panel.AddEntry("s1", "s2", "Kinetic", 100.0f, "Gun", true);
    panel.AddEntry("s1", "s2", "Kinetic", 0.0f, "Gun", false);
    panel.AddEntry("s1", "s2", "EM", 200.0f, "Laser", true);
    assert(panel.TotalHits() == 2);
    assert(panel.TotalMisses() == 1);
    float dmg = panel.TotalDamage();
    assert(dmg > 299.0f && dmg < 301.0f); // only hits count
}

void test_combat_log_panel_max_entries() {
    CombatLogPanel panel;
    for (int i = 0; i < 200; ++i) {
        int id = panel.AddEntry("s1", "s2", "Kinetic", 10.0f, "Gun", true);
        assert(id > 0);
    }
    assert(panel.AddEntry("s1", "s2", "Kinetic", 10.0f, "Gun", true) == -1);
    assert(panel.EntryCount() == 200);
}

void test_combat_log_panel_max_engagements() {
    CombatLogPanel panel;
    for (int i = 0; i < 20; ++i) {
        int id = panel.AddEngagement("Battle " + std::to_string(i), "Victory");
        assert(id > 0);
    }
    assert(panel.AddEngagement("Extra", "Victory") == -1);
    assert(panel.EngagementCount() == 20);
}

void test_combat_log_panel_export_json() {
    CombatLogPanel panel;
    panel.AddEntry("attacker1", "defender1", "Kinetic", 500.0f, "Railgun", true);
    panel.AddEngagement("TestBattle", "Victory");
    std::string json = panel.ExportJSON();
    assert(json.find("attacker1") != std::string::npos);
    assert(json.find("defender1") != std::string::npos);
    assert(json.find("Kinetic") != std::string::npos);
    assert(json.find("TestBattle") != std::string::npos);
    assert(json.find("Victory") != std::string::npos);
}

void test_combat_log_panel_headless_draw() {
    CombatLogPanel panel;
    panel.AddEntry("s1", "s2", "EM", 100.0f, "Laser", true);
    panel.Draw(); // should not crash without context
}
