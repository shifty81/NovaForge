#include "systems/data_binding_system.h"
#include "ecs/world.h"
#include <algorithm>
#include <memory>

namespace atlas {
namespace systems {

DataBindingSystem::DataBindingSystem(ecs::World* world)
    : SingleComponentSystem(world) {
}

void DataBindingSystem::updateComponent(ecs::Entity& /*entity*/, components::DataBinding& db, float /*delta_time*/) {
    // Process any pending notifications
    if (!db.pending_notifications.empty()) {
        for (const auto& pattern : db.pending_notifications) {
            for (auto& observer : db.observers) {
                if (observer.active && observer.pattern == pattern) {
                    db.total_notifications++;
                }
            }
        }
        db.pending_notifications.clear();
    }
}

bool DataBindingSystem::initializeBindings(const std::string& entity_id,
                                            const std::string& owner_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* existing = entity->getComponent<components::DataBinding>();
    if (existing) return false;

    auto comp = std::make_unique<components::DataBinding>();
    comp->binding_id = entity_id;
    comp->owner_id = owner_id;
    entity->addComponent(std::move(comp));
    return true;
}

bool DataBindingSystem::addBinding(const std::string& entity_id,
                                    const std::string& binding_id,
                                    const std::string& source_path,
                                    const std::string& target_widget,
                                    const std::string& transform_func) {
    auto* db = getComponentFor(entity_id);
    if (!db) return false;

    if (static_cast<int>(db->bindings.size()) >= db->max_bindings) return false;

    // Check for duplicate
    if (db->findBinding(binding_id)) return false;

    components::DataBinding::Binding b;
    b.binding_id = binding_id;
    b.source_path = source_path;
    b.target_widget = target_widget;
    b.transform_func = transform_func;
    b.dirty = true;
    db->bindings.push_back(b);
    return true;
}

bool DataBindingSystem::removeBinding(const std::string& entity_id,
                                       const std::string& binding_id) {
    auto* db = getComponentFor(entity_id);
    if (!db) return false;

    for (auto it = db->bindings.begin(); it != db->bindings.end(); ++it) {
        if (it->binding_id == binding_id) {
            db->bindings.erase(it);
            return true;
        }
    }
    return false;
}

bool DataBindingSystem::updateBinding(const std::string& entity_id,
                                       const std::string& binding_id,
                                       const std::string& new_value) {
    auto* db = getComponentFor(entity_id);
    if (!db) return false;

    auto* binding = db->findBinding(binding_id);
    if (!binding) return false;

    if (binding->last_value != new_value) {
        binding->last_value = new_value;
        binding->dirty = true;
        db->total_updates++;
    }
    return true;
}

bool DataBindingSystem::notifyObservers(const std::string& entity_id,
                                         const std::string& pattern) {
    auto* db = getComponentFor(entity_id);
    if (!db) return false;

    db->pending_notifications.push_back(pattern);
    return true;
}

bool DataBindingSystem::addObserver(const std::string& entity_id,
                                     const std::string& observer_id,
                                     const std::string& pattern,
                                     const std::string& callback_id) {
    auto* db = getComponentFor(entity_id);
    if (!db) return false;

    // Check for duplicate
    if (db->findObserver(observer_id)) return false;

    components::DataBinding::Observer obs;
    obs.observer_id = observer_id;
    obs.pattern = pattern;
    obs.callback_id = callback_id;
    obs.active = true;
    db->observers.push_back(obs);
    return true;
}

bool DataBindingSystem::removeObserver(const std::string& entity_id,
                                        const std::string& observer_id) {
    auto* db = getComponentFor(entity_id);
    if (!db) return false;

    for (auto it = db->observers.begin(); it != db->observers.end(); ++it) {
        if (it->observer_id == observer_id) {
            db->observers.erase(it);
            return true;
        }
    }
    return false;
}

bool DataBindingSystem::setDirty(const std::string& entity_id,
                                  const std::string& binding_id) {
    auto* db = getComponentFor(entity_id);
    if (!db) return false;

    auto* binding = db->findBinding(binding_id);
    if (!binding) return false;

    binding->dirty = true;
    return true;
}

int DataBindingSystem::getDirtyCount(const std::string& entity_id) const {
    const auto* db = getComponentFor(entity_id);
    if (!db) return 0;

    int count = 0;
    for (const auto& b : db->bindings) {
        if (b.dirty) count++;
    }
    return count;
}

int DataBindingSystem::getBindingCount(const std::string& entity_id) const {
    const auto* db = getComponentFor(entity_id);
    if (!db) return 0;

    return static_cast<int>(db->bindings.size());
}

int DataBindingSystem::getObserverCount(const std::string& entity_id) const {
    const auto* db = getComponentFor(entity_id);
    if (!db) return 0;

    return static_cast<int>(db->observers.size());
}

bool DataBindingSystem::processNotifications(const std::string& entity_id) {
    auto* db = getComponentFor(entity_id);
    if (!db) return false;

    if (db->pending_notifications.empty()) return false;

    for (const auto& pattern : db->pending_notifications) {
        for (auto& observer : db->observers) {
            if (observer.active && observer.pattern == pattern) {
                db->total_notifications++;
            }
        }
    }
    db->pending_notifications.clear();
    return true;
}

} // namespace systems
} // namespace atlas
