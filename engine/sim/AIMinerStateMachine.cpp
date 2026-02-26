#include "sim/AIMinerStateMachine.h"
#include <algorithm>

namespace atlas::sim {

bool AIMinerStateMachine::AddMiner(const MinerConfig& config) {
    for (auto& m : m_miners) {
        if (m.config.entityId == config.entityId) return false;
    }
    MinerRuntime rt;
    rt.config = config;
    rt.state  = MinerState::Idle;
    m_miners.push_back(rt);
    return true;
}

bool AIMinerStateMachine::RemoveMiner(uint32_t entityId) {
    auto it = std::remove_if(m_miners.begin(), m_miners.end(),
        [entityId](const MinerRuntime& m) { return m.config.entityId == entityId; });
    if (it == m_miners.end()) return false;
    m_miners.erase(it, m_miners.end());
    return true;
}

void AIMinerStateMachine::SetAvailableDeposits(const std::vector<uint32_t>& depositIds) {
    m_availableDeposits = depositIds;
}

void AIMinerStateMachine::Tick(float deltaSeconds) {
    for (auto& m : m_miners) {
        tickMiner(m, deltaSeconds);
    }
}

bool AIMinerStateMachine::KillMiner(uint32_t entityId) {
    for (auto& m : m_miners) {
        if (m.config.entityId == entityId && m.state != MinerState::Dead) {
            m.state = MinerState::Dead;
            m.stateTimer = 0.0f;
            m.respawnTimer = m.config.respawnDelay;
            m.cargoFill = 0.0f; // cargo lost on death
            return true;
        }
    }
    return false;
}

const MinerRuntime* AIMinerStateMachine::GetMiner(uint32_t entityId) const {
    for (auto& m : m_miners) {
        if (m.config.entityId == entityId) return &m;
    }
    return nullptr;
}

size_t AIMinerStateMachine::CountInState(MinerState state) const {
    size_t count = 0;
    for (auto& m : m_miners) {
        if (m.state == state) ++count;
    }
    return count;
}

float AIMinerStateMachine::TotalEarnings() const {
    float sum = 0.0f;
    for (auto& m : m_miners) {
        sum += m.totalEarnings;
    }
    return sum;
}

uint32_t AIMinerStateMachine::TotalCycles() const {
    uint32_t sum = 0;
    for (auto& m : m_miners) {
        sum += m.cyclesCompleted;
    }
    return sum;
}

void AIMinerStateMachine::tickMiner(MinerRuntime& m, float dt) {
    m.stateTimer += dt;

    switch (m.state) {
        case MinerState::Idle: {
            // Immediately try to select a target
            m.state = MinerState::SelectTarget;
            m.stateTimer = 0.0f;
            break;
        }
        case MinerState::SelectTarget: {
            uint32_t deposit = pickDeposit();
            if (deposit == 0) {
                // No deposits available, stay idle
                m.state = MinerState::Idle;
                m.stateTimer = 0.0f;
            } else {
                m.targetDepositId = deposit;
                m.state = MinerState::TravelToField;
                m.stateTimer = 0.0f;
                m.travelProgress = 0.0f;
            }
            break;
        }
        case MinerState::TravelToField: {
            float travelTime = m_fieldDistance / m.config.travelSpeed;
            m.travelProgress += dt / travelTime;
            if (m.travelProgress >= 1.0f) {
                m.travelProgress = 1.0f;
                m.state = MinerState::Mining;
                m.stateTimer = 0.0f;
            }
            break;
        }
        case MinerState::Mining: {
            float mined = m.config.miningYieldPerSec * dt;
            m.cargoFill += mined;
            if (m.cargoFill >= m.config.cargoCapacity) {
                m.cargoFill = m.config.cargoCapacity;
                m.state = MinerState::CargoFull;
                m.stateTimer = 0.0f;
            }
            break;
        }
        case MinerState::CargoFull: {
            // Transition to return
            m.state = MinerState::ReturnToStation;
            m.stateTimer = 0.0f;
            m.travelProgress = 0.0f;
            break;
        }
        case MinerState::ReturnToStation: {
            float travelTime = m_fieldDistance / m.config.travelSpeed;
            m.travelProgress += dt / travelTime;
            if (m.travelProgress >= 1.0f) {
                m.travelProgress = 1.0f;
                m.state = MinerState::SellOre;
                m.stateTimer = 0.0f;
            }
            break;
        }
        case MinerState::SellOre: {
            // Sell all cargo instantly
            float earnings = m.cargoFill * m.config.sellPricePerUnit;
            m.totalEarnings += earnings;
            m.cargoFill = 0.0f;
            m.cyclesCompleted++;
            // Go back to idle for next cycle
            m.state = MinerState::Idle;
            m.stateTimer = 0.0f;
            break;
        }
        case MinerState::Dead: {
            m.respawnTimer -= dt;
            if (m.respawnTimer <= 0.0f) {
                m.respawnTimer = 0.0f;
                m.state = MinerState::Idle;
                m.stateTimer = 0.0f;
            }
            break;
        }
    }
}

uint32_t AIMinerStateMachine::pickDeposit() const {
    if (m_availableDeposits.empty()) return 0;
    // Pick first available deposit
    return m_availableDeposits[0];
}

} // namespace atlas::sim
