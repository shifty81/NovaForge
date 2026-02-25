#include "systems/price_history_system.h"
#include "ecs/world.h"
#include <algorithm>
#include <cmath>
#include <memory>

namespace atlas {
namespace systems {

PriceHistorySystem::PriceHistorySystem(ecs::World* world)
    : System(world) {
}

void PriceHistorySystem::update(float delta_time) {
    current_game_time_ += delta_time;
    snapshot_timer_ += delta_time;

    // Automatic snapshot at intervals (handled by external calls to recordPrice)
    // The update just tracks time progression
}

components::PriceHistory* PriceHistorySystem::getOrCreateHistory(const std::string& region_id) {
    // Look for existing PriceHistory entity for this region
    auto entities = world_->getEntities<components::PriceHistory>();
    for (auto* entity : entities) {
        auto* history = entity->getComponent<components::PriceHistory>();
        if (history && history->region_id == region_id) {
            return history;
        }
    }

    // Create new entity with PriceHistory component
    auto* entity = world_->createEntity("price_history_" + region_id);
    auto hist = std::make_unique<components::PriceHistory>();
    hist->region_id = region_id;
    hist->max_entries_per_item = max_history_entries_;
    auto* history = hist.get();
    entity->addComponent(std::move(hist));
    return history;
}

void PriceHistorySystem::recordPrice(const std::string& region_id,
                                      const std::string& item_id,
                                      double sell_price,
                                      double buy_price,
                                      int volume,
                                      float timestamp) {
    auto* history = getOrCreateHistory(region_id);
    if (!history) return;

    history->addEntry(item_id, timestamp, sell_price, buy_price, volume);
}

std::vector<components::PriceHistory::PriceEntry>
PriceHistorySystem::getHistory(const std::string& region_id,
                                const std::string& item_id,
                                int max_entries) const {
    auto entities = world_->getEntities<components::PriceHistory>();
    for (auto* entity : entities) {
        auto* history = entity->getComponent<components::PriceHistory>();
        if (history && history->region_id == region_id) {
            return history->getHistory(item_id, max_entries);
        }
    }
    return {};
}

double PriceHistorySystem::getAveragePrice(const std::string& region_id,
                                            const std::string& item_id,
                                            float time_window_seconds,
                                            float current_time) const {
    auto history = getHistory(region_id, item_id, 0);
    if (history.empty()) return -1.0;

    double total = 0.0;
    int count = 0;
    float cutoff = current_time - time_window_seconds;

    for (const auto& entry : history) {
        if (entry.timestamp >= cutoff) {
            // Use sell price as primary, fallback to buy price
            double price = entry.sell_price > 0 ? entry.sell_price : entry.buy_price;
            if (price > 0) {
                total += price;
                count++;
            }
        }
    }

    return count > 0 ? total / count : -1.0;
}

float PriceHistorySystem::getPriceTrend(const std::string& region_id,
                                         const std::string& item_id,
                                         int sample_count) const {
    auto history = getHistory(region_id, item_id, sample_count);
    if (history.size() < 2) return 0.0f;

    // Calculate linear regression slope
    int n = static_cast<int>(history.size());
    double sum_x = 0, sum_y = 0, sum_xy = 0, sum_x2 = 0;

    for (int i = 0; i < n; i++) {
        double x = static_cast<double>(i);
        double y = history[i].sell_price > 0 ? history[i].sell_price : history[i].buy_price;
        sum_x += x;
        sum_y += y;
        sum_xy += x * y;
        sum_x2 += x * x;
    }

    double denominator = n * sum_x2 - sum_x * sum_x;
    if (std::abs(denominator) < 0.0001) return 0.0f;

    double slope = (n * sum_xy - sum_x * sum_y) / denominator;
    double avg_price = sum_y / n;

    if (avg_price <= 0.0) return 0.0f;

    // Normalize slope to -1 to 1 range based on relative change
    float normalized = static_cast<float>(slope / avg_price);
    return std::max(-1.0f, std::min(1.0f, normalized * 10.0f));
}

int PriceHistorySystem::getTotalVolume(const std::string& region_id,
                                        const std::string& item_id,
                                        float time_window_seconds,
                                        float current_time) const {
    auto history = getHistory(region_id, item_id, 0);
    if (history.empty()) return 0;

    int total = 0;
    float cutoff = current_time - time_window_seconds;

    for (const auto& entry : history) {
        if (entry.timestamp >= cutoff) {
            total += entry.volume;
        }
    }

    return total;
}

} // namespace systems
} // namespace atlas
