// Tests for: DatabasePersistenceSystem Tests
#include "test_log.h"
#include "ecs/component.h"
#include "ecs/system.h"
#include "systems/database_persistence_system.h"

using namespace atlas;

// ==================== DatabasePersistenceSystem Tests ====================

static void testDatabaseCreate() {
    std::cout << "\n=== DatabasePersistence: Create ===" << std::endl;
    ecs::World world;
    systems::DatabasePersistenceSystem sys(&world);
    auto* e = world.createEntity("db1");
    assertTrue(sys.createDatabase("db1", "players", 30.0f), "Create database succeeds");
    auto* db = e->getComponent<components::DatabasePersistence>();
    assertTrue(db != nullptr, "Component exists");
    assertTrue(db->db_name == "players", "DB name set");
    assertTrue(approxEqual(db->auto_save_interval, 30.0f), "Auto-save interval set");
}

static void testDatabaseWriteRead() {
    std::cout << "\n=== DatabasePersistence: Write/Read ===" << std::endl;
    ecs::World world;
    systems::DatabasePersistenceSystem sys(&world);
    world.createEntity("db1");
    sys.createDatabase("db1", "store", 60.0f);
    assertTrue(sys.write("db1", "name", "Alice"), "Write succeeds");
    assertTrue(sys.read("db1", "name") == "Alice", "Read returns written value");
    assertTrue(sys.read("db1", "missing") == "", "Read missing key returns empty");
}

static void testDatabaseRemove() {
    std::cout << "\n=== DatabasePersistence: Remove ===" << std::endl;
    ecs::World world;
    systems::DatabasePersistenceSystem sys(&world);
    world.createEntity("db1");
    sys.createDatabase("db1", "store", 60.0f);
    sys.write("db1", "key1", "val1");
    assertTrue(sys.remove("db1", "key1"), "Remove succeeds");
    assertTrue(sys.read("db1", "key1") == "", "Removed key returns empty");
    assertTrue(!sys.remove("db1", "key1"), "Remove non-existent fails");
}

static void testDatabaseDirty() {
    std::cout << "\n=== DatabasePersistence: Dirty Flag ===" << std::endl;
    ecs::World world;
    systems::DatabasePersistenceSystem sys(&world);
    world.createEntity("db1");
    sys.createDatabase("db1", "store", 60.0f);
    assertTrue(!sys.isDirty("db1"), "Not dirty initially");
    sys.write("db1", "k", "v");
    assertTrue(sys.isDirty("db1"), "Dirty after write");
    sys.save("db1");
    assertTrue(!sys.isDirty("db1"), "Not dirty after save");
}

static void testDatabaseSave() {
    std::cout << "\n=== DatabasePersistence: Explicit Save ===" << std::endl;
    ecs::World world;
    systems::DatabasePersistenceSystem sys(&world);
    world.createEntity("db1");
    sys.createDatabase("db1", "store", 60.0f);
    sys.write("db1", "k", "v");
    assertTrue(sys.save("db1"), "Save succeeds");
    assertTrue(sys.getSaveCount("db1") == 1, "Save count is 1");
    sys.save("db1");
    assertTrue(sys.getSaveCount("db1") == 2, "Save count is 2");
}

static void testDatabaseAutoSave() {
    std::cout << "\n=== DatabasePersistence: Auto-Save ===" << std::endl;
    ecs::World world;
    systems::DatabasePersistenceSystem sys(&world);
    auto* e = world.createEntity("db1");
    sys.createDatabase("db1", "store", 1.0f);
    sys.write("db1", "k", "v");
    sys.update(0.5f);
    assertTrue(sys.isDirty("db1"), "Still dirty before interval");
    sys.update(0.6f);
    assertTrue(!sys.isDirty("db1"), "Not dirty after auto-save");
    assertTrue(sys.getSaveCount("db1") == 1, "Auto-save incremented save count");
}

static void testDatabaseEntryCount() {
    std::cout << "\n=== DatabasePersistence: Entry Count ===" << std::endl;
    ecs::World world;
    systems::DatabasePersistenceSystem sys(&world);
    world.createEntity("db1");
    sys.createDatabase("db1", "store", 60.0f);
    assertTrue(sys.getEntryCount("db1") == 0, "0 entries initially");
    sys.write("db1", "a", "1");
    sys.write("db1", "b", "2");
    assertTrue(sys.getEntryCount("db1") == 2, "2 entries after writes");
    sys.remove("db1", "a");
    assertTrue(sys.getEntryCount("db1") == 1, "1 entry after remove");
}

static void testDatabaseWriteCount() {
    std::cout << "\n=== DatabasePersistence: Write Count ===" << std::endl;
    ecs::World world;
    systems::DatabasePersistenceSystem sys(&world);
    world.createEntity("db1");
    sys.createDatabase("db1", "store", 60.0f);
    sys.write("db1", "a", "1");
    sys.write("db1", "b", "2");
    sys.write("db1", "a", "3");
    assertTrue(sys.getTotalWrites("db1") == 3, "3 total writes");
}

static void testDatabaseOverwrite() {
    std::cout << "\n=== DatabasePersistence: Overwrite ===" << std::endl;
    ecs::World world;
    systems::DatabasePersistenceSystem sys(&world);
    world.createEntity("db1");
    sys.createDatabase("db1", "store", 60.0f);
    sys.write("db1", "key", "old");
    sys.write("db1", "key", "new");
    assertTrue(sys.read("db1", "key") == "new", "Overwritten value returned");
    assertTrue(sys.getEntryCount("db1") == 1, "Still 1 entry after overwrite");
}

static void testDatabaseMissing() {
    std::cout << "\n=== DatabasePersistence: Missing Entity ===" << std::endl;
    ecs::World world;
    systems::DatabasePersistenceSystem sys(&world);
    assertTrue(!sys.createDatabase("nonexistent", "db", 60.0f), "Create fails on missing");
    assertTrue(!sys.write("nonexistent", "k", "v"), "Write fails on missing");
    assertTrue(sys.read("nonexistent", "k") == "", "Read returns empty on missing");
    assertTrue(!sys.remove("nonexistent", "k"), "Remove fails on missing");
    assertTrue(!sys.save("nonexistent"), "Save fails on missing");
    assertTrue(sys.getEntryCount("nonexistent") == 0, "0 entries on missing");
    assertTrue(!sys.isDirty("nonexistent"), "Not dirty on missing");
    assertTrue(sys.getSaveCount("nonexistent") == 0, "0 saves on missing");
    assertTrue(sys.getTotalWrites("nonexistent") == 0, "0 writes on missing");
}


void run_database_persistence_system_tests() {
    testDatabaseCreate();
    testDatabaseWriteRead();
    testDatabaseRemove();
    testDatabaseDirty();
    testDatabaseSave();
    testDatabaseAutoSave();
    testDatabaseEntryCount();
    testDatabaseWriteCount();
    testDatabaseOverwrite();
    testDatabaseMissing();
}
