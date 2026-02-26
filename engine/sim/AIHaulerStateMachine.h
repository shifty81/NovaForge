#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace atlas::sim {

/**
 * States for an autonomous AI hauler entity.
 */
enum class HaulerState : uint8_t {
    Idle,              // Waiting for work
    FindCargo,         // Looking for cargo to haul
    TravelToPickup,    // Flying to pickup location
    LoadCargo,         // Loading cargo from source
    TravelToStation,   // Flying to sell destination
    UnloadCargo,       // Unloading / selling at station
    Dead               // Destroyed
};

/**
 * Configuration for a single AI hauler.
 */
struct HaulerConfig {
    uint32_t entityId         = 0;
    float    cargoCapacity    = 1000.0f;  // m³
    float    travelSpeed      = 80.0f;    // units/s
    float    loadRate         = 50.0f;    // m³/s loading speed
    float    sellPricePerUnit = 12.0f;    // credits per m³ (markup over miner)
    float    respawnDelay     = 90.0f;    // seconds
};

/**
 * A pickup request from a source (e.g. a miner or container).
 */
struct CargoRequest {
    uint32_t sourceId  = 0;      // entity offering cargo
    float    available = 0.0f;   // m³ available for pickup
    float    distance  = 0.0f;   // units to source
};

/**
 * Runtime state for one AI hauler.
 */
struct HaulerRuntime {
    HaulerConfig config;
    HaulerState  state           = HaulerState::Idle;
    float        cargoFill       = 0.0f;       // current m³
    float        stateTimer      = 0.0f;
    float        totalEarnings   = 0.0f;
    float        travelProgress  = 0.0f;       // 0-1
    uint32_t     targetSourceId  = 0;           // pickup source
    uint32_t     tripsCompleted  = 0;
    float        respawnTimer    = 0.0f;
};

/**
 * @brief Autonomous AI hauler state machine.
 *
 * Manages multiple hauler NPCs that pick up cargo from sources
 * (miners, containers, stations) and deliver to sell locations.
 *
 *   Idle → FindCargo → TravelToPickup → LoadCargo →
 *   TravelToStation → UnloadCargo → (repeat)
 *
 * Usage:
 *   AIHaulerStateMachine sm;
 *   sm.AddHauler({1, 1000.0f, 80.0f, 50.0f, 12.0f, 90.0f});
 *   sm.SetCargoRequests({{100, 200.0f, 300.0f}});
 *   sm.Tick(dt);
 */
class AIHaulerStateMachine {
public:
    AIHaulerStateMachine() = default;

    /** Add a new hauler. Returns false if entityId already tracked. */
    bool AddHauler(const HaulerConfig& config);

    /** Remove a hauler by entity id. */
    bool RemoveHauler(uint32_t entityId);

    /** Set available cargo pickup requests. */
    void SetCargoRequests(const std::vector<CargoRequest>& requests);

    /** Advance all haulers by deltaSeconds. */
    void Tick(float deltaSeconds);

    /** Force-kill a hauler. */
    bool KillHauler(uint32_t entityId);

    /** Get read-only runtime state. Returns nullptr if not found. */
    const HaulerRuntime* GetHauler(uint32_t entityId) const;

    /** Number of tracked haulers. */
    size_t HaulerCount() const { return m_haulers.size(); }

    /** Number of haulers in a given state. */
    size_t CountInState(HaulerState state) const;

    /** Aggregate lifetime earnings across all haulers. */
    float TotalEarnings() const;

    /** Aggregate completed delivery trips. */
    uint32_t TotalTrips() const;

    /** Station distance for sell-trip travel time. */
    void SetStationDistance(float distance) { m_stationDistance = distance; }
    float StationDistance() const { return m_stationDistance; }

private:
    void tickHauler(HaulerRuntime& h, float dt);
    const CargoRequest* pickBestCargo() const;

    std::vector<HaulerRuntime> m_haulers;
    std::vector<CargoRequest>  m_cargoRequests;
    float                      m_stationDistance = 400.0f;
};

} // namespace atlas::sim
