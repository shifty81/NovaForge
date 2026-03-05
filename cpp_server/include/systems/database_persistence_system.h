#ifndef NOVAFORGE_SYSTEMS_DATABASE_PERSISTENCE_SYSTEM_H
#define NOVAFORGE_SYSTEMS_DATABASE_PERSISTENCE_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/game_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Key-value database persistence abstraction
 *
 * Provides a simple key-value store with auto-save support,
 * tracking reads/writes and dirty state for persistence.
 */
class DatabasePersistenceSystem : public ecs::SingleComponentSystem<components::DatabasePersistence> {
public:
    explicit DatabasePersistenceSystem(ecs::World* world);
    ~DatabasePersistenceSystem() override = default;

    std::string getName() const override { return "DatabasePersistenceSystem"; }

    bool createDatabase(const std::string& entity_id, const std::string& db_name,
                        float auto_save_interval = 60.0f);
    bool write(const std::string& entity_id, const std::string& key, const std::string& value);
    std::string read(const std::string& entity_id, const std::string& key);
    bool remove(const std::string& entity_id, const std::string& key);
    bool save(const std::string& entity_id);
    int getEntryCount(const std::string& entity_id) const;
    bool isDirty(const std::string& entity_id) const;
    int getSaveCount(const std::string& entity_id) const;
    int getTotalWrites(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity, components::DatabasePersistence& db, float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_DATABASE_PERSISTENCE_SYSTEM_H
