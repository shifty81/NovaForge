#pragma once
/**
 * @file layer_tag_system.h
 * @brief Layer and tag management for categorising scene objects.
 *
 * LayerTagSystem lets the designer organise entities into named layers
 * (e.g. "Ships", "Props", "Hangar", "Characters") and attach arbitrary
 * tags for quick filtering.  Layers can be toggled visible/invisible so
 * the editor can reduce clutter during complex scene editing.
 *
 * This is purely editor-side state — it does not alter runtime
 * simulation or the DeltaEditStore.
 */

#include <algorithm>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace atlas::editor {

/**
 * A named layer that holds entity IDs and a visibility flag.
 */
struct Layer {
    std::string name;
    bool visible = true;
    std::unordered_set<uint32_t> entities;
};

/**
 * LayerTagSystem — manages layers and per-entity tags.
 *
 * Usage:
 *   LayerTagSystem lts;
 *   lts.CreateLayer("Ships");
 *   lts.AssignLayer(42, "Ships");
 *   lts.AddTag(42, "capital");
 *   lts.SetLayerVisible("Ships", false);
 *   auto visible = lts.VisibleEntities(); // excludes entity 42
 */
class LayerTagSystem {
public:
    // ── Layer operations ────────────────────────────────────────────

    /** Create a new named layer.  Returns false if it already exists. */
    bool CreateLayer(const std::string& name) {
        for (const auto& l : m_layers)
            if (l.name == name) return false;
        m_layers.push_back({name, true, {}});
        return true;
    }

    /** Remove a layer by name.  Returns false if not found. */
    bool RemoveLayer(const std::string& name) {
        auto it = std::find_if(m_layers.begin(), m_layers.end(),
                               [&](const Layer& l) { return l.name == name; });
        if (it == m_layers.end()) return false;
        m_layers.erase(it);
        return true;
    }

    /** Number of layers. */
    size_t LayerCount() const { return m_layers.size(); }

    /** Get a layer by name.  Returns nullptr if not found. */
    const Layer* GetLayer(const std::string& name) const {
        for (const auto& l : m_layers)
            if (l.name == name) return &l;
        return nullptr;
    }

    /** Set visibility of a layer.  Returns false if not found. */
    bool SetLayerVisible(const std::string& name, bool visible) {
        for (auto& l : m_layers) {
            if (l.name == name) {
                l.visible = visible;
                return true;
            }
        }
        return false;
    }

    /** Assign an entity to a layer.  Creates layer if it doesn't exist. */
    void AssignLayer(uint32_t entityID, const std::string& layerName) {
        for (auto& l : m_layers) {
            if (l.name == layerName) {
                l.entities.insert(entityID);
                return;
            }
        }
        Layer newLayer;
        newLayer.name = layerName;
        newLayer.entities.insert(entityID);
        m_layers.push_back(std::move(newLayer));
    }

    /** Remove an entity from a specific layer.  Returns false if not found. */
    bool RemoveFromLayer(uint32_t entityID, const std::string& layerName) {
        for (auto& l : m_layers) {
            if (l.name == layerName)
                return l.entities.erase(entityID) > 0;
        }
        return false;
    }

    /** Collect all entity IDs that belong to visible layers. */
    std::vector<uint32_t> VisibleEntities() const {
        std::unordered_set<uint32_t> result;
        for (const auto& l : m_layers)
            if (l.visible)
                result.insert(l.entities.begin(), l.entities.end());
        return {result.begin(), result.end()};
    }

    // ── Tag operations ──────────────────────────────────────────────

    /** Add a tag to an entity. */
    void AddTag(uint32_t entityID, const std::string& tag) {
        m_tags[entityID].insert(tag);
    }

    /** Remove a tag from an entity.  Returns false if tag was not present. */
    bool RemoveTag(uint32_t entityID, const std::string& tag) {
        auto it = m_tags.find(entityID);
        if (it == m_tags.end()) return false;
        return it->second.erase(tag) > 0;
    }

    /** Check whether an entity has a specific tag. */
    bool HasTag(uint32_t entityID, const std::string& tag) const {
        auto it = m_tags.find(entityID);
        if (it == m_tags.end()) return false;
        return it->second.count(tag) > 0;
    }

    /** Get all tags for an entity. */
    std::vector<std::string> GetTags(uint32_t entityID) const {
        auto it = m_tags.find(entityID);
        if (it == m_tags.end()) return {};
        return {it->second.begin(), it->second.end()};
    }

    /** Find all entities that have a given tag. */
    std::vector<uint32_t> EntitiesWithTag(const std::string& tag) const {
        std::vector<uint32_t> result;
        for (const auto& [id, tags] : m_tags)
            if (tags.count(tag) > 0)
                result.push_back(id);
        return result;
    }

    /** Total number of entities that have at least one tag. */
    size_t TaggedEntityCount() const { return m_tags.size(); }

    /** Clear all layers and tags. */
    void Clear() {
        m_layers.clear();
        m_tags.clear();
    }

    /** Read-only access to all layers. */
    const std::vector<Layer>& Layers() const { return m_layers; }

private:
    std::vector<Layer> m_layers;
    std::unordered_map<uint32_t, std::unordered_set<std::string>> m_tags;
};

} // namespace atlas::editor
