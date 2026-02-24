#include "ui/atlas/atlas_title_screen.h"
#include "ui/atlas/atlas_widgets.h"
#include <cstdlib>
#include <ctime>
#include <iostream>

namespace atlas {

AtlasTitleScreen::AtlasTitleScreen() {
    std::srand(static_cast<unsigned>(std::time(nullptr)));
}

void AtlasTitleScreen::render(AtlasContext& ctx) {
    if (!m_active) return;

    const auto& theme = ctx.theme();
    auto& r = ctx.renderer();
    float windowW = static_cast<float>(ctx.input().windowW);
    float windowH = static_cast<float>(ctx.input().windowH);

    // Full-screen background
    Rect bg(0.0f, 0.0f, windowW, windowH);
    r.drawRect(bg, Color(0.02f, 0.03f, 0.05f, 1.0f));

    // Sidebar strip (Atlas/Nexcom style left edge)
    Rect sidebar(0.0f, 0.0f, SIDEBAR_WIDTH, windowH);
    r.drawRect(sidebar, theme.bgHeader);

    // Sidebar accent line
    r.drawRect(Rect(SIDEBAR_WIDTH - 1.0f, 0.0f, 1.0f, windowH),
               theme.accentSecondary.withAlpha(0.3f));

    // "A" logo in sidebar (Atlas Engine branding)
    float logoY = 16.0f;
    r.drawText("A", Vec2(SIDEBAR_WIDTH * 0.5f - 4.0f, logoY),
               theme.accentSecondary);

    // Render current page BEFORE consuming mouse so buttons can process clicks
    switch (m_currentPage) {
        case Page::MAIN:              renderMainMenu(ctx);          break;
        case Page::SETTINGS:          renderSettings(ctx);          break;
        case Page::CHARACTER_CREATION:renderCharacterCreation(ctx); break;
        case Page::SHIP_SELECTION:    renderShipSelection(ctx);     break;
        case Page::SHIP_LOADOUT:      renderShipLoadout(ctx);       break;
        case Page::HANGAR_SPAWN:      renderHangarSpawn(ctx);       break;
    }

    // Consume mouse AFTER widgets have processed input to prevent
    // click-through to the 3D scene behind the title screen
    if (ctx.isHovered(bg)) {
        ctx.consumeMouse();
    }
}

// ─────────────────────────────────────────────────────────────────
// Main Menu: New Game · Multiplayer · Settings · Quit
// ─────────────────────────────────────────────────────────────────
void AtlasTitleScreen::renderMainMenu(AtlasContext& ctx) {
    const auto& theme = ctx.theme();
    auto& r = ctx.renderer();
    float windowW = static_cast<float>(ctx.input().windowW);
    float windowH = static_cast<float>(ctx.input().windowH);

    float contentX = SIDEBAR_WIDTH;
    float contentW = windowW - contentX;

    // Game title
    const char* titleLine1 = "NOVA FORGE";
    const char* titleLine2 = "A S T R A L I S";
    float title1W = r.measureText(titleLine1);
    float title2W = r.measureText(titleLine2);

    float titleY = windowH * 0.2f;
    r.drawText(titleLine1, Vec2(contentX + (contentW - title1W) * 0.5f, titleY),
               theme.accentSecondary);
    r.drawText(titleLine2, Vec2(contentX + (contentW - title2W) * 0.5f, titleY + 24.0f),
               theme.textSecondary);

    // Menu buttons — centered vertically below title
    float menuX = contentX + (contentW - MENU_WIDTH) * 0.5f;
    float menuY = windowH * 0.4f;

    // New Game
    Rect newGameBtn(menuX, menuY, MENU_WIDTH, BUTTON_HEIGHT);
    if (button(ctx, "New Game", newGameBtn)) {
        m_currentPage = Page::CHARACTER_CREATION;
        m_raceIdx = 0;
        m_bloodlineIdx = 0;
        m_careerIdx = 0;
    }
    menuY += BUTTON_HEIGHT + BUTTON_SPACING;

    // Multiplayer (placeholder)
    Rect mpBtn(menuX, menuY, MENU_WIDTH, BUTTON_HEIGHT);
    if (button(ctx, "Multiplayer", mpBtn)) {
        // TODO: multiplayer lobby / server browser
    }
    menuY += BUTTON_HEIGHT + BUTTON_SPACING;

    // Settings
    Rect settingsBtn(menuX, menuY, MENU_WIDTH, BUTTON_HEIGHT);
    if (button(ctx, "Settings", settingsBtn)) {
        m_currentPage = Page::SETTINGS;
    }
    menuY += BUTTON_HEIGHT + BUTTON_SPACING;

    // Quit
    Rect quitBtn(menuX, menuY, MENU_WIDTH, BUTTON_HEIGHT);
    if (button(ctx, "Quit", quitBtn)) {
        if (m_quitCb) m_quitCb();
    }

    // Version info at bottom
    const char* version = "Nova Forge v1.0.0  |  Atlas Engine";
    float verW = r.measureText(version);
    r.drawText(version, Vec2(contentX + (contentW - verW) * 0.5f, windowH - 30.0f),
               theme.textMuted);
}

// ─────────────────────────────────────────────────────────────────
// Settings (unchanged from original)
// ─────────────────────────────────────────────────────────────────
void AtlasTitleScreen::renderSettings(AtlasContext& ctx) {
    const auto& theme = ctx.theme();
    auto& r = ctx.renderer();
    float windowW = static_cast<float>(ctx.input().windowW);
    float windowH = static_cast<float>(ctx.input().windowH);

    float contentX = SIDEBAR_WIDTH;
    float contentW = windowW - contentX;

    // Settings title
    const char* title = "SETTINGS";
    float titleW = r.measureText(title);
    float titleY = windowH * 0.15f;
    r.drawText(title, Vec2(contentX + (contentW - titleW) * 0.5f, titleY),
               theme.accentSecondary);

    // Settings panel centered
    float panelW = 400.0f;
    float panelX = contentX + (contentW - panelW) * 0.5f;
    float panelY = windowH * 0.25f;

    // Audio section
    label(ctx, Vec2(panelX, panelY), "Audio", theme.accentSecondary);
    panelY += 24.0f;

    // Master Volume
    label(ctx, Vec2(panelX, panelY), "Master Volume", theme.textSecondary);
    panelY += 18.0f;
    Rect masterSlider(panelX, panelY, panelW, 20.0f);
    slider(ctx, "title_master_vol", masterSlider, &m_masterVolume, 0.0f, 1.0f, "%.0f%%");
    panelY += 20.0f + BUTTON_SPACING;

    // Music Volume
    label(ctx, Vec2(panelX, panelY), "Music Volume", theme.textSecondary);
    panelY += 18.0f;
    Rect musicSlider(panelX, panelY, panelW, 20.0f);
    slider(ctx, "title_music_vol", musicSlider, &m_musicVolume, 0.0f, 1.0f, "%.0f%%");
    panelY += 20.0f + BUTTON_SPACING;

    // SFX Volume
    label(ctx, Vec2(panelX, panelY), "SFX Volume", theme.textSecondary);
    panelY += 18.0f;
    Rect sfxSlider(panelX, panelY, panelW, 20.0f);
    slider(ctx, "title_sfx_vol", sfxSlider, &m_sfxVolume, 0.0f, 1.0f, "%.0f%%");
    panelY += 20.0f + BUTTON_SPACING * 2.0f;

    // Back button
    float btnW = 200.0f;
    float btnX = contentX + (contentW - btnW) * 0.5f;
    Rect backBtn(btnX, panelY, btnW, BUTTON_HEIGHT);
    if (button(ctx, "Back", backBtn)) {
        m_currentPage = Page::MAIN;
    }
}

// ─────────────────────────────────────────────────────────────────
// Character Creation: race → bloodline → career + 3D preview
// ─────────────────────────────────────────────────────────────────
void AtlasTitleScreen::renderCharacterCreation(AtlasContext& ctx) {
    const auto& theme = ctx.theme();
    auto& r = ctx.renderer();
    float windowW = static_cast<float>(ctx.input().windowW);
    float windowH = static_cast<float>(ctx.input().windowH);

    float contentX = SIDEBAR_WIDTH;
    float contentW = windowW - contentX;

    // Title
    const char* title = "CHARACTER CREATION";
    float titleW = r.measureText(title);
    r.drawText(title, Vec2(contentX + (contentW - titleW) * 0.5f, windowH * 0.08f),
               theme.accentSecondary);

    // ── Split layout: left (options) | right (3D preview) ───────
    float splitRatio = 0.45f;
    float leftW  = contentW * splitRatio;
    float rightW = contentW - leftW;
    float leftX  = contentX;
    float rightX = contentX + leftW;

    // ── Left panel: race & career selection ─────────────────────
    float panelW = leftW - PADDING * 2.0f;
    float panelX = leftX + PADDING;
    float panelY = windowH * 0.16f;

    // Track previous selections to detect changes
    int prevRace   = m_raceIdx;
    int prevCareer = m_careerIdx;

    label(ctx, Vec2(panelX, panelY), "Race", theme.accentSecondary);
    panelY += 24.0f;

    for (int i = 0; i < static_cast<int>(m_races.size()); ++i) {
        Rect btn(panelX, panelY, panelW, BUTTON_HEIGHT);
        const char* name = m_races[i].name.c_str();
        bool selected = (i == m_raceIdx);

        if (selected) {
            r.drawRect(btn, theme.accentSecondary.withAlpha(0.15f));
        }
        if (button(ctx, name, btn)) {
            m_raceIdx = i;
        }
        panelY += BUTTON_HEIGHT + 4.0f;
    }

    panelY += BUTTON_SPACING;

    label(ctx, Vec2(panelX, panelY), "Career", theme.accentSecondary);
    panelY += 24.0f;

    for (int i = 0; i < static_cast<int>(m_careers.size()); ++i) {
        Rect btn(panelX, panelY, panelW, BUTTON_HEIGHT);
        bool selected = (i == m_careerIdx);

        if (selected) {
            r.drawRect(btn, theme.accentSecondary.withAlpha(0.15f));
        }
        if (button(ctx, m_careers[i].name.c_str(), btn)) {
            m_careerIdx = i;
        }

        if (selected) {
            panelY += BUTTON_HEIGHT + 2.0f;
            label(ctx, Vec2(panelX + 8.0f, panelY),
                  m_careers[i].desc.c_str(), theme.textMuted);
            panelY += 16.0f;
        } else {
            panelY += BUTTON_HEIGHT + 4.0f;
        }
    }

    // Regenerate preview if selection changed
    if (m_raceIdx != prevRace || m_careerIdx != prevCareer) {
        m_previewDirty = true;
    }
    if (m_previewDirty) {
        regenerateCharacterPreview();
    }

    panelY += BUTTON_SPACING;

    // ── Navigation ──────────────────────────────────────────────
    float btnW = 150.0f;
    float gap  = 20.0f;
    float totalBtnW = btnW * 2 + gap;
    float startX = panelX + (panelW - totalBtnW) * 0.5f;

    Rect backBtn(startX, panelY, btnW, BUTTON_HEIGHT);
    if (button(ctx, "Back", backBtn)) {
        m_currentPage = Page::MAIN;
    }

    Rect nextBtn(startX + btnW + gap, panelY, btnW, BUTTON_HEIGHT);
    if (button(ctx, "Next", nextBtn)) {
        m_selectedRace      = m_races[m_raceIdx].id;
        m_selectedCareer    = m_careers[m_careerIdx].id;
        resolveStarterShip();
        m_currentPage = Page::SHIP_SELECTION;
    }

    // ── Right panel: 3D character preview viewport ──────────────
    float previewPad = PADDING;
    Rect previewArea(rightX + previewPad,
                     windowH * 0.16f,
                     rightW - previewPad * 2.0f,
                     windowH * 0.72f);
    renderCharacterPreviewViewport(ctx, previewArea);
}

// ─────────────────────────────────────────────────────────────────
// Ship Selection: shows the resolved starter ship
// ─────────────────────────────────────────────────────────────────
void AtlasTitleScreen::renderShipSelection(AtlasContext& ctx) {
    const auto& theme = ctx.theme();
    auto& r = ctx.renderer();
    float windowW = static_cast<float>(ctx.input().windowW);
    float windowH = static_cast<float>(ctx.input().windowH);

    float contentX = SIDEBAR_WIDTH;
    float contentW = windowW - contentX;

    // Title
    const char* title = "STARTER SHIP ASSIGNMENT";
    float titleW = r.measureText(title);
    r.drawText(title, Vec2(contentX + (contentW - titleW) * 0.5f, windowH * 0.1f),
               theme.accentSecondary);

    float panelW = 420.0f;
    float panelX = contentX + (contentW - panelW) * 0.5f;
    float panelY = windowH * 0.22f;

    // Career summary
    label(ctx, Vec2(panelX, panelY), "Career:", theme.accentSecondary);
    label(ctx, Vec2(panelX + 80.0f, panelY), m_selectedCareer.c_str(), theme.textPrimary);
    panelY += 28.0f;

    label(ctx, Vec2(panelX, panelY), "Race:", theme.accentSecondary);
    label(ctx, Vec2(panelX + 80.0f, panelY), m_selectedRace.c_str(), theme.textPrimary);
    panelY += 28.0f;

    // Assigned ship class
    label(ctx, Vec2(panelX, panelY), "Ship Class:", theme.accentSecondary);
    panelY += 24.0f;

    // Ship class display box
    Rect shipBox(panelX, panelY, panelW, 80.0f);
    r.drawRect(shipBox, theme.bgPanel);
    r.drawRect(Rect(panelX, panelY, panelW, 1.0f), theme.accentSecondary.withAlpha(0.4f));

    std::string shipLabel;
    std::string shipDesc;

    if (m_selectedShipClass == "destroyer") {
        shipLabel = "DESTROYER";
        shipDesc  = "Heavy hull - 7 high slots, 450 cargo capacity";
    } else {
        shipLabel = "FRIGATE";
        shipDesc  = "Light hull - 3 high slots, 250 cargo capacity";
    }

    float shipLabelW = r.measureText(shipLabel.c_str());
    r.drawText(shipLabel.c_str(),
               Vec2(panelX + (panelW - shipLabelW) * 0.5f, panelY + 20.0f),
               theme.accentSecondary);

    float descW = r.measureText(shipDesc.c_str());
    r.drawText(shipDesc.c_str(),
               Vec2(panelX + (panelW - descW) * 0.5f, panelY + 48.0f),
               theme.textSecondary);

    panelY += 80.0f + BUTTON_SPACING;

    // Industrialist note
    if (m_selectedCareer == "industrialist") {
        const char* note = "Industrialists always receive a destroyer hull.";
        float noteW = r.measureText(note);
        r.drawText(note, Vec2(panelX + (panelW - noteW) * 0.5f, panelY),
                   theme.textMuted);
        panelY += 24.0f;
    }

    panelY += BUTTON_SPACING;

    // Navigation
    float btnW = 150.0f;
    float gap  = 20.0f;
    float totalW = btnW * 2 + gap;
    float startX = contentX + (contentW - totalW) * 0.5f;

    Rect backBtn(startX, panelY, btnW, BUTTON_HEIGHT);
    if (button(ctx, "Back", backBtn)) {
        m_currentPage = Page::CHARACTER_CREATION;
    }

    Rect nextBtn(startX + btnW + gap, panelY, btnW, BUTTON_HEIGHT);
    if (button(ctx, "Configure Ship", nextBtn)) {
        // Set up loadout slots based on ship class
        if (m_selectedShipClass == "destroyer") {
            m_shipDisplayName  = "Starter Destroyer";
            m_loadoutHighSlots = 7;
            m_loadoutMidSlots  = 3;
            m_loadoutLowSlots  = 3;
        } else {
            m_shipDisplayName  = "Starter Frigate";
            m_loadoutHighSlots = 3;
            m_loadoutMidSlots  = 3;
            m_loadoutLowSlots  = 3;
        }
        m_currentPage = Page::SHIP_LOADOUT;
    }
}

// ─────────────────────────────────────────────────────────────────
// Ship Loadout: configure starting modules
// ─────────────────────────────────────────────────────────────────
void AtlasTitleScreen::renderShipLoadout(AtlasContext& ctx) {
    const auto& theme = ctx.theme();
    auto& r = ctx.renderer();
    float windowW = static_cast<float>(ctx.input().windowW);
    float windowH = static_cast<float>(ctx.input().windowH);

    float contentX = SIDEBAR_WIDTH;
    float contentW = windowW - contentX;

    const char* title = "SHIP LOADOUT";
    float titleW = r.measureText(title);
    r.drawText(title, Vec2(contentX + (contentW - titleW) * 0.5f, windowH * 0.1f),
               theme.accentSecondary);

    float panelW = 420.0f;
    float panelX = contentX + (contentW - panelW) * 0.5f;
    float panelY = windowH * 0.2f;

    // Ship name
    label(ctx, Vec2(panelX, panelY), m_shipDisplayName.c_str(), theme.textPrimary);
    panelY += 28.0f;

    // Slot summary
    char slotText[128];
    std::snprintf(slotText, sizeof(slotText),
                  "High: %d   Mid: %d   Low: %d",
                  m_loadoutHighSlots, m_loadoutMidSlots, m_loadoutLowSlots);
    label(ctx, Vec2(panelX, panelY), slotText, theme.textSecondary);
    panelY += 28.0f;

    // Slot placeholders
    auto drawSlotRow = [&](const char* name, int count) {
        label(ctx, Vec2(panelX, panelY), name, theme.accentSecondary);
        panelY += 20.0f;
        for (int i = 0; i < count; ++i) {
            Rect slot(panelX + static_cast<float>(i) * 52.0f, panelY, 48.0f, 32.0f);
            r.drawRect(slot, theme.bgPanel);
            r.drawRect(Rect(slot.x, slot.y, slot.w, 1.0f),
                       theme.accentSecondary.withAlpha(0.3f));
            label(ctx, Vec2(slot.x + 8.0f, slot.y + 8.0f), "---", theme.textMuted);
        }
        panelY += 32.0f + 12.0f;
    };

    drawSlotRow("High Slots", m_loadoutHighSlots);
    drawSlotRow("Mid Slots",  m_loadoutMidSlots);
    drawSlotRow("Low Slots",  m_loadoutLowSlots);

    panelY += BUTTON_SPACING;

    // Navigation
    float btnW = 150.0f;
    float gap  = 20.0f;
    float totalW = btnW * 2 + gap;
    float startX = contentX + (contentW - totalW) * 0.5f;

    Rect backBtn(startX, panelY, btnW, BUTTON_HEIGHT);
    if (button(ctx, "Back", backBtn)) {
        m_currentPage = Page::SHIP_SELECTION;
    }

    Rect nextBtn(startX + btnW + gap, panelY, btnW, BUTTON_HEIGHT);
    if (button(ctx, "Launch", nextBtn)) {
        m_currentPage = Page::HANGAR_SPAWN;
    }
}

// ─────────────────────────────────────────────────────────────────
// Hangar Spawn: you start on foot in a station hangar
// ─────────────────────────────────────────────────────────────────
void AtlasTitleScreen::renderHangarSpawn(AtlasContext& ctx) {
    const auto& theme = ctx.theme();
    auto& r = ctx.renderer();
    float windowW = static_cast<float>(ctx.input().windowW);
    float windowH = static_cast<float>(ctx.input().windowH);

    float contentX = SIDEBAR_WIDTH;
    float contentW = windowW - contentX;

    const char* title = "STATION HANGAR";
    float titleW = r.measureText(title);
    r.drawText(title, Vec2(contentX + (contentW - titleW) * 0.5f, windowH * 0.15f),
               theme.accentSecondary);

    float panelW = 480.0f;
    float panelX = contentX + (contentW - panelW) * 0.5f;
    float panelY = windowH * 0.28f;

    // Flavour text
    const char* desc1 = "You awake in a dimly-lit station hangar.";
    const char* desc2 = "Your ship sits on the landing pad, ready to launch.";
    const char* desc3 = "Look around — there may be supplies worth grabbing.";
    label(ctx, Vec2(panelX, panelY), desc1, theme.textSecondary); panelY += 22.0f;
    label(ctx, Vec2(panelX, panelY), desc2, theme.textSecondary); panelY += 22.0f;
    label(ctx, Vec2(panelX, panelY), desc3, theme.textSecondary); panelY += 36.0f;

    const char* hint1 = "• Explore the hangar on foot (FPS mode)";
    const char* hint2 = "• Loot anything lying around";
    const char* hint3 = "• Board your ship to access the cockpit";
    const char* hint4 = "• Use the Fleet Command Console for strategy view";
    label(ctx, Vec2(panelX, panelY), hint1, theme.textMuted); panelY += 20.0f;
    label(ctx, Vec2(panelX, panelY), hint2, theme.textMuted); panelY += 20.0f;
    label(ctx, Vec2(panelX, panelY), hint3, theme.textMuted); panelY += 20.0f;
    label(ctx, Vec2(panelX, panelY), hint4, theme.textMuted); panelY += 36.0f;

    // Undock button
    float btnW = 240.0f;
    float btnX = contentX + (contentW - btnW) * 0.5f;
    Rect undockBtn(btnX, panelY, btnW, BUTTON_HEIGHT);
    if (button(ctx, "Enter Hangar", undockBtn)) {
        m_active = false;
        if (m_playCb) m_playCb();
    }
}

// ─────────────────────────────────────────────────────────────────
// Regenerate the 3D character preview model
// ─────────────────────────────────────────────────────────────────
void AtlasTitleScreen::regenerateCharacterPreview() {
    m_previewDirty = false;
    m_previewGenerated = true;
    m_previewParts.clear();
    m_previewPalette.clear();

    // Derive a seed from the race and career index so changes are visible.
    int seed = m_raceIdx * 100 + m_careerIdx * 10 + 1;

    // Race-specific skin tones
    Color skinColor;
    switch (m_raceIdx) {
        case 0: skinColor = Color(0.87f, 0.74f, 0.62f, 1.0f); break; // Solari
        case 1: skinColor = Color(0.70f, 0.75f, 0.82f, 1.0f); break; // Veyren (blue-grey)
        case 2: skinColor = Color(0.92f, 0.80f, 0.68f, 1.0f); break; // Aurelian (golden)
        case 3: skinColor = Color(0.55f, 0.62f, 0.58f, 1.0f); break; // Keldari (olive)
        default: skinColor = Color(0.80f, 0.70f, 0.60f, 1.0f); break;
    }

    // Career-specific suit color
    Color suitColor;
    switch (m_careerIdx) {
        case 0: suitColor = Color(0.30f, 0.35f, 0.28f, 1.0f); break; // Soldier (olive drab)
        case 1: suitColor = Color(0.25f, 0.40f, 0.55f, 1.0f); break; // Explorer (teal)
        case 2: suitColor = Color(0.55f, 0.45f, 0.25f, 1.0f); break; // Industrialist (amber)
        case 3: suitColor = Color(0.40f, 0.35f, 0.50f, 1.0f); break; // Trader (muted purple)
        default: suitColor = Color(0.35f, 0.35f, 0.35f, 1.0f); break;
    }

    Color pantsColor = Color(suitColor.r * 0.7f, suitColor.g * 0.7f, suitColor.b * 0.7f, 1.0f);
    Color bootColor  = Color(0.15f, 0.14f, 0.13f, 1.0f);
    Color hairColor  = Color(0.20f + (seed % 3) * 0.15f,
                             0.15f + (seed % 5) * 0.05f,
                             0.10f, 1.0f);

    // Build body parts from ground up (3rd person FPS character)
    // Each part is a named box with position, scale, and color.
    m_previewParts = {
        {"Boots",      0.0f,   0.22f, 0.14f, 0.22f, bootColor},
        {"Legs",       0.14f,  0.18f, 0.38f, 0.18f, pantsColor},
        {"Torso",      0.52f,  0.30f, 0.40f, 0.20f, suitColor},
        {"Arms",       0.56f,  0.50f, 0.36f, 0.12f, suitColor},
        {"Hands",      0.52f,  0.10f, 0.10f, 0.10f, skinColor},
        {"Neck",       0.92f,  0.10f, 0.08f, 0.10f, skinColor},
        {"Head",       1.00f,  0.20f, 0.24f, 0.20f, skinColor},
        {"Hair",       1.18f,  0.22f, 0.10f, 0.22f, hairColor},
    };

    m_previewPalette = {skinColor, hairColor, suitColor, pantsColor};

    std::cout << "[CharGen] Preview generated: Race="
              << m_races[m_raceIdx].name
              << " Career=" << m_careers[m_careerIdx].name
              << std::endl;
}

// ─────────────────────────────────────────────────────────────────
// Render the 3D character preview viewport (3rd-person view)
// ─────────────────────────────────────────────────────────────────
void AtlasTitleScreen::renderCharacterPreviewViewport(AtlasContext& ctx, Rect area) {
    const auto& theme = ctx.theme();
    auto& r = ctx.renderer();

    // Viewport background (slightly lighter than main BG)
    r.drawRect(area, Color(0.04f, 0.05f, 0.07f, 1.0f));

    // Border
    r.drawRect(Rect(area.x, area.y, area.w, 1.0f),
               theme.accentSecondary.withAlpha(0.4f));
    r.drawRect(Rect(area.x, area.y, 1.0f, area.h),
               theme.accentSecondary.withAlpha(0.2f));
    r.drawRect(Rect(area.right() - 1.0f, area.y, 1.0f, area.h),
               theme.accentSecondary.withAlpha(0.2f));
    r.drawRect(Rect(area.x, area.bottom() - 1.0f, area.w, 1.0f),
               theme.accentSecondary.withAlpha(0.2f));

    // Header label
    const char* vpLabel = "3RD PERSON PREVIEW";
    float vpLabelW = r.measureText(vpLabel);
    r.drawText(vpLabel,
               Vec2(area.x + (area.w - vpLabelW) * 0.5f, area.y + 8.0f),
               theme.accentSecondary);

    if (!m_previewGenerated || m_previewParts.empty()) {
        const char* hint = "Select race & career to preview";
        float hintW = r.measureText(hint);
        r.drawText(hint,
                   Vec2(area.x + (area.w - hintW) * 0.5f,
                        area.y + area.h * 0.5f),
                   theme.textMuted);
        return;
    }

    // ── Fake 3D projection: draw character as stacked body-part boxes ──
    // The orbit camera yaw/pitch controls the perspective illusion.
    // Since this is a 2D UI renderer, we approximate the 3rd-person view
    // by projecting body part boxes with a pseudo-3D offset based on the
    // camera yaw, giving the effect of viewing the character from behind.

    float centerX = area.x + area.w * 0.5f;
    float groundY = area.bottom() - 60.0f;       // ground line
    float scale   = area.h * 0.38f;               // character height in pixels

    // Slight horizontal offset from camera yaw to simulate orbit
    float yawRad = (m_previewCamYaw - 180.0f) * 3.14159f / 180.0f;
    float orbitOffsetX = std::sin(yawRad) * 30.0f;

    // Draw ground shadow (ellipse approximated as a flat rect)
    Rect shadow(centerX - 40.0f + orbitOffsetX, groundY + 2.0f, 80.0f, 8.0f);
    r.drawRect(shadow, Color(0.0f, 0.0f, 0.0f, 0.3f));

    // Draw each body part as a colored rectangle, stacked upward
    for (const auto& part : m_previewParts) {
        float partH = part.scaleY * scale;
        float partW = part.scaleX * scale;
        float partX = centerX - partW * 0.5f + orbitOffsetX;
        float partY = groundY - (part.offsetY * scale) - partH;

        Rect partRect(partX, partY, partW, partH);
        r.drawRect(partRect, part.color);

        // Subtle edge highlight (pseudo-3D shading)
        r.drawRect(Rect(partX, partY, partW, 1.0f),
                   part.color.withAlpha(0.6f));
        r.drawRect(Rect(partX, partY, 1.0f, partH),
                   Color(1.0f, 1.0f, 1.0f, 0.08f));
    }

    // ── Orbit camera controls (drag to rotate) ─────────────────
    if (ctx.isHovered(area)) {
        // Left-drag to orbit the character preview
        if (ctx.input().mouseDown[0]) {
            // Simple yaw rotation based on horizontal mouse delta
            // (In a real implementation this would track frame-to-frame delta)
            float mouseRelX = (ctx.input().mousePos.x - area.x) / area.w;
            m_previewCamYaw = 180.0f + (mouseRelX - 0.5f) * 120.0f;
        }

        // Scroll to zoom
        if (ctx.input().scrollY != 0.0f) {
            m_previewCamDist -= ctx.input().scrollY * 0.3f;
            if (m_previewCamDist < 1.5f) m_previewCamDist = 1.5f;
            if (m_previewCamDist > 6.0f)  m_previewCamDist = 6.0f;
        }
    }

    // ── Info overlay at bottom of viewport ──────────────────────
    float infoY = area.bottom() - 48.0f;
    std::string raceLabel  = "Race: " + m_races[m_raceIdx].name;
    std::string careerLabel = "Career: " + m_careers[m_careerIdx].name;

    label(ctx, Vec2(area.x + 8.0f, infoY), raceLabel.c_str(), theme.textSecondary);
    infoY += 16.0f;
    label(ctx, Vec2(area.x + 8.0f, infoY), careerLabel.c_str(), theme.textSecondary);

    // Hint text
    const char* dragHint = "Drag to rotate | Scroll to zoom";
    float hintW = r.measureText(dragHint);
    r.drawText(dragHint,
               Vec2(area.x + (area.w - hintW) * 0.5f, area.bottom() - 16.0f),
               theme.textMuted);
}

// ─────────────────────────────────────────────────────────────────
// Resolve starter ship class based on career
// ─────────────────────────────────────────────────────────────────
void AtlasTitleScreen::resolveStarterShip() {
    if (m_selectedCareer == "industrialist") {
        // Industrialists always start with a destroyer hull
        m_selectedShipClass = "destroyer";
    } else {
        // All other careers: random roll (60% frigate, 40% destroyer)
        int roll = std::rand() % 100;
        m_selectedShipClass = (roll < 60) ? "frigate" : "destroyer";
    }
    std::cout << "[CharGen] Career=" << m_selectedCareer
              << " => StarterShip=" << m_selectedShipClass << std::endl;
}

} // namespace atlas
