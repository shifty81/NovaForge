/**
 * Tests for the Data Browser, Module Editor, and NPC Editor panels.
 */

#include <cassert>
#include <string>
#include "../editor/tools/DataBrowserPanel.h"
#include "../editor/tools/ModuleEditorPanel.h"
#include "../editor/tools/NPCEditorPanel.h"

using namespace atlas::editor;

// ══════════════════════════════════════════════════════════════════
// DataBrowserPanel tests
// ══════════════════════════════════════════════════════════════════

void test_data_browser_defaults() {
    DataBrowserPanel panel;
    assert(panel.EntryCount() == 0);
    assert(panel.SelectedEntry() == -1);
    assert(panel.CurrentCategory().empty());
    assert(panel.Filter().empty());
    assert(std::string(panel.Name()) == "Data Browser");
}

void test_data_browser_load_category() {
    DataBrowserPanel panel;
    panel.LoadCategory("modules");
    assert(panel.CurrentCategory() == "modules");
    assert(panel.EntryCount() == 0); // no entries until added
}

void test_data_browser_add_entry() {
    DataBrowserPanel panel;
    panel.LoadCategory("ships");

    DataEntry entry;
    entry.id = "fang";
    entry.fields.push_back({"name", "Fang", DataField::Type::String});
    entry.fields.push_back({"hull_hp", "350", DataField::Type::Number});

    size_t idx = panel.AddEntry(entry);
    assert(idx == 0);
    assert(panel.EntryCount() == 1);
    assert(panel.GetEntry(0).id == "fang");
    assert(panel.GetEntry(0).fields.size() == 2);
}

void test_data_browser_remove_entry() {
    DataBrowserPanel panel;
    panel.LoadCategory("skills");

    DataEntry e1; e1.id = "gunnery";
    DataEntry e2; e2.id = "missiles";
    panel.AddEntry(e1);
    panel.AddEntry(e2);
    assert(panel.EntryCount() == 2);

    bool removed = panel.RemoveEntry(0);
    assert(removed);
    assert(panel.EntryCount() == 1);
    assert(panel.GetEntry(0).id == "missiles");
}

void test_data_browser_update_field() {
    DataBrowserPanel panel;
    DataEntry entry;
    entry.id = "item";
    entry.fields.push_back({"damage", "42", DataField::Type::Number});
    panel.AddEntry(entry);

    bool updated = panel.UpdateEntryField(0, 0, "50");
    assert(updated);
    assert(panel.GetEntry(0).fields[0].value == "50");
}

void test_data_browser_selection() {
    DataBrowserPanel panel;
    DataEntry e1; e1.id = "a";
    DataEntry e2; e2.id = "b";
    panel.AddEntry(e1);
    panel.AddEntry(e2);

    panel.SelectEntry(1);
    assert(panel.SelectedEntry() == 1);
    panel.ClearSelection();
    assert(panel.SelectedEntry() == -1);
}

void test_data_browser_filter() {
    DataBrowserPanel panel;
    DataEntry e1; e1.id = "Ferrite";
    DataEntry e2; e2.id = "Galvite";
    DataEntry e3; e3.id = "FerroAlloy";
    panel.AddEntry(e1);
    panel.AddEntry(e2);
    panel.AddEntry(e3);

    panel.SetFilter("fer");
    assert(panel.FilteredCount() == 2); // Ferrite + FerroAlloy
    panel.SetFilter("");
    assert(panel.FilteredCount() == 3);
}

void test_data_browser_export_import_json() {
    DataBrowserPanel panel;
    panel.LoadCategory("test");

    DataEntry entry;
    entry.id = "test_item";
    entry.fields.push_back({"name", "Test Item", DataField::Type::String});
    entry.fields.push_back({"value", "100", DataField::Type::Number});
    panel.AddEntry(entry);

    std::string json = panel.ExportToJson();
    assert(!json.empty());
    assert(json.find("test_item") != std::string::npos);

    // Import into a new panel
    DataBrowserPanel panel2;
    panel2.LoadCategory("test");
    size_t imported = panel2.ImportFromJson(json);
    assert(imported == 1);
    assert(panel2.GetEntry(0).id == "test_item");
}

void test_data_browser_categories_count() {
    assert(DataBrowserPanel::kCategoryCount == 17);
}

// ══════════════════════════════════════════════════════════════════
// ModuleEditorPanel tests
// ══════════════════════════════════════════════════════════════════

static ModuleEntry makeValidModule(const std::string& id,
                                    const std::string& type,
                                    const std::string& slot) {
    ModuleEntry m;
    m.moduleId = id;
    m.name = id;
    m.type = type;
    m.slot = slot;
    m.cpu = 10;
    m.powergrid = 5;
    if (type == "weapon") {
        m.damage = 42.0f;
        m.rateOfFire = 5.0f;
        m.optimalRange = 1500.0f;
        m.falloffRange = 4000.0f;
    }
    return m;
}

void test_module_editor_defaults() {
    ModuleEditorPanel panel;
    assert(panel.ModuleCount() == 0);
    assert(panel.SelectedModule() == -1);
    assert(panel.TypeFilter().empty());
    assert(panel.SlotFilter().empty());
    assert(std::string(panel.Name()) == "Module Editor");
}

void test_module_editor_add_module() {
    ModuleEditorPanel panel;
    auto m = makeValidModule("200mm_auto", "weapon", "high");
    size_t idx = panel.AddModule(m);
    assert(idx == 0);
    assert(panel.ModuleCount() == 1);
    assert(panel.GetModule(0).moduleId == "200mm_auto");
}

void test_module_editor_remove_module() {
    ModuleEditorPanel panel;
    panel.AddModule(makeValidModule("a", "weapon", "high"));
    panel.AddModule(makeValidModule("b", "defense", "mid"));

    panel.SelectModule(0);
    bool removed = panel.RemoveModule(0);
    assert(removed);
    assert(panel.ModuleCount() == 1);
    assert(panel.SelectedModule() == -1);
}

void test_module_editor_update_module() {
    ModuleEditorPanel panel;
    panel.AddModule(makeValidModule("gun", "weapon", "high"));
    auto updated = panel.GetModule(0);
    updated.damage = 100.0f;
    assert(panel.UpdateModule(0, updated));
    assert(panel.GetModule(0).damage == 100.0f);
}

void test_module_editor_type_filter() {
    ModuleEditorPanel panel;
    panel.AddModule(makeValidModule("gun1", "weapon", "high"));
    panel.AddModule(makeValidModule("shield", "defense", "mid"));
    panel.AddModule(makeValidModule("gun2", "weapon", "high"));

    panel.SetTypeFilter("weapon");
    assert(panel.FilteredCount() == 2);
    panel.SetTypeFilter("defense");
    assert(panel.FilteredCount() == 1);
    panel.SetTypeFilter("");
    assert(panel.FilteredCount() == 3);
}

void test_module_editor_slot_filter() {
    ModuleEditorPanel panel;
    panel.AddModule(makeValidModule("gun", "weapon", "high"));
    panel.AddModule(makeValidModule("shield", "defense", "mid"));

    panel.SetSlotFilter("high");
    assert(panel.FilteredCount() == 1);
    panel.SetSlotFilter("");
    assert(panel.FilteredCount() == 2);
}

void test_module_editor_validate_valid() {
    auto m = makeValidModule("gun", "weapon", "high");
    std::string err;
    assert(ModuleEditorPanel::ValidateModule(m, err));
    assert(err.empty());
}

void test_module_editor_validate_missing_id() {
    ModuleEntry m;
    m.name = "Test";
    m.type = "weapon";
    m.slot = "high";
    m.damage = 10.0f;
    m.rateOfFire = 5.0f;
    std::string err;
    assert(!ModuleEditorPanel::ValidateModule(m, err));
    assert(err.find("ID") != std::string::npos);
}

void test_module_editor_validate_weapon_no_rof() {
    auto m = makeValidModule("gun", "weapon", "high");
    m.rateOfFire = 0.0f;
    std::string err;
    assert(!ModuleEditorPanel::ValidateModule(m, err));
    assert(err.find("fire") != std::string::npos);
}

void test_module_editor_validate_all() {
    ModuleEditorPanel panel;
    panel.AddModule(makeValidModule("good", "weapon", "high"));
    ModuleEntry bad;
    bad.moduleId = "bad";
    bad.name = "Bad";
    bad.type = "weapon";
    bad.slot = "high";
    bad.rateOfFire = 0.0f; // invalid
    panel.AddModule(bad);

    size_t invalid = panel.ValidateAll();
    assert(invalid == 1);
}

void test_module_editor_export_json() {
    ModuleEditorPanel panel;
    panel.AddModule(makeValidModule("gun", "weapon", "high"));
    std::string json = panel.ExportToJson();
    assert(!json.empty());
    assert(json.find("gun") != std::string::npos);
    assert(json.find("weapon") != std::string::npos);
}

// ══════════════════════════════════════════════════════════════════
// NPCEditorPanel tests
// ══════════════════════════════════════════════════════════════════

static NPCEntry makeValidNPC(const std::string& id,
                               const std::string& archetype) {
    NPCEntry n;
    n.npcId = id;
    n.name = id;
    n.type = "frigate";
    n.faction = "venom_syndicate";
    n.behavior = "aggressive";
    n.archetype = archetype;
    n.hullHp = 300.0f;
    n.armorHp = 250.0f;
    n.shieldHp = 350.0f;
    n.maxVelocity = 400.0f;
    n.bounty = 12500.0;
    n.startingWallet = 10000.0;
    n.ownedShipType = "Rifter";
    n.shipValue = 5000.0;
    return n;
}

void test_npc_editor_defaults() {
    NPCEditorPanel panel;
    assert(panel.NPCCount() == 0);
    assert(panel.SelectedNPC() == -1);
    assert(panel.FactionFilter().empty());
    assert(panel.ArchetypeFilter().empty());
    assert(std::string(panel.Name()) == "NPC Editor");
}

void test_npc_editor_add_npc() {
    NPCEditorPanel panel;
    auto n = makeValidNPC("scout_1", "pirate");
    size_t idx = panel.AddNPC(n);
    assert(idx == 0);
    assert(panel.NPCCount() == 1);
    assert(panel.GetNPC(0).npcId == "scout_1");
}

void test_npc_editor_remove_npc() {
    NPCEditorPanel panel;
    panel.AddNPC(makeValidNPC("a", "pirate"));
    panel.AddNPC(makeValidNPC("b", "miner"));

    panel.SelectNPC(0);
    bool removed = panel.RemoveNPC(0);
    assert(removed);
    assert(panel.NPCCount() == 1);
    assert(panel.SelectedNPC() == -1);
}

void test_npc_editor_update_npc() {
    NPCEditorPanel panel;
    panel.AddNPC(makeValidNPC("pirate_1", "pirate"));
    auto updated = panel.GetNPC(0);
    updated.bounty = 50000.0;
    assert(panel.UpdateNPC(0, updated));
    assert(panel.GetNPC(0).bounty == 50000.0);
}

void test_npc_editor_faction_filter() {
    NPCEditorPanel panel;
    auto n1 = makeValidNPC("a", "pirate");
    n1.faction = "venom_syndicate";
    auto n2 = makeValidNPC("b", "pirate");
    n2.faction = "iron_corsairs";
    panel.AddNPC(n1);
    panel.AddNPC(n2);

    panel.SetFactionFilter("venom_syndicate");
    assert(panel.FilteredCount() == 1);
    panel.SetFactionFilter("");
    assert(panel.FilteredCount() == 2);
}

void test_npc_editor_archetype_filter() {
    NPCEditorPanel panel;
    panel.AddNPC(makeValidNPC("a", "pirate"));
    panel.AddNPC(makeValidNPC("b", "miner"));
    panel.AddNPC(makeValidNPC("c", "hauler"));

    panel.SetArchetypeFilter("miner");
    assert(panel.FilteredCount() == 1);
    panel.SetArchetypeFilter("");
    assert(panel.FilteredCount() == 3);
}

void test_npc_editor_validate_valid() {
    auto n = makeValidNPC("scout", "pirate");
    std::string err;
    assert(NPCEditorPanel::ValidateNPC(n, err));
    assert(err.empty());
}

void test_npc_editor_validate_zero_hp() {
    NPCEntry n;
    n.npcId = "bad";
    n.name = "Bad NPC";
    n.type = "frigate";
    n.archetype = "pirate";
    // All HP zero
    std::string err;
    assert(!NPCEditorPanel::ValidateNPC(n, err));
    assert(err.find("HP") != std::string::npos);
}

void test_npc_editor_validate_miner_needs_wallet() {
    auto n = makeValidNPC("miner_1", "miner");
    n.startingWallet = 0.0;
    std::string err;
    assert(!NPCEditorPanel::ValidateNPC(n, err));
    assert(err.find("wallet") != std::string::npos);
}

void test_npc_editor_validate_all() {
    NPCEditorPanel panel;
    panel.AddNPC(makeValidNPC("good", "pirate"));
    NPCEntry bad;
    bad.npcId = "bad";
    bad.name = "Bad";
    bad.type = "frigate";
    bad.archetype = "pirate";
    // zero HP
    panel.AddNPC(bad);

    size_t invalid = panel.ValidateAll();
    assert(invalid == 1);
}

void test_npc_editor_export_json() {
    NPCEditorPanel panel;
    auto n = makeValidNPC("pirate_1", "pirate");
    n.weapons.push_back({"small_hybrid", 28.0f});
    n.lootTable.push_back("scrap_metal");
    panel.AddNPC(n);

    std::string json = panel.ExportToJson();
    assert(!json.empty());
    assert(json.find("pirate_1") != std::string::npos);
    assert(json.find("small_hybrid") != std::string::npos);
    assert(json.find("scrap_metal") != std::string::npos);
}

void test_npc_editor_hauler_station() {
    NPCEditorPanel panel;
    auto n = makeValidNPC("hauler_1", "hauler");
    n.haulStationId = "jita_station";
    panel.AddNPC(n);

    assert(panel.GetNPC(0).haulStationId == "jita_station");
    std::string json = panel.ExportToJson();
    assert(json.find("jita_station") != std::string::npos);
}
