#include "System.h"
#include <algorithm>
#include <cstring>

namespace atlas::ecs {

void SystemRegistry::Register(std::unique_ptr<ISystem> system) {
    if (!system) return;
    m_systems.push_back({std::move(system), false});
    m_sorted = false;
}

bool SystemRegistry::Unregister(ISystem* system) {
    if (!system) return false;
    auto it = std::find_if(m_systems.begin(), m_systems.end(),
        [system](const Entry& e) { return e.system.get() == system; });
    if (it == m_systems.end()) return false;
    m_systems.erase(it);
    return true;
}

void SystemRegistry::InitAll(World& world) {
    SortByPriority();
    for (auto& entry : m_systems) {
        if (!entry.initialized) {
            entry.system->Init(world);
            entry.initialized = true;
        }
    }
}

void SystemRegistry::UpdateAll(World& world, float dt) {
    if (!m_sorted) SortByPriority();
    for (auto& entry : m_systems) {
        if (entry.system->IsEnabled()) {
            entry.system->Execute(world, dt);
        }
    }
}

void SystemRegistry::ShutdownAll(World& world) {
    // Shutdown in reverse priority order.
    for (auto it = m_systems.rbegin(); it != m_systems.rend(); ++it) {
        if (it->initialized) {
            it->system->Shutdown(world);
            it->initialized = false;
        }
    }
}

ISystem* SystemRegistry::FindByName(const char* name) const {
    if (!name) return nullptr;
    for (const auto& entry : m_systems) {
        if (std::strcmp(entry.system->Name(), name) == 0) {
            return entry.system.get();
        }
    }
    return nullptr;
}

void SystemRegistry::SortByPriority() {
    std::stable_sort(m_systems.begin(), m_systems.end(),
        [](const Entry& a, const Entry& b) {
            return a.system->Priority() < b.system->Priority();
        });
    m_sorted = true;
}

} // namespace atlas::ecs
