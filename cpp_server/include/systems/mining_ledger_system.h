#ifndef NOVAFORGE_SYSTEMS_MINING_LEDGER_SYSTEM_H
#define NOVAFORGE_SYSTEMS_MINING_LEDGER_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/economy_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Mining session history tracker
 *
 * Records mining events (ore type, quantity, ISK value) and maintains
 * aggregate statistics.  The ledger is capped at max_entries (default 100);
 * when the cap is reached the oldest entry is removed.  Lifetime aggregates
 * (total_quantity, total_isk, total_entries) are never reduced by purging.
 */
class MiningLedgerSystem
    : public ecs::SingleComponentSystem<components::MiningLedgerState> {
public:
    explicit MiningLedgerSystem(ecs::World* world);
    ~MiningLedgerSystem() override = default;

    std::string getName() const override { return "MiningLedgerSystem"; }

    // --- Lifecycle ---
    bool initialize(const std::string& entity_id,
                    const std::string& owner_id = "");

    // --- Entry management ---
    bool addEntry(const std::string& entity_id,
                  const std::string& entry_id,
                  const std::string& ore_type,
                  int quantity,
                  float isk_value);
    bool removeEntry(const std::string& entity_id,
                     const std::string& entry_id);
    bool clearLedger(const std::string& entity_id);

    // --- Configuration ---
    bool setOwner(const std::string& entity_id,
                  const std::string& owner_id);
    bool setMaxEntries(const std::string& entity_id, int max);

    // --- Queries ---
    int         getEntryCount(const std::string& entity_id) const;
    int         getTotalEntries(const std::string& entity_id) const;
    int         getTotalQuantity(const std::string& entity_id) const;
    float       getTotalIsk(const std::string& entity_id) const;
    std::string getOwner(const std::string& entity_id) const;
    bool        hasEntry(const std::string& entity_id,
                         const std::string& entry_id) const;
    int         getQuantityByOreType(const std::string& entity_id,
                                     const std::string& ore_type) const;
    float       getIskByOreType(const std::string& entity_id,
                                const std::string& ore_type) const;

protected:
    void updateComponent(ecs::Entity& entity,
                         components::MiningLedgerState& comp,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_MINING_LEDGER_SYSTEM_H
