#include "sim/AIPirateStateMachine.h"
#include <algorithm>

namespace atlas::sim {

bool AIPirateStateMachine::AddPirate(const PirateConfig& config) {
    for (auto& p : m_pirates) {
        if (p.config.entityId == config.entityId) return false;
    }
    PirateRuntime rt;
    rt.config    = config;
    rt.state     = PirateState::Idle;
    rt.currentHP = config.hitPoints;
    m_pirates.push_back(rt);
    return true;
}

bool AIPirateStateMachine::RemovePirate(uint32_t entityId) {
    auto it = std::remove_if(m_pirates.begin(), m_pirates.end(),
        [entityId](const PirateRuntime& p) { return p.config.entityId == entityId; });
    if (it == m_pirates.end()) return false;
    m_pirates.erase(it, m_pirates.end());
    return true;
}

void AIPirateStateMachine::SetPatrolWaypoints(const std::vector<uint32_t>& waypoints) {
    m_patrolWaypoints = waypoints;
}

void AIPirateStateMachine::SetTargetsInRange(const std::vector<uint32_t>& targets) {
    m_targetsInRange = targets;
}

void AIPirateStateMachine::Tick(float deltaSeconds) {
    for (auto& p : m_pirates) {
        tickPirate(p, deltaSeconds);
    }
}

bool AIPirateStateMachine::DamagePirate(uint32_t entityId, float damage) {
    for (auto& p : m_pirates) {
        if (p.config.entityId == entityId && p.state != PirateState::Dead) {
            p.currentHP -= damage;
            if (p.currentHP <= 0.0f) {
                p.currentHP = 0.0f;
                p.state = PirateState::Dead;
                p.stateTimer = 0.0f;
                p.respawnTimer = p.config.respawnDelay;
                p.lootFill = 0.0f;
            }
            return true;
        }
    }
    return false;
}

bool AIPirateStateMachine::KillPirate(uint32_t entityId) {
    for (auto& p : m_pirates) {
        if (p.config.entityId == entityId && p.state != PirateState::Dead) {
            p.state = PirateState::Dead;
            p.stateTimer = 0.0f;
            p.currentHP = 0.0f;
            p.respawnTimer = p.config.respawnDelay;
            p.lootFill = 0.0f;
            return true;
        }
    }
    return false;
}

const PirateRuntime* AIPirateStateMachine::GetPirate(uint32_t entityId) const {
    for (auto& p : m_pirates) {
        if (p.config.entityId == entityId) return &p;
    }
    return nullptr;
}

size_t AIPirateStateMachine::CountInState(PirateState state) const {
    size_t count = 0;
    for (auto& p : m_pirates) {
        if (p.state == state) ++count;
    }
    return count;
}

uint32_t AIPirateStateMachine::TotalKills() const {
    uint32_t sum = 0;
    for (auto& p : m_pirates) {
        sum += p.killCount;
    }
    return sum;
}

float AIPirateStateMachine::TotalLoot() const {
    float sum = 0.0f;
    for (auto& p : m_pirates) {
        sum += p.lootFill;
    }
    return sum;
}

void AIPirateStateMachine::tickPirate(PirateRuntime& p, float dt) {
    p.stateTimer += dt;

    switch (p.state) {
        case PirateState::Idle: {
            // Immediately begin patrolling
            p.state = PirateState::Patrol;
            p.stateTimer = 0.0f;
            p.travelProgress = 0.0f;
            break;
        }
        case PirateState::Patrol: {
            float travelTime = m_patrolDistance / p.config.travelSpeed;
            p.travelProgress += dt / travelTime;
            if (p.travelProgress >= 1.0f) {
                p.travelProgress = 1.0f;
                p.state = PirateState::ScanForTargets;
                p.stateTimer = 0.0f;
            }
            break;
        }
        case PirateState::ScanForTargets: {
            uint32_t target = pickTarget();
            if (target == 0) {
                // No targets, resume patrol
                p.state = PirateState::Patrol;
                p.stateTimer = 0.0f;
                p.travelProgress = 0.0f;
            } else {
                p.targetEntityId = target;
                p.state = PirateState::PursueTarget;
                p.stateTimer = 0.0f;
                p.travelProgress = 0.0f;
            }
            break;
        }
        case PirateState::PursueTarget: {
            float travelTime = p.config.scanRange / p.config.travelSpeed;
            p.travelProgress += dt / travelTime;
            if (p.travelProgress >= 1.0f) {
                p.travelProgress = 1.0f;
                p.state = PirateState::Attack;
                p.stateTimer = 0.0f;
            }
            break;
        }
        case PirateState::Attack: {
            // Attack for 3 seconds
            if (p.stateTimer >= 3.0f) {
                p.killCount++;
                if (p.currentHP < p.config.hitPoints * 0.25f) {
                    p.state = PirateState::Flee;
                    p.stateTimer = 0.0f;
                    p.travelProgress = 0.0f;
                } else {
                    p.state = PirateState::LootCargo;
                    p.stateTimer = 0.0f;
                }
            }
            break;
        }
        case PirateState::LootCargo: {
            float looted = 50.0f * dt; // 50 m³/s
            p.lootFill += looted;
            if (p.lootFill >= p.config.lootCapacity) {
                p.lootFill = p.config.lootCapacity;
                p.state = PirateState::Patrol;
                p.stateTimer = 0.0f;
                p.travelProgress = 0.0f;
            }
            break;
        }
        case PirateState::Flee: {
            float travelTime = m_patrolDistance / p.config.travelSpeed;
            p.travelProgress += dt / travelTime;
            if (p.travelProgress >= 1.0f) {
                p.travelProgress = 1.0f;
                p.state = PirateState::Patrol;
                p.stateTimer = 0.0f;
                p.travelProgress = 0.0f;
            }
            break;
        }
        case PirateState::Dead: {
            p.respawnTimer -= dt;
            if (p.respawnTimer <= 0.0f) {
                p.respawnTimer = 0.0f;
                p.currentHP = p.config.hitPoints;
                p.state = PirateState::Idle;
                p.stateTimer = 0.0f;
            }
            break;
        }
    }
}

uint32_t AIPirateStateMachine::pickTarget() const {
    if (m_targetsInRange.empty()) return 0;
    // Pick first available target
    return m_targetsInRange[0];
}

} // namespace atlas::sim
