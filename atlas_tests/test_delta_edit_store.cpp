/**
 * Tests for DeltaEditStore:
 *   - Recording edits
 *   - Seed management
 *   - Clear
 *   - JSON round-trip serialization
 */

#include <cassert>
#include <cstring>
#include <string>
#include "../engine/ecs/DeltaEditStore.h"

using namespace atlas::ecs;

// ══════════════════════════════════════════════════════════════════
// DeltaEditStore tests
// ══════════════════════════════════════════════════════════════════

void test_delta_store_empty_by_default() {
    DeltaEditStore store;
    assert(store.Count() == 0);
    assert(store.Seed() == 0);
}

void test_delta_store_set_seed() {
    DeltaEditStore store(42);
    assert(store.Seed() == 42);
    store.SetSeed(999);
    assert(store.Seed() == 999);
}

void test_delta_store_record_add() {
    DeltaEditStore store;
    DeltaEdit edit{};
    edit.type       = DeltaEditType::AddObject;
    edit.entityID   = 1;
    edit.objectType = "planet";
    edit.position[0] = 10.0f;
    edit.position[1] = 20.0f;
    edit.position[2] = 30.0f;
    store.Record(edit);
    assert(store.Count() == 1);
    assert(store.Edits()[0].entityID == 1);
    assert(store.Edits()[0].objectType == "planet");
}

void test_delta_store_record_multiple() {
    DeltaEditStore store;
    DeltaEdit e1{};
    e1.type     = DeltaEditType::AddObject;
    e1.entityID = 1;
    DeltaEdit e2{};
    e2.type     = DeltaEditType::MoveObject;
    e2.entityID = 2;
    DeltaEdit e3{};
    e3.type     = DeltaEditType::RemoveObject;
    e3.entityID = 3;
    store.Record(e1);
    store.Record(e2);
    store.Record(e3);
    assert(store.Count() == 3);
    assert(store.Edits()[0].type == DeltaEditType::AddObject);
    assert(store.Edits()[1].type == DeltaEditType::MoveObject);
    assert(store.Edits()[2].type == DeltaEditType::RemoveObject);
}

void test_delta_store_clear() {
    DeltaEditStore store(100);
    DeltaEdit edit{};
    edit.type = DeltaEditType::AddObject;
    store.Record(edit);
    store.Record(edit);
    assert(store.Count() == 2);
    store.Clear();
    assert(store.Count() == 0);
    assert(store.Seed() == 100); // seed preserved
}

void test_delta_store_set_property() {
    DeltaEditStore store;
    DeltaEdit edit{};
    edit.type          = DeltaEditType::SetProperty;
    edit.entityID      = 5;
    edit.propertyName  = "hitpoints";
    edit.propertyValue = "500";
    store.Record(edit);
    assert(store.Count() == 1);
    assert(store.Edits()[0].propertyName == "hitpoints");
    assert(store.Edits()[0].propertyValue == "500");
}

void test_delta_type_name() {
    assert(std::string(DeltaEditTypeName(DeltaEditType::AddObject))    == "AddObject");
    assert(std::string(DeltaEditTypeName(DeltaEditType::RemoveObject)) == "RemoveObject");
    assert(std::string(DeltaEditTypeName(DeltaEditType::MoveObject))   == "MoveObject");
    assert(std::string(DeltaEditTypeName(DeltaEditType::SetProperty))  == "SetProperty");
}

void test_delta_store_serialize_empty() {
    DeltaEditStore store(123);
    std::string json = store.SerializeToJSON();
    assert(json.find("\"worldSeed\": 123") != std::string::npos);
    assert(json.find("\"deltaEdits\": [") != std::string::npos);
}

void test_delta_store_roundtrip() {
    DeltaEditStore store(42);

    DeltaEdit e1{};
    e1.type        = DeltaEditType::AddObject;
    e1.entityID    = 1;
    e1.objectType  = "station";
    e1.position[0] = 100.0f;
    e1.position[1] = 200.0f;
    e1.position[2] = 300.0f;
    store.Record(e1);

    DeltaEdit e2{};
    e2.type          = DeltaEditType::SetProperty;
    e2.entityID      = 2;
    e2.propertyName  = "name";
    e2.propertyValue = "Alpha Base";
    store.Record(e2);

    std::string json = store.SerializeToJSON();

    DeltaEditStore loaded;
    assert(loaded.DeserializeFromJSON(json));
    assert(loaded.Seed() == 42);
    assert(loaded.Count() == 2);
    assert(loaded.Edits()[0].type == DeltaEditType::AddObject);
    assert(loaded.Edits()[0].entityID == 1);
    assert(loaded.Edits()[0].objectType == "station");
    assert(loaded.Edits()[1].type == DeltaEditType::SetProperty);
    assert(loaded.Edits()[1].propertyName == "name");
    assert(loaded.Edits()[1].propertyValue == "Alpha Base");
}

void test_delta_store_deserialize_invalid() {
    DeltaEditStore store;
    assert(!store.DeserializeFromJSON("not json"));
    assert(store.Count() == 0);
}

void test_delta_store_roundtrip_move() {
    DeltaEditStore store(7);
    DeltaEdit edit{};
    edit.type        = DeltaEditType::MoveObject;
    edit.entityID    = 10;
    edit.position[0] = -5.5f;
    edit.position[1] = 0.0f;
    edit.position[2] = 12.3f;
    store.Record(edit);

    std::string json = store.SerializeToJSON();

    DeltaEditStore loaded;
    assert(loaded.DeserializeFromJSON(json));
    assert(loaded.Count() == 1);
    assert(loaded.Edits()[0].type == DeltaEditType::MoveObject);
    assert(loaded.Edits()[0].entityID == 10);
    // Float comparison with tolerance
    float dx = loaded.Edits()[0].position[0] - (-5.5f);
    float dz = loaded.Edits()[0].position[2] - 12.3f;
    assert(dx > -0.01f && dx < 0.01f);
    assert(dz > -0.01f && dz < 0.01f);
}
