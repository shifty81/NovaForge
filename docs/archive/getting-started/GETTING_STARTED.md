# Nova Forge - Getting Started Guide

## Welcome to Nova Forge!

This guide will help you get started with the Nova Forge custom game engine.

## What is Nova Forge?

Nova Forge is a PVE-focused space MMO inspired by EVE ONLINE and Astrox Imperium, designed for small groups of players (2-20). It features:

- Custom-built Python game engine with ECS architecture
- Server-authoritative multiplayer
- EVE-like ship combat, fitting, skills, and progression
- Fully moddable via JSON data files
- No PVP - cooperative PVE only

## Installation

### Prerequisites
- Python 3.11 or higher
- Git (to clone the repository)

### Steps

1. **Clone the repository:**
```bash
git clone https://github.com/shifty81/NovaForge.git
cd NovaForge
```

2. **Optional: Create a virtual environment:**
```bash
python -m venv venv
source venv/bin/activate  # On Windows: venv\Scripts\activate
```

3. **Install dependencies (optional, currently none required):**
```bash
pip install -r requirements.txt
```

## Quick Test

Run the engine tests to verify everything works:

```bash
python tests/test_engine.py
```

You should see:
```
✅ ALL TESTS PASSED!
```

## Running the Game

### Option 1: Demo Script (Recommended for First Time)

```bash
python demo.py
```

This interactive script lets you:
- Run engine tests
- Start the server
- Launch clients
- Try multiplayer mode

### Option 2: Manual Setup

**Terminal 1 - Start the Server:**
```bash
python server/server.py
```

You should see:
```
[Server] Initializing Nova Forge Server
[DataLoader] Loading game data...
[Server] Server running on ('127.0.0.1', 8765)
```

**Terminal 2 - Start a Client:**
```bash
python client/client.py "YourCharacterName"
```

You should see:
```
[Client] Connecting to localhost:8765...
[Client] Connected! Welcome to Nova Forge!
```

**Terminal 3+ - More Clients (Optional):**
```bash
python client/client.py "Pilot2"
python client/client.py "Pilot3"
```

## Understanding the Current Build

### What's Implemented

✅ **Core Engine:**
- Entity Component System (ECS)
- Game loop and state management
- Data-driven architecture

✅ **Game Systems:**
- Movement system
- Combat system (damage calculation, resistances)
- Shield/capacitor recharge
- NPC AI (basic behaviors)
- Targeting system

✅ **Networking:**
- Server-client architecture
- State synchronization
- Chat system

✅ **Game Data:**
- 4 frigates (Rifter, Merlin, Tristan, Punisher)
- 9 modules (weapons, shields, armor, etc.)
- 15 skills
- 4 NPC types (Serpentis, Guristas, Blood Raiders)
- 5 mission templates
- 4 solar systems

### What's Not Yet Implemented

❌ Graphics/UI (text-based currently)
❌ Mission system activation
❌ Ship fitting implementation
❌ Skills training
❌ Economy/trading
❌ Actual gameplay loop

## Current Gameplay

Right now, the game demonstrates the core architecture:

1. Server simulates a persistent world
2. Clients connect and spawn ships
3. Server updates ship positions and states
4. Clients receive state updates
5. Basic chat works

This is **Phase 1** - the foundation is complete!

## Next Steps for Development

**Phase 2 will add:**
- 2D graphics rendering (Pygame)
- Basic UI
- Clickable interface
- Mission system
- Ship fitting

**Phase 3 will add:**
- More content (ships, modules)
- Mining mechanics
- Economy system
- Better AI

## Modding

All game content is in JSON files in the `data/` directory:

```
data/
├── ships/frigates.json      # Ship definitions
├── modules/weapons.json     # Module definitions
├── skills/skills.json       # Skill definitions
├── npcs/pirates.json        # NPC definitions
├── missions/level1_missions.json  # Mission templates
└── universe/systems.json    # Solar system data
```

You can edit these files to add new content! See [DOCUMENTATION.md](DOCUMENTATION.md) for modding guides.

## Troubleshooting

**Port already in use:**
- Close any other programs using port 8765
- Or edit `server/server.py` to use a different port

**Can't connect:**
- Make sure server is running first
- Check firewall settings
- Verify host/port in client match server

**Data not loading:**
- Check that `data/` directory exists
- Verify JSON files are valid (use a JSON validator)

## Documentation

- **[README.md](README.md)** - Project overview
- **[DESIGN.md](DESIGN.md)** - Game design and EVE mechanics
- **[ASTROX_RESEARCH.md](ASTROX_RESEARCH.md)** - Astrox Imperium research
- **[DOCUMENTATION.md](DOCUMENTATION.md)** - Developer API docs

## Community

This is an open-source project. Contributions welcome!

- Report bugs via GitHub Issues
- Submit improvements via Pull Requests
- Share your mods!

## Credits

- Inspired by **EVE ONLINE** (CCP Games)
- Modding approach inspired by **Astrox Imperium** (Jace Masula)
- Built by the Nova Forge community

---

**Ready to explore the universe?** Run `python demo.py` to get started! 🚀
