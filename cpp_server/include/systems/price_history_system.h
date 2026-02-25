#ifndef NOVAFORGE_SYSTEMS_PRICE_HISTORY_SYSTEM_H
#define NOVAFORGE_SYSTEMS_PRICE_HISTORY_SYSTEM_H

#include "ecs/system.h"
#include "ecs/entity.h"
#include "components/game_components.h"
#include <string>
#include <vector>
#include <map>

namespace atlas {
namespace systems {

/**
 * @brief Tracks historical price data for market items to enable dynamic price graphs
 *
 * Records price snapshots at configurable intervals, maintaining a rolling window
 * of historical data. Supports multiple time scales for different graph views
 * (hourly, daily, weekly).
 */
class PriceHistorySystem : public ecs::System {
public:
    explicit PriceHistorySystem(ecs::World* world);
    ~PriceHistorySystem() override = default;

    void update(float delta_time) override;
    std::string getName() const override { return "PriceHistorySystem"; }

    /**
     * @brief Record a price snapshot for an item in a region
     * @param region_id Region or station identifier
     * @param item_id Item type identifier
     * @param sell_price Current lowest sell price
     * @param buy_price Current highest buy price
     * @param volume Trade volume in this period
     * @param timestamp Game time when recorded
     */
    void recordPrice(const std::string& region_id,
                     const std::string& item_id,
                     double sell_price,
                     double buy_price,
                     int volume,
                     float timestamp);

    /**
     * @brief Get price history for an item
     * @param region_id Region or station identifier
     * @param item_id Item type identifier
     * @param max_entries Maximum number of entries to return (0 = all)
     * @return Vector of price entries, newest first
     */
    std::vector<components::PriceHistory::PriceEntry> getHistory(
        const std::string& region_id,
        const std::string& item_id,
        int max_entries = 0) const;

    /**
     * @brief Get the average price over a time window
     * @param region_id Region or station identifier
     * @param item_id Item type identifier
     * @param time_window_seconds Time window to average over
     * @param current_time Current game timestamp
     * @return Average price, or -1 if no data
     */
    double getAveragePrice(const std::string& region_id,
                           const std::string& item_id,
                           float time_window_seconds,
                           float current_time) const;

    /**
     * @brief Get price trend (positive = rising, negative = falling)
     * @param region_id Region or station identifier
     * @param item_id Item type identifier
     * @param sample_count Number of recent samples to analyze
     * @return Trend value between -1.0 (falling) and 1.0 (rising)
     */
    float getPriceTrend(const std::string& region_id,
                        const std::string& item_id,
                        int sample_count = 10) const;

    /**
     * @brief Get total trading volume for an item over a time window
     */
    int getTotalVolume(const std::string& region_id,
                       const std::string& item_id,
                       float time_window_seconds,
                       float current_time) const;

    /**
     * @brief Set snapshot recording interval
     */
    void setSnapshotInterval(float seconds) { snapshot_interval_ = seconds; }
    float getSnapshotInterval() const { return snapshot_interval_; }

    /**
     * @brief Set maximum history entries per item
     */
    void setMaxHistoryEntries(int max) { max_history_entries_ = max; }
    int getMaxHistoryEntries() const { return max_history_entries_; }

private:
    float snapshot_timer_ = 0.0f;
    float snapshot_interval_ = 3600.0f;  // 1 hour game time by default
    int max_history_entries_ = 720;       // 30 days of hourly data
    float current_game_time_ = 0.0f;

    // Helper to find or create price history component for region
    components::PriceHistory* getOrCreateHistory(const std::string& region_id);
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_PRICE_HISTORY_SYSTEM_H
