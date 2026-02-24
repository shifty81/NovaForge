#include "GamePackagerPanel.h"

namespace atlas::editor {

void GamePackagerPanel::Draw() {
    // Headless: state tracking only
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
