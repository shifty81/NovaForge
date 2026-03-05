#ifndef NOVAFORGE_SYSTEMS_DATA_BINDING_SYSTEM_H
#define NOVAFORGE_SYSTEMS_DATA_BINDING_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/ui_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Observer-pattern data binding system (Phase 15)
 *
 * Manages data bindings between source paths and UI widgets,
 * observer notifications, dirty tracking, and batch updates.
 */
class DataBindingSystem : public ecs::SingleComponentSystem<components::DataBinding> {
public:
    explicit DataBindingSystem(ecs::World* world);
    ~DataBindingSystem() override = default;

    std::string getName() const override { return "DataBindingSystem"; }

    // Initialization
    bool initializeBindings(const std::string& entity_id, const std::string& owner_id);

    // Binding management
    bool addBinding(const std::string& entity_id, const std::string& binding_id,
                    const std::string& source_path, const std::string& target_widget,
                    const std::string& transform_func);
    bool removeBinding(const std::string& entity_id, const std::string& binding_id);
    bool updateBinding(const std::string& entity_id, const std::string& binding_id,
                       const std::string& new_value);

    // Observer management
    bool notifyObservers(const std::string& entity_id, const std::string& pattern);
    bool addObserver(const std::string& entity_id, const std::string& observer_id,
                     const std::string& pattern, const std::string& callback_id);
    bool removeObserver(const std::string& entity_id, const std::string& observer_id);

    // Dirty tracking
    bool setDirty(const std::string& entity_id, const std::string& binding_id);
    int getDirtyCount(const std::string& entity_id) const;

    // Query
    int getBindingCount(const std::string& entity_id) const;
    int getObserverCount(const std::string& entity_id) const;

    // Notification processing
    bool processNotifications(const std::string& entity_id);

protected:
    void updateComponent(ecs::Entity& entity, components::DataBinding& db, float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_DATA_BINDING_SYSTEM_H
