#include "systems/signature_analysis_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

SignatureAnalysisSystem::SignatureAnalysisSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void SignatureAnalysisSystem::updateComponent(ecs::Entity& /*entity*/,
                                               components::SignatureAnalysisState& comp,
                                               float delta_time) {
    if (!comp.active) return;
    comp.elapsed += delta_time;
}

bool SignatureAnalysisSystem::initialize(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::SignatureAnalysisState>();
    entity->addComponent(std::move(comp));
    return true;
}

bool SignatureAnalysisSystem::addSignature(
        const std::string& entity_id,
        const std::string& sig_id,
        const std::string& sig_name,
        components::SignatureAnalysisState::SigType type,
        float strength,
        float scan_strength_required) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (sig_id.empty()) return false;
    if (sig_name.empty()) return false;
    if (strength < 0.0f || strength > 100.0f) return false;
    if (scan_strength_required <= 0.0f) return false;
    if (static_cast<int>(comp->signatures.size()) >= comp->max_signatures) return false;

    auto it = std::find_if(comp->signatures.begin(), comp->signatures.end(),
                           [&](const components::SignatureAnalysisState::Signature& s) {
                               return s.sig_id == sig_id;
                           });
    if (it != comp->signatures.end()) return false;

    components::SignatureAnalysisState::Signature sig;
    sig.sig_id                 = sig_id;
    sig.sig_name               = sig_name;
    sig.type                   = type;
    sig.strength               = strength;
    sig.scan_strength_required = scan_strength_required;
    comp->signatures.push_back(sig);
    return true;
}

bool SignatureAnalysisSystem::removeSignature(const std::string& entity_id,
                                               const std::string& sig_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    auto it = std::find_if(comp->signatures.begin(), comp->signatures.end(),
                           [&](const components::SignatureAnalysisState::Signature& s) {
                               return s.sig_id == sig_id;
                           });
    if (it == comp->signatures.end()) return false;
    comp->signatures.erase(it);
    return true;
}

bool SignatureAnalysisSystem::clearSignatures(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->signatures.clear();
    return true;
}

bool SignatureAnalysisSystem::scanTick(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (auto& sig : comp->signatures) {
        if (sig.identified) continue;
        sig.scan_progress += comp->scan_contribution;
        if (sig.scan_progress >= 100.0f) {
            sig.scan_progress = 100.0f;
            sig.identified    = true;
            comp->total_identified++;
        }
    }
    return true;
}

bool SignatureAnalysisSystem::setScanContribution(const std::string& entity_id,
                                                   float contribution) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->scan_contribution = contribution;
    return true;
}

int SignatureAnalysisSystem::getSigCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    return static_cast<int>(comp->signatures.size());
}

int SignatureAnalysisSystem::getIdentifiedCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    int count = 0;
    for (const auto& s : comp->signatures)
        if (s.identified) count++;
    return count;
}

bool SignatureAnalysisSystem::isIdentified(const std::string& entity_id,
                                            const std::string& sig_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    auto it = std::find_if(comp->signatures.begin(), comp->signatures.end(),
                           [&](const components::SignatureAnalysisState::Signature& s) {
                               return s.sig_id == sig_id;
                           });
    if (it == comp->signatures.end()) return false;
    return it->identified;
}

float SignatureAnalysisSystem::getScanProgress(const std::string& entity_id,
                                                const std::string& sig_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0f;
    auto it = std::find_if(comp->signatures.begin(), comp->signatures.end(),
                           [&](const components::SignatureAnalysisState::Signature& s) {
                               return s.sig_id == sig_id;
                           });
    if (it == comp->signatures.end()) return 0.0f;
    return it->scan_progress;
}

float SignatureAnalysisSystem::getStrength(const std::string& entity_id,
                                            const std::string& sig_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0f;
    auto it = std::find_if(comp->signatures.begin(), comp->signatures.end(),
                           [&](const components::SignatureAnalysisState::Signature& s) {
                               return s.sig_id == sig_id;
                           });
    if (it == comp->signatures.end()) return 0.0f;
    return it->strength;
}

bool SignatureAnalysisSystem::hasSignature(const std::string& entity_id,
                                            const std::string& sig_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    return std::find_if(comp->signatures.begin(), comp->signatures.end(),
                        [&](const components::SignatureAnalysisState::Signature& s) {
                            return s.sig_id == sig_id;
                        }) != comp->signatures.end();
}

int SignatureAnalysisSystem::getTotalIdentified(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    return comp->total_identified;
}

int SignatureAnalysisSystem::getCountByType(
        const std::string& entity_id,
        components::SignatureAnalysisState::SigType type) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    int count = 0;
    for (const auto& s : comp->signatures)
        if (s.type == type) count++;
    return count;
}

} // namespace systems
} // namespace atlas
