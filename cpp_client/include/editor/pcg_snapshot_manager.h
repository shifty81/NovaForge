#pragma once
/**
 * @file pcg_snapshot_manager.h
 * @brief Concrete ITool for capturing and restoring PCG state snapshots.
 *
 * PCGSnapshotManager lets the editor save named snapshots of the
 * DeltaEditStore (seed + edits serialized to JSON) and roll back to
 * any previous snapshot.  Every modification is posted to the
 * UndoableCommandBus so that snapshot operations integrate with the
 * global undo/redo history.
 */

#include "editor/itool.h"
#include "editor/undoable_command_bus.h"
#include "../../engine/ecs/DeltaEditStore.h"
#include <string>
#include <vector>
#include <cstdint>
#include <algorithm>

namespace atlas::editor {

/**
 * Named point-in-time capture of DeltaEditStore state.
 */
struct PCGSnapshot {
    std::string name;
    std::string json;       ///< Serialized DeltaEditStore state.
    uint64_t    timestamp = 0;
};

// Forward declaration so commands can reference the manager.
class PCGSnapshotManager;

/**
 * Undoable command: take a snapshot of the current DeltaEditStore state.
 */
class TakeSnapshotCommand : public IUndoableCommand {
public:
    TakeSnapshotCommand(PCGSnapshotManager& manager,
                        atlas::ecs::DeltaEditStore& store,
                        const std::string& name,
                        uint64_t timestamp)
        : m_manager(manager), m_store(store),
          m_name(name), m_timestamp(timestamp) {}

    void Execute() override;
    void Undo() override;

    const char* Description() const override { return "Take Snapshot"; }

private:
    PCGSnapshotManager& m_manager;
    atlas::ecs::DeltaEditStore& m_store;
    std::string m_name;
    uint64_t    m_timestamp;
};

/**
 * Undoable command: restore a previously saved snapshot.
 *
 * Execute saves the current state as a pre-restore backup, then
 * deserializes the target snapshot into the DeltaEditStore.
 * Undo restores the pre-restore backup.
 */
class RestoreSnapshotCommand : public IUndoableCommand {
public:
    RestoreSnapshotCommand(PCGSnapshotManager& manager,
                           atlas::ecs::DeltaEditStore& store,
                           const std::string& name)
        : m_manager(manager), m_store(store), m_name(name) {}

    void Execute() override;
    void Undo() override;

    const char* Description() const override { return "Restore Snapshot"; }

private:
    PCGSnapshotManager& m_manager;
    atlas::ecs::DeltaEditStore& m_store;
    std::string m_name;
    std::string m_backupJson;   ///< State captured before the restore.
    uint64_t    m_backupSeed = 0;
};

/**
 * PCGSnapshotManager — concrete ITool for PCG snapshot/rollback.
 *
 * Provides helpers that create undoable commands and post them to a
 * command bus, while managing a list of named snapshots internally.
 */
class PCGSnapshotManager : public ITool {
public:
    PCGSnapshotManager(UndoableCommandBus& bus,
                       atlas::ecs::DeltaEditStore& store)
        : m_bus(bus), m_store(store) {}

    // ── ITool interface ─────────────────────────────────────────────
    const char* Name() const override { return "PCG Snapshot"; }

    void Activate() override   { m_active = true; }
    void Deactivate() override { m_active = false; }
    void Update(float /*dt*/) override {}
    bool IsActive() const override { return m_active; }

    // ── Snapshot helpers (post undoable commands) ────────────────────

    /** Capture the current DeltaEditStore state as a named snapshot. */
    void TakeSnapshot(const std::string& name) {
        m_bus.PostCommand(std::make_unique<TakeSnapshotCommand>(
            *this, m_store, name, NextTimestamp()));
    }

    /** Restore a previously saved snapshot by name. */
    void RestoreSnapshot(const std::string& name) {
        m_bus.PostCommand(std::make_unique<RestoreSnapshotCommand>(
            *this, m_store, name));
    }

    /** Remove a snapshot by name. Returns true if it was found. */
    bool RemoveSnapshot(const std::string& name) {
        auto it = std::find_if(m_snapshots.begin(), m_snapshots.end(),
                               [&](const PCGSnapshot& s) { return s.name == name; });
        if (it == m_snapshots.end())
            return false;
        m_snapshots.erase(it);
        return true;
    }

    /** Check whether a snapshot with the given name exists. */
    bool HasSnapshot(const std::string& name) const {
        return std::any_of(m_snapshots.begin(), m_snapshots.end(),
                           [&](const PCGSnapshot& s) { return s.name == name; });
    }

    /** Return a list of all snapshot names. */
    std::vector<std::string> SnapshotNames() const {
        std::vector<std::string> names;
        names.reserve(m_snapshots.size());
        for (const auto& s : m_snapshots)
            names.push_back(s.name);
        return names;
    }

    /** Return how many snapshots are stored. */
    size_t SnapshotCount() const { return m_snapshots.size(); }

    /** Look up a snapshot by name. Returns nullptr if not found. */
    const PCGSnapshot* GetSnapshot(const std::string& name) const {
        auto it = std::find_if(m_snapshots.begin(), m_snapshots.end(),
                               [&](const PCGSnapshot& s) { return s.name == name; });
        return (it != m_snapshots.end()) ? &(*it) : nullptr;
    }

private:
    friend class TakeSnapshotCommand;
    friend class RestoreSnapshotCommand;

    /** Append a snapshot to the internal list. */
    void AddSnapshotEntry(const PCGSnapshot& snapshot) {
        m_snapshots.push_back(snapshot);
    }

    /** Monotonically increasing timestamp counter. */
    uint64_t NextTimestamp() { return m_nextTimestamp++; }

    UndoableCommandBus& m_bus;
    atlas::ecs::DeltaEditStore& m_store;
    std::vector<PCGSnapshot> m_snapshots;
    uint64_t m_nextTimestamp = 1;
    bool m_active = false;
};

// ── Inline command implementations (need full PCGSnapshotManager def) ───

inline void TakeSnapshotCommand::Execute() {
    PCGSnapshot snap;
    snap.name      = m_name;
    snap.json      = m_store.SerializeToJSON();
    snap.timestamp = m_timestamp;
    m_manager.AddSnapshotEntry(snap);
}

inline void TakeSnapshotCommand::Undo() {
    m_manager.RemoveSnapshot(m_name);
}

inline void RestoreSnapshotCommand::Execute() {
    // Save current state so Undo can revert.
    m_backupJson = m_store.SerializeToJSON();
    m_backupSeed = m_store.Seed();

    const PCGSnapshot* snap = m_manager.GetSnapshot(m_name);
    if (snap)
        m_store.DeserializeFromJSON(snap->json);
}

inline void RestoreSnapshotCommand::Undo() {
    m_store.DeserializeFromJSON(m_backupJson);
    m_store.SetSeed(m_backupSeed);
}

} // namespace atlas::editor
