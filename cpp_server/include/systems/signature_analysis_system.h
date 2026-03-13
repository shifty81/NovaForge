#ifndef NOVAFORGE_SYSTEMS_SIGNATURE_ANALYSIS_SYSTEM_H
#define NOVAFORGE_SYSTEMS_SIGNATURE_ANALYSIS_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/exploration_components.h"
#include <string>

namespace atlas {
namespace systems {

class SignatureAnalysisSystem
    : public ecs::SingleComponentSystem<components::SignatureAnalysisState> {
public:
    explicit SignatureAnalysisSystem(ecs::World* world);
    ~SignatureAnalysisSystem() override = default;

    std::string getName() const override { return "SignatureAnalysisSystem"; }

    bool initialize(const std::string& entity_id);

    bool addSignature(const std::string& entity_id,
                      const std::string& sig_id,
                      const std::string& sig_name,
                      components::SignatureAnalysisState::SigType type,
                      float strength,
                      float scan_strength_required);
    bool removeSignature(const std::string& entity_id, const std::string& sig_id);
    bool clearSignatures(const std::string& entity_id);
    bool scanTick(const std::string& entity_id);
    bool setScanContribution(const std::string& entity_id, float contribution);

    int   getSigCount(const std::string& entity_id) const;
    int   getIdentifiedCount(const std::string& entity_id) const;
    bool  isIdentified(const std::string& entity_id, const std::string& sig_id) const;
    float getScanProgress(const std::string& entity_id, const std::string& sig_id) const;
    float getStrength(const std::string& entity_id, const std::string& sig_id) const;
    bool  hasSignature(const std::string& entity_id, const std::string& sig_id) const;
    int   getTotalIdentified(const std::string& entity_id) const;
    int   getCountByType(const std::string& entity_id,
                         components::SignatureAnalysisState::SigType type) const;

protected:
    void updateComponent(ecs::Entity& entity,
                         components::SignatureAnalysisState& comp,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_SIGNATURE_ANALYSIS_SYSTEM_H
