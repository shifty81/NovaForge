#include "sim/AISecurityStateMachine.h"
#include <algorithm>

namespace atlas::sim {

bool AISecurityStateMachine::AddGuard(const SecurityConfig& config) {
    for (auto& g : m_guards) {
        if (g.config.entityId == config.entityId) return false;
    }
    SecurityRuntime rt;
    rt.config    = config;
    rt.state     = SecurityState::Idle;
    rt.currentHP = config.hitPoints;
    m_guards.push_back(rt);
    return true;
}

bool AISecurityStateMachine::RemoveGuard(uint32_t entityId) {
    auto it = std::remove_if(m_guards.begin(), m_guards.end(),
        [entityId](const SecurityRuntime& g) { return g.config.entityId == entityId; });
    if (it == m_guards.end()) return false;
    m_guards.erase(it, m_guards.end());
    return true;
}

void AISecurityStateMachine::SetActiveThreats(const std::vector<uint32_t>& threats) {
    m_activeThreats = threats;
}

void AISecurityStateMachine::Tick(float deltaSeconds) {
    for (auto& g : m_guards) {
        tickGuard(g, deltaSeconds);
    }
}

bool AISecurityStateMachine::DamageGuard(uint32_t entityId, float damage) {
    for (auto& g : m_guards) {
        if (g.config.entityId == entityId && g.state != SecurityState::Dead) {
            g.currentHP -= damage;
            if (g.currentHP <= 0.0f) {
                g.currentHP = 0.0f;
                g.state = SecurityState::Dead;
                g.stateTimer = 0.0f;
                g.respawnTimer = g.config.respawnDelay;
            }
            return true;
        }
    }
    return false;
}

bool AISecurityStateMachine::KillGuard(uint32_t entityId) {
    for (auto& g : m_guards) {
        if (g.config.entityId == entityId && g.state != SecurityState::Dead) {
            g.state = SecurityState::Dead;
            g.stateTimer = 0.0f;
            g.currentHP = 0.0f;
            g.respawnTimer = g.config.respawnDelay;
            return true;
        }
    }
    return false;
}

const SecurityRuntime* AISecurityStateMachine::GetGuard(uint32_t entityId) const {
    for (auto& g : m_guards) {
        if (g.config.entityId == entityId) return &g;
    }
    return nullptr;
}

size_t AISecurityStateMachine::CountInState(SecurityState state) const {
    size_t count = 0;
    for (auto& g : m_guards) {
        if (g.state == state) ++count;
    }
    return count;
}

uint32_t AISecurityStateMachine::TotalNeutralized() const {
    uint32_t sum = 0;
    for (auto& g : m_guards) {
        sum += g.threatsNeutralized;
    }
    return sum;
}

void AISecurityStateMachine::tickGuard(SecurityRuntime& g, float dt) {
    g.stateTimer += dt;

    switch (g.state) {
        case SecurityState::Idle: {
            // Immediately go to standby
            g.state = SecurityState::Standby;
            g.stateTimer = 0.0f;
            break;
        }
        case SecurityState::Standby: {
            // Wait responseTime before scanning
            if (g.stateTimer >= g.config.responseTime) {
                g.state = SecurityState::ScanForThreats;
                g.stateTimer = 0.0f;
            }
            break;
        }
        case SecurityState::ScanForThreats: {
            uint32_t threat = pickThreat();
            if (threat == 0) {
                // No threats, return to standby
                g.state = SecurityState::Standby;
                g.stateTimer = 0.0f;
            } else {
                g.targetThreatId = threat;
                g.state = SecurityState::WarpToThreat;
                g.stateTimer = 0.0f;
                g.travelProgress = 0.0f;
            }
            break;
        }
        case SecurityState::WarpToThreat: {
            float travelTime = m_postDistance / g.config.travelSpeed;
            g.travelProgress += dt / travelTime;
            if (g.travelProgress >= 1.0f) {
                g.travelProgress = 1.0f;
                g.state = SecurityState::EngageThreat;
                g.stateTimer = 0.0f;
            }
            break;
        }
        case SecurityState::EngageThreat: {
            // Fight for up to 5 seconds
            if (g.currentHP < g.config.hitPoints * 0.25f) {
                // HP low, disengage
                g.state = SecurityState::ReturnToPost;
                g.stateTimer = 0.0f;
                g.travelProgress = 0.0f;
            } else if (g.stateTimer >= 5.0f) {
                g.threatsNeutralized++;
                g.state = SecurityState::ReturnToPost;
                g.stateTimer = 0.0f;
                g.travelProgress = 0.0f;
            }
            break;
        }
        case SecurityState::ReturnToPost: {
            float travelTime = m_postDistance / g.config.travelSpeed;
            g.travelProgress += dt / travelTime;
            if (g.travelProgress >= 1.0f) {
                g.travelProgress = 1.0f;
                g.state = SecurityState::Standby;
                g.stateTimer = 0.0f;
            }
            break;
        }
        case SecurityState::Dead: {
            g.respawnTimer -= dt;
            if (g.respawnTimer <= 0.0f) {
                g.respawnTimer = 0.0f;
                g.currentHP = g.config.hitPoints;
                g.state = SecurityState::Idle;
                g.stateTimer = 0.0f;
            }
            break;
        }
    }
}

uint32_t AISecurityStateMachine::pickThreat() const {
    if (m_activeThreats.empty()) return 0;
    // Pick first available threat
    return m_activeThreats[0];
}

} // namespace atlas::sim
