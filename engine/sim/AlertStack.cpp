#include "sim/AlertStack.h"
#include <algorithm>

namespace atlas::sim {

AlertStack::AlertStack(size_t maxAlerts)
    : m_maxAlerts(maxAlerts) {}

uint32_t AlertStack::Push(const std::string& message,
                           const std::string& category,
                           AlertPriority priority,
                           float durationSeconds) {
    Alert a;
    a.id       = m_nextId++;
    a.message  = message;
    a.category = category;
    a.priority = priority;
    a.ttl      = durationSeconds;
    a.age      = 0.0f;
    a.dismissed = false;
    m_alerts.push_back(a);
    evictIfNeeded();
    return a.id;
}

void AlertStack::Tick(float deltaSeconds) {
    for (auto& a : m_alerts) {
        a.age += deltaSeconds;
        a.ttl -= deltaSeconds;
    }
    // Remove expired
    m_alerts.erase(
        std::remove_if(m_alerts.begin(), m_alerts.end(),
                        [](const Alert& a) { return a.ttl <= 0.0f || a.dismissed; }),
        m_alerts.end());
}

bool AlertStack::Dismiss(uint32_t id) {
    for (auto& a : m_alerts) {
        if (a.id == id) {
            a.dismissed = true;
            m_alerts.erase(
                std::remove_if(m_alerts.begin(), m_alerts.end(),
                                [](const Alert& a) { return a.dismissed; }),
                m_alerts.end());
            return true;
        }
    }
    return false;
}

size_t AlertStack::DismissCategory(const std::string& category) {
    size_t count = 0;
    for (auto& a : m_alerts) {
        if (a.category == category) {
            a.dismissed = true;
            ++count;
        }
    }
    m_alerts.erase(
        std::remove_if(m_alerts.begin(), m_alerts.end(),
                        [](const Alert& a) { return a.dismissed; }),
        m_alerts.end());
    return count;
}

void AlertStack::Clear() {
    m_alerts.clear();
}

std::vector<Alert> AlertStack::Active() const {
    std::vector<Alert> result;
    result.reserve(m_alerts.size());
    for (auto& a : m_alerts) {
        if (!a.dismissed && a.ttl > 0.0f) {
            result.push_back(a);
        }
    }
    // Sort: highest priority first, then youngest first
    std::sort(result.begin(), result.end(), [](const Alert& a, const Alert& b) {
        if (a.priority != b.priority)
            return static_cast<uint8_t>(a.priority) > static_cast<uint8_t>(b.priority);
        return a.age < b.age;
    });
    return result;
}

std::vector<Alert> AlertStack::ActiveInCategory(const std::string& category) const {
    auto all = Active();
    all.erase(
        std::remove_if(all.begin(), all.end(),
                        [&](const Alert& a) { return a.category != category; }),
        all.end());
    return all;
}

size_t AlertStack::ActiveCount() const {
    size_t count = 0;
    for (auto& a : m_alerts) {
        if (!a.dismissed && a.ttl > 0.0f) ++count;
    }
    return count;
}

void AlertStack::evictIfNeeded() {
    while (m_alerts.size() > m_maxAlerts) {
        // Evict the lowest-priority, oldest alert
        auto worst = m_alerts.begin();
        for (auto it = m_alerts.begin(); it != m_alerts.end(); ++it) {
            if (static_cast<uint8_t>(it->priority) < static_cast<uint8_t>(worst->priority) ||
                (it->priority == worst->priority && it->age > worst->age)) {
                worst = it;
            }
        }
        m_alerts.erase(worst);
    }
}

} // namespace atlas::sim
