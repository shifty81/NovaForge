#include "sim/AITraderStateMachine.h"
#include <algorithm>

namespace atlas::sim {

bool AITraderStateMachine::AddTrader(const TraderConfig& config) {
    for (auto& t : m_traders) {
        if (t.config.entityId == config.entityId) return false;
    }
    TraderRuntime rt;
    rt.config    = config;
    rt.state     = TraderState::Idle;
    rt.credits   = config.startingCredits;
    rt.currentHP = config.hitPoints;
    m_traders.push_back(rt);
    return true;
}

bool AITraderStateMachine::RemoveTrader(uint32_t entityId) {
    auto it = std::remove_if(m_traders.begin(), m_traders.end(),
        [entityId](const TraderRuntime& t) { return t.config.entityId == entityId; });
    if (it == m_traders.end()) return false;
    m_traders.erase(it, m_traders.end());
    return true;
}

void AITraderStateMachine::SetMarketListings(const std::vector<MarketListing>& listings) {
    m_marketListings = listings;
}

void AITraderStateMachine::Tick(float deltaSeconds) {
    for (auto& t : m_traders) {
        tickTrader(t, deltaSeconds);
    }
}

bool AITraderStateMachine::DamageTrader(uint32_t entityId, float damage) {
    for (auto& t : m_traders) {
        if (t.config.entityId == entityId && t.state != TraderState::Dead) {
            t.currentHP -= damage;
            if (t.currentHP <= 0.0f) {
                t.currentHP = 0.0f;
                t.state = TraderState::Dead;
                t.stateTimer = 0.0f;
                t.respawnTimer = t.config.respawnDelay;
                t.cargoFill = 0.0f;
            }
            return true;
        }
    }
    return false;
}

bool AITraderStateMachine::KillTrader(uint32_t entityId) {
    for (auto& t : m_traders) {
        if (t.config.entityId == entityId && t.state != TraderState::Dead) {
            t.state = TraderState::Dead;
            t.stateTimer = 0.0f;
            t.currentHP = 0.0f;
            t.respawnTimer = t.config.respawnDelay;
            t.cargoFill = 0.0f;
            return true;
        }
    }
    return false;
}

const TraderRuntime* AITraderStateMachine::GetTrader(uint32_t entityId) const {
    for (auto& t : m_traders) {
        if (t.config.entityId == entityId) return &t;
    }
    return nullptr;
}

size_t AITraderStateMachine::CountInState(TraderState state) const {
    size_t count = 0;
    for (auto& t : m_traders) {
        if (t.state == state) ++count;
    }
    return count;
}

float AITraderStateMachine::TotalProfit() const {
    float sum = 0.0f;
    for (auto& t : m_traders) {
        sum += t.totalProfit;
    }
    return sum;
}

uint32_t AITraderStateMachine::TotalTrades() const {
    uint32_t sum = 0;
    for (auto& t : m_traders) {
        sum += t.tradesCompleted;
    }
    return sum;
}

void AITraderStateMachine::tickTrader(TraderRuntime& t, float dt) {
    t.stateTimer += dt;

    switch (t.state) {
        case TraderState::Idle: {
            t.state = TraderState::EvaluateMarket;
            t.stateTimer = 0.0f;
            break;
        }
        case TraderState::EvaluateMarket: {
            uint32_t buyStation = 0, sellStation = 0;
            float buyPrice = 0.0f, sellPrice = 0.0f;
            if (!findBestTrade(buyStation, sellStation, buyPrice, sellPrice)) {
                // No profitable trade, go back to idle
                t.state = TraderState::Idle;
                t.stateTimer = 0.0f;
            } else {
                t.buyStationId  = buyStation;
                t.sellStationId = sellStation;
                t.cargoBuyPrice = buyPrice;
                t.state = TraderState::TravelToBuy;
                t.stateTimer = 0.0f;
                t.travelProgress = 0.0f;
            }
            break;
        }
        case TraderState::TravelToBuy: {
            // Find distance to buy station
            float distance = 0.0f;
            for (auto& l : m_marketListings) {
                if (l.stationId == t.buyStationId) {
                    distance = l.distance;
                    break;
                }
            }
            if (distance <= 0.0f) distance = m_defaultDistance;

            float travelTime = distance / t.config.travelSpeed;
            t.travelProgress += dt / travelTime;
            if (t.travelProgress >= 1.0f) {
                t.travelProgress = 1.0f;
                t.state = TraderState::BuyGoods;
                t.stateTimer = 0.0f;
            }
            break;
        }
        case TraderState::BuyGoods: {
            // Buy as much as we can afford and carry
            float available = 0.0f;
            for (auto& l : m_marketListings) {
                if (l.stationId == t.buyStationId) {
                    available = l.available;
                    break;
                }
            }
            float affordable = (t.cargoBuyPrice > 0.001f) ? (t.credits / t.cargoBuyPrice) : 0.0f;
            float canCarry = t.config.cargoCapacity - t.cargoFill;
            float bought = std::min({available, affordable, canCarry});

            if (bought > 0.0f) {
                t.cargoFill += bought;
                t.credits -= bought * t.cargoBuyPrice;
                t.state = TraderState::TravelToSell;
                t.stateTimer = 0.0f;
                t.travelProgress = 0.0f;
            } else {
                // Can't buy anything, go back to idle
                t.state = TraderState::Idle;
                t.stateTimer = 0.0f;
            }
            break;
        }
        case TraderState::TravelToSell: {
            // Find distance to sell station
            float distance = 0.0f;
            for (auto& l : m_marketListings) {
                if (l.stationId == t.sellStationId) {
                    distance = l.distance;
                    break;
                }
            }
            if (distance <= 0.0f) distance = m_defaultDistance;

            float travelTime = distance / t.config.travelSpeed;
            t.travelProgress += dt / travelTime;
            if (t.travelProgress >= 1.0f) {
                t.travelProgress = 1.0f;
                t.state = TraderState::SellGoods;
                t.stateTimer = 0.0f;
            }
            break;
        }
        case TraderState::SellGoods: {
            // Sell all cargo at the sell station price
            float sellPrice = 0.0f;
            for (auto& l : m_marketListings) {
                if (l.stationId == t.sellStationId) {
                    sellPrice = l.sellPrice;
                    break;
                }
            }
            float revenue = t.cargoFill * sellPrice;
            float cost    = t.cargoFill * t.cargoBuyPrice;
            float profit  = revenue - cost;

            t.credits += revenue;
            t.totalProfit += profit;
            t.cargoFill = 0.0f;
            t.tradesCompleted++;
            t.state = TraderState::Idle;
            t.stateTimer = 0.0f;
            break;
        }
        case TraderState::Flee: {
            float travelTime = m_defaultDistance / t.config.travelSpeed;
            t.travelProgress += dt / travelTime;
            if (t.travelProgress >= 1.0f) {
                t.travelProgress = 1.0f;
                t.state = TraderState::Idle;
                t.stateTimer = 0.0f;
            }
            break;
        }
        case TraderState::Dead: {
            t.respawnTimer -= dt;
            if (t.respawnTimer <= 0.0f) {
                t.respawnTimer = 0.0f;
                t.currentHP = t.config.hitPoints;
                t.credits = t.config.startingCredits;
                t.state = TraderState::Idle;
                t.stateTimer = 0.0f;
            }
            break;
        }
    }
}

bool AITraderStateMachine::findBestTrade(uint32_t& outBuyStation, uint32_t& outSellStation,
                                          float& outBuyPrice, float& outSellPrice) const {
    if (m_marketListings.size() < 2) return false;

    float bestMargin = 0.0f;
    bool found = false;

    for (size_t i = 0; i < m_marketListings.size(); ++i) {
        for (size_t j = 0; j < m_marketListings.size(); ++j) {
            if (i == j) continue;
            const auto& buy  = m_marketListings[i];
            const auto& sell = m_marketListings[j];
            if (buy.available <= 0.0f) continue;
            float margin = sell.sellPrice - buy.buyPrice;
            if (margin > bestMargin) {
                bestMargin    = margin;
                outBuyStation  = buy.stationId;
                outSellStation = sell.stationId;
                outBuyPrice    = buy.buyPrice;
                outSellPrice   = sell.sellPrice;
                found = true;
            }
        }
    }
    return found;
}

} // namespace atlas::sim
