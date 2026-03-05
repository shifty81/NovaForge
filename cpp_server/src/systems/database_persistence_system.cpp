#include "systems/database_persistence_system.h"
#include "ecs/world.h"
#include <memory>

namespace atlas {
namespace systems {

DatabasePersistenceSystem::DatabasePersistenceSystem(ecs::World* world)
    : SingleComponentSystem(world) {
}

void DatabasePersistenceSystem::updateComponent(ecs::Entity& /*entity*/, components::DatabasePersistence& db, float delta_time) {
    if (db.auto_save_enabled && db.dirty) {
        db.save_timer -= delta_time;
        if (db.save_timer <= 0.0f) {
            db.save_count++;
            db.dirty = false;
            db.save_timer = db.auto_save_interval;
        }
    }
}

bool DatabasePersistenceSystem::createDatabase(const std::string& entity_id,
                                                const std::string& db_name,
                                                float auto_save_interval) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    if (entity->getComponent<components::DatabasePersistence>()) return false;

    auto comp = std::make_unique<components::DatabasePersistence>();
    comp->db_name = db_name;
    comp->auto_save_interval = auto_save_interval;
    comp->save_timer = auto_save_interval;
    entity->addComponent(std::move(comp));
    return true;
}

bool DatabasePersistenceSystem::write(const std::string& entity_id,
                                       const std::string& key,
                                       const std::string& value) {
    auto* db = getComponentFor(entity_id);
    if (!db) return false;

    db->store[key] = value;
    db->total_writes++;
    db->dirty = true;
    return true;
}

std::string DatabasePersistenceSystem::read(const std::string& entity_id,
                                             const std::string& key) {
    auto* db = getComponentFor(entity_id);
    if (!db) return "";

    auto it = db->store.find(key);
    if (it != db->store.end()) {
        db->total_reads++;
        return it->second;
    }
    return "";
}

bool DatabasePersistenceSystem::remove(const std::string& entity_id,
                                        const std::string& key) {
    auto* db = getComponentFor(entity_id);
    if (!db) return false;

    auto it = db->store.find(key);
    if (it == db->store.end()) return false;

    db->store.erase(it);
    db->dirty = true;
    return true;
}

bool DatabasePersistenceSystem::save(const std::string& entity_id) {
    auto* db = getComponentFor(entity_id);
    if (!db) return false;

    db->save_count++;
    db->dirty = false;
    db->save_timer = db->auto_save_interval;
    return true;
}

int DatabasePersistenceSystem::getEntryCount(const std::string& entity_id) const {
    const auto* db = getComponentFor(entity_id);
    if (!db) return 0;

    return static_cast<int>(db->store.size());
}

bool DatabasePersistenceSystem::isDirty(const std::string& entity_id) const {
    const auto* db = getComponentFor(entity_id);
    if (!db) return false;

    return db->dirty;
}

int DatabasePersistenceSystem::getSaveCount(const std::string& entity_id) const {
    const auto* db = getComponentFor(entity_id);
    if (!db) return 0;

    return db->save_count;
}

int DatabasePersistenceSystem::getTotalWrites(const std::string& entity_id) const {
    const auto* db = getComponentFor(entity_id);
    if (!db) return 0;

    return db->total_writes;
}

} // namespace systems
} // namespace atlas
