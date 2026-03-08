#ifndef NOVAFORGE_PCG_GENERATION_STYLE_H
#define NOVAFORGE_PCG_GENERATION_STYLE_H

#include "pcg_context.h"
#include "pcg_manager.h"
#include "ship_generator.h"
#include "station_generator.h"
#include "interior_generator.h"
#include "star_system_generator.h"
#include "deterministic_rng.h"
#include "hash_utils.h"
#include <cstdint>
#include <string>
#include <vector>

namespace atlas {
namespace pcg {

// ── Generation style type (what domain this style targets) ──────────

enum class GenerationStyleType : uint32_t {
    ShipLayout       = 0,
    StationLayout    = 1,
    InteriorLayout   = 2,
    StarSystem       = 3,
    AsteroidField    = 4,
    FleetComposition = 5,
};

static constexpr int GENERATION_STYLE_TYPE_COUNT = 6;

// ── Placement entry — a single designer-authored position ───────────

struct PlacementEntry {
    uint32_t    slotIndex;               ///< Grid/slot position index.
    float       posX, posY, posZ;        ///< 3D position override.
    uint32_t    contentType;             ///< Domain-specific content ID.
    std::string label;                   ///< Human-readable label.
    bool        locked;                  ///< If true, PCG will not move/modify this.
};

// ── Parameter override — designer-tunable generation knob ───────────

struct ParameterOverride {
    std::string name;                    ///< Parameter name (e.g. "moduleCount").
    float       value;                   ///< Current value.
    float       minValue;                ///< Allowable minimum.
    float       maxValue;                ///< Allowable maximum.
    bool        enabled;                 ///< Whether this override is active.
};

// ── Complete generation style (designer blueprint) ──────────────────

/**
 * @brief A designer-authored generation blueprint.
 *
 * Captures the designer's intent for how PCG should produce content.
 * Placements pin specific items at specific positions; parameters
 * override default generator knobs.  The PCG engine fills in anything
 * the designer leaves unspecified, producing a hybrid of hand-crafted
 * and procedural content.
 *
 * Same style + same seed → identical output, always.
 */
struct GenerationStyle {
    std::string          name;
    GenerationStyleType  type;
    uint32_t             version;
    uint64_t             baseSeed;

    std::vector<PlacementEntry>    placements;
    std::vector<ParameterOverride> parameters;

    bool valid;  ///< Set by validate().
};

// ── Asteroid field generation result ────────────────────────────────

struct GeneratedAsteroid {
    uint32_t    asteroidId;
    float       posX, posY, posZ;
    float       radius;        ///< Metres.
    uint32_t    oreType;       ///< 0=veldspar … 7=arkonor.
    float       richness;      ///< Yield multiplier.
};

struct GeneratedAsteroidField {
    uint64_t    fieldId;
    std::string fieldName;
    float       centerX, centerY, centerZ;
    float       fieldRadius;
    std::vector<GeneratedAsteroid> asteroids;
    int         clusterCount;
    bool        valid;
};

// ── Fleet composition generation result ─────────────────────────────

struct GeneratedFleetShip {
    uint32_t    shipId;
    HullClass   hullClass;
    std::string role;          ///< "dps", "logistics", "ewar", "tackle", "command".
    std::string shipName;
};

struct GeneratedFleetCompositionResult {
    uint64_t    fleetId;
    std::string doctrineName;
    std::vector<GeneratedFleetShip> ships;
    int         capitalCount;
    int         subcapCount;
    float       aggressionRating;
    bool        valid;
};

// ── Result of style-driven generation ───────────────────────────────

struct StyleGenerationResult {
    GenerationStyleType sourceType;
    std::string         styleName;
    bool                success;
    std::string         errorMessage;

    // Domain-specific results (populated based on sourceType).
    GeneratedShip                  shipResult;
    GeneratedStation               stationResult;
    GeneratedInterior              interiorResult;
    GeneratedStarSystem            starSystemResult;
    GeneratedAsteroidField         asteroidFieldResult;
    GeneratedFleetCompositionResult fleetResult;

    int placementsApplied;   ///< How many designer placements were honoured.
    int parametersApplied;   ///< How many parameter overrides took effect.
};

// ── Public API ──────────────────────────────────────────────────────

/**
 * @brief Engine that generates content from designer-created styles.
 *
 * Workflow:
 *   1. Designer creates a GenerationStyle in the editor.
 *   2. Adds PlacementEntry items (e.g. "Power module at slot 0").
 *   3. Tweaks ParameterOverride values (e.g. "moduleCount = 8").
 *   4. Calls generate() — PCG fills in everything else.
 *   5. Serialises the style to disk for reuse.
 */
class GenerationStyleEngine {
public:
    /// Create a default style with sensible parameters for the given type.
    static GenerationStyle createDefaultStyle(GenerationStyleType type,
                                              const std::string& name = "");

    /// Validate a style (sets style.valid).
    static bool validate(GenerationStyle& style);

    /// Apply designer placements and generate content.
    static StyleGenerationResult generate(const PCGContext& ctx,
                                          const GenerationStyle& style);

    /// Serialise a style to a text string (simple key=value format).
    static std::string serialize(const GenerationStyle& style);

    /// Deserialise a style from a text string.
    static GenerationStyle deserialize(const std::string& data);

    /// List the available parameter names for a style type.
    static std::vector<std::string> availableParameters(
        GenerationStyleType type);

    /// Human-readable style type name.
    static const char* styleTypeName(GenerationStyleType type);

    /// Look up a parameter override by name (nullptr if not found).
    static const ParameterOverride* findParameter(
        const GenerationStyle& style, const std::string& name);

private:
    static StyleGenerationResult generateShipLayout(
        const PCGContext& ctx, const GenerationStyle& style);
    static StyleGenerationResult generateStationLayout(
        const PCGContext& ctx, const GenerationStyle& style);
    static StyleGenerationResult generateInteriorLayout(
        const PCGContext& ctx, const GenerationStyle& style);
    static StyleGenerationResult generateStarSystem(
        const PCGContext& ctx, const GenerationStyle& style);
    static StyleGenerationResult generateAsteroidField(
        const PCGContext& ctx, const GenerationStyle& style);
    static StyleGenerationResult generateFleetComposition(
        const PCGContext& ctx, const GenerationStyle& style);

    /// Apply locked placements to a station (pin modules at positions).
    static int applyStationPlacements(
        GeneratedStation& station,
        const std::vector<PlacementEntry>& placements);
};

} // namespace pcg
} // namespace atlas

#endif // NOVAFORGE_PCG_GENERATION_STYLE_H
