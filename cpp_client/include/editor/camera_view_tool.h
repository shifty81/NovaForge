#pragma once
/**
 * @file camera_view_tool.h
 * @brief Concrete ITool for editor camera view management.
 *
 * CameraViewTool lets the designer switch between free-fly, orbit, and
 * orthographic camera views for precise asset placement and scene
 * inspection.  Every camera configuration change is posted to the
 * UndoableCommandBus and recorded in the DeltaEditStore so camera
 * bookmarks can persist on top of the PCG seed.
 *
 * Camera properties are stored as SetProperty edits with the property
 * name prefixed by "camera:" to distinguish them from ordinary
 * properties.
 */

#include "editor/itool.h"
#include "editor/undoable_command_bus.h"
#include "../../engine/ecs/DeltaEditStore.h"
#include <string>
#include <utility>
#include <vector>

namespace atlas::editor {

/** Supported camera view modes. */
enum class CameraViewMode {
    FreeFly,
    Orbit,
    Orthographic
};

inline const char* CameraViewModeName(CameraViewMode mode) {
    switch (mode) {
        case CameraViewMode::FreeFly:       return "FreeFly";
        case CameraViewMode::Orbit:         return "Orbit";
        case CameraViewMode::Orthographic:  return "Orthographic";
    }
    return "Unknown";
}

/**
 * Undoable command: change the camera view mode.
 */
class SetCameraViewModeCommand : public IUndoableCommand {
public:
    SetCameraViewModeCommand(atlas::ecs::DeltaEditStore& store,
                             uint32_t entityID,
                             CameraViewMode oldMode,
                             CameraViewMode newMode)
        : m_store(store), m_entityID(entityID),
          m_oldMode(oldMode), m_newMode(newMode) {}

    void Execute() override {
        atlas::ecs::DeltaEdit edit{};
        edit.type          = atlas::ecs::DeltaEditType::SetProperty;
        edit.entityID      = m_entityID;
        edit.propertyName  = "camera:view_mode";
        edit.propertyValue = CameraViewModeName(m_newMode);
        m_store.Record(edit);
    }

    void Undo() override {
        atlas::ecs::DeltaEdit edit{};
        edit.type          = atlas::ecs::DeltaEditType::SetProperty;
        edit.entityID      = m_entityID;
        edit.propertyName  = "camera:view_mode";
        edit.propertyValue = CameraViewModeName(m_oldMode);
        m_store.Record(edit);
    }

    const char* Description() const override { return "Set Camera View Mode"; }

private:
    atlas::ecs::DeltaEditStore& m_store;
    uint32_t       m_entityID;
    CameraViewMode m_oldMode;
    CameraViewMode m_newMode;
};

/**
 * Undoable command: set a single camera property (e.g. position, target, fov).
 */
class SetCameraPropertyCommand : public IUndoableCommand {
public:
    SetCameraPropertyCommand(atlas::ecs::DeltaEditStore& store,
                             uint32_t entityID,
                             const std::string& propertyName,
                             const std::string& oldValue,
                             const std::string& newValue)
        : m_store(store), m_entityID(entityID),
          m_propertyName(propertyName),
          m_oldValue(oldValue), m_newValue(newValue) {}

    void Execute() override {
        atlas::ecs::DeltaEdit edit{};
        edit.type          = atlas::ecs::DeltaEditType::SetProperty;
        edit.entityID      = m_entityID;
        edit.propertyName  = "camera:" + m_propertyName;
        edit.propertyValue = m_newValue;
        m_store.Record(edit);
    }

    void Undo() override {
        atlas::ecs::DeltaEdit edit{};
        edit.type          = atlas::ecs::DeltaEditType::SetProperty;
        edit.entityID      = m_entityID;
        edit.propertyName  = "camera:" + m_propertyName;
        edit.propertyValue = m_oldValue;
        m_store.Record(edit);
    }

    const char* Description() const override { return "Set Camera Property"; }

private:
    atlas::ecs::DeltaEditStore& m_store;
    uint32_t    m_entityID;
    std::string m_propertyName;
    std::string m_oldValue;
    std::string m_newValue;
};

/**
 * Undoable command: save a camera bookmark (position + target + mode).
 */
class SaveCameraBookmarkCommand : public IUndoableCommand {
public:
    using PropertyList = std::vector<std::pair<std::string, std::string>>;

    SaveCameraBookmarkCommand(atlas::ecs::DeltaEditStore& store,
                              uint32_t entityID,
                              const std::string& bookmarkName,
                              const PropertyList& oldProps,
                              const PropertyList& newProps)
        : m_store(store), m_entityID(entityID),
          m_bookmarkName(bookmarkName),
          m_oldProps(oldProps), m_newProps(newProps) {}

    void Execute() override {
        for (const auto& [name, value] : m_newProps) {
            atlas::ecs::DeltaEdit edit{};
            edit.type          = atlas::ecs::DeltaEditType::SetProperty;
            edit.entityID      = m_entityID;
            edit.propertyName  = "camera:bookmark:" + m_bookmarkName + ":" + name;
            edit.propertyValue = value;
            m_store.Record(edit);
        }
    }

    void Undo() override {
        for (const auto& [name, value] : m_oldProps) {
            atlas::ecs::DeltaEdit edit{};
            edit.type          = atlas::ecs::DeltaEditType::SetProperty;
            edit.entityID      = m_entityID;
            edit.propertyName  = "camera:bookmark:" + m_bookmarkName + ":" + name;
            edit.propertyValue = value;
            m_store.Record(edit);
        }
    }

    const char* Description() const override { return "Save Camera Bookmark"; }

private:
    atlas::ecs::DeltaEditStore& m_store;
    uint32_t    m_entityID;
    std::string m_bookmarkName;
    PropertyList m_oldProps;
    PropertyList m_newProps;
};

/**
 * CameraViewTool — concrete ITool for editor camera view management.
 *
 * Provides helpers that create undoable commands for switching camera
 * modes (free-fly, orbit, orthographic), adjusting camera properties
 * (position, target, FOV, near/far planes), and saving/restoring
 * camera bookmarks.  All changes are recorded in the DeltaEditStore
 * for persistence on top of the PCG seed.
 */
class CameraViewTool : public ITool {
public:
    CameraViewTool(UndoableCommandBus& bus,
                   atlas::ecs::DeltaEditStore& store)
        : m_bus(bus), m_store(store) {}

    // ── ITool interface ─────────────────────────────────────────────
    const char* Name() const override { return "Camera View"; }

    void Activate() override   { m_active = true; }
    void Deactivate() override { m_active = false; }
    void Update(float /*dt*/) override {}
    bool IsActive() const override { return m_active; }

    // ── Helpers that create and post commands ────────────────────────

    void SetViewMode(uint32_t entityID,
                     CameraViewMode oldMode,
                     CameraViewMode newMode) {
        m_bus.PostCommand(std::make_unique<SetCameraViewModeCommand>(
            m_store, entityID, oldMode, newMode));
    }

    void SetProperty(uint32_t entityID,
                     const std::string& propName,
                     const std::string& oldVal,
                     const std::string& newVal) {
        m_bus.PostCommand(std::make_unique<SetCameraPropertyCommand>(
            m_store, entityID, propName, oldVal, newVal));
    }

    void SaveBookmark(uint32_t entityID,
                      const std::string& bookmarkName,
                      const std::vector<std::pair<std::string, std::string>>& oldProps,
                      const std::vector<std::pair<std::string, std::string>>& newProps) {
        m_bus.PostCommand(std::make_unique<SaveCameraBookmarkCommand>(
            m_store, entityID, bookmarkName, oldProps, newProps));
    }

private:
    UndoableCommandBus& m_bus;
    atlas::ecs::DeltaEditStore& m_store;
    bool m_active = false;
};

} // namespace atlas::editor
