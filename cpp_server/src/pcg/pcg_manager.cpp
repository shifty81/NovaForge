#include "pcg/pcg_manager.h"

namespace atlas {
namespace pcg {

PCGManager::PCGManager()
    : universeSeed_(0)
    , initialized_(false)
{}

void PCGManager::initialize(uint64_t universeSeed) {
    universeSeed_ = universeSeed;
    initialized_  = true;
}

bool PCGManager::isInitialized() const {
    return initialized_;
}

PCGContext PCGManager::makeContext(PCGDomain domain,
                                  uint64_t parentSeed,
                                  uint64_t objectId,
                                  uint32_t version) const {
    uint64_t seed = hash64(
        static_cast<uint64_t>(domain),
        parentSeed,
        objectId,
        static_cast<uint64_t>(version)
    );
    return { seed, version };
}

PCGContext PCGManager::makeRootContext(PCGDomain domain,
                                      uint64_t objectId,
                                      uint32_t version) const {
    return makeContext(domain, universeSeed_, objectId, version);
}

uint64_t PCGManager::universeSeed() const {
    return universeSeed_;
}

} // namespace pcg
} // namespace atlas
