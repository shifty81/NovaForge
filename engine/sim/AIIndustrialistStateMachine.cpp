#include "sim/AIIndustrialistStateMachine.h"
#include <algorithm>

namespace atlas::sim {

bool AIIndustrialistStateMachine::AddIndustrialist(const IndustrialistConfig& config) {
    for (auto& ind : m_industrialists) {
        if (ind.config.entityId == config.entityId) return false;
    }
    IndustrialistRuntime rt;
    rt.config  = config;
    rt.state   = IndustrialistState::Idle;
    rt.credits = config.startingCredits;
    m_industrialists.push_back(rt);
    return true;
}

bool AIIndustrialistStateMachine::RemoveIndustrialist(uint32_t entityId) {
    auto it = std::remove_if(m_industrialists.begin(), m_industrialists.end(),
        [entityId](const IndustrialistRuntime& ind) { return ind.config.entityId == entityId; });
    if (it == m_industrialists.end()) return false;
    m_industrialists.erase(it, m_industrialists.end());
    return true;
}

void AIIndustrialistStateMachine::SetAvailableBlueprints(const std::vector<BlueprintEntry>& blueprints) {
    m_blueprints = blueprints;
}

void AIIndustrialistStateMachine::Tick(float deltaSeconds) {
    for (auto& ind : m_industrialists) {
        tickIndustrialist(ind, deltaSeconds);
    }
}

bool AIIndustrialistStateMachine::KillIndustrialist(uint32_t entityId) {
    for (auto& ind : m_industrialists) {
        if (ind.config.entityId == entityId && ind.state != IndustrialistState::Dead) {
            ind.state = IndustrialistState::Dead;
            ind.stateTimer = 0.0f;
            ind.respawnTimer = ind.config.respawnDelay;
            ind.outputFill = 0.0f;
            ind.buildProgress = 0.0f;
            return true;
        }
    }
    return false;
}

const IndustrialistRuntime* AIIndustrialistStateMachine::GetIndustrialist(uint32_t entityId) const {
    for (auto& ind : m_industrialists) {
        if (ind.config.entityId == entityId) return &ind;
    }
    return nullptr;
}

size_t AIIndustrialistStateMachine::CountInState(IndustrialistState state) const {
    size_t count = 0;
    for (auto& ind : m_industrialists) {
        if (ind.state == state) ++count;
    }
    return count;
}

float AIIndustrialistStateMachine::TotalProfit() const {
    float sum = 0.0f;
    for (auto& ind : m_industrialists) {
        sum += ind.totalProfit;
    }
    return sum;
}

uint32_t AIIndustrialistStateMachine::TotalJobs() const {
    uint32_t sum = 0;
    for (auto& ind : m_industrialists) {
        sum += ind.jobsCompleted;
    }
    return sum;
}

void AIIndustrialistStateMachine::tickIndustrialist(IndustrialistRuntime& ind, float dt) {
    ind.stateTimer += dt;

    switch (ind.state) {
        case IndustrialistState::Idle: {
            ind.state = IndustrialistState::SelectBlueprint;
            ind.stateTimer = 0.0f;
            break;
        }
        case IndustrialistState::SelectBlueprint: {
            const BlueprintEntry* bp = pickBestBlueprint(ind.credits);
            if (!bp) {
                // No affordable blueprint, go back to idle
                ind.state = IndustrialistState::Idle;
                ind.stateTimer = 0.0f;
            } else {
                ind.activeBlueprintId  = bp->blueprintId;
                ind.activeOutputValue  = bp->outputValue;
                ind.activeMaterialCost = bp->materialCost;
                ind.activeBuildTime    = bp->buildTime;
                ind.state = IndustrialistState::AcquireMaterials;
                ind.stateTimer = 0.0f;
            }
            break;
        }
        case IndustrialistState::AcquireMaterials: {
            // Calculate how many units we can produce (limited by credits and capacity)
            float unitsAffordable = ind.credits / ind.activeMaterialCost;
            float unitsByCapacity = ind.config.productionCapacity;
            float units = std::min(unitsAffordable, unitsByCapacity);

            if (units >= 1.0f) {
                float cost = units * ind.activeMaterialCost;
                ind.credits -= cost;
                ind.outputFill = units; // units to be produced
                ind.state = IndustrialistState::Manufacturing;
                ind.stateTimer = 0.0f;
                ind.buildProgress = 0.0f;
            } else {
                // Can't afford materials, go back to idle
                ind.state = IndustrialistState::Idle;
                ind.stateTimer = 0.0f;
            }
            break;
        }
        case IndustrialistState::Manufacturing: {
            ind.buildProgress += dt / ind.activeBuildTime;
            if (ind.buildProgress >= 1.0f) {
                ind.buildProgress = 1.0f;
                ind.state = IndustrialistState::DeliverGoods;
                ind.stateTimer = 0.0f;
                ind.travelProgress = 0.0f;
            }
            break;
        }
        case IndustrialistState::DeliverGoods: {
            float travelTime = m_marketDistance / ind.config.travelSpeed;
            ind.travelProgress += dt / travelTime;
            if (ind.travelProgress >= 1.0f) {
                ind.travelProgress = 1.0f;
                ind.state = IndustrialistState::SellGoods;
                ind.stateTimer = 0.0f;
            }
            break;
        }
        case IndustrialistState::SellGoods: {
            float revenue = ind.outputFill * ind.activeOutputValue;
            float cost    = ind.outputFill * ind.activeMaterialCost;
            float profit  = revenue - cost;

            ind.credits += revenue;
            ind.totalProfit += profit;
            ind.outputFill = 0.0f;
            ind.jobsCompleted++;
            ind.state = IndustrialistState::Idle;
            ind.stateTimer = 0.0f;
            break;
        }
        case IndustrialistState::Dead: {
            ind.respawnTimer -= dt;
            if (ind.respawnTimer <= 0.0f) {
                ind.respawnTimer = 0.0f;
                ind.credits = ind.config.startingCredits;
                ind.state = IndustrialistState::Idle;
                ind.stateTimer = 0.0f;
            }
            break;
        }
    }
}

const BlueprintEntry* AIIndustrialistStateMachine::pickBestBlueprint(float availableCredits) const {
    if (m_blueprints.empty()) return nullptr;

    const BlueprintEntry* best = nullptr;
    float bestMargin = 0.0f;

    for (auto& bp : m_blueprints) {
        if (bp.materialCost <= 0.0f) continue;
        if (bp.materialCost > availableCredits) continue;
        float margin = bp.outputValue - bp.materialCost;
        if (margin > bestMargin) {
            bestMargin = margin;
            best = &bp;
        }
    }
    return best;
}

} // namespace atlas::sim
