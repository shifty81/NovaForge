#ifndef NOVAFORGE_COMPONENTS_NAVIGATION_COMPONENTS_H
#define NOVAFORGE_COMPONENTS_NAVIGATION_COMPONENTS_H

#include "ecs/component.h"
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <cstdint>
#include <cmath>

namespace atlas {
namespace components {

/**
 * @brief Solar system properties for wormhole space
 *
 * Tracks the wormhole class (C1-C6), active system-wide effects,
 * and whether dormant NPCs have already been spawned.
 */
class SolarSystem : public ecs::Component {
public:
    std::string system_id;
    std::string system_name;
    int wormhole_class = 0;               // 0 = k-space, 1-6 = wormhole class
    std::string effect_name;              // e.g. "magnetar", "pulsar", "" for none
    bool dormants_spawned = false;
    
    COMPONENT_TYPE(SolarSystem)
};

/**
 * @brief A wormhole connection between two systems
 *
 * Models mass limits, remaining stability, and lifetime so that
 * the WormholeSystem can decay and eventually collapse connections.
 */
class WormholeConnection : public ecs::Component {
public:
    std::string wormhole_id;
    std::string source_system;            // system entity id
    std::string destination_system;       // system entity id
    double max_mass = 500000000.0;        // kg total mass allowed
    double remaining_mass = 500000000.0;  // kg remaining before collapse
    double max_jump_mass = 20000000.0;    // kg max single-ship mass
    float max_lifetime_hours = 24.0f;     // hours until natural collapse
    float elapsed_hours = 0.0f;           // hours elapsed since spawn
    bool collapsed = false;
    
    bool isStable() const {
        return !collapsed && elapsed_hours < max_lifetime_hours && remaining_mass > 0.0;
    }
    
    COMPONENT_TYPE(WormholeConnection)
};
/**
 * @brief Warp phase tracking (for warp anomaly system)
 */
class WarpState : public ecs::Component {
public:
    enum class WarpPhase { None, Align, Entry, Cruise, Event, Exit };

    WarpPhase phase = WarpPhase::None;
    float warp_time = 0.0f;
    float distance_remaining = 0.0f;
    float warp_speed = 3.0f;    // AU/s (initialized from Ship component)
    float mass_norm = 0.0f;     // 0=frigate, 1=capital
    float intensity = 0.0f;     // computed from time + mass
    int warp_disrupt_strength = 0;  // total disruption applied to this entity

    COMPONENT_TYPE(WarpState)
};
/**
 * @brief Warp tunnel visual layer configuration (cinematic warp system)
 *
 * Stores per-entity shader layer intensities computed by WarpCinematicSystem.
 * Client reads these to drive the multi-layer warp tunnel overlay.
 */
class WarpTunnelConfig : public ecs::Component {
public:
    // Shader layer intensities (0.0–1.0)
    float radial_distortion = 0.0f;    // Radial distortion layer
    float starfield_bloom   = 0.0f;    // Starfield velocity bloom
    float tunnel_skin       = 0.0f;    // Tunnel skin/noise layer
    float vignette          = 0.0f;    // Edge vignette darkening

    // Composite intensity derived from ship mass + phase
    float composite_intensity = 0.0f;

    COMPONENT_TYPE(WarpTunnelConfig)
};

/**
 * @brief Warp audio profile for adaptive warp sounds
 *
 * Drives three audio channels during warp: engine core (sub-bass),
 * warp field harmonics, and environmental shimmer.
 */
class WarpAudioProfile : public ecs::Component {
public:
    float engine_core_volume  = 0.0f;   // Sub-bass engine drone (0.0–1.0)
    float harmonics_volume    = 0.0f;   // Warp field harmonics (0.0–1.0)
    float shimmer_volume      = 0.0f;   // Environmental shimmer (0.0–1.0)
    float engine_core_pitch   = 1.0f;   // Pitch multiplier for engine core
    float harmonics_pitch     = 1.0f;   // Pitch multiplier for harmonics

    COMPONENT_TYPE(WarpAudioProfile)
};

/**
 * @brief Accessibility settings for warp visual/audio effects
 *
 * Allows players to reduce motion, bass, and blur intensity
 * to accommodate different sensitivities.
 */
class WarpAccessibility : public ecs::Component {
public:
    float motion_intensity = 1.0f;   // Motion effect scale (0.0–1.0, 0=off)
    float bass_intensity   = 1.0f;   // Sub-bass volume scale (0.0–1.0)
    float blur_intensity   = 1.0f;   // Blur/distortion scale (0.0–1.0)
    bool  tunnel_geometry_enabled = true;  // false = star streaks only, no full warp skin

    COMPONENT_TYPE(WarpAccessibility)
};

/**
 * @brief HUD travel mode during warp — softens edges, desaturates, adds padding
 *
 * During warp cruise the HUD transitions into a softer, less intrusive mode:
 * - Edges soften, bright colors desaturate, tactical warnings muted
 * - Safe-area padding pushes HUD inward (32–48 px)
 * - Optional UI flair: animated brackets, glow synced to engine bass, parallax
 */
class WarpHUDTravelMode : public ecs::Component {
public:
    // Soft edge treatment (0=normal, 1=fully softened)
    float edge_softness = 0.0f;

    // Color desaturation (0=normal, 1=fully desaturated)
    float color_desaturation = 0.0f;

    // Warning muting (0=normal, 1=fully muted tactical warnings)
    float warning_mute = 0.0f;

    // Safe-area padding in pixels (0=normal, target 32–48px during cruise)
    float safe_area_padding = 0.0f;

    // HUD scale factor (1.0=normal, 0.95=scaled inward during warp)
    float hud_scale = 1.0f;

    // Optional UI flair (player toggle)
    bool  ui_flair_enabled = false;
    float bracket_animation = 0.0f;     // animated bracket offset (0–1)
    float ui_glow_intensity = 0.0f;     // glow synced to engine bass (0–1)
    float hud_parallax_offset = 0.0f;   // subtle parallax shift (pixels)

    COMPONENT_TYPE(WarpHUDTravelMode)
};

/**
 * @brief Auto-comfort rules for warp visual effects
 *
 * Automatically reduces warp visual intensity when performance drops
 * or ultrawide aspect ratios are detected to prevent discomfort.
 */
class WarpAutoComfort : public ecs::Component {
public:
    float target_fps = 60.0f;              // Desired frame rate
    float current_fps = 60.0f;             // Measured frame rate
    float comfort_reduction = 0.0f;        // Auto-applied reduction (0=full effects, 1=minimum)
    bool  ultrawide_detected = false;      // True if aspect ratio > 2.2
    float max_distortion_ultrawide = 0.5f; // Clamp distortion on ultrawide displays

    COMPONENT_TYPE(WarpAutoComfort)
};
// ==================== Phase 8: Warp Cinematic Components ====================

class WarpProfile : public ecs::Component {
public:
    float warp_speed = 3.0f;        // AU/s
    float mass_norm = 0.0f;         // 0=frigate, 1=capital
    float intensity = 0.0f;         // composite visual/audio intensity
    float comfort_scale = 1.0f;     // accessibility scaling (0-1)

    COMPONENT_TYPE(WarpProfile)
};

class WarpVisual : public ecs::Component {
public:
    float distortion_strength = 0.0f;   // radial distortion amount
    float tunnel_noise_scale = 1.0f;    // procedural noise skin scale
    float vignette_amount = 0.0f;       // peripheral darkening
    float bloom_strength = 0.0f;        // velocity bloom intensity
    float starfield_speed = 1.0f;       // starfield streak multiplier

    COMPONENT_TYPE(WarpVisual)
};

class WarpEvent : public ecs::Component {
public:
    std::string current_event;          // active anomaly event id (empty = none)
    float event_timer = 0.0f;           // remaining duration of event
    int severity = 0;                   // 0=none, 1=visual, 2=sensory, 3=shear, 4=legendary

    COMPONENT_TYPE(WarpEvent)
};

// ==================== Phase 8: Warp Audio Enhancement Components ====================

class WarpMeditationLayer : public ecs::Component {
public:
    bool active = false;
    float fade_timer = 0.0f;
    float fade_duration = 5.0f;
    float volume = 0.0f;
    float activation_delay = 15.0f;
    float warp_cruise_time = 0.0f;

    COMPONENT_TYPE(WarpMeditationLayer)
};

class WarpAudioProgression : public ecs::Component {
public:
    enum class Phase { Tension, Stabilize, Bloom, Meditative };

    Phase current_phase = Phase::Tension;
    float phase_timer = 0.0f;
    float tension_duration = 3.0f;
    float stabilize_duration = 5.0f;
    float bloom_duration = 4.0f;
    float blend_factor = 0.0f;

    float computeOverallProgression() const {
        float total = tension_duration + stabilize_duration + bloom_duration;
        float elapsed = phase_timer;
        if (elapsed >= total) return 1.0f;
        if (elapsed <= 0.0f) return 0.0f;
        return elapsed / total;
    }

    COMPONENT_TYPE(WarpAudioProgression)
};

// ==================== Phase 8: Warp Performance Budget ====================

/**
 * @brief GPU performance budget tracking for warp visual layers
 *
 * Tracks per-layer GPU cost in milliseconds and enforces a total
 * budget (default ≤1.2 ms).  The system disables the most expensive
 * layers first and provides a scale_factor for shaders.
 *
 * Layers (index order):
 *   0 = radial distortion
 *   1 = starfield velocity bloom
 *   2 = tunnel skin / noise
 *   3 = vignette
 *   4 = ship silhouette anchor
 */
class WarpPerformanceBudget : public ecs::Component {
public:
    float layer_costs[5] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f};  // measured GPU ms per layer
    bool  layer_enabled[5] = {true, true, true, true, true};   // which layers are active
    float budget_ms = 1.2f;       // max allowed total GPU cost
    float total_cost_ms = 0.0f;   // last-frame total cost
    float scale_factor = 1.0f;    // 0–1 utilisation of budget (for shader LOD)

    COMPONENT_TYPE(WarpPerformanceBudget)
};

class JumpDriveState : public ecs::Component {
public:
    enum class JumpPhase { Idle, SpoolingUp, Jumping, Cooldown };
    enum class FuelType { Hydrogen, Helium, Nitrogen, Oxygen };

    JumpPhase phase = JumpPhase::Idle;
    FuelType fuel_type = FuelType::Hydrogen;
    float spool_time = 10.0f;           // seconds to spool up before jump
    float cooldown_time = 300.0f;       // 5 minutes cooldown after jump
    float phase_timer = 0.0f;           // progress in current phase
    float max_range_ly = 5.0f;          // max jump range in light years
    float fuel_per_ly = 500.0f;         // fuel consumed per light year jumped
    float current_fuel = 5000.0f;       // current fuel amount
    float max_fuel = 10000.0f;          // max fuel capacity
    float fatigue_hours = 0.0f;         // accumulated jump fatigue
    float fatigue_decay_rate = 0.1f;    // fatigue decay per real-second (for simulation)
    float fatigue_per_jump = 1.0f;      // fatigue added per jump
    float max_fatigue = 10.0f;          // max fatigue (at max, cannot jump)
    std::string cyno_target_id;         // entity with active cynosural field
    std::string destination_system;     // target system for jump
    float jump_distance_ly = 0.0f;     // distance of current/last jump
    int total_jumps = 0;                // total jumps performed
    bool requires_cyno = true;          // whether cyno field is required

    static std::string phaseToString(JumpPhase p) {
        switch (p) {
            case JumpPhase::Idle: return "idle";
            case JumpPhase::SpoolingUp: return "spooling_up";
            case JumpPhase::Jumping: return "jumping";
            case JumpPhase::Cooldown: return "cooldown";
            default: return "unknown";
        }
    }

    static std::string fuelTypeToString(FuelType f) {
        switch (f) {
            case FuelType::Hydrogen: return "hydrogen";
            case FuelType::Helium: return "helium";
            case FuelType::Nitrogen: return "nitrogen";
            case FuelType::Oxygen: return "oxygen";
            default: return "unknown";
        }
    }

    COMPONENT_TYPE(JumpDriveState)
};

// ==================== Space-Planet Transition System (Phase 14) ====================

/**
 * @brief Seamless zoom from orbit to surface with multi-phase transition state machine
 *
 * Tracks a multi-phase descent/launch sequence between space and a planetary surface,
 * including atmospheric heating, gravity changes, and autopilot control.
 */
class SpacePlanetTransition : public ecs::Component {
public:
    enum class TransitionState {
        InSpace,
        OrbitEntry,
        AtmosphereEntry,
        DescentPhase,
        LandingApproach,
        Landed,
        LaunchSequence,
        AtmosphereExit,
        OrbitExit
    };

    std::string entity_id;
    std::string planet_id;
    TransitionState transition_state = TransitionState::InSpace;
    float progress = 0.0f;              // 0-1 within current phase
    float altitude = 1000.0f;           // km, starts high
    float speed = 0.0f;
    float target_landing_x = 0.0f;
    float target_landing_y = 0.0f;
    float current_phase_time = 0.0f;
    float total_transition_time = 0.0f;
    bool has_atmosphere = false;
    float atmosphere_density = 0.0f;    // 0-1
    float heat_buildup = 0.0f;          // atmospheric entry heating
    float max_heat_tolerance = 100.0f;
    float hull_stress = 0.0f;
    float gravity_factor = 0.0f;
    bool is_autopilot = false;

    bool isInTransition() const {
        return transition_state != TransitionState::InSpace &&
               transition_state != TransitionState::Landed;
    }

    bool isLanded() const {
        return transition_state == TransitionState::Landed;
    }

    bool isInSpace() const {
        return transition_state == TransitionState::InSpace;
    }

    static std::string stateToString(TransitionState s) {
        switch (s) {
            case TransitionState::InSpace: return "in_space";
            case TransitionState::OrbitEntry: return "orbit_entry";
            case TransitionState::AtmosphereEntry: return "atmosphere_entry";
            case TransitionState::DescentPhase: return "descent_phase";
            case TransitionState::LandingApproach: return "landing_approach";
            case TransitionState::Landed: return "landed";
            case TransitionState::LaunchSequence: return "launch_sequence";
            case TransitionState::AtmosphereExit: return "atmosphere_exit";
            case TransitionState::OrbitExit: return "orbit_exit";
            default: return "unknown";
        }
    }

    static float getGravityForAltitude(float alt) {
        return 1.0f / (1.0f + alt * 0.001f);
    }

    COMPONENT_TYPE(SpacePlanetTransition)
};

/**
 * @brief Player-saved location bookmarks for quick navigation
 *
 * Stores named waypoints with coordinates, categories, and favorites.
 */
class NavigationBookmark : public ecs::Component {
public:
    struct Bookmark {
        std::string bookmark_id;
        std::string label;
        std::string system_id;
        float x = 0.0f;
        float y = 0.0f;
        float z = 0.0f;
        std::string category = "Personal"; // Personal, Corp, Shared
        std::string notes;
        float created_at = 0.0f;
        bool is_favorite = false;
    };

    std::vector<Bookmark> bookmarks;
    int max_bookmarks = 100;
    int total_created = 0;
    bool active = true;

    COMPONENT_TYPE(NavigationBookmark)
};

// ==================== Autopilot System ====================

class Autopilot : public ecs::Component {
public:
    struct Waypoint {
        std::string waypoint_id;
        std::string label;
        float x = 0.0f;
        float y = 0.0f;
        float z = 0.0f;
        bool reached = false;
    };

    std::string owner_id;
    std::vector<Waypoint> waypoints;
    int current_waypoint_index = 0;
    int max_waypoints = 50;
    float speed = 100.0f;        // m/s
    float arrival_distance = 50.0f;  // meters to consider "arrived"
    float distance_to_next = 0.0f;
    float total_distance_traveled = 0.0f;
    int waypoints_reached = 0;
    bool engaged = false;
    bool loop = false;           // loop back to first waypoint
    bool active = true;

    COMPONENT_TYPE(Autopilot)
};

// ==================== Jump Gate System ====================

class JumpGate : public ecs::Component {
public:
    struct Gate {
        std::string gate_id;
        std::string destination_system;
        std::string destination_gate_id;
        float activation_time = 10.0f;
        float cooldown_time = 30.0f;
        float fuel_cost = 50.0f;
        float security_level = 1.0f;
        bool online = true;
        bool in_use = false;
        float current_cooldown = 0.0f;
        float activation_progress = 0.0f;
        int total_jumps = 0;
    };

    std::string system_id;
    std::vector<Gate> gates;
    int max_gates = 10;
    int total_jumps_processed = 0;
    int total_activations = 0;
    bool active = true;

    COMPONENT_TYPE(JumpGate)
};

/**
 * @brief Network jitter buffer for smooth client interpolation
 *
 * Tracks packet timing, computes jitter metrics, and maintains an
 * adaptive buffer size for smooth networked entity interpolation.
 * Supports the Network Smoothness priority.
 */
class JitterBuffer : public ecs::Component {
public:
    struct PacketSample {
        int sequence = 0;
        float arrival_time = 0.0f;
        float expected_time = 0.0f;
        float jitter = 0.0f;           // |arrival - expected|
    };

    std::vector<PacketSample> samples;
    int max_samples = 100;
    float buffer_size_ms = 50.0f;       // current adaptive buffer size in ms
    float min_buffer_ms = 20.0f;
    float max_buffer_ms = 200.0f;
    float average_jitter = 0.0f;
    float peak_jitter = 0.0f;
    float adaptation_rate = 0.1f;       // how fast buffer adapts (0-1)
    int total_packets = 0;
    int lost_packets = 0;
    int underruns = 0;                  // buffer too small events
    int overruns = 0;                   // buffer too large events
    float elapsed = 0.0f;
    int last_sequence = 0;
    bool active = true;

    COMPONENT_TYPE(JitterBuffer)
};

// ==================== Undock Sequence ====================

/**
 * @brief Manages the multi-phase undock animation sequence from stations
 *
 * Tracks phase progression from Docked through HangarExit, TunnelTraversal,
 * ExitAnimation to Ejected/Complete. Provides post-undock invulnerability window.
 */
class UndockSequence : public ecs::Component {
public:
    enum UndockPhase {
        Docked = 0,
        RequestingUndock = 1,
        HangarExit = 2,
        TunnelTraversal = 3,
        ExitAnimation = 4,
        Ejected = 5,
        Complete = 6
    };

    UndockPhase phase = Docked;
    float phase_progress = 0.0f;     // 0.0 to 1.0 within current phase
    float phase_speed = 0.5f;        // progress per second
    std::string station_id;
    float exit_x = 0.0f;
    float exit_y = 0.0f;
    float exit_z = 0.0f;
    float exit_velocity = 50.0f;
    float undock_timer = 0.0f;
    float total_undock_time = 0.0f;
    float alignment_angle = 0.0f;
    bool is_invulnerable = false;
    float invulnerability_duration = 30.0f;
    float invulnerability_timer = 0.0f;
    int undock_count = 0;
    bool active = true;

    COMPONENT_TYPE(UndockSequence)
};

// ==================== System Map ====================

/**
 * @brief In-system map data for celestials, bookmarks, and signatures
 *
 * Manages the objects visible on the system map including celestial bodies,
 * player bookmarks, and scanned signatures with distance calculations.
 */
class SystemMap : public ecs::Component {
public:
    struct Celestial {
        std::string celestial_id;
        std::string name;
        std::string type;       // "Star", "Planet", "Moon", "Station", "Gate", "Belt"
        float x = 0.0f;
        float y = 0.0f;
        float z = 0.0f;
        float radius = 0.0f;
        bool visible = true;
    };

    struct Bookmark {
        std::string bookmark_id;
        std::string label;
        std::string folder;
        float x = 0.0f;
        float y = 0.0f;
        float z = 0.0f;
        float created_at = 0.0f;
    };

    struct Signature {
        std::string sig_id;
        std::string type;       // "Anomaly", "Wormhole", "Data", "Relic", "Gas", "Combat"
        float scan_strength = 0.0f;  // 0.0 - 1.0
        float x = 0.0f;
        float y = 0.0f;
        float z = 0.0f;
        bool resolved = false;
    };

    std::string system_name;
    float security_level = 1.0f;
    std::vector<Celestial> celestials;
    std::vector<Bookmark> bookmarks;
    std::vector<Signature> signatures;
    int max_celestials = 50;
    int max_bookmarks = 100;
    int max_signatures = 30;
    int total_bookmarks_created = 0;
    int total_signatures_scanned = 0;
    float elapsed = 0.0f;
    bool active = true;

    COMPONENT_TYPE(SystemMap)
};

// ==================== Warp Disruption ====================

/**
 * @brief Warp disruption/scramble state for combat interdiction
 *
 * Tracks active warp disruptors/scramblers applied to this entity.
 * Total strength vs warp core strength determines if warp is blocked.
 */
class WarpDisruption : public ecs::Component {
public:
    struct Disruptor {
        std::string source_id;
        int strength = 1;          // disruption points (1 = disruptor, 2 = scrambler)
        float range = 24000.0f;    // effective range in meters
        float duration = 0.0f;     // elapsed seconds
        bool active = true;
    };

    std::vector<Disruptor> disruptors;
    int total_disruption_strength = 0;
    int warp_core_strength = 1;    // base warp core strength (from Ship component)
    bool warp_blocked = false;
    int max_disruptors = 10;
    int total_disruptions_applied = 0;
    int total_escapes = 0;
    float elapsed = 0.0f;
    bool component_active = true;

    COMPONENT_TYPE(WarpDisruption)
};

// ==================== Invulnerability Timer ====================

/**
 * @brief Temporary invulnerability after undocking, spawning, or jump
 *
 * Provides a timed immunity window where the entity cannot take damage.
 * Breaks on movement, module activation, or timer expiry.
 */
class InvulnerabilityTimer : public ecs::Component {
public:
    enum class Reason { Undock, Spawn, JumpIn, Resurrection };

    Reason reason = Reason::Undock;
    float duration = 30.0f;        // total invulnerability seconds
    float remaining = 0.0f;        // seconds left
    bool invulnerable = false;
    bool broken_by_action = false;  // set true if broken early
    int total_invulns_granted = 0;
    int total_broken_early = 0;
    float elapsed = 0.0f;
    bool active = true;

    COMPONENT_TYPE(InvulnerabilityTimer)
};

// ==================== NPC Patrol Route ====================

/**
 * @brief Waypoint-based patrol route for security and pirate NPCs
 *
 * Defines a sequence of patrol waypoints with idle times and alert
 * radii.  The system advances the NPC through waypoints, handles idle
 * pauses, detects hostiles within the alert radius, and optionally
 * loops the route.
 */
class NPCPatrolRoute : public ecs::Component {
public:
    enum class PatrolState { Idle, Travelling, Waiting, Alert, Complete };

    struct PatrolWaypoint {
        std::string waypoint_id;
        float x = 0.0f;
        float y = 0.0f;
        float z = 0.0f;
        float idle_time = 5.0f;       // seconds to wait at waypoint
        float alert_radius = 1000.0f; // metres — trigger alert if hostile within
    };

    std::vector<PatrolWaypoint> waypoints;
    PatrolState state = PatrolState::Idle;
    int current_waypoint_index = 0;
    float idle_timer = 0.0f;
    float travel_speed = 100.0f;      // m/s
    float distance_to_next = 0.0f;
    bool loop = true;                 // restart route after last waypoint
    bool hostile_detected = false;
    std::string detected_hostile_id;
    int max_waypoints = 20;
    int total_patrols_completed = 0;
    int total_alerts_triggered = 0;
    int waypoints_visited = 0;
    float elapsed = 0.0f;
    bool active = true;

    COMPONENT_TYPE(NPCPatrolRoute)
};

// ==================== System Traffic Monitor ====================

/**
 * @brief Monitors NPC and player traffic in a star system
 *
 * Tracks entity counts by category (player ships, NPC traders, NPC
 * miners, NPC pirates, NPC security), computes density, detects
 * congestion, and provides traffic snapshots for situational awareness.
 */
class SystemTrafficMonitor : public ecs::Component {
public:
    enum class TrafficCategory { PlayerShip, NPCTrader, NPCMiner, NPCPirate, NPCSecurity };

    struct TrafficEntry {
        std::string entity_id;
        TrafficCategory category = TrafficCategory::PlayerShip;
        float time_in_system = 0.0f;
    };

    std::vector<TrafficEntry> entries;
    std::string system_id;
    int player_count = 0;
    int npc_trader_count = 0;
    int npc_miner_count = 0;
    int npc_pirate_count = 0;
    int npc_security_count = 0;
    float congestion_threshold = 50.0f;  // entity count above which congestion triggers
    bool congested = false;
    float snapshot_interval = 30.0f;     // seconds between traffic snapshots
    float snapshot_timer = 0.0f;
    int total_snapshots = 0;
    int total_entities_tracked = 0;
    int max_entries = 200;
    float elapsed = 0.0f;
    bool active = true;

    COMPONENT_TYPE(SystemTrafficMonitor)
};

/**
 * @brief Safe zone around stations where weapons are disabled
 *
 * Defines a protection perimeter around a station or structure.
 * Entities inside the zone cannot activate weapons and receive a
 * tethering speed bonus.  The zone can be toggled on/off and
 * tracks how many entities are currently sheltered.
 */
class SafeZone : public ecs::Component {
public:
    enum class ZoneState { Disabled, Active, Reinforced };

    std::string zone_id;
    std::string station_id;
    ZoneState state = ZoneState::Active;
    float radius = 5000.0f;              // metres from station centre
    float tether_speed_bonus = 0.5f;     // 50 % speed boost while tethered
    int entities_inside = 0;
    int max_entities = 200;
    bool weapons_disabled = true;
    int total_entries = 0;
    int total_exits = 0;
    int total_weapons_blocked = 0;
    float elapsed = 0.0f;
    bool active = true;

    COMPONENT_TYPE(SafeZone)
};

/**
 * @brief Navigation beacon for waypoints and fleet warp destinations
 *
 * Placed in star systems to provide navigational reference points.
 * Beacons can be public (visible to all) or private (owner only).
 * Fleet commanders use beacons as fleet warp targets.  Beacons
 * degrade over time and require maintenance to stay online.
 */
class NavigationBeacon : public ecs::Component {
public:
    enum class BeaconType { Waypoint, FleetWarp, Emergency, Survey };
    enum class BeaconState { Online, Degraded, Offline, Destroyed };

    std::string beacon_id;
    std::string owner_id;
    std::string system_id;
    std::string label;
    BeaconType type = BeaconType::Waypoint;
    BeaconState state = BeaconState::Online;
    double x = 0.0, y = 0.0, z = 0.0;
    float signal_strength = 1.0f;      // 0.0–1.0, degrades over time
    float degradation_rate = 0.0001f;  // per second
    float scan_range = 1000.0f;        // detection range
    bool is_public = true;
    int total_warps_to = 0;            // times used as warp destination
    int total_scans = 0;               // times scanned
    float elapsed = 0.0f;
    bool active = true;

    COMPONENT_TYPE(NavigationBeacon)
};

// ==================== Docking Request ====================

class DockingRequest : public ecs::Component {
public:
    enum class Phase {
        Idle,       // No docking in progress
        Approach,   // Moving toward station
        Requested,  // Permission sent
        Granted,    // Station accepted; tether extending
        Docked      // Fully docked
    };

    std::string station_id;
    Phase phase = Phase::Idle;
    float approach_distance = 0.0f;
    float docking_range = 2500.0f;     // meters — must be within this range to request
    float tether_progress = 0.0f;       // 0..1 — fills while granted
    float tether_speed = 0.5f;          // fraction per second
    int total_dockings = 0;
    int denied_count = 0;
    float elapsed = 0.0f;
    bool active = true;

    COMPONENT_TYPE(DockingRequest)
};

// ==================== Ship Approach / Orbit ====================

/**
 * @brief Ship navigation command state for approach, orbit, and keep-at-range
 *
 * Holds the current navigation command (approach / orbit / keep_at_range /
 * none), target entity, distance tracking, and orbit angle.  Only one
 * command is active at a time — issuing a new command replaces the old one.
 */
class ApproachOrbitState : public ecs::Component {
public:
    std::string command_type = "none";   // "none", "approach", "orbit", "keep_at_range"
    std::string target_id;
    float current_distance = 0.0f;      // meters to target
    float desired_distance = 0.0f;      // meters (orbit radius, keep range, 0 for approach)
    float max_speed = 100.0f;           // m/s
    float current_speed = 0.0f;         // m/s
    float orbit_angle = 0.0f;           // degrees (0-360)
    float elapsed = 0.0f;
    bool active = true;

    COMPONENT_TYPE(ApproachOrbitState)
};

/**
 * @brief Warp drive charge-up state with alignment gating and disruption
 *
 * Charge time = base_charge_time × mass_factor.  Charging only progresses
 * while the ship is aligned.  A disruption resets progress.  After a
 * successful warp, the drive enters a cooldown period (default 10s).
 */
class WarpChargeState : public ecs::Component {
public:
    float base_charge_time = 5.0f;  // seconds
    float mass_factor = 1.0f;
    float charge_progress = 0.0f;   // 0.0–1.0
    float cooldown_duration = 10.0f;
    float cooldown_remaining = 0.0f;
    std::string destination_id;
    bool charging = false;
    bool aligned = false;
    int total_warps_completed = 0;
    int total_disruptions = 0;
    float elapsed = 0.0f;
    bool active = true;

    COMPONENT_TYPE(WarpChargeState)
};

/**
 * @brief Hyperspace route state for multi-system navigation
 *
 * Tracks calculated routes, waypoints, and route analytics for
 * hyperspace navigation between star systems via jump gates.
 */
class HyperspaceRoute : public ecs::Component {
public:
    struct Waypoint {
        std::string system_id;
        std::string gate_id;
        float estimated_travel_time = 0.0f;
        bool visited = false;
    };

    std::string origin_system;
    std::string destination_system;
    std::vector<Waypoint> waypoints;
    int current_waypoint_index = -1;
    float total_estimated_time = 0.0f;
    float elapsed_travel_time = 0.0f;
    int total_routes_calculated = 0;
    int total_jumps_completed = 0;
    bool route_active = false;
    bool route_valid = true;
    float elapsed = 0.0f;
    bool active = true;

    int remainingJumps() const {
        if (current_waypoint_index < 0) return static_cast<int>(waypoints.size());
        return std::max(0, static_cast<int>(waypoints.size()) - current_waypoint_index - 1);
    }

    COMPONENT_TYPE(HyperspaceRoute)
};

class JumpGateState : public ecs::Component {
public:
    std::string gate_id;
    std::string destination_system;
    std::string destination_gate_id;
    float charge_time = 10.0f;         // seconds to fully charge
    float current_charge = 0.0f;       // 0.0-1.0 charge progress
    float cooldown_time = 30.0f;       // seconds before gate can be reused
    float current_cooldown = 0.0f;     // remaining cooldown
    float fuel_cost = 50.0f;           // ISC fuel cost per jump
    int total_jumps = 0;
    int max_queue = 5;                 // max ships queued for jump
    int current_queue = 0;
    float elapsed = 0.0f;
    bool active = true;
    bool charging = false;
    bool on_cooldown = false;

    bool isReady() const { return !charging && !on_cooldown && active; }

    COMPONENT_TYPE(JumpGateState)
};

/**
 * @brief Asteroid scanner state for revealing ore composition
 *
 * Tracks scan progress, resolution, and discovered ore types for
 * an asteroid entity. Higher scan resolution reveals rarer ore types.
 */
class AsteroidScannerState : public ecs::Component {
public:
    struct OreReading {
        std::string ore_type;       // "Veldspar", "Scordite", "Pyroxeres", etc.
        float concentration = 0.0f; // 0.0-1.0
        float estimated_value = 0.0f;
    };

    std::string target_asteroid_id;
    float scan_duration = 5.0f;       // seconds for full scan
    float scan_progress = 0.0f;       // 0.0-1.0
    float scan_resolution = 1.0f;     // 1.0 = standard, 2.0 = deep scan
    std::vector<OreReading> readings;
    int max_readings = 10;
    int total_scans_completed = 0;
    float total_value_scanned = 0.0f;
    float elapsed = 0.0f;
    bool active = true;
    bool scanning = false;
    bool scan_complete = false;

    bool isScanning() const { return scanning && !scan_complete; }

    COMPONENT_TYPE(AsteroidScannerState)
};

/**
 * @brief Per-connection network quality metrics
 *
 * Tracks latency, jitter, and packet loss for a player connection so the
 * server can adapt snapshot rate and interpolation delay.  Updated each
 * time a measurement sample arrives.
 */
class NetworkQualityState : public ecs::Component {
public:
    std::string connection_id;
    float latency_ms = 0.0f;          // smoothed round-trip time
    float jitter_ms = 0.0f;           // variance in RTT
    float packet_loss_pct = 0.0f;     // 0-100
    float snapshot_rate_hz = 20.0f;   // adaptive snapshot send rate
    int samples_received = 0;
    int packets_lost = 0;
    int packets_total = 0;
    float min_latency_ms = 999.0f;
    float max_latency_ms = 0.0f;
    float elapsed = 0.0f;
    bool active = true;
    bool degraded = false;            // true when quality drops below threshold

    COMPONENT_TYPE(NetworkQualityState)
};

} // namespace components
} // namespace atlas

#endif // NOVAFORGE_COMPONENTS_NAVIGATION_COMPONENTS_H
