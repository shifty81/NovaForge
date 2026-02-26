#include "sim/AIHaulerStateMachine.h"
#include <algorithm>

namespace atlas::sim {

bool AIHaulerStateMachine::AddHauler(const HaulerConfig& config) {
    for (auto& h : m_haulers) {
        if (h.config.entityId == config.entityId) return false;
    }
    HaulerRuntime rt;
    rt.config = config;
    rt.state  = HaulerState::Idle;
    m_haulers.push_back(rt);
    return true;
}

bool AIHaulerStateMachine::RemoveHauler(uint32_t entityId) {
    auto it = std::remove_if(m_haulers.begin(), m_haulers.end(),
        [entityId](const HaulerRuntime& h) { return h.config.entityId == entityId; });
    if (it == m_haulers.end()) return false;
    m_haulers.erase(it, m_haulers.end());
    return true;
}

void AIHaulerStateMachine::SetCargoRequests(const std::vector<CargoRequest>& requests) {
    m_cargoRequests = requests;
}

void AIHaulerStateMachine::Tick(float deltaSeconds) {
    for (auto& h : m_haulers) {
        tickHauler(h, deltaSeconds);
    }
}

bool AIHaulerStateMachine::KillHauler(uint32_t entityId) {
    for (auto& h : m_haulers) {
        if (h.config.entityId == entityId && h.state != HaulerState::Dead) {
            h.state = HaulerState::Dead;
            h.stateTimer = 0.0f;
            h.respawnTimer = h.config.respawnDelay;
            h.cargoFill = 0.0f; // cargo lost on death
            return true;
        }
    }
    return false;
}

const HaulerRuntime* AIHaulerStateMachine::GetHauler(uint32_t entityId) const {
    for (auto& h : m_haulers) {
        if (h.config.entityId == entityId) return &h;
    }
    return nullptr;
}

size_t AIHaulerStateMachine::CountInState(HaulerState state) const {
    size_t count = 0;
    for (auto& h : m_haulers) {
        if (h.state == state) ++count;
    }
    return count;
}

float AIHaulerStateMachine::TotalEarnings() const {
    float sum = 0.0f;
    for (auto& h : m_haulers) {
        sum += h.totalEarnings;
    }
    return sum;
}

uint32_t AIHaulerStateMachine::TotalTrips() const {
    uint32_t sum = 0;
    for (auto& h : m_haulers) {
        sum += h.tripsCompleted;
    }
    return sum;
}

void AIHaulerStateMachine::tickHauler(HaulerRuntime& h, float dt) {
    h.stateTimer += dt;

    switch (h.state) {
        case HaulerState::Idle: {
            h.state = HaulerState::FindCargo;
            h.stateTimer = 0.0f;
            break;
        }
        case HaulerState::FindCargo: {
            const CargoRequest* best = pickBestCargo();
            if (!best) {
                // No cargo available, go back to idle
                h.state = HaulerState::Idle;
                h.stateTimer = 0.0f;
            } else {
                h.targetSourceId = best->sourceId;
                h.state = HaulerState::TravelToPickup;
                h.stateTimer = 0.0f;
                h.travelProgress = 0.0f;
            }
            break;
        }
        case HaulerState::TravelToPickup: {
            // Find the distance to the target source
            float distance = 0.0f;
            for (auto& r : m_cargoRequests) {
                if (r.sourceId == h.targetSourceId) {
                    distance = r.distance;
                    break;
                }
            }
            if (distance <= 0.0f) distance = m_stationDistance;

            float travelTime = distance / h.config.travelSpeed;
            h.travelProgress += dt / travelTime;
            if (h.travelProgress >= 1.0f) {
                h.travelProgress = 1.0f;
                h.state = HaulerState::LoadCargo;
                h.stateTimer = 0.0f;
            }
            break;
        }
        case HaulerState::LoadCargo: {
            // Find available cargo
            float available = 0.0f;
            for (auto& r : m_cargoRequests) {
                if (r.sourceId == h.targetSourceId) {
                    available = r.available;
                    break;
                }
            }
            float loadable = std::min(available, h.config.cargoCapacity - h.cargoFill);
            float loaded = std::min(h.config.loadRate * dt, loadable);
            h.cargoFill += loaded;

            // Done loading if cargo is full or source is exhausted
            if (h.cargoFill >= h.config.cargoCapacity || loaded <= 0.0f) {
                if (h.cargoFill > 0.0f) {
                    h.state = HaulerState::TravelToStation;
                    h.stateTimer = 0.0f;
                    h.travelProgress = 0.0f;
                } else {
                    // Nothing to load, go find new cargo
                    h.state = HaulerState::Idle;
                    h.stateTimer = 0.0f;
                }
            }
            break;
        }
        case HaulerState::TravelToStation: {
            float travelTime = m_stationDistance / h.config.travelSpeed;
            h.travelProgress += dt / travelTime;
            if (h.travelProgress >= 1.0f) {
                h.travelProgress = 1.0f;
                h.state = HaulerState::UnloadCargo;
                h.stateTimer = 0.0f;
            }
            break;
        }
        case HaulerState::UnloadCargo: {
            float earnings = h.cargoFill * h.config.sellPricePerUnit;
            h.totalEarnings += earnings;
            h.cargoFill = 0.0f;
            h.tripsCompleted++;
            h.state = HaulerState::Idle;
            h.stateTimer = 0.0f;
            break;
        }
        case HaulerState::Dead: {
            h.respawnTimer -= dt;
            if (h.respawnTimer <= 0.0f) {
                h.respawnTimer = 0.0f;
                h.state = HaulerState::Idle;
                h.stateTimer = 0.0f;
            }
            break;
        }
    }
}

const CargoRequest* AIHaulerStateMachine::pickBestCargo() const {
    if (m_cargoRequests.empty()) return nullptr;
    // Pick request with most available cargo (simple heuristic)
    const CargoRequest* best = nullptr;
    for (auto& r : m_cargoRequests) {
        if (r.available > 0.0f) {
            if (!best || r.available > best->available) {
                best = &r;
            }
        }
    }
    return best;
}

} // namespace atlas::sim
