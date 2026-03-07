/**
 * Tests for the Mission Editor panel, Undo/Redo stack,
 * and Template AI Backend.
 */

#include <iostream>
#include <cassert>
#include <string>
#include "../editor/tools/MissionEditorPanel.h"
#include "../editor/ui/UndoStack.h"
#include "../editor/ai/TemplateAIBackend.h"

using namespace atlas::editor;
using namespace atlas::ai;

// ══════════════════════════════════════════════════════════════════
// Helper: create a valid mission template entry
// ══════════════════════════════════════════════════════════════════

static MissionTemplateEntry makeValidTemplate(const std::string& id,
                                               const std::string& type,
                                               int level) {
    MissionTemplateEntry entry;
    entry.templateId = id;
    entry.namePattern = id + ": {system}";
    entry.type = type;
    entry.level = level;
    entry.objectives.push_back({"destroy", "pirate_frigate", 1, 5});
    entry.baseIsc = 100000.0;
    return entry;
}

// ══════════════════════════════════════════════════════════════════
// Mission Editor Panel tests
// ══════════════════════════════════════════════════════════════════

void test_mission_editor_defaults() {
    MissionEditorPanel panel;
    assert(panel.TemplateCount() == 0);
    assert(panel.SelectedTemplate() == -1);
    assert(panel.TypeFilter().empty());
    assert(panel.LevelFilter() == 0);
    assert(std::string(panel.Name()) == "Mission Editor");
}

void test_mission_editor_add_template() {
    MissionEditorPanel panel;
    auto entry = makeValidTemplate("combat_1", "combat", 1);
    size_t idx = panel.AddTemplate(entry);
    assert(idx == 0);
    assert(panel.TemplateCount() == 1);
    assert(panel.GetTemplate(0).templateId == "combat_1");
    assert(panel.GetTemplate(0).type == "combat");
}

void test_mission_editor_add_multiple() {
    MissionEditorPanel panel;
    panel.AddTemplate(makeValidTemplate("combat_1", "combat", 1));
    panel.AddTemplate(makeValidTemplate("mining_1", "mining", 2));
    panel.AddTemplate(makeValidTemplate("courier_1", "courier", 3));
    assert(panel.TemplateCount() == 3);
    assert(panel.GetTemplate(1).type == "mining");
    assert(panel.GetTemplate(2).level == 3);
}

void test_mission_editor_remove_template() {
    MissionEditorPanel panel;
    panel.AddTemplate(makeValidTemplate("a", "combat", 1));
    panel.AddTemplate(makeValidTemplate("b", "mining", 2));
    panel.AddTemplate(makeValidTemplate("c", "courier", 3));
    assert(panel.RemoveTemplate(1));
    assert(panel.TemplateCount() == 2);
    assert(panel.GetTemplate(0).templateId == "a");
    assert(panel.GetTemplate(1).templateId == "c");
}

void test_mission_editor_remove_out_of_range() {
    MissionEditorPanel panel;
    panel.AddTemplate(makeValidTemplate("a", "combat", 1));
    assert(!panel.RemoveTemplate(5));
    assert(panel.TemplateCount() == 1);
}

void test_mission_editor_update_template() {
    MissionEditorPanel panel;
    panel.AddTemplate(makeValidTemplate("a", "combat", 1));
    auto updated = makeValidTemplate("a_v2", "mining", 3);
    assert(panel.UpdateTemplate(0, updated));
    assert(panel.GetTemplate(0).templateId == "a_v2");
    assert(panel.GetTemplate(0).type == "mining");
}

void test_mission_editor_update_out_of_range() {
    MissionEditorPanel panel;
    auto entry = makeValidTemplate("a", "combat", 1);
    assert(!panel.UpdateTemplate(0, entry));
}

void test_mission_editor_select_template() {
    MissionEditorPanel panel;
    panel.AddTemplate(makeValidTemplate("a", "combat", 1));
    panel.AddTemplate(makeValidTemplate("b", "mining", 2));
    panel.SelectTemplate(1);
    assert(panel.SelectedTemplate() == 1);
}

void test_mission_editor_select_invalid_ignored() {
    MissionEditorPanel panel;
    panel.AddTemplate(makeValidTemplate("a", "combat", 1));
    panel.SelectTemplate(5);
    assert(panel.SelectedTemplate() == -1);
    panel.SelectTemplate(-1);
    assert(panel.SelectedTemplate() == -1);
}

void test_mission_editor_clear_selection() {
    MissionEditorPanel panel;
    panel.AddTemplate(makeValidTemplate("a", "combat", 1));
    panel.SelectTemplate(0);
    assert(panel.SelectedTemplate() == 0);
    panel.ClearSelection();
    assert(panel.SelectedTemplate() == -1);
}

void test_mission_editor_remove_fixes_selection() {
    MissionEditorPanel panel;
    panel.AddTemplate(makeValidTemplate("a", "combat", 1));
    panel.AddTemplate(makeValidTemplate("b", "mining", 2));
    panel.AddTemplate(makeValidTemplate("c", "courier", 3));
    panel.SelectTemplate(1); // select "b"
    panel.RemoveTemplate(1); // remove "b"
    assert(panel.SelectedTemplate() == -1);
}

void test_mission_editor_type_filter() {
    MissionEditorPanel panel;
    panel.AddTemplate(makeValidTemplate("a", "combat", 1));
    panel.AddTemplate(makeValidTemplate("b", "mining", 2));
    panel.AddTemplate(makeValidTemplate("c", "combat", 3));
    assert(panel.FilteredCount() == 3); // no filter
    panel.SetTypeFilter("combat");
    assert(panel.FilteredCount() == 2);
    panel.SetTypeFilter("mining");
    assert(panel.FilteredCount() == 1);
    panel.SetTypeFilter("");
    assert(panel.FilteredCount() == 3);
}

void test_mission_editor_level_filter() {
    MissionEditorPanel panel;
    panel.AddTemplate(makeValidTemplate("a", "combat", 1));
    panel.AddTemplate(makeValidTemplate("b", "combat", 2));
    panel.AddTemplate(makeValidTemplate("c", "combat", 1));
    panel.SetLevelFilter(1);
    assert(panel.FilteredCount() == 2);
    panel.SetLevelFilter(2);
    assert(panel.FilteredCount() == 1);
    panel.SetLevelFilter(0);
    assert(panel.FilteredCount() == 3);
}

void test_mission_editor_combined_filter() {
    MissionEditorPanel panel;
    panel.AddTemplate(makeValidTemplate("a", "combat", 1));
    panel.AddTemplate(makeValidTemplate("b", "mining", 1));
    panel.AddTemplate(makeValidTemplate("c", "combat", 2));
    panel.SetTypeFilter("combat");
    panel.SetLevelFilter(1);
    assert(panel.FilteredCount() == 1);
}

void test_mission_editor_validate_valid() {
    auto entry = makeValidTemplate("test", "combat", 3);
    std::string err;
    assert(MissionEditorPanel::ValidateTemplate(entry, err));
    assert(err.empty());
}

void test_mission_editor_validate_empty_id() {
    MissionTemplateEntry entry;
    entry.type = "combat";
    entry.level = 1;
    entry.objectives.push_back({"destroy", "enemy", 1, 5});
    std::string err;
    assert(!MissionEditorPanel::ValidateTemplate(entry, err));
    assert(err.find("ID") != std::string::npos);
}

void test_mission_editor_validate_empty_type() {
    MissionTemplateEntry entry;
    entry.templateId = "test";
    entry.level = 1;
    entry.objectives.push_back({"destroy", "enemy", 1, 5});
    std::string err;
    assert(!MissionEditorPanel::ValidateTemplate(entry, err));
    assert(err.find("type") != std::string::npos);
}

void test_mission_editor_validate_bad_level() {
    auto entry = makeValidTemplate("test", "combat", 0);
    std::string err;
    assert(!MissionEditorPanel::ValidateTemplate(entry, err));

    entry.level = 6;
    assert(!MissionEditorPanel::ValidateTemplate(entry, err));
}

void test_mission_editor_validate_no_objectives() {
    MissionTemplateEntry entry;
    entry.templateId = "test";
    entry.type = "combat";
    entry.level = 1;
    std::string err;
    assert(!MissionEditorPanel::ValidateTemplate(entry, err));
    assert(err.find("objective") != std::string::npos);
}

void test_mission_editor_validate_bad_objective() {
    auto entry = makeValidTemplate("test", "combat", 1);
    entry.objectives[0].countMax = 0; // less than countMin
    std::string err;
    assert(!MissionEditorPanel::ValidateTemplate(entry, err));
}

void test_mission_editor_validate_negative_isc() {
    auto entry = makeValidTemplate("test", "combat", 1);
    entry.baseIsc = -100.0;
    std::string err;
    assert(!MissionEditorPanel::ValidateTemplate(entry, err));
}

void test_mission_editor_validate_all() {
    MissionEditorPanel panel;
    panel.AddTemplate(makeValidTemplate("good1", "combat", 1));
    panel.AddTemplate(makeValidTemplate("good2", "mining", 2));
    assert(panel.ValidateAll() == 0);

    MissionTemplateEntry bad;
    bad.templateId = "";  // invalid: empty ID
    bad.type = "combat";
    bad.level = 1;
    panel.AddTemplate(bad);
    assert(panel.ValidateAll() == 1);
}

void test_mission_editor_export_json() {
    MissionEditorPanel panel;
    panel.AddTemplate(makeValidTemplate("combat_1", "combat", 1));
    panel.AddTemplate(makeValidTemplate("mining_1", "mining", 2));
    std::string json = panel.ExportToJson();
    assert(json.find("mission_templates") != std::string::npos);
    assert(json.find("combat_1") != std::string::npos);
    assert(json.find("mining_1") != std::string::npos);
    assert(json.find("pirate_frigate") != std::string::npos);
}

void test_mission_editor_import_json() {
    MissionEditorPanel panel;
    std::string json = R"({"mission_templates":[
        {"template_id":"imported_1","type":"combat"},
        {"template_id":"imported_2","type":"mining"}
    ]})";
    size_t count = panel.ImportFromJson(json);
    assert(count == 2);
    assert(panel.TemplateCount() == 2);
    assert(panel.GetTemplate(0).templateId == "imported_1");
    assert(panel.GetTemplate(1).templateId == "imported_2");
}

void test_mission_editor_draw_does_not_crash() {
    MissionEditorPanel panel;
    panel.AddTemplate(makeValidTemplate("a", "combat", 1));
    panel.SelectTemplate(0);
    panel.Draw(); // headless — no context
}

void test_mission_editor_log_after_actions() {
    MissionEditorPanel panel;
    panel.AddTemplate(makeValidTemplate("a", "combat", 1));
    panel.SelectTemplate(0);
    panel.RemoveTemplate(0);
    assert(panel.Log().size() >= 3); // init + add + select + remove
}

// ══════════════════════════════════════════════════════════════════
// Undo Stack tests
// ══════════════════════════════════════════════════════════════════

void test_undo_stack_defaults() {
    UndoStack stack;
    assert(!stack.CanUndo());
    assert(!stack.CanRedo());
    assert(stack.UndoCount() == 0);
    assert(stack.RedoCount() == 0);
    assert(stack.UndoDescription().empty());
    assert(stack.RedoDescription().empty());
}

void test_undo_stack_push_and_undo() {
    int value = 0;
    UndoStack stack;
    stack.PushAction({"Set to 1",
        [&]() { value = 0; },
        [&]() { value = 1; }});
    value = 1; // simulate the action
    assert(stack.CanUndo());
    assert(stack.UndoCount() == 1);
    assert(stack.UndoDescription() == "Set to 1");

    assert(stack.Undo());
    assert(value == 0);
    assert(!stack.CanUndo());
    assert(stack.CanRedo());
}

void test_undo_stack_redo() {
    int value = 0;
    UndoStack stack;
    stack.PushAction({"Set to 1",
        [&]() { value = 0; },
        [&]() { value = 1; }});
    value = 1;

    stack.Undo();
    assert(value == 0);
    assert(stack.CanRedo());
    assert(stack.RedoDescription() == "Set to 1");

    assert(stack.Redo());
    assert(value == 1);
    assert(!stack.CanRedo());
}

void test_undo_stack_multiple_actions() {
    int value = 0;
    UndoStack stack;
    stack.PushAction({"Set to 1",
        [&]() { value = 0; },
        [&]() { value = 1; }});
    value = 1;
    stack.PushAction({"Set to 2",
        [&]() { value = 1; },
        [&]() { value = 2; }});
    value = 2;

    assert(stack.UndoCount() == 2);
    stack.Undo();
    assert(value == 1);
    stack.Undo();
    assert(value == 0);
    assert(!stack.CanUndo());

    stack.Redo();
    assert(value == 1);
    stack.Redo();
    assert(value == 2);
}

void test_undo_stack_push_clears_redo() {
    int value = 0;
    UndoStack stack;
    stack.PushAction({"A", [&]() { value = 0; }, [&]() { value = 1; }});
    value = 1;
    stack.PushAction({"B", [&]() { value = 1; }, [&]() { value = 2; }});
    value = 2;

    stack.Undo(); // back to B's undo → value = 1
    assert(stack.CanRedo());

    // Push a new action — should discard the redo future (B)
    stack.PushAction({"C", [&]() { value = 1; }, [&]() { value = 3; }});
    value = 3;
    assert(!stack.CanRedo());
    assert(stack.UndoCount() == 2); // A and C
}

void test_undo_stack_clear() {
    UndoStack stack;
    stack.PushAction({"A", [](){}, [](){}});
    stack.PushAction({"B", [](){}, [](){}});
    assert(stack.UndoCount() == 2);
    stack.Clear();
    assert(stack.UndoCount() == 0);
    assert(stack.RedoCount() == 0);
    assert(!stack.CanUndo());
    assert(!stack.CanRedo());
}

void test_undo_stack_max_depth() {
    UndoStack stack(3); // max depth 3
    assert(stack.MaxDepth() == 3);

    stack.PushAction({"A", [](){}, [](){}});
    stack.PushAction({"B", [](){}, [](){}});
    stack.PushAction({"C", [](){}, [](){}});
    assert(stack.UndoCount() == 3);

    // Push a 4th — oldest (A) should be dropped
    stack.PushAction({"D", [](){}, [](){}});
    assert(stack.UndoCount() == 3);

    // The oldest should now be B
    stack.Undo(); // D → C
    stack.Undo(); // C → B
    assert(stack.CanUndo()); // B is still there
    stack.Undo();
    assert(!stack.CanUndo()); // no more
}

void test_undo_stack_empty_undo_returns_false() {
    UndoStack stack;
    assert(!stack.Undo());
}

void test_undo_stack_empty_redo_returns_false() {
    UndoStack stack;
    assert(!stack.Redo());
}

// ══════════════════════════════════════════════════════════════════
// Template AI Backend tests
// ══════════════════════════════════════════════════════════════════

void test_template_ai_defaults_installed() {
    TemplateAIBackend backend;
    assert(backend.TemplateCount() > 0);
}

void test_template_ai_query_match() {
    TemplateAIBackend backend;
    AIContext ctx;
    auto resp = backend.Query("I need a frigate build", ctx);
    assert(resp.confidence > 0.0f);
    assert(resp.content.find("frigate") != std::string::npos);
}

void test_template_ai_query_no_match() {
    TemplateAIBackend backend;
    backend.ClearTemplates();
    backend.AddTemplate("xyz123", "response", 0.5f);
    AIContext ctx;
    auto resp = backend.Query("completely unrelated prompt", ctx);
    assert(resp.confidence == 0.0f);
}

void test_template_ai_case_insensitive() {
    TemplateAIBackend backend;
    backend.ClearTemplates();
    backend.AddTemplate("CRUISER", "cruiser response", 0.7f);
    AIContext ctx;
    auto resp = backend.Query("tell me about a cruiser", ctx);
    assert(resp.confidence > 0.0f);
    assert(resp.content == "cruiser response");
}

void test_template_ai_best_match_wins() {
    TemplateAIBackend backend;
    backend.ClearTemplates();
    backend.AddTemplate("ship", "generic ship", 0.3f);
    backend.AddTemplate("battleship", "battleship specific", 0.8f);
    AIContext ctx;
    auto resp = backend.Query("design a battleship", ctx);
    assert(resp.content == "battleship specific");
    assert(resp.confidence >= 0.8f);
}

void test_template_ai_multiple_matches_boost() {
    TemplateAIBackend backend;
    backend.ClearTemplates();
    backend.AddTemplate("combat", "combat suggestion", 0.5f);
    backend.AddTemplate("mission", "mission suggestion", 0.6f);
    AIContext ctx;
    // "combat mission" matches both keywords
    auto resp = backend.Query("combat mission", ctx);
    // Best base is 0.6 (mission), boosted by +0.05 for extra match
    assert(resp.confidence >= 0.6f);
}

void test_template_ai_add_and_remove() {
    TemplateAIBackend backend;
    backend.ClearTemplates();
    assert(backend.TemplateCount() == 0);

    backend.AddTemplate("test_kw", "test response", 0.5f);
    assert(backend.TemplateCount() == 1);

    assert(backend.RemoveTemplate("test_kw"));
    assert(backend.TemplateCount() == 0);
}

void test_template_ai_remove_nonexistent() {
    TemplateAIBackend backend;
    backend.ClearTemplates();
    assert(!backend.RemoveTemplate("nonexistent"));
}

void test_template_ai_add_empty_keyword_ignored() {
    TemplateAIBackend backend;
    backend.ClearTemplates();
    backend.AddTemplate("", "should not be added", 0.5f);
    assert(backend.TemplateCount() == 0);
}

void test_template_ai_clear_templates() {
    TemplateAIBackend backend;
    assert(backend.TemplateCount() > 0);
    backend.ClearTemplates();
    assert(backend.TemplateCount() == 0);
}

void test_template_ai_with_aggregator() {
    AIAggregator agg;
    TemplateAIBackend backend;
    agg.RegisterBackend(&backend);
    AIContext ctx;
    auto resp = agg.Execute(AIRequestType::CodeAssist,
                            "suggest a combat mission", ctx);
    assert(resp.confidence > 0.0f);
    assert(!resp.content.empty());
}
