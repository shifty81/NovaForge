#pragma once
/**
 * @file snap_align_tool.h
 * @brief Concrete ITool for grid snapping, surface snapping, and alignment.
 *
 * SnapAlignTool provides precision placement helpers for the editor.
 * Grid-snap rounds positions to a configurable cell size, surface-snap
 * projects onto a target plane, and alignment distributes entities
 * along an axis.  All placement changes are posted to the
 * UndoableCommandBus and recorded in the DeltaEditStore.
 */

#include "editor/itool.h"
#include "editor/undoable_command_bus.h"
#include "../../engine/ecs/DeltaEditStore.h"
#include <cmath>
#include <string>
#include <utility>
#include <vector>

namespace atlas::editor {

/**
 * Undoable command: snap an entity's position to the nearest grid cell.
 */
class SnapToGridCommand : public IUndoableCommand {
public:
    SnapToGridCommand(atlas::ecs::DeltaEditStore& store,
                      uint32_t entityID,
                      float oldX, float oldY, float oldZ,
                      float gridSize)
        : m_store(store), m_entityID(entityID),
          m_oldX(oldX), m_oldY(oldY), m_oldZ(oldZ),
          m_gridSize(gridSize) {
        m_newX = std::round(oldX / gridSize) * gridSize;
        m_newY = std::round(oldY / gridSize) * gridSize;
        m_newZ = std::round(oldZ / gridSize) * gridSize;
    }

    void Execute() override {
        atlas::ecs::DeltaEdit edit{};
        edit.type        = atlas::ecs::DeltaEditType::MoveObject;
        edit.entityID    = m_entityID;
        edit.position[0] = m_newX;
        edit.position[1] = m_newY;
        edit.position[2] = m_newZ;
        m_store.Record(edit);
    }

    void Undo() override {
        atlas::ecs::DeltaEdit edit{};
        edit.type        = atlas::ecs::DeltaEditType::MoveObject;
        edit.entityID    = m_entityID;
        edit.position[0] = m_oldX;
        edit.position[1] = m_oldY;
        edit.position[2] = m_oldZ;
        m_store.Record(edit);
    }

    const char* Description() const override { return "Snap To Grid"; }

    float SnappedX() const { return m_newX; }
    float SnappedY() const { return m_newY; }
    float SnappedZ() const { return m_newZ; }

private:
    atlas::ecs::DeltaEditStore& m_store;
    uint32_t m_entityID;
    float m_oldX, m_oldY, m_oldZ;
    float m_newX, m_newY, m_newZ;
    float m_gridSize;
};

/**
 * Undoable command: snap an entity to a surface (project onto a plane).
 */
class SnapToSurfaceCommand : public IUndoableCommand {
public:
    SnapToSurfaceCommand(atlas::ecs::DeltaEditStore& store,
                         uint32_t entityID,
                         float oldX, float oldY, float oldZ,
                         float surfaceY)
        : m_store(store), m_entityID(entityID),
          m_oldX(oldX), m_oldY(oldY), m_oldZ(oldZ),
          m_surfaceY(surfaceY) {}

    void Execute() override {
        atlas::ecs::DeltaEdit edit{};
        edit.type        = atlas::ecs::DeltaEditType::MoveObject;
        edit.entityID    = m_entityID;
        edit.position[0] = m_oldX;
        edit.position[1] = m_surfaceY;
        edit.position[2] = m_oldZ;
        m_store.Record(edit);
    }

    void Undo() override {
        atlas::ecs::DeltaEdit edit{};
        edit.type        = atlas::ecs::DeltaEditType::MoveObject;
        edit.entityID    = m_entityID;
        edit.position[0] = m_oldX;
        edit.position[1] = m_oldY;
        edit.position[2] = m_oldZ;
        m_store.Record(edit);
    }

    const char* Description() const override { return "Snap To Surface"; }

private:
    atlas::ecs::DeltaEditStore& m_store;
    uint32_t m_entityID;
    float m_oldX, m_oldY, m_oldZ;
    float m_surfaceY;
};

/**
 * Undoable command: align a set of entities along an axis.
 *
 * Distributes entities evenly between the first and last positions
 * along the specified axis (0 = X, 1 = Y, 2 = Z), keeping the other
 * axes unchanged.
 */
class AlignEntitiesCommand : public IUndoableCommand {
public:
    struct EntityPos {
        uint32_t entityID;
        float x, y, z;
    };

    AlignEntitiesCommand(atlas::ecs::DeltaEditStore& store,
                         const std::vector<EntityPos>& entities,
                         int axis,
                         float alignValue)
        : m_store(store), m_entities(entities),
          m_axis(axis), m_alignValue(alignValue) {}

    void Execute() override {
        for (const auto& e : m_entities) {
            atlas::ecs::DeltaEdit edit{};
            edit.type        = atlas::ecs::DeltaEditType::MoveObject;
            edit.entityID    = e.entityID;
            edit.position[0] = (m_axis == 0) ? m_alignValue : e.x;
            edit.position[1] = (m_axis == 1) ? m_alignValue : e.y;
            edit.position[2] = (m_axis == 2) ? m_alignValue : e.z;
            m_store.Record(edit);
        }
    }

    void Undo() override {
        for (const auto& e : m_entities) {
            atlas::ecs::DeltaEdit edit{};
            edit.type        = atlas::ecs::DeltaEditType::MoveObject;
            edit.entityID    = e.entityID;
            edit.position[0] = e.x;
            edit.position[1] = e.y;
            edit.position[2] = e.z;
            m_store.Record(edit);
        }
    }

    const char* Description() const override { return "Align Entities"; }

private:
    atlas::ecs::DeltaEditStore& m_store;
    std::vector<EntityPos> m_entities;
    int m_axis;
    float m_alignValue;
};

/**
 * SnapAlignTool — concrete ITool for precision placement.
 *
 * Provides helpers that create undoable commands for grid snapping,
 * surface snapping, and multi-entity alignment.  All changes are
 * recorded in the DeltaEditStore for persistence on top of the PCG seed.
 */
class SnapAlignTool : public ITool {
public:
    SnapAlignTool(UndoableCommandBus& bus,
                  atlas::ecs::DeltaEditStore& store,
                  float gridSize = 1.0f)
        : m_bus(bus), m_store(store), m_gridSize(gridSize) {}

    // ── ITool interface ─────────────────────────────────────────────
    const char* Name() const override { return "Snap & Align"; }

    void Activate() override   { m_active = true; }
    void Deactivate() override { m_active = false; }
    void Update(float /*dt*/) override {}
    bool IsActive() const override { return m_active; }

    // ── Configuration ───────────────────────────────────────────────

    float GridSize() const { return m_gridSize; }
    void  SetGridSize(float size) { if (size > 0.0f) m_gridSize = size; }

    // ── Helpers that create and post commands ────────────────────────

    void SnapToGrid(uint32_t entityID,
                    float curX, float curY, float curZ) {
        m_bus.PostCommand(std::make_unique<SnapToGridCommand>(
            m_store, entityID, curX, curY, curZ, m_gridSize));
    }

    void SnapToSurface(uint32_t entityID,
                       float curX, float curY, float curZ,
                       float surfaceY) {
        m_bus.PostCommand(std::make_unique<SnapToSurfaceCommand>(
            m_store, entityID, curX, curY, curZ, surfaceY));
    }

    void AlignOnAxis(const std::vector<AlignEntitiesCommand::EntityPos>& entities,
                     int axis,
                     float alignValue) {
        m_bus.PostCommand(std::make_unique<AlignEntitiesCommand>(
            m_store, entities, axis, alignValue));
    }

private:
    UndoableCommandBus& m_bus;
    atlas::ecs::DeltaEditStore& m_store;
    float m_gridSize;
    bool m_active = false;
};

} // namespace atlas::editor
