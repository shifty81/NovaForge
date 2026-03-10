// Tests for: CorporationLogoSystem
#include "test_log.h"
#include "components/game_components.h"
#include "ecs/system.h"
#include "systems/corporation_logo_system.h"

using namespace atlas;
using LT = components::CorporationLogo::LayerType;

// ==================== CorporationLogoSystem Tests ====================

static void testLogoInit() {
    std::cout << "\n=== Logo: Init ===" << std::endl;
    ecs::World world;
    systems::CorporationLogoSystem sys(&world);
    world.createEntity("corp1");
    assertTrue(sys.initialize("corp1", "c1", "Star Corp"), "Init succeeds");
    assertTrue(sys.getLayerCount("corp1") == 0, "Zero layers initially");
    assertTrue(sys.getActiveLayerId("corp1").empty(), "No active layer initially");
    assertTrue(!sys.isPublished("corp1"), "Not published initially");
    assertTrue(sys.getTotalEdits("corp1") == 0, "Zero edits initially");
    assertTrue(sys.getCorpName("corp1") == "Star Corp", "Corp name stored");
}

static void testLogoInitFails() {
    std::cout << "\n=== Logo: InitFails ===" << std::endl;
    ecs::World world;
    systems::CorporationLogoSystem sys(&world);
    assertTrue(!sys.initialize("nonexistent", "c1", "Name"), "Fails on missing entity");
    world.createEntity("corp1");
    assertTrue(!sys.initialize("corp1", "", "Name"), "Fails with empty corp_id");
    assertTrue(!sys.initialize("corp1", "c1", ""), "Fails with empty corp_name");
}

static void testLogoAddLayer() {
    std::cout << "\n=== Logo: AddLayer ===" << std::endl;
    ecs::World world;
    systems::CorporationLogoSystem sys(&world);
    world.createEntity("corp1");
    sys.initialize("corp1", "c1", "Star Corp");

    assertTrue(sys.addLayer("corp1", "bg", "Background", LT::Background,
               "#000011", 1.0f, 1.0f), "Add background layer");
    assertTrue(sys.addLayer("corp1", "fg", "Foreground", LT::Foreground,
               "#FF4400", 0.9f, 0.8f), "Add foreground layer");
    assertTrue(sys.getLayerCount("corp1") == 2, "Two layers stored");
    assertTrue(sys.getTotalEdits("corp1") == 2, "Two edits recorded");

    assertTrue(!sys.addLayer("corp1", "bg", "Dup", LT::Background,
               "#000", 1.0f, 1.0f), "Duplicate layer_id rejected");
    assertTrue(!sys.addLayer("corp1", "", "Empty ID", LT::Background,
               "#000", 1.0f, 1.0f), "Empty layer_id rejected");
    assertTrue(!sys.addLayer("corp1", "x", "", LT::Background,
               "#000", 1.0f, 1.0f), "Empty name rejected");
    assertTrue(!sys.addLayer("corp1", "x", "Bad Opacity", LT::Background,
               "#000", 1.5f, 1.0f), "Opacity > 1 rejected");
    assertTrue(!sys.addLayer("corp1", "x", "Bad Scale", LT::Background,
               "#000", 1.0f, 0.0f), "Zero scale rejected");
}

static void testLogoRemoveLayer() {
    std::cout << "\n=== Logo: RemoveLayer ===" << std::endl;
    ecs::World world;
    systems::CorporationLogoSystem sys(&world);
    world.createEntity("corp1");
    sys.initialize("corp1", "c1", "Star Corp");
    sys.addLayer("corp1", "bg", "Background", LT::Background, "#000", 1.0f, 1.0f);
    sys.addLayer("corp1", "fg", "Foreground", LT::Foreground, "#F00", 1.0f, 1.0f);
    sys.setActiveLayer("corp1", "bg");

    assertTrue(sys.removeLayer("corp1", "bg"), "Remove existing layer");
    assertTrue(sys.getLayerCount("corp1") == 1, "Count decremented");
    assertTrue(!sys.hasLayer("corp1", "bg"), "Removed layer gone");
    assertTrue(sys.getActiveLayerId("corp1").empty(), "Active cleared after removal");
    assertTrue(!sys.removeLayer("corp1", "nonexistent"), "Remove unknown fails");
}

static void testLogoSetActiveLayer() {
    std::cout << "\n=== Logo: SetActiveLayer ===" << std::endl;
    ecs::World world;
    systems::CorporationLogoSystem sys(&world);
    world.createEntity("corp1");
    sys.initialize("corp1", "c1", "Star Corp");
    sys.addLayer("corp1", "bg", "Background", LT::Background, "#000", 1.0f, 1.0f);
    sys.addLayer("corp1", "fg", "Foreground", LT::Foreground, "#F00", 1.0f, 1.0f);

    assertTrue(sys.setActiveLayer("corp1", "bg"), "Set active to bg");
    assertTrue(sys.getActiveLayerId("corp1") == "bg", "bg is active");
    assertTrue(sys.setActiveLayer("corp1", "fg"), "Switch active to fg");
    assertTrue(sys.getActiveLayerId("corp1") == "fg", "fg is now active");
    assertTrue(!sys.setActiveLayer("corp1", "nonexistent"), "Cannot set unknown active");
}

static void testLogoEditActiveLayer() {
    std::cout << "\n=== Logo: EditActiveLayer ===" << std::endl;
    ecs::World world;
    systems::CorporationLogoSystem sys(&world);
    world.createEntity("corp1");
    sys.initialize("corp1", "c1", "Star Corp");
    sys.addLayer("corp1", "bg", "Background", LT::Background, "#000", 1.0f, 1.0f);
    sys.setActiveLayer("corp1", "bg");

    int before = sys.getTotalEdits("corp1");
    assertTrue(sys.setLayerColor("corp1", "#FF0000"), "Set color");
    assertTrue(sys.setLayerOpacity("corp1", 0.75f), "Set opacity");
    assertTrue(sys.setLayerScale("corp1", 1.5f), "Set scale");
    assertTrue(sys.setLayerOffset("corp1", 10.0f, -5.0f), "Set offset");
    assertTrue(sys.getTotalEdits("corp1") == before + 4, "4 edit operations recorded");
}

static void testLogoEditValidation() {
    std::cout << "\n=== Logo: EditValidation ===" << std::endl;
    ecs::World world;
    systems::CorporationLogoSystem sys(&world);
    world.createEntity("corp1");
    sys.initialize("corp1", "c1", "Star Corp");
    sys.addLayer("corp1", "bg", "Background", LT::Background, "#000", 1.0f, 1.0f);
    sys.setActiveLayer("corp1", "bg");

    assertTrue(!sys.setLayerOpacity("corp1", -0.1f), "Negative opacity rejected");
    assertTrue(!sys.setLayerOpacity("corp1", 1.1f), "Opacity > 1 rejected");
    assertTrue(!sys.setLayerScale("corp1", 0.0f), "Zero scale rejected");
    assertTrue(!sys.setLayerScale("corp1", -1.0f), "Negative scale rejected");
}

static void testLogoEditNoActive() {
    std::cout << "\n=== Logo: EditNoActive ===" << std::endl;
    ecs::World world;
    systems::CorporationLogoSystem sys(&world);
    world.createEntity("corp1");
    sys.initialize("corp1", "c1", "Star Corp");
    sys.addLayer("corp1", "bg", "Background", LT::Background, "#000", 1.0f, 1.0f);
    // No active layer set

    assertTrue(!sys.setLayerColor("corp1", "#FF0000"), "Cannot edit without active layer");
    assertTrue(!sys.setLayerOpacity("corp1", 0.5f), "Cannot set opacity without active");
    assertTrue(!sys.setLayerScale("corp1", 2.0f), "Cannot set scale without active");
    assertTrue(!sys.setLayerOffset("corp1", 1.0f, 1.0f), "Cannot set offset without active");
}

static void testLogoPublish() {
    std::cout << "\n=== Logo: Publish ===" << std::endl;
    ecs::World world;
    systems::CorporationLogoSystem sys(&world);
    world.createEntity("corp1");
    sys.initialize("corp1", "c1", "Star Corp");
    sys.addLayer("corp1", "bg", "Background", LT::Background, "#000", 1.0f, 1.0f);
    sys.setActiveLayer("corp1", "bg");

    assertTrue(!sys.isPublished("corp1"), "Not published before publish");
    assertTrue(sys.publishLogo("corp1"), "Publish succeeds");
    assertTrue(sys.isPublished("corp1"), "Published after publish");

    // Published logo blocks edits
    assertTrue(!sys.addLayer("corp1", "ov", "Overlay", LT::Overlay, "#FFF", 1.0f, 1.0f),
               "Add blocked after publish");
    assertTrue(!sys.removeLayer("corp1", "bg"), "Remove blocked after publish");
    assertTrue(!sys.setActiveLayer("corp1", "bg"), "SetActive blocked after publish");
    assertTrue(!sys.setLayerColor("corp1", "#F00"), "Edit blocked after publish");

    assertTrue(!sys.publishLogo("corp1"), "Cannot publish twice");
}

static void testLogoPublishEmpty() {
    std::cout << "\n=== Logo: PublishEmpty ===" << std::endl;
    ecs::World world;
    systems::CorporationLogoSystem sys(&world);
    world.createEntity("corp1");
    sys.initialize("corp1", "c1", "Star Corp");
    assertTrue(!sys.publishLogo("corp1"), "Cannot publish empty logo");
}

static void testLogoReset() {
    std::cout << "\n=== Logo: Reset ===" << std::endl;
    ecs::World world;
    systems::CorporationLogoSystem sys(&world);
    world.createEntity("corp1");
    sys.initialize("corp1", "c1", "Star Corp");
    sys.addLayer("corp1", "bg", "Background", LT::Background, "#000", 1.0f, 1.0f);
    sys.setActiveLayer("corp1", "bg");
    sys.publishLogo("corp1");

    assertTrue(sys.isPublished("corp1"), "Published before reset");
    assertTrue(sys.resetLogo("corp1"), "Reset succeeds");
    assertTrue(!sys.isPublished("corp1"), "Not published after reset");
    assertTrue(sys.getLayerCount("corp1") == 0, "Layers cleared after reset");
    assertTrue(sys.getActiveLayerId("corp1").empty(), "Active cleared after reset");

    // Can add new layers and re-publish after reset
    assertTrue(sys.addLayer("corp1", "newbg", "New BG", LT::Background,
               "#111", 1.0f, 1.0f), "Can add layer after reset");
    assertTrue(sys.publishLogo("corp1"), "Can publish again after reset");
}

static void testLogoMissing() {
    std::cout << "\n=== Logo: Missing ===" << std::endl;
    ecs::World world;
    systems::CorporationLogoSystem sys(&world);

    assertTrue(!sys.addLayer("nonexistent", "l1", "L", LT::Background, "#0", 1.0f, 1.0f),
               "AddLayer fails on missing");
    assertTrue(!sys.removeLayer("nonexistent", "l1"), "RemoveLayer fails on missing");
    assertTrue(!sys.setActiveLayer("nonexistent", "l1"), "SetActive fails on missing");
    assertTrue(!sys.setLayerColor("nonexistent", "#F"), "SetColor fails on missing");
    assertTrue(!sys.setLayerOpacity("nonexistent", 0.5f), "SetOpacity fails on missing");
    assertTrue(!sys.setLayerScale("nonexistent", 1.0f), "SetScale fails on missing");
    assertTrue(!sys.setLayerOffset("nonexistent", 0.0f, 0.0f), "SetOffset fails on missing");
    assertTrue(!sys.publishLogo("nonexistent"), "Publish fails on missing");
    assertTrue(!sys.resetLogo("nonexistent"), "Reset fails on missing");
    assertTrue(sys.getLayerCount("nonexistent") == 0, "Zero layers on missing");
    assertTrue(sys.getActiveLayerId("nonexistent").empty(), "Empty active on missing");
    assertTrue(!sys.isPublished("nonexistent"), "Not published on missing");
    assertTrue(sys.getTotalEdits("nonexistent") == 0, "Zero edits on missing");
    assertTrue(!sys.hasLayer("nonexistent", "l1"), "HasLayer false on missing");
    assertTrue(sys.getCorpName("nonexistent").empty(), "Empty name on missing");
}

void run_corporation_logo_system_tests() {
    testLogoInit();
    testLogoInitFails();
    testLogoAddLayer();
    testLogoRemoveLayer();
    testLogoSetActiveLayer();
    testLogoEditActiveLayer();
    testLogoEditValidation();
    testLogoEditNoActive();
    testLogoPublish();
    testLogoPublishEmpty();
    testLogoReset();
    testLogoMissing();
}
