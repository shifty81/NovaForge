#ifndef NOVAFORGE_SYSTEMS_CORP_TAX_LEDGER_SYSTEM_H
#define NOVAFORGE_SYSTEMS_CORP_TAX_LEDGER_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/economy_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Corporation tax ledger tracking system
 *
 * Records taxes collected from member activities (bounties, mission rewards,
 * PI exports, industry jobs, market transactions).  Each entry captures the
 * gross amount, applied tax rate, and resulting tax collected.  The ledger is
 * capped at max_entries (default 200); oldest entries are purged when the cap
 * is reached.  Lifetime aggregates (total_collected, total_entries_ever) are
 * maintained independently of purges.
 */
class CorpTaxLedgerSystem
    : public ecs::SingleComponentSystem<components::CorpTaxLedgerState> {
public:
    explicit CorpTaxLedgerSystem(ecs::World* world);
    ~CorpTaxLedgerSystem() override = default;

    std::string getName() const override { return "CorpTaxLedgerSystem"; }

    // --- Lifecycle ---
    bool initialize(const std::string& entity_id);

    // --- Entry management ---
    bool addEntry(const std::string& entity_id,
                  const std::string& entry_id,
                  components::CorpTaxLedgerState::TaxType tax_type,
                  const std::string& member_id,
                  double gross_amount);
    bool addEntryWithRate(const std::string& entity_id,
                          const std::string& entry_id,
                          components::CorpTaxLedgerState::TaxType tax_type,
                          const std::string& member_id,
                          double gross_amount,
                          double tax_rate);
    bool removeEntry(const std::string& entity_id,
                     const std::string& entry_id);
    bool clearLedger(const std::string& entity_id);

    // --- Configuration ---
    bool setCorpId(const std::string& entity_id, const std::string& corp_id);
    bool setDefaultTaxRate(const std::string& entity_id, double rate);
    bool setMaxEntries(const std::string& entity_id, int max_entries);

    // --- Queries ---
    int    getEntryCount(const std::string& entity_id) const;
    int    getTotalEntriesEver(const std::string& entity_id) const;
    double getTotalCollected(const std::string& entity_id) const;
    double getDefaultTaxRate(const std::string& entity_id) const;
    std::string getCorpId(const std::string& entity_id) const;
    bool   hasEntry(const std::string& entity_id,
                    const std::string& entry_id) const;
    double getCollectedByType(const std::string& entity_id,
                              components::CorpTaxLedgerState::TaxType type) const;
    double getCollectedByMember(const std::string& entity_id,
                                const std::string& member_id) const;
    int    getCountByType(const std::string& entity_id,
                          components::CorpTaxLedgerState::TaxType type) const;

protected:
    void updateComponent(ecs::Entity& entity,
                         components::CorpTaxLedgerState& comp,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_CORP_TAX_LEDGER_SYSTEM_H
