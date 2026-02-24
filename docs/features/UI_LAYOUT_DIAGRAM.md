# EVE-Styled UI Layout Reference

## Screen Layout Diagram

```
┌───────────────────────────────────────────────────────────────────────────────┐
│                          Nova Forge - 3D Client                              │
├─┬─────────────────────────────────────────────────────────────────────────┬───┤
│N│ ┌──────────────────────┐                               ┌──────────────┐│   │
│E│ │ ◢ NAVIGATION          │                               │              ││ O │
│O│ │ Speed: 150.5 m/s     │                               │              ││ V │
│C│ │ X: 1000  Y: 2000     │                               │              ││ E │
│O│ └──────────────────────┘                               │              ││ R │
│M│                                                         │              ││ V │
│ │                                                         │  ◢ OVERVIEW  ││ I │
│≡│                                                         │              ││ E │
│ │                                                         │  [Nearby     ││ W │
│I│                                                         │   Objects]   ││   │
│ │                                                         │              ││   │
│F│                                                         │              ││   │
│ │                                                         │              ││   │
│M│                      * Star Field *                     │              ││   │
│ │                                                         │              ││   │
│$│                                                         │              ││   │
│ │                                                         │              ││   │
│@│                                                         └──────────────┘│   │
│ │                                                         ┌──────────────┐│   │
│ │                                                         │ ◢ TARGET     ││   │
│ │                                                         │ Guristas Fri ││   │
│ │                                                         │ Distance:5.5k││   │
│ │                                                         │ SHIELD:  75% ││   │
│ │                                                         │ ARMOR:   83% ││   │
│ │                                                         └──────────────┘│   │
├─┴─────────────────────────────────────────────────────────────────────────┴───┤
│                                                                               │
│ ┌──────────────────────┐     ┌────┐                ┌────────────────────────┐│
│ │ ◢ SHIP STATUS        │     │ ◎  │                │ ◢ COMBAT LOG           ││
│ │ Ship: Rifter         │     │◎⚡◎│                │ ◢ Target locked        ││
│ │ SHIELD:  500/500 (…) │     │ ◎  │                │ ◢ Shields holding      ││
│ │ ARMOR:   400/400 (…) │     └────┘                │ ◢ Capacitor stable     ││
│ │ STRUCT:  350/350 (…) │                           │ ◢ All systems nominal  ││
│ │ CAPACITOR: 100/100   │                           └────────────────────────┘│
│ └──────────────────────┘                                                      │
└───────────────────────────────────────────────────────────────────────────────┘
```

## Component Positions

### Top Row
- **Navigation Panel** (Top-Left): Position (-0.9, 0, 0.85), Size (0.4, 0.2)
- **Target Info Panel** (Top-Right): Position (0.9, 0, 0.75), Size (0.6, 0.3)

### Center
- **Ship & Capacitor** (Center): Position (0, 0, -0.6)
  - Ship indicator (tiny square)
  - Health rings (shield/armor/hull) - concentric circles
  - Capacitor ring (yellow circular gauge)

### Bottom Row
- **Ship Status Panel** (Bottom-Left): Position (-0.9, 0, -0.75), Size (0.6, 0.3)
- **Combat Log Panel** (Bottom-Right): Position (0.45, 0, -0.75), Size (0.9, 0.3)

### Sides
- **Nexcom Sidebar** (Far-Left): Position (-0.94, 0, 0), Size (0.12, 1.8)
- **Overview Panel** (Right): Position (0.5, 0, 0), Size (0.8, 1.2)

## Center HUD Detail

```
        ╔═══════════════════╗
        ║                   ║
        ║      SHIELD       ║  ← Outer ring (blue)
        ║   ┌───────────┐   ║
        ║   │   ARMOR   │   ║  ← Middle ring (yellow)
        ║   │ ┌───────┐ │   ║
        ║   │ │ HULL  │ │   ║  ← Inner ring (red)
        ║   │ │  ╬╬╬  │ │   ║  ← Ship indicator
        ║   │ └───────┘ │   ║
        ║   └───────────┘   ║
        ║                   ║
        ╚═══════════════════╝
             CAPACITOR       ← Outer yellow ring
```

## Panel Structure

```
┌─────────────────────────────┐
│ ━━━━━━━━━━━━━━━━━━━━━━━━━━━ │  ← Border accent (colored)
│ ◢ PANEL TITLE               │  ← Header (dark background)
├─────────────────────────────┤
│                             │
│  Label:            Value    │  ← Content rows
│  Label:            Value    │
│  Label:            Value    │
│                             │
└─────────────────────────────┘
```

## Color Legend

### Backgrounds
- `█` Dark blue-black (opacity 90%)
- `▓` Lighter blue (opacity 75%)
- `▒` Header blue (opacity 85%)

### Accents
- `━` Border lines (teal/cyan/red depending on panel)
- `◢` Title prefix (EVE-style arrow)

### Status Indicators
- `◎` Capacitor ring (yellow)
- Outer ring: Shield (blue)
- Middle ring: Armor (yellow)
- Inner ring: Hull (red)
- `╬╬╬` Ship indicator (teal)

## Sizing Reference

All sizes in Panda3D normalized coordinates (-1 to 1):

| Component | Width | Height | Aspect Ratio |
|-----------|-------|--------|--------------|
| Ship Status | 0.6 | 0.3 | 2:1 |
| Target Info | 0.6 | 0.3 | 2:1 |
| Navigation | 0.4 | 0.2 | 2:1 |
| Combat Log | 0.9 | 0.3 | 3:1 |
| Overview | 0.8 | 1.2 | 2:3 |
| Nexcom | 0.12 | 1.8 | 1:15 |

## Z-Order (Layering)

From back to front:
1. Star field (3D space)
2. Ship and entities (3D)
3. Panel backgrounds
4. Panel borders
5. Text elements
6. Tooltips (future)

## Spacing and Margins

- Panel padding: 0.02 units
- Panel margin: 0.01 units
- Text line spacing: 0.03 units
- Header height: 0.05 units
- Border thickness: 0.002 units

## Responsive Elements

Elements that update in real-time:
- ⚡ Capacitor ring (depletes/regenerates)
- 🛡️ Shield ring (percentage of full circle)
- 🛡️ Armor ring (percentage of full circle)
- ❤️ Hull ring (percentage of full circle)
- 📊 All text values (HP, speed, distance, etc.)
- 📝 Combat log (scrolls with new messages)
