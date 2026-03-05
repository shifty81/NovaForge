#include "systems/captain_transfer_system.h"
#include "ecs/world.h"

namespace atlas {
namespace systems {

CaptainTransferSystem::CaptainTransferSystem(ecs::World* world)
    : SingleComponentSystem(world) {
}

void CaptainTransferSystem::updateComponent(ecs::Entity& entity, components::CaptainTransferRequest& req, float /*delta_time*/) {
    auto* morale = entity.getComponent<components::FleetMorale>();
    if (!morale) return;

    if (!req.request_pending) {
        if (morale->morale_score >= 80.0f) {
            req.request_pending = true;
            req.request_type = components::CaptainTransferRequest::TransferType::BiggerShip;
            req.morale_at_request = morale->morale_score;
        } else if (morale->morale_score <= 30.0f) {
            req.request_pending = true;
            req.request_type = components::CaptainTransferRequest::TransferType::EscortOnly;
            req.morale_at_request = morale->morale_score;
        }
    }
}

bool CaptainTransferSystem::hasPendingRequest(const std::string& entity_id) const {
    const auto* req = getComponentFor(entity_id);
    if (!req) return false;
    return req->request_pending;
}

void CaptainTransferSystem::approveTransfer(const std::string& entity_id) {
    auto* req = getComponentFor(entity_id);
    if (!req) return;
    req->request_pending = false;
}

void CaptainTransferSystem::denyTransfer(const std::string& entity_id) {
    auto* req = getComponentFor(entity_id);
    if (!req) return;
    req->request_pending = false;
}

std::vector<std::string> CaptainTransferSystem::getPendingTransfers() const {
    std::vector<std::string> result;
    auto entities = world_->getEntities<components::CaptainTransferRequest>();
    for (auto* entity : entities) {
        auto* req = entity->getComponent<components::CaptainTransferRequest>();
        if (req && req->request_pending) {
            result.push_back(entity->getId());
        }
    }
    return result;
}

} // namespace systems
} // namespace atlas
