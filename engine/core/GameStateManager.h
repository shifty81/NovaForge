#pragma once
/**
 * @file GameStateManager.h
 * @brief Central game state tracking for the NovaForge vertical slice.
 *
 * Tracks the current game mode, player entity, star system, docking
 * status, credits, and provides a simple notification callback so
 * subsystems can react to state transitions.
 */

#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace atlas {

/** High-level game phase. */
enum class GamePhase : uint8_t {
    MainMenu,       ///< Title / character select screen
    Loading,        ///< Transition / loading screen
    InSpace,        ///< Player is flying in a star system
    Docked,         ///< Player is docked at a station
    InWarp,         ///< Player is warping between locations
    Paused          ///< Game is paused (single-player)
};

/** Returns a human-readable name for a GamePhase value. */
const char* GamePhaseName(GamePhase phase);

/**
 * Lightweight game state container.
 *
 * Designed to be owned by the RuntimeBootstrap or the top-level
 * Application object.  All fields are value types — no heap allocation
 * beyond std::string / std::vector.
 */
class GameStateManager {
public:
    // ── Phase management ────────────────────────────────────────────

    GamePhase Phase() const { return m_phase; }

    /**
     * Transition to a new phase.  If @p phase differs from the current
     * phase the on-transition callback fires.
     */
    void SetPhase(GamePhase phase);

    /**
     * Register a callback that fires on every phase transition.
     * The callback receives (oldPhase, newPhase).
     */
    using PhaseCallback = std::function<void(GamePhase, GamePhase)>;
    void OnPhaseChange(PhaseCallback cb);

    // ── Player entity ───────────────────────────────────────────────

    uint32_t PlayerEntity() const { return m_playerEntity; }
    void SetPlayerEntity(uint32_t id) { m_playerEntity = id; }

    // ── Star system ─────────────────────────────────────────────────

    uint32_t CurrentSystem() const { return m_currentSystem; }
    void SetCurrentSystem(uint32_t systemId) { m_currentSystem = systemId; }

    // ── Station / docking ───────────────────────────────────────────

    uint32_t DockedStation() const { return m_dockedStation; }
    void SetDockedStation(uint32_t stationId) { m_dockedStation = stationId; }

    // ── Credits / wallet ────────────────────────────────────────────

    int64_t Credits() const { return m_credits; }
    void SetCredits(int64_t credits) { m_credits = credits; }
    void AddCredits(int64_t amount) { m_credits += amount; }

    /** Returns true if the player can afford @p cost. */
    bool CanAfford(int64_t cost) const { return m_credits >= cost; }

    /**
     * Deduct @p cost from credits.
     * Returns true and deducts if affordable, false otherwise.
     */
    bool Spend(int64_t cost);

    // ── Inventory (simple item-id list) ─────────────────────────────

    const std::vector<uint32_t>& Inventory() const { return m_inventory; }
    void AddItem(uint32_t itemId);
    bool RemoveItem(uint32_t itemId);
    bool HasItem(uint32_t itemId) const;
    size_t InventorySize() const { return m_inventory.size(); }

    // ── Reset ───────────────────────────────────────────────────────

    /** Reset all state to defaults (e.g. new game). */
    void Reset();

private:
    GamePhase m_phase = GamePhase::MainMenu;
    uint32_t m_playerEntity = 0;
    uint32_t m_currentSystem = 0;
    uint32_t m_dockedStation = 0;
    int64_t  m_credits = 0;
    std::vector<uint32_t> m_inventory;
    std::vector<PhaseCallback> m_phaseCallbacks;
};

} // namespace atlas
