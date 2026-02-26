#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <functional>

namespace atlas::sim {

/**
 * Priority level for an alert. Higher priorities are shown first.
 */
enum class AlertPriority : uint8_t {
    Low    = 0,
    Medium = 1,
    High   = 2,
    Critical = 3
};

/**
 * A single alert entry displayed to the player.
 */
struct Alert {
    uint32_t      id       = 0;
    std::string   message;
    std::string   category; // e.g. "Combat", "Navigation", "System"
    AlertPriority priority = AlertPriority::Medium;
    float         ttl      = 5.0f;  // remaining seconds
    float         age      = 0.0f;  // seconds since creation
    bool          dismissed = false;
};

/**
 * @brief In-game alert / notification stack.
 *
 * Maintains a priority-sorted queue of alerts with automatic timeout.
 * Intended for HUD overlay: shield warnings, navigation alerts,
 * mission updates, system messages, etc.
 *
 * Usage:
 *   AlertStack stack;
 *   uint32_t id = stack.Push("Shields low!", "Combat", AlertPriority::High, 8.0f);
 *   stack.Tick(deltaTime);           // expire old alerts
 *   auto visible = stack.Active();   // sorted by priority desc
 */
class AlertStack {
public:
    explicit AlertStack(size_t maxAlerts = 8);

    /**
     * Push a new alert. Returns the assigned alert id.
     * If the stack is full, the lowest-priority oldest alert is evicted.
     */
    uint32_t Push(const std::string& message,
                  const std::string& category,
                  AlertPriority priority = AlertPriority::Medium,
                  float durationSeconds = 5.0f);

    /** Advance time, expiring alerts whose TTL has elapsed. */
    void Tick(float deltaSeconds);

    /** Dismiss an alert by id. */
    bool Dismiss(uint32_t id);

    /** Dismiss all alerts in a category. */
    size_t DismissCategory(const std::string& category);

    /** Clear all alerts. */
    void Clear();

    /** Active (non-expired, non-dismissed) alerts sorted by priority desc, then age asc. */
    std::vector<Alert> Active() const;

    /** Active alerts filtered to a specific category. */
    std::vector<Alert> ActiveInCategory(const std::string& category) const;

    /** Total number of active alerts. */
    size_t ActiveCount() const;

    /** Maximum alerts retained. */
    size_t MaxAlerts() const { return m_maxAlerts; }

    /** Total alerts ever pushed (monotonic counter). */
    uint32_t TotalPushed() const { return m_nextId - 1; }

private:
    void evictIfNeeded();

    std::vector<Alert> m_alerts;
    size_t   m_maxAlerts;
    uint32_t m_nextId = 1;
};

} // namespace atlas::sim
