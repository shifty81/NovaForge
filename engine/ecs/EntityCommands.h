#pragma once
/**
 * @file EntityCommands.h
 * @brief Concrete ICommand implementations that bridge EditorCommandBus to
 *        the ECS World, with undo/redo support via UndoStack.
 *
 * These commands allow the editor tooling layer to safely create, destroy,
 * and modify entities in a decoupled, undoable fashion.
 */

#include "ECS.h"
#include "../cpp_client/include/editor/editor_command_bus.h"
#include <any>
#include <functional>
#include <string>
#include <typeindex>
#include <unordered_map>

namespace atlas::ecs {

/**
 * Create a new entity in the World.
 *
 * On Execute: creates the entity and stores the ID.
 * The optional onCreated callback receives the new EntityID.
 */
class CreateEntityCommand : public atlas::editor::ICommand {
public:
    explicit CreateEntityCommand(World& world,
                                 std::function<void(EntityID)> onCreated = nullptr)
        : m_world(world), m_onCreated(std::move(onCreated)) {}

    void Execute() override {
        m_createdID = m_world.CreateEntity();
        if (m_onCreated) m_onCreated(m_createdID);
    }

    const char* Description() const override { return "Create Entity"; }

    /** The entity ID that was created (valid after Execute). */
    EntityID CreatedID() const { return m_createdID; }

private:
    World& m_world;
    std::function<void(EntityID)> m_onCreated;
    EntityID m_createdID = 0;
};

/**
 * Destroy an entity in the World.
 *
 * On Execute: destroys the entity.
 */
class DestroyEntityCommand : public atlas::editor::ICommand {
public:
    DestroyEntityCommand(World& world, EntityID id)
        : m_world(world), m_entityID(id) {}

    void Execute() override {
        m_wasAlive = m_world.IsAlive(m_entityID);
        if (m_wasAlive) {
            m_world.DestroyEntity(m_entityID);
        }
    }

    const char* Description() const override { return "Destroy Entity"; }

    /** Whether the entity was alive before Execute. */
    bool WasAlive() const { return m_wasAlive; }

    EntityID TargetID() const { return m_entityID; }

private:
    World& m_world;
    EntityID m_entityID;
    bool m_wasAlive = false;
};

/**
 * Typed helper to set a component on an entity.
 *
 * On Execute: adds/replaces the component.
 * Stores the previous value (if any) so callers can wire undo.
 */
template<typename T>
class SetComponentCommand : public atlas::editor::ICommand {
public:
    SetComponentCommand(World& world, EntityID id, const T& value)
        : m_world(world), m_entityID(id), m_newValue(value) {}

    void Execute() override {
        T* existing = m_world.GetComponent<T>(m_entityID);
        if (existing) {
            m_hadPrevious = true;
            m_previousValue = *existing;
        }
        m_world.AddComponent<T>(m_entityID, m_newValue);
    }

    const char* Description() const override { return "Set Component"; }

    bool HadPrevious() const { return m_hadPrevious; }
    const T& PreviousValue() const { return m_previousValue; }
    EntityID TargetID() const { return m_entityID; }

private:
    World& m_world;
    EntityID m_entityID;
    T m_newValue;
    T m_previousValue{};
    bool m_hadPrevious = false;
};

/**
 * Typed helper to remove a component from an entity.
 *
 * On Execute: removes the component if present.
 * Stores the previous value (if any) so callers can wire undo.
 */
template<typename T>
class RemoveComponentCommand : public atlas::editor::ICommand {
public:
    RemoveComponentCommand(World& world, EntityID id)
        : m_world(world), m_entityID(id) {}

    void Execute() override {
        T* existing = m_world.GetComponent<T>(m_entityID);
        if (existing) {
            m_hadPrevious = true;
            m_previousValue = *existing;
            m_world.RemoveComponent<T>(m_entityID);
        }
    }

    const char* Description() const override { return "Remove Component"; }

    bool HadPrevious() const { return m_hadPrevious; }
    const T& PreviousValue() const { return m_previousValue; }
    EntityID TargetID() const { return m_entityID; }

private:
    World& m_world;
    EntityID m_entityID;
    T m_previousValue{};
    bool m_hadPrevious = false;
};

} // namespace atlas::ecs
