# EVE-Styled UI Quick Start Guide

## Overview

NovaForge now features a comprehensive EVE Online Photon UI styled interface! The new UI matches the official EVE Online look and feel with dark blue panels, teal accents, and circular displays.

## What's New?

### Visual Changes
- 🎨 **EVE Color Scheme**: Dark blue-black panels with teal/cyan accents
- ⚡ **Circular Capacitor**: Yellow ring around ship showing energy levels
- 🛡️ **Health Rings**: Concentric blue/yellow/red rings for shield/armor/hull
- 📊 **EVE-Styled Panels**: Semi-transparent panels with colored borders
- 📍 **Proper Layout**: Matches EVE's screen positioning

### UI Components

```
┌─────────────────────────────────────────────────────┐
│  [Nav]              [Star Field]        [Target]    │
│  [Neo]                                  [Overview]  │
│  [com]              Ship + Rings                    │
│  [Ship Status]                          [Combat]    │
└─────────────────────────────────────────────────────┘
```

## How to Use

### Running with EVE-Styled UI (Default)

The new UI is enabled by default in the 3D client:

```bash
# Start the 3D client
python client_3d.py "YourPilotName"

# Or with server connection
python client_3d.py "YourPilotName" localhost 8765
```

The UI will automatically use the EVE style!

### Switching to Legacy UI

If you prefer the old UI:

Edit `client_3d/core/game_client.py` line 69:
```python
# Change this:
self.hud = create_hud(self.aspect2d, self.render2d, style='eve')

# To this:
self.hud = create_hud(self.aspect2d, style='legacy')
```

### Testing the UI

#### Standalone UI Demo
View the UI without connecting to a server:
```bash
python test_eve_hud.py
```

Features:
- Press **T** to toggle target display
- Press **SPACE** to simulate damage
- Press **ESC** to exit
- Watch animated capacitor and health bars

#### Unit Tests
Verify all components work:
```bash
python test_eve_ui_components.py
```

Should see:
```
✓ ALL TESTS PASSED
```

## UI Components Explained

### Center HUD
**Location**: Center of screen

What you see:
- Small teal square = your ship
- Inner red ring = hull health
- Middle yellow ring = armor health
- Outer blue ring = shield health
- Yellow outer ring = capacitor energy

The rings deplete as you take damage or use energy!

### Ship Status Panel
**Location**: Bottom left corner

Shows:
- Ship name
- Shield HP and percentage (blue)
- Armor HP and percentage (yellow)
- Hull HP and percentage (red)
- Capacitor HP and percentage (yellow)

Colors change from bright to dim as values decrease.

### Target Info Panel
**Location**: Top right corner

Shows (when target locked):
- Target name
- Distance (in meters or kilometers)
- Shield percentage
- Armor percentage

Red accent indicates hostile target.

### Navigation Panel
**Location**: Top left corner

Shows:
- Current speed in m/s
- Position coordinates (X, Y, Z)

### Combat Log
**Location**: Bottom right

Shows:
- Last 8 combat messages
- Auto-scrolls with new messages
- Messages prefixed with "◢"

### Overview Panel
**Location**: Right side

Shows:
- Nearby objects (placeholder)
- Future: filtering and sorting

### Nexcom Sidebar
**Location**: Far left edge

Shows:
- Menu button (≡)
- Quick access icons:
  - **I** = Inventory
  - **F** = Fitting
  - **M** = Map
  - **$** = Market
  - **@** = Character

## Color Guide

### Status Colors
- **Blue** = Shields (healthy)
- **Yellow** = Armor, Capacitor (active)
- **Red** = Hull, Hostile (danger)
- **Green** = Friendly (safe)
- **Teal** = UI accents, information
- **White** = Primary text
- **Gray** = Secondary text

### Health States
- **Bright colors** = High health/energy (75-100%)
- **Normal colors** = Medium (50-75%)
- **Dimmed colors** = Low (25-50%)
- **Dark/Red** = Critical (0-25%)

## Customization

### Changing Colors
Edit `client_3d/ui/eve_style.py`:

```python
# Example: Change accent color from teal to purple
ACCENT_PRIMARY = Vec4(0.6, 0.2, 0.8, 1.0)  # Purple
```

### Adjusting Panel Positions
Edit `client_3d/ui/eve_style.py` in `EVELayoutPresets`:

```python
SHIP_STATUS = {
    'pos': (-0.9, 0, -0.75),  # Change these coordinates
    'size': (0.6, 0.3),
}
```

### Adding Custom Panels
See `client_3d/ui/eve_hud.py` for examples of panel creation.

## Tips & Tricks

### Performance
- The UI is optimized for 60+ FPS
- Circular displays use efficient geometry
- Panel updates are batched

### Visibility
- All panels have semi-transparent backgrounds
- You can see the space behind them
- This matches EVE's design philosophy

### Readability
- Text uses high contrast colors
- Important information is bright white
- Secondary info is gray-blue
- Status uses color coding

## Troubleshooting

### UI Not Showing
1. Check Panda3D is installed: `pip install panda3d`
2. Verify imports work: `python test_eve_ui_components.py`
3. Check for error messages in console

### Wrong Colors
1. Ensure you're using EVE style: `style='eve'` in game_client.py
2. Check color definitions in `eve_style.py`
3. Verify transparency is enabled

### Layout Issues
1. Check screen resolution (works best at 1280x720+)
2. Verify Panda3D window settings
3. Check layout presets in `eve_style.py`

### Performance Issues
1. Reduce number of segments in capacitor/rings
2. Disable some panels if needed
3. Check graphics settings

## Documentation

For more details, see:
- **Technical Docs**: `docs/features/EVE_UI_ENHANCEMENTS.md`
- **Layout Diagram**: `docs/features/UI_LAYOUT_DIAGRAM.md`
- **Implementation**: `docs/features/UI_IMPLEMENTATION_SUMMARY.md`

## Comparison to EVE Online

### What Matches
✅ Color scheme (dark blue, teal accents)
✅ Circular displays (capacitor, health rings)
✅ Panel layout and positioning
✅ Text styling and colors
✅ Semi-transparent panels
✅ Border accents

### What's Simplified
🔲 Module slots (placeholder icons)
🔲 Overview content (structure only)
🔲 Icon graphics (text placeholders)

### Future Additions
🔜 Animated target brackets
🔜 Interactive module rack
🔜 Draggable panels
🔜 Custom themes

## Credits

- **Design**: Based on CCP Games' EVE Online Photon UI
- **Implementation**: NovaForge Project
- **Research**: EVE Online documentation and community resources

---

**Enjoy the EVE-styled UI!** 🚀

For questions or issues, check the documentation or test files.
