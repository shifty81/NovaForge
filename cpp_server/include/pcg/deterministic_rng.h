#ifndef EVE_PCG_DETERMINISTIC_RNG_H
#define EVE_PCG_DETERMINISTIC_RNG_H

#include <cstdint>

namespace atlas {
namespace pcg {

/**
 * @brief Platform-independent deterministic random number generator.
 *
 * Uses xorshift64* — fast, well-distributed, and bit-identical across
 * all compilers and platforms.  Every PCG system must use this instead
 * of std::rand, std::mt19937, or platform entropy.
 */
class DeterministicRNG {
public:
    explicit DeterministicRNG(uint64_t seed) : state_(seed ? seed : 1) {}

    /** Generate the next unsigned 32-bit value. */
    uint32_t nextU32() {
        return static_cast<uint32_t>(advance() >> 32);
    }

    /** Uniform float in [0, 1). */
    float nextFloat() {
        constexpr float INV_2_24 = 1.0f / 16777216.0f;
        return (nextU32() >> 8) * INV_2_24;
    }

    /** Uniform integer in [min, max] (inclusive). */
    int range(int min, int max) {
        if (min >= max) return min;
        uint32_t span = static_cast<uint32_t>(max - min + 1);
        return min + static_cast<int>(nextU32() % span);
    }

    /** Uniform float in [min, max]. */
    float rangeFloat(float min, float max) {
        return min + nextFloat() * (max - min);
    }

    /** Boolean with given probability of being true. */
    bool chance(float probability) {
        return nextFloat() < probability;
    }

    /** Current internal state (for debug / serialisation). */
    uint64_t state() const { return state_; }

private:
    uint64_t state_;

    uint64_t advance() {
        state_ ^= state_ >> 12;
        state_ ^= state_ << 25;
        state_ ^= state_ >> 27;
        return state_ * 0x2545F4914F6CDD1DULL;
    }
};

} // namespace pcg
} // namespace atlas

#endif // EVE_PCG_DETERMINISTIC_RNG_H
