/**
 * Tests for PhysicsTunerPanel:
 *   - Construction & default presets
 *   - Object CRUD
 *   - Selection & type filtering
 *   - Preset application & custom environment
 *   - Simulation controls
 *   - JSON export/import round-trip
 */

#include <cassert>
#include <string>
#include <cmath>
#include "../editor/tools/PhysicsTunerPanel.h"

using namespace atlas::editor;

static bool approx(float a, float b, float eps = 0.01f) {
    return std::fabs(a - b) < eps;
}

// ══════════════════════════════════════════════════════════════════
// PhysicsTunerPanel tests
// ══════════════════════════════════════════════════════════════════

void test_physics_tuner_defaults() {
    PhysicsTunerPanel panel;
    assert(panel.ObjectCount() == 0);
    assert(panel.SelectedObject() == -1);
    assert(panel.TypeFilter().empty());
    assert(panel.PresetCount() == 4);
    assert(panel.ActivePreset() == 2); // Earth-Like
    assert(!panel.IsSimulationPaused());
    assert(panel.SimulationStepCount() == 0);
    assert(std::string(panel.Name()) == "Physics Tuner");
}

void test_physics_tuner_preset_names() {
    PhysicsTunerPanel panel;
    assert(panel.GetPreset(0).name == "Zero-G");
    assert(panel.GetPreset(1).name == "Low-G Planet");
    assert(panel.GetPreset(2).name == "Earth-Like");
    assert(panel.GetPreset(3).name == "Windy");
}

void test_physics_tuner_preset_values() {
    PhysicsTunerPanel panel;

    // Zero-G
    assert(approx(panel.GetPreset(0).gravity, 0.0f));
    assert(approx(panel.GetPreset(0).atmosphereDensity, 0.0f));

    // Low-G
    assert(approx(panel.GetPreset(1).gravity, 1.62f));
    assert(approx(panel.GetPreset(1).atmosphereDensity, 0.3f));

    // Earth-Like
    assert(approx(panel.GetPreset(2).gravity, 9.81f));
    assert(approx(panel.GetPreset(2).atmosphereDensity, 1.0f));

    // Windy
    assert(approx(panel.GetPreset(3).gravity, 9.81f));
    assert(approx(panel.GetPreset(3).windStrength, 15.0f));
    assert(approx(panel.GetPreset(3).atmosphereDensity, 1.2f));
}

void test_physics_tuner_add_object() {
    PhysicsTunerPanel panel;

    PhysicsObjectEntry obj;
    obj.objectId   = "rb_1";
    obj.objectName = "Hull Plate";
    obj.objectType = "rigidbody";
    obj.mass       = 10.0f;

    size_t idx = panel.AddObject(obj);
    assert(idx == 0);
    assert(panel.ObjectCount() == 1);
    assert(panel.GetObject(0).objectId == "rb_1");
    assert(approx(panel.GetObject(0).mass, 10.0f));
}

void test_physics_tuner_remove_object() {
    PhysicsTunerPanel panel;

    PhysicsObjectEntry e1; e1.objectId = "a"; e1.objectName = "A";
    PhysicsObjectEntry e2; e2.objectId = "b"; e2.objectName = "B";
    panel.AddObject(e1);
    panel.AddObject(e2);
    assert(panel.ObjectCount() == 2);

    bool removed = panel.RemoveObject(0);
    assert(removed);
    assert(panel.ObjectCount() == 1);
    assert(panel.GetObject(0).objectId == "b");

    assert(!panel.RemoveObject(99));
}

void test_physics_tuner_update_object() {
    PhysicsTunerPanel panel;

    PhysicsObjectEntry obj;
    obj.objectId = "rb_1"; obj.objectName = "Plate"; obj.mass = 5.0f;
    panel.AddObject(obj);

    PhysicsObjectEntry updated = obj;
    updated.objectName = "Heavy Plate";
    updated.mass = 50.0f;

    bool ok = panel.UpdateObject(0, updated);
    assert(ok);
    assert(panel.GetObject(0).objectName == "Heavy Plate");
    assert(approx(panel.GetObject(0).mass, 50.0f));

    assert(!panel.UpdateObject(99, updated));
}

void test_physics_tuner_selection() {
    PhysicsTunerPanel panel;

    PhysicsObjectEntry e1; e1.objectId = "a"; e1.objectName = "A";
    PhysicsObjectEntry e2; e2.objectId = "b"; e2.objectName = "B";
    panel.AddObject(e1);
    panel.AddObject(e2);

    panel.SelectObject(1);
    assert(panel.SelectedObject() == 1);

    panel.ClearSelection();
    assert(panel.SelectedObject() == -1);

    panel.SelectObject(99);
    assert(panel.SelectedObject() == -1);
}

void test_physics_tuner_type_filter() {
    PhysicsTunerPanel panel;

    PhysicsObjectEntry rb; rb.objectId = "r1"; rb.objectName = "RB"; rb.objectType = "rigidbody";
    PhysicsObjectEntry cl; cl.objectId = "c1"; cl.objectName = "Cape"; cl.objectType = "cloth";
    PhysicsObjectEntry jt; jt.objectId = "j1"; jt.objectName = "Joint"; jt.objectType = "joint";
    panel.AddObject(rb);
    panel.AddObject(cl);
    panel.AddObject(jt);

    assert(panel.FilteredCount() == 3);

    panel.SetTypeFilter("cloth");
    assert(panel.FilteredCount() == 1);

    panel.SetTypeFilter("rigidbody");
    assert(panel.FilteredCount() == 1);

    panel.SetTypeFilter("");
    assert(panel.FilteredCount() == 3);
}

void test_physics_tuner_apply_preset() {
    PhysicsTunerPanel panel;

    // Default is Earth-Like (index 2)
    assert(approx(panel.ActiveEnvironment().gravity, 9.81f));

    // Apply Zero-G
    bool ok = panel.ApplyPreset(0);
    assert(ok);
    assert(panel.ActivePreset() == 0);
    assert(approx(panel.ActiveEnvironment().gravity, 0.0f));
    assert(panel.ActiveEnvironment().name == "Zero-G");

    // Apply Windy
    ok = panel.ApplyPreset(3);
    assert(ok);
    assert(panel.ActivePreset() == 3);
    assert(approx(panel.ActiveEnvironment().windStrength, 15.0f));

    // Out of bounds
    assert(!panel.ApplyPreset(99));
}

void test_physics_tuner_custom_environment() {
    PhysicsTunerPanel panel;

    panel.SetGravity(3.7f);
    assert(approx(panel.ActiveEnvironment().gravity, 3.7f));
    assert(panel.ActivePreset() == -1); // custom
    assert(panel.ActiveEnvironment().name == "Custom");

    panel.SetWindStrength(8.0f);
    assert(approx(panel.ActiveEnvironment().windStrength, 8.0f));

    panel.SetAtmosphereDensity(0.6f);
    assert(approx(panel.ActiveEnvironment().atmosphereDensity, 0.6f));
}

void test_physics_tuner_simulation_controls() {
    PhysicsTunerPanel panel;

    assert(!panel.IsSimulationPaused());
    assert(panel.SimulationStepCount() == 0);

    panel.PauseSimulation();
    assert(panel.IsSimulationPaused());

    panel.StepSimulation();
    assert(panel.SimulationStepCount() == 1);

    panel.StepSimulation();
    assert(panel.SimulationStepCount() == 2);

    panel.ResumeSimulation();
    assert(!panel.IsSimulationPaused());
}

void test_physics_tuner_cloth_params() {
    PhysicsTunerPanel panel;

    PhysicsObjectEntry cape;
    cape.objectId   = "cape_1";
    cape.objectName = "Player Cape";
    cape.objectType = "cloth";
    cape.clothParams.stiffness        = 0.5f;
    cape.clothParams.damping          = 0.2f;
    cape.clothParams.gravityInfluence = 0.8f;
    cape.clothParams.drag             = 0.15f;

    panel.AddObject(cape);
    assert(panel.ObjectCount() == 1);
    assert(panel.GetObject(0).objectType == "cloth");
    assert(approx(panel.GetObject(0).clothParams.stiffness, 0.5f));
    assert(approx(panel.GetObject(0).clothParams.damping, 0.2f));
    assert(approx(panel.GetObject(0).clothParams.gravityInfluence, 0.8f));
    assert(approx(panel.GetObject(0).clothParams.drag, 0.15f));
}

void test_physics_tuner_export_import_json() {
    PhysicsTunerPanel panel;

    PhysicsObjectEntry rb;
    rb.objectId = "rb_1"; rb.objectName = "Hull";
    rb.objectType = "rigidbody"; rb.mass = 25.0f; rb.friction = 0.6f;
    panel.AddObject(rb);

    PhysicsObjectEntry cape;
    cape.objectId = "cl_1"; cape.objectName = "Cape";
    cape.objectType = "cloth"; cape.mass = 0.5f;
    cape.clothParams.stiffness = 0.7f;
    cape.clothParams.damping = 0.25f;
    panel.AddObject(cape);

    std::string json = panel.ExportToJson();
    assert(!json.empty());
    assert(json.find("physicsTuner") != std::string::npos);
    assert(json.find("Hull") != std::string::npos);
    assert(json.find("Cape") != std::string::npos);

    // Import into fresh panel
    PhysicsTunerPanel panel2;
    size_t count = panel2.ImportFromJson(json);
    assert(count == 2);
    assert(panel2.ObjectCount() == 2);
    assert(panel2.GetObject(0).objectId == "rb_1");
    assert(panel2.GetObject(0).objectName == "Hull");
    assert(panel2.GetObject(1).objectId == "cl_1");
    assert(panel2.GetObject(1).objectType == "cloth");
}
