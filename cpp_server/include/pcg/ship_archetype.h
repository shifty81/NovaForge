#ifndef EVE_PCG_SHIP_ARCHETYPE_H
#define EVE_PCG_SHIP_ARCHETYPE_H

#include "pcg_context.h"
#include "pcg_manager.h"
#include "ship_generator.h"
#include "interior_generator.h"
#include "pcg_asset_style.h"
#include "deterministic_rng.h"
#include "hash_utils.h"
#include <cstdint>
#include <string>
#include <vector>

namespace atlas {
namespace pcg {

// ═══════════════════════════════════════════════════════════════════
//  Door placement — connections between interior rooms
// ═══════════════════════════════════════════════════════════════════

/**
 * @brief A designer-placed door that connects two rooms.
 *
 * Doors are authored in the editor on the archetype's interior
 * layout.  PCG respects locked doors verbatim and may add additional
 * doors to ensure connectivity.
 */
struct DoorPlacement {
    uint32_t doorId;
    int      fromRoomId;
    int      toRoomId;
    float    posX, posY, posZ;      ///< Position on the hull boundary.
    float    width;                  ///< Door width (metres).
    float    height;                 ///< Door height (metres).
    bool     isAirlock;              ///< true = exterior / EVA door.
    bool     locked;                 ///< If true, PCG will not move this door.
};

// ═══════════════════════════════════════════════════════════════════
//  Turret hardpoint — weapon mount position on the hull
// ═══════════════════════════════════════════════════════════════════

/**
 * @brief A specific weapon mount point authored on the archetype hull.
 *
 * Hardpoints define where weapons can be fitted.  The ship's final
 * turret count and sizes come from the archetype; PCG may add minor
 * positional variation but honours the designer's intent.
 */
struct HardpointDefinition {
    uint32_t   hardpointId;
    float      posX, posY, posZ;    ///< Position on hull (local space).
    float      facingDeg;           ///< Nominal facing (0 = forward).
    float      arcDeg;              ///< Total fire arc (degrees).
    WeaponSize size;                ///< Small / Medium / Large / XLarge.
    bool       isDorsal;            ///< true = top mount, false = ventral.
    std::string groupTag;           ///< e.g. "fore", "aft", "broadside".
};

// ═══════════════════════════════════════════════════════════════════
//  Subsystem slots — T3-style modular regions that change geometry
// ═══════════════════════════════════════════════════════════════════

/**
 * @brief Subsystem categories (inspired by EVE T3 Strategic Cruisers).
 *
 * Each subsystem type controls a different region of the ship's
 * hull geometry and stat profile.
 */
enum class SubsystemType : uint32_t {
    Offensive    = 0,   ///< Weapon mounts, fire control.
    Defensive    = 1,   ///< Shield emitters, armor plating.
    Propulsion   = 2,   ///< Engine nacelles, warp drives.
    Engineering  = 3,   ///< Reactor, capacitor, utility.
    Core         = 4,   ///< Bridge, sensor array, hull spine.
};

static constexpr int SUBSYSTEM_TYPE_COUNT = 5;

/**
 * @brief A visual variant for one subsystem slot.
 *
 * When the player fits a particular subsystem module, the ship
 * geometry in that region morphs to match.  Each variant carries
 * a shape modifier (geometry deformation) and stat bonuses.
 */
struct SubsystemVariant {
    std::string    name;           ///< e.g. "Assault Batteries"
    SubsystemType  type;
    ShapeProfile   shapeModifier;  ///< Geometry deformation when active.
    float          hullBonus;      ///< % hull HP modifier.
    float          shieldBonus;    ///< % shield HP modifier.
    float          speedBonus;     ///< % speed modifier.
    float          dpsBonus;       ///< % DPS modifier.
};

/**
 * @brief A subsystem slot on the ship archetype.
 *
 * Each slot has a position/radius defining the hull region it
 * affects, and a list of variants (the different subsystem modules
 * that can be fitted there).
 */
struct SubsystemSlot {
    SubsystemType  type;
    float          regionPosX, regionPosY, regionPosZ;
    float          regionRadius;   ///< Influence radius on hull.
    std::vector<SubsystemVariant> variants;
    int            activeVariant;  ///< Currently selected (-1 = none).
};

// ═══════════════════════════════════════════════════════════════════
//  Module visual rules — how fitting changes appearance
// ═══════════════════════════════════════════════════════════════════

/**
 * @brief Defines how a fitted module category modifies the ship's look.
 *
 * When the player fits a shield module, an emitter geometry appears;
 * fitting armor adds plating; fitting large weapons adds barrel
 * geometry at the appropriate hardpoint.
 */
struct ModuleVisualRule {
    std::string moduleCategory;    ///< "shield", "armor", "weapon", "engine"
    std::string effectType;        ///< "emitter", "plating", "barrel", "nacelle"
    float       scaleMin;          ///< Min visual scale (relative to module size).
    float       scaleMax;          ///< Max visual scale.
    float       posX, posY, posZ;  ///< Offset from hull attachment point.
};

// ═══════════════════════════════════════════════════════════════════
//  Ship archetype — the complete designer-authored template
// ═══════════════════════════════════════════════════════════════════

/**
 * @brief A complete ship archetype for one hull class.
 *
 * The designer creates one archetype per hull class in the editor.
 * It defines the reference hull shape, interior layout, door
 * locations, turret hardpoints, subsystem slots, module visual rules,
 * and the bounds within which PCG may vary the output.
 *
 * All ships generated from this archetype will share the same
 * "family resemblance" while varying within the allowed bounds.
 * In-game, the ship's appearance further morphs based on the
 * player's fitting (like EVE T3 ships — every ship in this game
 * works this way).
 */
struct ShipArchetype {
    std::string          name;
    HullClass            hullClass;
    uint32_t             version;

    // ── Hull shape (designer reference geometry) ────────────────
    ShapeProfile         hullShape;

    // ── Interior template ───────────────────────────────────────
    std::vector<InteriorRoom>    rooms;
    std::vector<DoorPlacement>   doors;
    int                          deckCount;

    // ── Hardpoints ──────────────────────────────────────────────
    std::vector<HardpointDefinition> hardpoints;

    // ── Subsystem slots (T3-style) ──────────────────────────────
    std::vector<SubsystemSlot>   subsystems;

    // ── Module visual rules ─────────────────────────────────────
    std::vector<ModuleVisualRule> moduleVisuals;

    // ── PCG variation bounds ────────────────────────────────────
    float shapeVariation;    ///< [0,1] How much PCG can deviate shape.
    float sizeVariation;     ///< [0,1] Size range multiplier.
    float detailVariation;   ///< [0,1] Greeble/detail variation.

    bool valid;
};

// ═══════════════════════════════════════════════════════════════════
//  Generated variant — the output of archetype-based generation
// ═══════════════════════════════════════════════════════════════════

/**
 * @brief Result of generating a ship from an archetype + seed.
 *
 * Contains the base generated ship, the interior, the actual
 * hardpoints (after PCG variation), and the active subsystem
 * configuration.
 */
struct ArchetypeVariant {
    std::string         archetypeName;
    GeneratedShip       ship;
    GeneratedInterior   interior;
    std::vector<HardpointDefinition> hardpoints;
    std::vector<DoorPlacement>       doors;
    std::vector<int>    activeSubsystems;  ///< Index per subsystem slot.

    int  variationsApplied;
    bool valid;
};

// ═══════════════════════════════════════════════════════════════════
//  Ship Archetype Engine — generation and subsystem application
// ═══════════════════════════════════════════════════════════════════

/**
 * @brief Engine for archetype-based ship generation.
 *
 * Workflow:
 *   1. Designer creates ShipArchetype in the editor for each hull class.
 *   2. At generation time, the engine starts from the archetype and
 *      applies seeded variations within the allowed bounds.
 *   3. In-game, fitting changes trigger subsystem/visual updates via
 *      applySubsystems() and applyModuleVisuals().
 *
 * Same archetype + same seed → identical ship, always.
 */
class ShipArchetypeEngine {
public:
    /// Create a default archetype for the given hull class.
    static ShipArchetype createDefault(HullClass hull);

    /// Validate an archetype (sets archetype.valid).
    static bool validate(ShipArchetype& archetype);

    /// Generate a ship variant from an archetype using PCG.
    static ArchetypeVariant generateFromArchetype(
        const PCGContext& ctx, const ShipArchetype& archetype);

    /// Apply a specific subsystem configuration.
    static void applySubsystems(
        ArchetypeVariant& variant,
        const ShipArchetype& archetype,
        const std::vector<int>& activeVariants);

    /// Apply module fitting visual changes.
    static void applyModuleVisuals(
        ArchetypeVariant& variant,
        const ShipArchetype& archetype,
        const std::vector<std::string>& fittedModules);

    /// Serialise an archetype.
    static std::string serialize(const ShipArchetype& archetype);

    /// Deserialise an archetype.
    static ShipArchetype deserialize(const std::string& data);

    /// Human-readable subsystem type name.
    static const char* subsystemTypeName(SubsystemType type);

private:
    /// Apply seeded shape variation to hardpoints.
    static void varyHardpoints(
        DeterministicRNG& rng,
        std::vector<HardpointDefinition>& hardpoints,
        float variation);

    /// Apply seeded shape variation to doors.
    static void varyDoors(
        DeterministicRNG& rng,
        std::vector<DoorPlacement>& doors,
        float variation);

    /// Apply seeded ship stat variation.
    static void varyShipStats(
        DeterministicRNG& rng,
        GeneratedShip& ship,
        float sizeVariation);
};

} // namespace pcg
} // namespace atlas

#endif // EVE_PCG_SHIP_ARCHETYPE_H
