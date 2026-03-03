#include "GamePackagerPanel.h"

namespace atlas::editor {

void GamePackagerPanel::Draw() {
    // Headless: state tracking only
    if (!GetContext()) return;

    auto& ctx = *GetContext();
    ApplyDockBounds(m_panelState);
    if (!atlas::panelBeginStateful(ctx, "Game Packager", m_panelState)) {
        atlas::panelEnd(ctx);
        return;
    }

    const float pad     = ctx.theme().padding;
    const float rowH    = ctx.theme().rowHeight;
    const atlas::Rect& b = m_panelState.bounds;
    const float headerH = ctx.theme().headerHeight;
    float y = b.y + headerH + pad;
    float scrollOff = 0.0f;

    // Build target toggle
    {
        std::string targetLabel = "Target: " + TargetToString(m_settings.target);
        atlas::Rect targetBtn{b.x + pad, y, b.w - 2.0f * pad, rowH + pad};
        if (!IsPackaging() && atlas::button(ctx, targetLabel.c_str(), targetBtn)) {
            m_settings.target = (m_settings.target == BuildTarget::Client)
                                ? BuildTarget::Server : BuildTarget::Client;
        } else {
            atlas::label(ctx, {b.x + pad, y + 2},
                "Target: " + TargetToString(m_settings.target), ctx.theme().textPrimary);
        }
    }
    y += rowH + pad;

    // Build mode toggle
    {
        std::string modeLabel = "Mode: " + ModeToString(m_settings.mode);
        atlas::Rect modeBtn{b.x + pad, y, b.w - 2.0f * pad, rowH + pad};
        if (!IsPackaging() && atlas::button(ctx, modeLabel.c_str(), modeBtn)) {
            if (m_settings.mode == BuildMode::Debug)
                m_settings.mode = BuildMode::Development;
            else if (m_settings.mode == BuildMode::Development)
                m_settings.mode = BuildMode::Release;
            else
                m_settings.mode = BuildMode::Debug;
        } else {
            atlas::label(ctx, {b.x + pad, y + 2},
                "Mode: " + ModeToString(m_settings.mode), ctx.theme().textPrimary);
        }
    }
    y += rowH + pad;

    // Output path
    atlas::label(ctx, {b.x + pad, y},
        "Output: " + m_settings.outputPath, ctx.theme().textSecondary);
    y += rowH + pad;

    // Option checkboxes (editable only when not packaging)
    if (!IsPackaging()) {
        atlas::checkbox(ctx, "Single EXE", {b.x + pad, y, b.w - 2.0f * pad, rowH + pad}, &m_settings.singleExe);
        y += rowH + pad;
        atlas::checkbox(ctx, "Include Mods", {b.x + pad, y, b.w - 2.0f * pad, rowH + pad}, &m_settings.includeMods);
        y += rowH + pad;
        atlas::checkbox(ctx, "Strip Editor Data", {b.x + pad, y, b.w - 2.0f * pad, rowH + pad}, &m_settings.stripEditorData);
        y += rowH + pad;
    }

    // Progress bar
    atlas::Rect progRect{b.x + pad, y, b.w - 2.0f * pad, rowH + pad};
    std::string progLabel = StepToString(m_status.currentStep)
        + " " + std::to_string(static_cast<int>(m_status.progress * 100.0f)) + "%";
    atlas::progressBar(ctx, progRect, m_status.progress,
                       ctx.theme().accentPrimary, progLabel.c_str());
    y += rowH + pad + pad;

    // Start / Cancel buttons
    const float btnW = 100.0f;
    if (!IsPackaging()) {
        if (atlas::button(ctx, "Start Package", {b.x + pad, y, btnW + 20.0f, rowH + pad})) {
            StartPackage();
        }
    } else {
        if (atlas::button(ctx, "Cancel", {b.x + pad, y, btnW, rowH + pad})) {
            CancelPackage();
        }
    }
    y += rowH + pad + pad;

    // Status step label
    atlas::label(ctx, {b.x + pad, y}, m_status.statusMessage, ctx.theme().textSecondary);
    y += rowH + pad;

    atlas::separator(ctx, {b.x + pad, y}, b.w - 2.0f * pad);
    y += pad;

    // Log area
    atlas::Rect logRect{b.x + pad, y, b.w - 2.0f * pad, b.y + b.h - y - pad};
    atlas::combatLogWidget(ctx, logRect, m_status.log, scrollOff);

    atlas::panelEnd(ctx);
}

void GamePackagerPanel::SetSettings(const PackageSettings& s) {
    m_settings = s;
}

void GamePackagerPanel::StartPackage() {
    m_status = PackageStatus{};
    if (m_settings.outputPath.empty()) {
        m_status.currentStep = PackageStep::Failed;
        m_status.hasErrors = true;
        m_status.statusMessage = "Output path is empty";
        m_status.log.push_back("ERROR: Output path must not be empty");
        return;
    }
    m_status.currentStep = PackageStep::Validate;
    m_status.progress = 0.0f;
    m_status.statusMessage = "Validating settings...";
    m_status.log.push_back("Started packaging: " + TargetToString(m_settings.target) + " / " + ModeToString(m_settings.mode));
}

void GamePackagerPanel::AdvanceStep() {
    switch (m_status.currentStep) {
    case PackageStep::Validate:
        m_status.currentStep = PackageStep::CookAssets;
        m_status.progress = 0.25f;
        m_status.statusMessage = "Cooking assets...";
        m_status.log.push_back("Validation passed");
        break;
    case PackageStep::CookAssets:
        m_status.currentStep = PackageStep::Compile;
        m_status.progress = 0.50f;
        m_status.statusMessage = "Compiling...";
        m_status.log.push_back("Assets cooked");
        break;
    case PackageStep::Compile:
        m_status.currentStep = PackageStep::Bundle;
        m_status.progress = 0.75f;
        m_status.statusMessage = "Bundling...";
        m_status.log.push_back("Compilation finished");
        break;
    case PackageStep::Bundle:
        m_status.currentStep = PackageStep::Complete;
        m_status.progress = 1.0f;
        m_status.statusMessage = "Package complete";
        m_status.log.push_back("Bundle created at " + m_settings.outputPath);
        break;
    default:
        break;
    }
}

void GamePackagerPanel::CancelPackage() {
    m_status = PackageStatus{};
}

bool GamePackagerPanel::IsPackaging() const {
    return m_status.currentStep != PackageStep::Idle &&
           m_status.currentStep != PackageStep::Complete &&
           m_status.currentStep != PackageStep::Failed;
}

std::string GamePackagerPanel::StepToString(PackageStep step) {
    switch (step) {
    case PackageStep::Idle:       return "Idle";
    case PackageStep::Validate:   return "Validate";
    case PackageStep::CookAssets: return "Cook Assets";
    case PackageStep::Compile:    return "Compile";
    case PackageStep::Bundle:     return "Bundle";
    case PackageStep::Complete:   return "Complete";
    case PackageStep::Failed:     return "Failed";
    }
    return "Unknown";
}

std::string GamePackagerPanel::TargetToString(BuildTarget target) {
    switch (target) {
    case BuildTarget::Client: return "Client";
    case BuildTarget::Server: return "Server";
    }
    return "Unknown";
}

std::string GamePackagerPanel::ModeToString(BuildMode mode) {
    switch (mode) {
    case BuildMode::Debug:       return "Debug";
    case BuildMode::Development: return "Development";
    case BuildMode::Release:     return "Release";
    }
    return "Unknown";
}

}
