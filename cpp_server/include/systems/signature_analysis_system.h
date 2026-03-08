#ifndef NOVAFORGE_SYSTEMS_SIGNATURE_ANALYSIS_SYSTEM_H
#define NOVAFORGE_SYSTEMS_SIGNATURE_ANALYSIS_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/game_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Cosmic signature exploration scanning system
 *
 * Manages scanning and analysis of cosmic signatures found during
 * exploration. Tracks signal readings, scan resolution progress,
 * and identification status. Key for the exploration vertical slice.
 */
class SignatureAnalysisSystem : public ecs::SingleComponentSystem<components::SignatureAnalysisState> {
public:
    explicit SignatureAnalysisSystem(ecs::World* world);
    ~SignatureAnalysisSystem() override = default;

    std::string getName() const override { return "SignatureAnalysisSystem"; }

public:
    // Initialization
    bool initialize(const std::string& entity_id, float scan_power = 1.0f);

    // Reading management
    bool addReading(const std::string& entity_id, const std::string& sig_id,
                    const std::string& sig_type, float signal_strength);
    bool removeReading(const std::string& entity_id, const std::string& sig_id);

    // Operations
    bool analyzeSignature(const std::string& entity_id, const std::string& sig_id);
    bool setScanPower(const std::string& entity_id, float power);
    bool setAnalysisSpeed(const std::string& entity_id, float speed);

    // Queries
    int getReadingCount(const std::string& entity_id) const;
    int getIdentifiedCount(const std::string& entity_id) const;
    float getSignalStrength(const std::string& entity_id, const std::string& sig_id) const;
    bool isIdentified(const std::string& entity_id, const std::string& sig_id) const;
    float getScanPower(const std::string& entity_id) const;
    int getTotalScans(const std::string& entity_id) const;
    int getTotalIdentified(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity, components::SignatureAnalysisState& comp,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_SIGNATURE_ANALYSIS_SYSTEM_H
