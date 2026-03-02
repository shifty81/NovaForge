#ifndef NOVAFORGE_SYSTEMS_CARGO_SCAN_SYSTEM_H
#define NOVAFORGE_SYSTEMS_CARGO_SCAN_SYSTEM_H

#include "ecs/system.h"
#include "components/ship_components.h"
#include <string>
#include <vector>

namespace atlas {
namespace systems {

/**
 * @brief Cargo scanning and contraband detection system
 *
 * Manages cargo scanning: initiate scan on target, progress timer,
 * detect contraband based on detection_chance, issue fines.
 * Customs scanners at gates/stations can auto-scan passing ships.
 */
class CargoScanSystem : public ecs::System {
public:
    explicit CargoScanSystem(ecs::World* world);
    ~CargoScanSystem() override = default;

    void update(float delta_time) override;
    std::string getName() const override { return "CargoScanSystem"; }

    // Commands
    bool initiateScan(const std::string& scanner_id, const std::string& target_id);
    bool cancelScan(const std::string& scanner_id);
    bool setDetectionChance(const std::string& scanner_id, float chance);
    bool markAsCustomsScanner(const std::string& scanner_id, bool is_customs);
    bool plantContraband(const std::string& entity_id, components::CargoScanState::ContrabandType type);
    bool removeContraband(const std::string& entity_id, components::CargoScanState::ContrabandType type);

    // Query API
    std::string getPhase(const std::string& scanner_id) const;
    float getScanProgress(const std::string& scanner_id) const;
    int getContrabandFound(const std::string& scanner_id) const;
    int getTotalScans(const std::string& scanner_id) const;
    int getTotalContrabandDetected(const std::string& scanner_id) const;
    double getTotalFinesIssued(const std::string& scanner_id) const;
    bool isCustomsScanner(const std::string& scanner_id) const;
    std::vector<std::string> getDetectedTypes(const std::string& scanner_id) const;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_CARGO_SCAN_SYSTEM_H
