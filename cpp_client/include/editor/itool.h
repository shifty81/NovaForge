#pragma once
/**
 * @file itool.h
 * @brief Base interface for editor tools within the ToolingLayer.
 *
 * Every editor tool (asset placement, physics tuning, PCG preview, etc.)
 * implements ITool so the ToolingLayer can manage activation, deactivation,
 * and per-frame updates uniformly.  When the NOVAFORGE_EDITOR_TOOLS flag
 * is removed for a release build, all ITool implementations disappear
 * along with the layer that owns them.
 */

#include <string>

namespace atlas::editor {

class ITool {
public:
    virtual ~ITool() = default;

    /** Human-readable name shown in the tool palette / menu. */
    virtual const char* Name() const = 0;

    /** Called once when the tool is selected / opened. */
    virtual void Activate() = 0;

    /** Called once when the tool is deselected / closed. */
    virtual void Deactivate() = 0;

    /** Per-frame tick while the tool is active. */
    virtual void Update(float deltaTime) = 0;

    /** Whether the tool is currently active. */
    virtual bool IsActive() const = 0;
};

} // namespace atlas::editor
