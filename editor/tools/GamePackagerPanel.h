#pragma once
#include "../ui/EditorPanel.h"
#include "../../cpp_client/include/ui/atlas/atlas_widgets.h"
#include <string>
#include <vector>

namespace atlas::editor {

enum class BuildTarget {
    Client,
    Server,
};

enum class BuildMode {
    Debug,
    Development,
    Release,
};

enum class PackageStep {
    Idle,
    Validate,
    CookAssets,
    Compile,
    Bundle,
    Complete,
    Failed,
};

struct PackageSettings {
    BuildTarget target = BuildTarget::Client;
    BuildMode mode = BuildMode::Debug;
    bool singleExe = false;
    bool includeMods = false;
    bool stripEditorData = true;
    std::string outputPath = "./build/output";
};

struct PackageStatus {
    PackageStep currentStep = PackageStep::Idle;
    float progress = 0.0f;
    std::string statusMessage;
    std::vector<std::string> log;
    bool hasErrors = false;
};

class GamePackagerPanel : public EditorPanel {
public:
    const char* Name() const override { return "Game Packager"; }
    void Draw() override;

    const PackageSettings& Settings() const { return m_settings; }
    void SetSettings(const PackageSettings& s);
    void StartPackage();
    void AdvanceStep();
    void CancelPackage();
    const PackageStatus& Status() const { return m_status; }
    bool IsPackaging() const;

    static std::string StepToString(PackageStep step);
    static std::string TargetToString(BuildTarget target);
    static std::string ModeToString(BuildMode mode);

private:
    PackageSettings m_settings;
    PackageStatus m_status;
    atlas::PanelState m_panelState;
    float m_scrollOffset = 0.0f;
};

}
