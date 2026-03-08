#ifndef NOVAFORGE_SYSTEMS_ASTEROID_SCANNER_SYSTEM_H
#define NOVAFORGE_SYSTEMS_ASTEROID_SCANNER_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/navigation_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Scans asteroids to reveal ore composition and estimated value
 *
 * Manages the scan lifecycle: initiate scan, progress tracking, and
 * completion. Higher scan resolution reveals rarer ore types. Integrates
 * with mining and economy systems to provide actionable data.
 */
class AsteroidScannerSystem : public ecs::SingleComponentSystem<components::AsteroidScannerState> {
public:
    explicit AsteroidScannerSystem(ecs::World* world);
    ~AsteroidScannerSystem() override = default;

    std::string getName() const override { return "AsteroidScannerSystem"; }

public:
    bool initialize(const std::string& entity_id, const std::string& target_asteroid);
    bool startScan(const std::string& entity_id);
    bool cancelScan(const std::string& entity_id);
    bool addReading(const std::string& entity_id, const std::string& ore_type,
                    float concentration, float value);
    bool removeReading(const std::string& entity_id, const std::string& ore_type);
    bool setScanDuration(const std::string& entity_id, float duration);
    bool setScanResolution(const std::string& entity_id, float resolution);
    float getScanProgress(const std::string& entity_id) const;
    bool isScanning(const std::string& entity_id) const;
    bool isScanComplete(const std::string& entity_id) const;
    int getReadingCount(const std::string& entity_id) const;
    int getTotalScans(const std::string& entity_id) const;
    float getTotalValueScanned(const std::string& entity_id) const;
    std::string getTargetAsteroid(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity, components::AsteroidScannerState& state, float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_ASTEROID_SCANNER_SYSTEM_H
