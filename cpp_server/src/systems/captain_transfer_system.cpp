#include "systems/captain_transfer_system.h"
#include "ecs/world.h"

namespace atlas {
namespace systems {

CaptainTransferSystem::CaptainTransferSystem(ecs::World* world)
    : System(world) {
}

void CaptainTransferSystem::update(float /*delta_time*/) {
    auto entities = world_->getEntities<components::CaptainTransferRequest>();
    for (auto* entity : entities) {
        auto* req = entity->getComponent<components::CaptainTransferRequest>();
        auto* morale = entity->getComponent<components::FleetMorale>();
        if (!req || !morale) continue;

        if (!req->request_pending) {
            if (morale->morale_score >= 80.0f) {
                req->request_pending = true;
                req->request_type = components::CaptainTransferRequest::TransferType::BiggerShip;
                req->morale_at_request = morale->morale_score;
            } else if (morale->morale_score <= 30.0f) {
                req->request_pending = true;
                req->request_type = components::CaptainTransferRequest::TransferType::EscortOnly;
                req->morale_at_request = morale->morale_score;
            }
        }
    }
}

bool CaptainTransferSystem::hasPendingRequest(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto* req = entity->getComponent<components::CaptainTransferRequest>();
    if (!req) return false;
    return req->request_pending;
}

void CaptainTransferSystem::approveTransfer(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return;
    auto* req = entity->getComponent<components::CaptainTransferRequest>();
    if (!req) return;
    req->request_pending = false;
}

void CaptainTransferSystem::denyTransfer(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return;
    auto* req = entity->getComponent<components::CaptainTransferRequest>();
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
