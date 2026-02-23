#ifndef EVE_PCG_MANAGER_H
#define EVE_PCG_MANAGER_H

#include "pcg_context.h"
#include "hash_utils.h"
#include <cstdint>

namespace atlas {
namespace pcg {

/**
 * @brief Central authority for deterministic procedural generation.
 *
 * The PCGManager owns the universe seed and creates scoped PCGContext
 * objects for every generator.  No generator may invent its own seed;
 * all randomness flows from the universe seed through this manager.
 *
 * Thread-safety: the manager itself is read-only after Initialize().
 */
class PCGManager {
public:
    PCGManager();

    /**
     * @brief Set the universe seed.  Must be called once before any
     *        generation.  Calling again resets the entire PCG tree.
     */
    void initialize(uint64_t universeSeed);

    /** Check whether the manager has been initialized. */
    bool isInitialized() const;

    /**
     * @brief Create a context for the given domain and object.
     *
     * @param domain    Logical domain (Ship, Asteroid, Fleet …).
     * @param parentSeed  Seed of the enclosing scope (e.g. sector seed).
     * @param objectId    Unique id within the domain.
     * @param version     Rules version for this domain.
     * @return A PCGContext whose seed is a deterministic function of all
     *         the inputs.
     */
    PCGContext makeContext(PCGDomain domain,
                          uint64_t parentSeed,
                          uint64_t objectId,
                          uint32_t version) const;

    /** Convenience: derive a context directly from the universe seed. */
    PCGContext makeRootContext(PCGDomain domain,
                              uint64_t objectId,
                              uint32_t version) const;

    /** @return The universe seed. */
    uint64_t universeSeed() const;

private:
    uint64_t universeSeed_;
    bool     initialized_;
};

} // namespace pcg
} // namespace atlas

#endif // EVE_PCG_MANAGER_H
