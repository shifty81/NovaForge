/**
 * Tests for LayerTagSystem:
 *   - Create/remove layers
 *   - Assign/remove entities from layers
 *   - Toggle layer visibility
 *   - Visible entities filtering
 *   - Add/remove/query tags
 *   - Find entities by tag
 *   - Clear all state
 */

#include <cassert>
#include <algorithm>
#include <string>
#include "../cpp_client/include/editor/layer_tag_system.h"

using namespace atlas::editor;

// ══════════════════════════════════════════════════════════════════
// LayerTagSystem tests
// ══════════════════════════════════════════════════════════════════

void test_layer_tag_defaults() {
    LayerTagSystem lts;
    assert(lts.LayerCount() == 0);
    assert(lts.TaggedEntityCount() == 0);
    assert(lts.VisibleEntities().empty());
}

void test_layer_create_and_count() {
    LayerTagSystem lts;
    assert(lts.CreateLayer("Ships"));
    assert(lts.CreateLayer("Props"));
    assert(lts.LayerCount() == 2);
}

void test_layer_create_duplicate() {
    LayerTagSystem lts;
    assert(lts.CreateLayer("Ships"));
    assert(!lts.CreateLayer("Ships"));
    assert(lts.LayerCount() == 1);
}

void test_layer_remove() {
    LayerTagSystem lts;
    lts.CreateLayer("Ships");
    lts.CreateLayer("Props");
    assert(lts.RemoveLayer("Ships"));
    assert(lts.LayerCount() == 1);
    assert(!lts.RemoveLayer("Ships"));
}

void test_layer_assign_entity() {
    LayerTagSystem lts;
    lts.CreateLayer("Ships");
    lts.AssignLayer(1, "Ships");
    const auto* layer = lts.GetLayer("Ships");
    assert(layer != nullptr);
    assert(layer->entities.count(1) == 1);
}

void test_layer_assign_creates_if_missing() {
    LayerTagSystem lts;
    lts.AssignLayer(1, "Auto");
    assert(lts.LayerCount() == 1);
    assert(lts.GetLayer("Auto") != nullptr);
    assert(lts.GetLayer("Auto")->entities.count(1) == 1);
}

void test_layer_remove_entity() {
    LayerTagSystem lts;
    lts.CreateLayer("Ships");
    lts.AssignLayer(1, "Ships");
    assert(lts.RemoveFromLayer(1, "Ships"));
    assert(!lts.RemoveFromLayer(1, "Ships"));
}

void test_layer_visibility_default() {
    LayerTagSystem lts;
    lts.CreateLayer("Ships");
    const auto* layer = lts.GetLayer("Ships");
    assert(layer != nullptr);
    assert(layer->visible);
}

void test_layer_toggle_visibility() {
    LayerTagSystem lts;
    lts.CreateLayer("Ships");
    lts.AssignLayer(1, "Ships");
    lts.AssignLayer(2, "Ships");

    assert(lts.SetLayerVisible("Ships", false));
    auto visible = lts.VisibleEntities();
    assert(visible.empty());

    assert(lts.SetLayerVisible("Ships", true));
    visible = lts.VisibleEntities();
    assert(visible.size() == 2);
}

void test_layer_visible_entities_multi_layer() {
    LayerTagSystem lts;
    lts.CreateLayer("Ships");
    lts.CreateLayer("Props");
    lts.AssignLayer(1, "Ships");
    lts.AssignLayer(2, "Props");
    lts.AssignLayer(3, "Props");

    lts.SetLayerVisible("Ships", false);
    auto visible = lts.VisibleEntities();
    assert(visible.size() == 2);

    lts.SetLayerVisible("Props", false);
    visible = lts.VisibleEntities();
    assert(visible.empty());
}

void test_tag_add_and_has() {
    LayerTagSystem lts;
    lts.AddTag(1, "capital");
    assert(lts.HasTag(1, "capital"));
    assert(!lts.HasTag(1, "small"));
    assert(!lts.HasTag(2, "capital"));
}

void test_tag_remove() {
    LayerTagSystem lts;
    lts.AddTag(1, "capital");
    assert(lts.RemoveTag(1, "capital"));
    assert(!lts.HasTag(1, "capital"));
    assert(!lts.RemoveTag(1, "capital"));
}

void test_tag_get_all() {
    LayerTagSystem lts;
    lts.AddTag(1, "capital");
    lts.AddTag(1, "armored");
    auto tags = lts.GetTags(1);
    assert(tags.size() == 2);
}

void test_tag_entities_with_tag() {
    LayerTagSystem lts;
    lts.AddTag(1, "capital");
    lts.AddTag(2, "capital");
    lts.AddTag(3, "small");
    auto entities = lts.EntitiesWithTag("capital");
    assert(entities.size() == 2);
}

void test_layer_tag_clear() {
    LayerTagSystem lts;
    lts.CreateLayer("Ships");
    lts.AssignLayer(1, "Ships");
    lts.AddTag(1, "capital");
    lts.Clear();
    assert(lts.LayerCount() == 0);
    assert(lts.TaggedEntityCount() == 0);
}
