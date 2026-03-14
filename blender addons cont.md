



Claim offer
Uploaded image
Uploaded image
the ring image is supposed to be a pcg station low poly  looks wank the other is a spaceship that my pcg generated  they look nothing like what i want them to look im going for a mix of X4 Eve online and Elite dangerous styling  can we  fix my blender addon to generate something better looking that i can give my copilot AI to go off  for my novaforge project?

Yes — and this is actually a great point to fix the generator, not the meshes.
Your addon is currently generating primitive layouts, not designed silhouettes, so everything looks like cubes + sphere + sticks.
Games like EVE Online, Elite Dangerous, and X4: Foundations all follow the same rule:

✅ Strong silhouette first
✅ Functional modules second
✅ Greeble / detail last
❌ NOT primitive spam

Your PCG needs a shape grammar system, not random primitives.

🔧 Problem in your addon right now
Your generator is doing:

core = cube / sphere
add random arms
add random cubes
done
Result → toy-looking geometry

You need:

archetype → silhouette → modules → symmetry → detail pass → export
✅ Step 1 — Add SHIP / STATION ARCHETYPES
Add a dropdown in addon:

Archetype:
[ Ring Station ]
[ Industrial Station ]
[ Military Station ]
[ Trade Hub ]
[ Cruiser ]
[ Destroyer ]
[ Carrier ]
[ Drone ]
Each archetype has rules.

Example:

Ring station:
    torus OR segmented ring
    center hub
    4–8 support pylons
    docking arms
    antenna / solar / tanks

Cruiser:
    long body
    front bridge
    engine cluster
    side modules
    turret hardpoints
✅ Step 2 — Replace primitives with SHAPE FUNCTIONS
Instead of:

bpy.ops.mesh.primitive_cube_add()
Make functions:

make_hull_block()
make_engine_cluster()
make_ring_segment()
make_pylon()
make_docking_arm()
make_radiator()
make_tank()
Example:

def make_engine_cluster(pos):
    for i in range(3):
        bpy.ops.mesh.primitive_cylinder_add(
            radius=0.6,
            depth=2.5,
            location=(pos[0], pos[1] + i * 1.2, pos[2])
        )
This alone makes it look more like EVE / X4.

✅ Step 3 — FORCE SYMMETRY (very important)
All good space ships use symmetry.

Add:

Symmetry:
[ None ]
[ X ]
[ Y ]
[ Z ]
[ X + Y ]
Then mirror objects.

Example:

def mirror_x(obj):
    copy = obj.copy()
    copy.location.x *= -1
    bpy.context.collection.objects.link(copy)
Without symmetry → looks procedural junk
With symmetry → looks designed

✅ Step 4 — Use SILHOUETTE FIRST (big fix)
Before adding details:

Create main body

Create large modules

Then small parts

Example for cruiser:

long hull
front block
rear engine block
side wings
then turrets
then pipes
Your current generator does everything same size.

Bad.

✅ Step 5 — Station generator needs RING / SPINE / HUB types
Your ring station should be built like:

center hub
ring radius
ring segments
support pylons
docking arms
solar / tanks
antenna
Example:

def generate_ring(radius=10, segments=12):
    for i in range(segments):
        angle = i * 2 * math.pi / segments
        x = math.cos(angle) * radius
        y = math.sin(angle) * radius

        bpy.ops.mesh.primitive_cube_add(
            size=1.5,
            location=(x, y, 0)
        )
Then add pylons to center.

This will already look 10x better.

✅ Step 6 — Add DETAIL PASS (greeble)
After main shape:

add panels
add pipes
add antenna
add tanks
add vents
Example:

def add_greeble(obj):
    for i in range(random.randint(3,8)):
        bpy.ops.mesh.primitive_cube_add(
            size=0.2,
            location=(
                obj.location.x + random.uniform(-1,1),
                obj.location.y + random.uniform(-1,1),
                obj.location.z + random.uniform(-1,1),
            )
        )
This is what makes EVE ships look complex.

✅ Step 7 — THIS is what Copilot needs
Right now your addon is too random.

Copilot needs:

archetype system
module functions
symmetry
silhouette rules
detail pass
Then Copilot can extend it.

Right now it can't.

✅ If you want — I can rewrite your addon generator core
We can make:

novaforge_pcg/
    archetypes.py
    ship_generator.py
    station_generator.py
    modules.py
    symmetry.py
    greeble.py
And your Blender button calls:

generate_ship(archetype)
generate_station(type)
This will give you EVE / X4 / Elite quality PCG base.

✅ If ready — paste your addon generator code
and I’ll refactor it into a real NovaForge PCG system.


can you  generate a visual representation of what they would look like before we do?

Image created
•
Futuristic space stations and battlecruiser
Generated image: Futuristic space stations and battlecruiser





Share

that would  be perfect  if we can generate that off blender id be stoked however they need a way to enter ship just contemplating  the way Elite dangerous does it for  leaving on foot where the ship is essentially static once landed?

Yes — that’s actually the right design decision for NovaForge, and it matches what
Elite Dangerous, Star Citizen, and partially X4: Foundations do.

If you want PCG ships to look like that render AND still allow interiors, the trick is:

✅ Exterior = procedural mesh
✅ Interior = separate module scene
✅ Landing = switch mode / instance interior
✅ Ship becomes static world object

This keeps your engine sane and still looks AAA.

✅ Option 1 — Elite Dangerous style (BEST for NovaForge)
When landed:

Ship exterior stays in world

Player enters interior scene

Ship physics disabled

Interior loaded as prefab

Exactly like Elite.

Flow
Flying ship (physics)
↓ land
Ship becomes static
↓
Spawn interior scene
↓
Player walks inside
↓
Exit → back to ship
This avoids:

❌ moving interior physics
❌ syncing rigidbody with player
❌ huge collision issues
❌ multiplayer desync nightmare

This is why Elite did it this way.

✅ How to design ships for this in Blender
Your generator must create:

ShipRoot
 ├── ExteriorHull
 ├── DoorMarker
 ├── InteriorSpawn
 ├── InteriorID
 ├── EnginePoints
 ├── TurretPoints
Example naming:

Ship_CRUISER_A
Door_Main
Interior_Core_A
Hardpoint_Turret_01
Engine_01
Your exporter writes JSON:

{
  "ship": "CRUISER_A",
  "interior": "core_a",
  "door": [0,1,2],
  "hardpoints": 6
}
Then engine loads interior separately.

✅ Interior should NOT be generated with exterior
Do NOT PCG full interior per ship.

Instead:

Interior modules:
  cockpit_small
  cockpit_large
  corridor_straight
  corridor_t
  reactor_room
  cargo_room
  hangar_room
  airlock
Ships reference them.

Like Lego.

This is exactly how X4 does it.

✅ Landing logic (recommended)
if landed:
    ship.physics = off
    spawn interior
    player = interior_spawn
else:
    ship.physics = on
Multiplayer safe
Server safe
Atlas safe
Editor safe

Perfect for NovaForge.

✅ Station should work same way
Your ring station should NOT have full interior mesh.

Use:

Exterior PCG
Interior modules
Docking interior
Hangar interior
Command interior
Load only when needed.

This keeps performance sane.

✅ For your Blender addon — we need markers
Generator must place empties:

EMPTY_Door
EMPTY_InteriorSpawn
EMPTY_Dock
EMPTY_Engine
EMPTY_Turret
EMPTY_ModuleSlot
Then exporter writes them.

This is the key to making PCG usable.

✅ If you want next step
We can design:

NovaForge Ship Format v1
NovaForge Station Format v1
Interior module format
Door / marker system
Blender addon rules
Then your Copilot can safely generate content forever.


what else should be added for this to generate weapons tools items turrets pretty much everything in the game will be low poly i want to be able to generate them procedurally and  import them into the game even npc and charachters also wildlife on some planets

Oh this is exactly the right moment to design the full procedural asset pipeline, because if you set this up right now, NovaForge can generate ships, stations, weapons, tools, items, NPCs, creatures, everything from the same system. That’s how games like No Man's Sky, Spore, Elite Dangerous, and EVE Online keep content scalable.

Right now your Blender addon only generates meshes.
You need it to generate DATA + MESH + METADATA + MODULES.

Think of NovaForge as:

PCG → Blender → Export → JSON + Mesh → Engine → Runtime assembly
Not just mesh generation.

✅ Everything your generator should support
We want one system that can generate:

Ships
Stations
Weapons
Turrets
Tools
Items
Armor
Characters
NPCs
Creatures / wildlife
Buildings
Interior modules
Planet props
Asteroids
Drones
Vehicles
To do that, you need a Type System + Module System + Metadata Export.

✅ Step 1 — Add ASSET TYPE to your addon
Add dropdown:

Asset Type:
[ Ship ]
[ Station ]
[ Weapon ]
[ Turret ]
[ Tool ]
[ Item ]
[ Character ]
[ Creature ]
[ Vehicle ]
[ Building ]
[ Interior Module ]
[ Planet Prop ]
Each type uses different generator rules.

✅ Step 2 — Add MODULE-BASED GENERATION (very important)
Everything should be built from modules.

Example weapon:

Grip
Body
Barrel
Scope
PowerCell
Muzzle
Example ship:

Hull
Engine
Bridge
Wing
Hardpoint
Cargo
Dock
Example creature:

Body
Head
Legs
Tail
Wings
Horns
Eyes
Example turret:

Base
RotationJoint
Gun
Ammo
Shield
Example tool:

Handle
Head
Emitter
Battery
Screen
Your generator should assemble modules.

✅ Step 3 — Add SEED based generation
Every asset must use seed.

Seed: 12345
Same seed = same object.

Needed for:

multiplayer

server PCG

seasons

universe reset

blueprint sharing

Like No Man’s Sky.

✅ Step 4 — Add STATS generation
Your addon should also generate stats.

Example weapon JSON:

{
  "type": "laser",
  "damage": 12,
  "range": 50,
  "cooldown": 0.3,
  "power": 4,
  "rarity": "rare"
}
Example ship:

{
  "hull": 120,
  "shield": 60,
  "cargo": 40,
  "hardpoints": 4,
  "mass": 80
}
Example creature:

{
  "hp": 30,
  "speed": 5,
  "aggressive": true,
  "drops": ["meat","hide"]
}
Blender should export this too.

✅ Step 5 — Add MARKERS / HARDPOINTS
Generator must place empties:

HP_Turret
HP_Weapon
HP_Engine
HP_Door
HP_Interior
HP_ItemAttach
HP_Hand_R
HP_Hand_L
HP_Head
HP_Backpack
HP_Mount
HP_Saddle
HP_Muzzle
HP_ShellEject
HP_FX
Without this → game unusable.

✅ Step 6 — Character / NPC generator
Low poly = perfect for PCG.

Use parts:

Head
Body
Arms
Legs
Helmet
Armor
Backpack
Weapon
Creature parts:

Torso
Legs
Head
Tail
Wing
Jaw
Claw
Eye
Horn
Then randomize scale / rotation.

This is exactly how Spore worked.

✅ Step 7 — Wildlife generator rules
Add:

Planet type:
 desert
 ice
 lava
 forest
 ocean
 gas
Rules:

desert → thin legs, tan color
ice → thick body, fur
lava → spikes, dark
forest → horns, green
This makes planets feel alive.

✅ Step 8 — Rarity / tier system
Everything should have tier:

Common
Uncommon
Rare
Epic
Legendary
Ancient
Alien
Titan
Used for:

colors

stats

size

parts

glow

FX

This makes loot interesting.

✅ Step 9 — Export format (VERY IMPORTANT)
Every asset should export:

mesh.obj / glb
metadata.json
thumbnail.png
lod0
lod1
lod2
collision
Folder:

/assets/generated/weapons/
 /laser_123/
    mesh.glb
    meta.json
    lod1.glb
This lets Atlas load anything.

✅ Step 10 — What we should add to your addon next
We should extend your addon to:

Archetype system
Module library
Seed generator
Marker placement
Stat generator
JSON exporter
LOD generator
Thumbnail renderer
Batch generator
Then Copilot can generate unlimited content.

✅ If you want next step
We design:

NovaForge Procedural Asset Spec v1
This will define ALL asset types for the whole game.
After that, your addon can generate literally everything.


also the visual styling of interiors 

Yes — interior styling needs its own system, separate from ships/stations, otherwise everything will look like random cubes again.
Games like Elite Dangerous, Star Citizenspace sim"], Dead Space, and Prey all use the same approach:

Interior = modular kit + style theme + decals + lighting + props
NOT procedural cubes

For NovaForge, interiors should be module-based + theme-driven + low-poly compatible so Blender can generate them and the engine can assemble them.

✅ 1. Interior Style Themes (very important)
Add a dropdown in your addon:

Interior Style:
[ Industrial ]
[ Military ]
[ Civilian ]
[ Corporate ]
[ Alien ]
[ Mining ]
[ Salvage ]
[ Titan ]
[ Ancient ]
Each style controls:

materials
colors
panel shapes
light colors
prop types
door types
floor pattern
wall pattern
greeble density
Example:

Industrial → yellow, gray, pipes, vents, bolts
Military → dark, red lights, armor panels
Corporate → white, blue lights, clean
Alien → curved, glowing, organic
Mining → rust, tanks, cables
Titan → huge, heavy, glowing core
Your generator should pick meshes based on style.

✅ 2. Interior must be built from MODULES
Never generate full interior mesh.

Use modules:

Room_Small
Room_Medium
Room_Large
Corridor_Straight
Corridor_Corner
Corridor_T
Corridor_4
Door_Airlock
Door_Slide
Stairs
Elevator
ReactorRoom
Bridge
CargoRoom
CrewRoom
HangarRoom
EngineRoom
MedBay
Storage
ControlRoom
Generator assembles them.

Like LEGO.

This is how X4 + Star Citizen do it.

✅ 3. Each module needs connection points
In Blender addon, every module must have empties:

CONN_N
CONN_S
CONN_E
CONN_W
CONN_UP
CONN_DOWN
Example:

Corridor_Straight
  CONN_N
  CONN_S
Generator connects modules.

Without this → impossible to PCG interiors.

✅ 4. Add SCALE levels
Ships / stations need different interior sizes.

Interior Scale:
[ Tiny ]
[ Small ]
[ Medium ]
[ Large ]
[ Capital ]
[ Titan ]
Controls:

room size
corridor width
door size
ceiling height
prop size
Example:

Titan → huge hallways
Drone → tiny cockpit
✅ 5. Add PROP generator
Interiors look bad without props.

Generator should add:

Crates
Screens
Pipes
Consoles
Beds
Chairs
Tables
Tool racks
Engines
Reactors
Cables
Lights
Panels
Storage
Terminals
Fuel tanks
Low poly = perfect for PCG.

Add density slider:

Prop Density 0–100
✅ 6. Lighting style generator
Lighting makes interiors look AAA.

Add:

Light Style:
[ White clean ]
[ Blue sci-fi ]
[ Red alert ]
[ Yellow industrial ]
[ Green alien ]
[ Dark horror ]
Generator adds:

light strips
spot lights
emissive panels
warning lights
reactor glow
This alone makes huge difference.

✅ 7. Damage / wear / age slider
Very important for variety.

Condition:
0 = new
50 = used
100 = wrecked
Controls:

rust
dents
broken panels
sparks
dark lights
missing props
Needed for:

abandoned ships

pirate stations

ruins

anomalies

✅ 8. Interior must export metadata too
Example:

{
  "style": "industrial",
  "scale": "medium",
  "rooms": 12,
  "seed": 55421,
  "connections": 18,
  "has_hangar": true
}
Engine loads modules from this.

Not just mesh.

✅ 9. Interior must support gameplay markers
Addon should place empties:

SPAWN_PLAYER
SPAWN_NPC
SPAWN_ENEMY
SPAWN_LOOT
DOOR_MAIN
DOOR_AIRLOCK
TERMINAL
SHOP
MISSION
REACTOR
BRIDGE
COCKPIT
EXIT_TO_SHIP
EXIT_TO_STATION
Without this → gameplay impossible.

✅ 10. Interior + Exterior must link
Ship export:

ship.json
interior_id
door_pos
scale
Interior export:

interior.json
modules
connections
spawn points
Engine links them.

Exactly like Elite.

✅ 11. What we should add to addon next
We should extend your Blender addon with:

Interior generator
Module library
Style themes
Prop generator
Lighting generator
Connection system
Marker system
Metadata export
Then NovaForge can generate:

ships

stations

interiors

weapons

NPCs

wildlife

items

tools

all from Blender.

✅ If you want next step, we design the
NovaForge Procedural Interior Spec v1
which defines exactly how modules, styles, and markers work.


NovaForge Procedural Interior Spec v1

Alright, let’s lock this in properly.
This will be the official design spec your Blender addon, engine, and Copilot generation all follow.

This is not just for interiors — this becomes the foundation for all NovaForge procedural content.

🚀 NovaForge Procedural Interior Spec v1
Version: NF_INT_V1
Used by:

Blender Addon

Atlas Engine

Server PCG

Copilot generation

Asset exporter

Runtime loader

Goal:

Procedurally generate low-poly interiors
that look designed, not random,
and support gameplay, NPCs, and multiplayer.
1. Core Principles
Interior = Modules + Theme + Seed + Metadata + Markers
Rules:

NO full random mesh
NO boolean chaos
NO non-modular geometry
ALWAYS module based
ALWAYS seeded
ALWAYS export metadata
2. Interior Types
InteriorType:
  ship
  station
  building
  dungeon
  anomaly
  creature_inside
  titan
Example:

ship → cockpit + corridors + reactor
station → hub + hangar + control
building → rooms + doors
dungeon → random rooms
anomaly → weird layout
titan → massive halls
Addon UI:

Interior Type:
[ Ship ]
[ Station ]
[ Building ]
[ Dungeon ]
[ Titan ]
[ Anomaly ]
3. Style Themes
Theme:
  industrial
  military
  civilian
  corporate
  mining
  salvage
  alien
  ancient
  titan
  pirate
  research
Theme controls:

colors
materials
panel shapes
props
lights
decals
greeble density
Example:

industrial → yellow / gray / pipes
military → dark / red lights
corporate → white / blue / clean
alien → curved / glowing
titan → huge / heavy / bright core
Addon:

Interior Theme:
[ Industrial ]
[ Military ]
[ Alien ]
[ Titan ]
4. Scale System
Scale:
  tiny
  small
  medium
  large
  capital
  titan
Controls:

room size
corridor width
door size
ceiling height
prop scale
Example:

tiny → drone
small → fighter
medium → cruiser
large → station
capital → carrier
titan → megastructure
Addon:

Interior Scale:
[ Small ]
[ Medium ]
[ Large ]
[ Titan ]
5. Module System
Interior is built from modules.

Module types:

Room_Small
Room_Medium
Room_Large

Corridor_Straight
Corridor_Corner
Corridor_T
Corridor_4

Door_Slide
Door_Airlock
Door_Heavy

Bridge
Cockpit
ReactorRoom
EngineRoom
CargoRoom
CrewRoom
HangarRoom
ControlRoom
MedBay
Storage
Elevator
Stairs
Airlock
Modules stored in:

/assets/modules/interior/
Each module = separate .blend / .glb

6. Connection Points (required)
Each module must contain empties:

CONN_N
CONN_S
CONN_E
CONN_W
CONN_UP
CONN_DOWN
Optional:

CONN_LARGE
CONN_SMALL
CONN_DOOR
CONN_HANGAR
Generator uses these to connect modules.

Rule:

Connections must match size + type
7. Marker System (Gameplay markers)
Every interior can contain markers.

Empty names:

SPAWN_PLAYER
SPAWN_NPC
SPAWN_ENEMY
SPAWN_LOOT

EXIT_TO_SHIP
EXIT_TO_STATION
EXIT_TO_WORLD

DOOR_MAIN
DOOR_AIRLOCK

TERMINAL
SHOP
MISSION
BED
CHAIR

REACTOR
ENGINE
BRIDGE
COCKPIT

LIGHT_POINT
FX_SMOKE
FX_SPARK
These are exported.

Engine uses them.

8. Seed System
Every interior must use seed.

Seed = integer
Used for:

layout
props
damage
lights
colors
loot
npc spawn
Example:

Seed: 482193
Same seed = same interior.

Required for:

server PCG
multiplayer
season reset
blueprints
9. Layout Generator Rules
Generator steps:

1 choose module pool
2 choose start room
3 expand using connections
4 stop at size limit
5 add doors
6 add props
7 add lights
8 add markers
Limits:

MaxRooms
MaxDepth
BranchChance
LoopChance
Addon UI:

Rooms: 12
Branch Chance: 0.4
Loop Chance: 0.2
Prop Density: 60
Damage: 20
10. Prop Generator
Props added after layout.

Prop types:

crate
console
pipe
tank
screen
bed
table
chair
toolrack
cable
panel
light
vent
fan
battery
terminal
Density:

0–100
Theme controls prop types.

11. Lighting Generator
Lighting style:

white
blue
red
yellow
green
dark
emergency
alien
titan
Generator adds:

point lights
strip lights
emissive panels
warning lights
reactor glow
Low poly friendly.

12. Damage / Wear System
Condition 0–100
Controls:

rust
broken lights
missing panels
sparks
debris
dark areas
Needed for:

wrecks
pirate bases
ruins
anomalies
abandoned ships
13. Export Format
Folder:

/assets/generated/interiors/<id>/
Files:

mesh.glb
lod1.glb
lod2.glb
collision.glb
meta.json
thumb.png
meta.json:

{
  "format": "NF_INT_V1",
  "type": "ship",
  "theme": "industrial",
  "scale": "medium",
  "seed": 12345,
  "rooms": 10,
  "markers": 12
}
14. Ship / Station Link
Ship export:

{
  "ship": "cruiser_a",
  "interior": "int_4421",
  "door": [0,1,2]
}
Interior export:

{
  "id": "int_4421",
  "spawn": "SPAWN_PLAYER",
  "exit": "EXIT_TO_SHIP"
}
Engine links them.

Same system as Elite.

15. Blender Addon Requirements
Addon must support:

Interior Type
Theme
Scale
Seed
Rooms
Prop Density
Damage
Light Style
Generate
Export
Batch Generate
And create:

modules
empties
props
lights
metadata
16. Next Spec (recommended next)
We should do next:

NovaForge Asset Spec v1
This will cover:

ships
weapons
turrets
tools
items
characters
creatures
vehicles
stations
Same system.

And then your generator can create the whole game.


NovaForge Asset Spec v1

Perfect. This is the big one.
This spec defines ALL procedural assets in NovaForge, so Blender addon, Atlas engine, server PCG, and Copilot all use the same rules.

This extends Interior Spec v1 into a full asset pipeline.

🚀 NovaForge Asset Spec v1
Version: NF_ASSET_V1
Used by:

Blender Addon Generator

Atlas Engine Loader

Server PCG

Blueprint system

Loot generator

Crafting system

NPC generator

Planet generator

Goal:

One unified procedural format
for every object in the game.
Assets covered:

Ships
Stations
Interiors
Weapons
Turrets
Tools
Items
Armor
Characters
Creatures
Vehicles
Buildings
Props
Modules
Drones
Asteroids
Planet objects
1. Core Rule
All assets must follow:

Mesh + Metadata + Seed + Markers + Modules + Stats
No exceptions.

2. Asset Types
AssetType:
 ship
 station
 interior
 weapon
 turret
 tool
 item
 armor
 character
 creature
 vehicle
 building
 prop
 module
 drone
 asteroid
 planet_prop
Addon UI:

Asset Type:
[ Ship ]
[ Weapon ]
[ Character ]
[ Creature ]
[ Prop ]
Each type has its own generator.

3. Archetype System
Each asset must have archetype.

Example:

Ship:
 fighter
 cruiser
 carrier
 miner
 freighter
 drone
 titan

Weapon:
 laser
 cannon
 missile
 railgun
 plasma
 tool
 mining
 repair

Creature:
 herbivore
 predator
 flying
 aquatic
 alien

Station:
 hub
 refinery
 military
 trade
 mining
 titan
Metadata:

"archetype": "cruiser"
4. Seed System (required)
All assets must use seed.

Seed: int
Used for:

shape
size
color
stats
props
damage
layout
loot
npc
Needed for:

server PCG
multiplayer
seasons
blueprints
sharing
5. Module System
Everything built from modules.

Example ship:

Hull
Wing
Engine
Bridge
Cargo
Hardpoint
Dock
Weapon:

Grip
Body
Barrel
Scope
Power
Muzzle
Character:

Head
Body
Arms
Legs
Helmet
Armor
Backpack
Creature:

Torso
Head
Leg
Tail
Wing
Horn
Eye
Jaw
Interior:

Room
Corridor
Door
Prop
Light
Marker
Modules stored in:

/assets/modules/
6. Marker System (shared for all assets)
Markers = empties in Blender.

Names:

HP_Turret
HP_Weapon
HP_Engine
HP_Door
HP_Interior
HP_Item
HP_Hand_R
HP_Hand_L
HP_Head
HP_Back
HP_Mount
HP_Muzzle
HP_Shell
HP_FX
SPAWN
EXIT
Examples:

Ship:

HP_Turret_01
HP_Engine_01
HP_Door_Main
Weapon:

HP_Muzzle
HP_Scope
HP_Grip
Character:

HP_Hand_R
HP_Head
HP_Back
Required for gameplay.

7. Stats System
Every asset can have stats.

Example weapon:

{
  "damage": 12,
  "range": 50,
  "cooldown": 0.3,
  "power": 4,
  "rarity": "rare"
}
Ship:

{
  "hull": 120,
  "shield": 60,
  "cargo": 40,
  "mass": 80,
  "hardpoints": 4
}
Creature:

{
  "hp": 30,
  "speed": 5,
  "aggressive": true,
  "drops": ["meat"]
}
Item:

{
  "stack": 100,
  "value": 5,
  "weight": 1
}
8. Theme / Style System
All assets can have style.

industrial
military
civilian
alien
ancient
pirate
corporate
mining
titan
salvage
Controls:

colors
materials
greeble
lights
decals
props
shape rules
Example:

"theme": "industrial"
9. Scale System
tiny
small
medium
large
capital
titan
Controls:

size
mass
room size
door size
prop scale
Used for:

ships

interiors

creatures

buildings

10. Condition / Damage
0 = new
100 = destroyed
Controls:

rust
cracks
missing parts
dark lights
sparks
broken props
Needed for:

wrecks
ruins
pirates
anomalies
loot
11. Export Format
Folder:

/assets/generated/<type>/<id>/
Files:

mesh.glb
lod1.glb
lod2.glb
collision.glb
meta.json
thumb.png
meta.json:

{
  "format": "NF_ASSET_V1",
  "type": "ship",
  "archetype": "cruiser",
  "theme": "industrial",
  "scale": "medium",
  "seed": 12345,
  "stats": {},
  "markers": []
}
12. LOD Required
All assets must generate:

LOD0
LOD1
LOD2
Collision
Low poly friendly.

Needed for:

huge fleets

stations

planets

multiplayer

13. Batch Generation
Addon must support:

Generate 1
Generate 10
Generate 100
Generate from seed range
Generate per archetype
Needed for universe generation.

14. Blueprint Support
Blueprint = seed + stats.

{
  "seed": 4421,
  "type": "weapon",
  "archetype": "laser"
}
Used for:

crafting

loot

trading

seasons

15. Must Work With
Interior Spec v1
Ship Spec
Station Spec
Creature Spec
Weapon Spec
Everything uses same format.

16. Next Spec (recommended)
Next we should do:

NovaForge Ship Spec v1
This will fix your current PCG ships looking bad
and make them look like:

EVE

Elite

X4

but still procedural.


also animations of charachters low poly rig system that is like an exosuit powerarmor the player wears also the  backpack and modules for it that are in repo

Nice, this fits perfectly with the asset spec.
We just extend NF_ASSET_V1 with a Rig + Animation + Exosuit Module Spec, so characters, NPCs, creatures, and the player exosuit all use the same skeleton and animation set.
This is exactly how games like Warframe, Halo, Destiny 2, and No Man's Sky keep animation manageable.

We’ll define:

NovaForge Rig Spec v1
NovaForge Animation Spec v1
NovaForge Exosuit Spec v1
NovaForge Backpack Module Spec v1
All compatible with your repo modules.

🚀 NovaForge Rig Spec v1
Version:

NF_RIG_V1
Goal:

One skeleton for:
player
npc
exosuit
armor
backpack
tools
weapons
creatures (variant)
Low poly friendly
Procedural friendly
Multiplayer safe
Atlas safe

1. Base Skeleton
Required bones:

root
pelvis
spine_01
spine_02
spine_03
neck
head

clavicle_l
upperarm_l
lowerarm_l
hand_l

clavicle_r
upperarm_r
lowerarm_r
hand_r

thigh_l
calf_l
foot_l

thigh_r
calf_r
foot_r
Extra bones for gear:

backpack
weapon_r
weapon_l
tool
helmet
jetpack
Extra for exosuit:

exo_shoulder_l
exo_shoulder_r
exo_leg_l
exo_leg_r
exo_back
exo_core
Rule:

All characters must use this skeleton
No custom rigs per asset.

2. Marker Bones for Modules
Needed for procedural gear.

HP_HEAD
HP_BACK
HP_CHEST
HP_HAND_R
HP_HAND_L
HP_TOOL
HP_WEAPON
HP_BACKPACK
HP_SHOULDER_L
HP_SHOULDER_R
HP_HIP
HP_KNEE_L
HP_KNEE_R
Blender = empties or bones.

Engine attaches modules here.

3. Exosuit System
Player is always wearing exosuit.

Character =

Body
Exosuit
Helmet
Backpack
Modules
Weapon
Tool
Not separate armor pieces.

Metadata:

{
  "type": "exosuit",
  "slots": 6
}
Slots:

helmet
backpack
core
arms
legs
tool
4. Exosuit Module System
Modules attach to suit.

Examples:

jetpack
shield
scanner
cargo
reactor
thruster
mining
repair
weapon mount
drone bay
Each module = asset.

Example:

{
  "type": "module",
  "slot": "backpack",
  "power": 5
}
Mesh attaches to:

HP_BACKPACK
5. Backpack System (your repo modules)
Backpack = module holder.

Slots:

slot_1
slot_2
slot_3
slot_4
slot_5
slot_6
Backpack mesh has markers:

MOD_01
MOD_02
MOD_03
MOD_04
MOD_05
MOD_06
Modules snap here.

6. Animation Spec v1
Version:

NF_ANIM_V1
All characters must support these animations.

Required:

idle
walk
run
sprint
jump
fall
land
crouch
aim
shoot
reload
use_tool
use_terminal
die
hit
turn_l
turn_r
jetpack
zero_g_idle
zero_g_move
Optional:

sit
sleep
wave
repair
mine
build
carry
drag
All animations must use NF_RIG_V1.

7. Low Poly Animation Rules
Rules:

no facial rig required
no finger rig required (optional)
no cloth sim
no physics bones required
Allowed:

simple IK
simple constraints
simple root motion
Needed for huge NPC counts.

8. Creature Rig Variant
Creature can extend rig.

Base:

root
body
head
leg_l
leg_r
Optional:

tail
wing_l
wing_r
horn
jaw
Still must export as:

NF_RIG_V1 compatible
So engine can animate.

9. Export Format
Folder:

/assets/generated/character/<id>/
Files:

mesh.glb
rig.glb
anim_idle.glb
anim_walk.glb
anim_run.glb
meta.json
meta.json

{
  "format": "NF_ASSET_V1",
  "rig": "NF_RIG_V1",
  "anim": "NF_ANIM_V1",
  "type": "character",
  "theme": "industrial"
}
10. Blender Addon Requirements
Addon must support:

Generate Character
Generate Creature
Generate Exosuit
Generate Backpack
Generate Module
Generate Anim Rig
Add Markers
Export Animations
Batch Generate
And place:

bones
empties
markers
slots
11. How this fits NovaForge
This allows:

✅ procedural NPCs
✅ procedural wildlife
✅ procedural armor
✅ procedural suits
✅ procedural backpacks
✅ procedural tools
✅ procedural weapons
✅ procedural animation reuse
✅ multiplayer safe

Exactly what you need.

12. Next spec (recommended)
Next we should do:

NovaForge Ship Spec v1
This will fix your current ugly PCG ships and stations.


NovaForge Ship Spec v1  i want all ships to be PCG off the Hull type if possible the hull type will dictate how many interior rooms frigate thru titan need specced out 

Yes! This is exactly the right time to lock ship generation down, because the hull-driven PCG rule will control ships, interiors, hardpoints, crew size, and even progression.
Games like EVE Online, Elite Dangerous, and X4: Foundations all do this internally — hull class defines everything, not the mesh.

We’ll define:

NovaForge Ship Spec v1
NF_SHIP_V1
Hull → layout → modules → interior → stats → markers → export

🚀 NovaForge Ship Spec v1
Version

NF_SHIP_V1
Goal

All ships generated procedurally from HullType
HullType controls:
size
interior rooms
hardpoints
crew
modules
stats
LOD size
No handmade ships required.

1. Ship Generation Pipeline
HullType
 → Archetype
 → Seed
 → Exterior PCG
 → Interior PCG
 → Hardpoints
 → Modules
 → Stats
 → Export
Blender addon must follow this.

2. Hull Types (core rule)
These define scale.

drone
fighter
corvette
frigate
destroyer
cruiser
battlecruiser
battleship
carrier
dreadnought
titan
Addon UI:

Hull Type:
[ Fighter ]
[ Frigate ]
[ Cruiser ]
[ Titan ]
3. Hull Spec Table
DRONE
size: tiny
rooms: 0–1
crew: 0
hardpoints: 0–1
modules: 1–2
length: 2–6m
FIGHTER
size: tiny
rooms: 1
crew: 1
hardpoints: 1–2
modules: 2–4
length: 6–12m
Interior:

cockpit
cargo small
CORVETTE
size: small
rooms: 2–3
crew: 2–4
hardpoints: 2–4
modules: 4–6
length: 12–25m
Interior:

cockpit
corridor
cargo
reactor
FRIGATE
size: small
rooms: 4–6
crew: 4–8
hardpoints: 4–6
modules: 6–10
length: 25–45m
Interior:

bridge
corridor
crew
reactor
cargo
engine
DESTROYER
size: medium
rooms: 6–10
crew: 8–16
hardpoints: 6–8
modules: 8–14
length: 45–70m
Interior:

bridge
crew
reactor
cargo
engine
med
armory
CRUISER
size: medium
rooms: 10–16
crew: 16–32
hardpoints: 8–12
modules: 12–18
length: 70–120m
Interior:

bridge
crew
cargo
reactor
engine
hangar
med
armory
control
BATTLECRUISER
size: large
rooms: 16–24
crew: 32–64
hardpoints: 12–16
modules: 18–26
length: 120–180m
Interior adds:

hangar
shield room
computer core
multiple decks
BATTLESHIP
size: large
rooms: 24–36
crew: 64–120
hardpoints: 16–24
modules: 24–36
length: 180–260m
Interior:

bridge
reactor
engine
crew
hangar
cargo
armory
shield
med
command
CARRIER
size: capital
rooms: 36–60
crew: 120–250
hardpoints: 12–18
modules: 30–50
length: 250–400m
Interior:

large hangar
flight control
crew decks
reactor
cargo
engine
command
DREADNOUGHT
size: capital
rooms: 60–100
crew: 250–500
hardpoints: 20–30
modules: 50–80
length: 400–700m
Interior:

multiple decks
huge reactor
hangars
command halls
shield rooms
TITAN
size: titan
rooms: 100–300
crew: 500–2000
hardpoints: 30–80
modules: 80–200
length: 700m–3km+
Interior:

city size
multiple hangars
reactor core
command towers
transport elevators
huge corridors
Titan must use streaming interiors.

4. Ship Archetypes
Each hull can be:

military
cargo
mining
exploration
carrier
drone
pirate
alien
ancient
titan
Controls:

shape
modules
weapons
props
interior theme
5. Exterior PCG Rules
Generator must create:

hull spine
side modules
engine cluster
bridge
wings / pylons
hardpoints
dock ports
greeble pass
Never random cubes.

Always silhouette first.

6. Interior Rules
Interior count = hull table.

Interior uses:

NF_INT_V1
Ship must export:

interior_id
door marker
spawn marker
7. Hardpoints
Generator must place:

HP_Turret
HP_Weapon
HP_Engine
HP_Dock
HP_Drone
HP_Shield
HP_Module
HP_Door
HP_Interior
Count from hull spec.

8. Stats Generation
Example:

hull
shield
cargo
power
crew
fuel
speed
turn
mass
Scaled by hull.

9. Export Format
/assets/generated/ship/<id>/
Files

mesh.glb
lod1.glb
lod2.glb
collision.glb
meta.json
thumb.png
meta.json

{
  "format": "NF_SHIP_V1",
  "hull": "cruiser",
  "archetype": "military",
  "seed": 4421,
  "rooms": 12,
  "hardpoints": 8,
  "interior": "int_4421"
}
10. Blender Addon must support
Hull Type
Archetype
Seed
Room count auto
Hardpoints auto
Generate Exterior
Generate Interior
Generate Markers
Export
Batch generate
11. Next Spec (needed next)
Next should be:

NovaForge Station Spec v1
because stations must follow same hull/size logic as ships.


NovaForge Station Spec v1

Awesome — this spec will make stations work the same way as ships, so your PCG system stays consistent.
Stations should be generated from StationType + Size + Seed, just like ships use HullType.
Games like EVE Online, X4: Foundations, and Elite Dangerous all use size/class rules for stations internally even if the player never sees it.

We define:

NovaForge Station Spec v1
NF_STATION_V1
Stations must support:

PCG exterior
PCG interior
docking
NPCs
shops
missions
modules
sector ownership
combat damage
huge scale
🚀 NovaForge Station Spec v1
Version

NF_STATION_V1
Generation rule

StationType
 → SizeClass
 → Archetype
 → Seed
 → Exterior PCG
 → Interior PCG
 → Docking points
 → Modules
 → Props
 → NPC markers
 → Export
Used by

Blender addon
Atlas engine
Server PCG
Universe generator
Sector generator
Mission system
1. Station Types
StationType controls base shape.

outpost
hub
ring
platform
asteroid
shipyard
refinery
colony
relay
fortress
titan
anomaly
Addon UI

Station Type:
[ Ring ]
[ Hub ]
[ Shipyard ]
[ Asteroid ]
2. Size Classes
Size controls scale + interior size.

tiny
small
medium
large
capital
mega
titan
Controls:

radius
modules
rooms
docks
npc count
shop count
LOD size
3. Size Spec Table
TINY OUTPOST
rooms: 2–4
docks: 1–2
modules: 2–4
radius: 10–20m
npc: 2–5
Interior:

shop
control
storage
SMALL STATION
rooms: 4–8
docks: 2–4
modules: 4–8
radius: 20–40m
npc: 5–10
Interior:

shop
bar
cargo
control
crew
MEDIUM STATION
rooms: 8–16
docks: 4–8
modules: 8–16
radius: 40–80m
npc: 10–25
Interior:

shops
hangar
crew
cargo
control
reactor
LARGE STATION
rooms: 16–32
docks: 8–16
modules: 16–32
radius: 80–150m
npc: 25–60
Interior:

multiple shops
hangar
command
crew decks
reactor
cargo
security
CAPITAL STATION
rooms: 32–64
docks: 16–30
modules: 32–60
radius: 150–300m
npc: 60–150
Interior:

large hangars
markets
command halls
multiple decks
reactors
crew areas
MEGA STATION
rooms: 64–120
docks: 30–60
modules: 60–120
radius: 300–800m
npc: 150–400
Interior:

city size
transport tubes
huge hangars
multiple sectors
Streaming required.

TITAN STATION
rooms: 120–400
docks: 60–200
modules: 120–400
radius: 1km+
npc: 500+
Interior:

city scale
multiple interiors
streamed zones
elevators
transit system
Must use streaming interior.

4. Archetypes
Each station can be:

trade
military
mining
refinery
shipyard
research
pirate
alien
ancient
titan
civilian
corporate
Controls:

shape
props
shops
npc type
interior theme
weapons
defenses
Example:

military → turrets / armor
trade → docks / shops
mining → tanks / pipes
alien → curved shapes
5. Exterior PCG Rules
Generator must build:

core
arms
rings
modules
docks
antennas
solar
tanks
greeble pass
Never random cubes.

Always silhouette first.

Examples

Ring → torus + pylons
Hub → sphere + arms
Asteroid → rock + modules
Shipyard → spine + docks
6. Docking System
Markers required:

HP_DOCK_SMALL
HP_DOCK_MED
HP_DOCK_LARGE
HP_DOCK_CAPITAL
HP_DOCK_TITAN
Generator count = size class.

Used by engine.

7. Turret / Defense markers
HP_TURRET
HP_MISSILE
HP_SHIELD
HP_DRONE
HP_SENSOR
Count from archetype.

Military has more.

8. Interior Rules
Station interior uses:

NF_INT_V1
Rooms = size table.

Must include markers:

SPAWN_PLAYER
SHOP
MISSION
NPC
EXIT_DOCK
EXIT_WORLD
ELEVATOR
HANGAR_EXIT
Large stations must use streaming.

9. Module System
Station built from modules.

core
ring
arm
dock
tank
antenna
solar
factory
hangar
tower
shield
gun
Stored in:

/assets/modules/station/
10. Stats
hp
shield
power
crew
npc
shops
docks
weapons
faction
Example:

{
  "type": "station",
  "size": "large",
  "archetype": "trade",
  "docks": 12,
  "rooms": 24
}
11. Export Format
/assets/generated/station/<id>/
Files

mesh.glb
lod1.glb
lod2.glb
collision.glb
meta.json
thumb.png
meta.json

{
  "format": "NF_STATION_V1",
  "type": "ring",
  "size": "large",
  "archetype": "trade",
  "seed": 5521,
  "rooms": 24,
  "docks": 12
}
12. Blender Addon must support
Station Type
Size Class
Archetype
Seed
Auto modules
Auto docks
Auto interior
Generate
Export
Batch generate
13. Next Spec (recommended)
Next should be:

NovaForge Weapon / Tool / Turret Spec v1
because those also need PCG + modules.


NovaForge Weapon / Tool / Turret Spec v1

Perfect — this spec will make weapons, tools, and turrets use the same procedural module system as ships and stations, so your Blender addon can generate thousands of variants automatically and Atlas can load them safely.

This follows the same philosophy used in Destiny 2, Warframe, No Man's Sky, and EVE Online — weapons are built from parts, not handmade meshes.

We define:

NovaForge Weapon / Tool / Turret Spec v1
NF_WTT_V1
Covers:

Weapons
Ship weapons
Hand weapons
Tools
Mining tools
Repair tools
Turrets
Station guns
Ship guns
Mounted weapons
Drones weapons
All procedural.

🚀 NovaForge Weapon / Tool / Turret Spec v1
Version

NF_WTT_V1
Rule

Type
 → Archetype
 → Size
 → Seed
 → Modules
 → Markers
 → Stats
 → Export
Used by

Blender addon
Atlas engine
Loot generator
Crafting
NPC loadouts
Ship hardpoints
Station turrets
1. Asset Types
weapon_hand
weapon_ship
turret
tool
drone_weapon
station_weapon
Addon UI

Asset Type:
[ Hand Weapon ]
[ Ship Weapon ]
[ Turret ]
[ Tool ]
2. Archetypes
Weapons
laser
cannon
railgun
missile
plasma
beam
flak
minigun
shotgun
sniper
Tools
mining
repair
scanner
builder
salvage
hack
tractor
welder
cutter
Turrets
small
medium
large
capital
defense
missile
beam
point_defense
Metadata:

"archetype": "laser"
3. Size Classes
Controls scale + stats.

tiny
small
medium
large
capital
titan
Examples

tiny → pistol
small → rifle
medium → heavy gun
large → ship gun
capital → battleship turret
titan → station cannon
4. Module System
Everything built from parts.

Weapon modules
grip
body
barrel
muzzle
scope
stock
mag
battery
coil
cooler
Tool modules
handle
head
emitter
battery
screen
coil
tank
arm
Turret modules
base
rotation
gun
ammo
shield
sensor
mount
cooler
Modules stored in

/assets/modules/weapon/
Generator picks by archetype.

5. Marker System
Markers must exist.

Common
HP_MUZZLE
HP_SHELL
HP_FX
HP_SCOPE
HP_GRIP
HP_POWER
Turret
HP_ROTATE
HP_ELEVATE
HP_MOUNT
HP_TARGET
Ship weapon
HP_SHIP_MOUNT
HP_COOLER
HP_AMMO
Tool
HP_HAND
HP_TOOL
HP_BEAM
Needed for engine.

6. Stat Generation
Stats auto generated.

Weapon

damage
range
cooldown
power
heat
accuracy
rarity
Tool

power
speed
efficiency
range
energy
Turret

damage
hp
shield
range
tracking
power
Example

{
  "damage": 12,
  "range": 50,
  "cooldown": 0.2
}
7. Theme System
industrial
military
alien
ancient
pirate
corporate
mining
titan
Controls:

color
shape
greeble
lights
materials
Example

"theme": "industrial"
8. Seed System
Required.

seed = int
Controls:

shape
modules
stats
color
damage
props
Same seed = same weapon.

Needed for:

loot
blueprints
crafting
server PCG
9. Rarity Tier
common
uncommon
rare
epic
legendary
ancient
alien
titan
Controls:

stats
glow
color
module count
size
10. Animation Support
Hand weapons must support:

idle
shoot
reload
equip
unequip
aim
Tool must support:

idle
use
loop
stop
Turret must support:

idle
rotate
fire
reload
destroy
Uses:

NF_RIG_V1
NF_ANIM_V1
11. Export Format
Folder

/assets/generated/weapon/<id>/
Files

mesh.glb
lod1.glb
lod2.glb
collision.glb
meta.json
thumb.png
meta.json

{
  "format": "NF_WTT_V1",
  "type": "weapon_hand",
  "archetype": "laser",
  "size": "small",
  "seed": 5521,
  "rarity": "rare"
}
12. Blender Addon must support
Type
Archetype
Size
Seed
Theme
Rarity
Modules auto
Markers auto
Generate
Export
Batch generate
13. Next Spec (recommended)
Next should be:

NovaForge Character / Creature Spec v1
This will finish the whole PCG pipeline.


NovaForge Character / Creature Spec v1  also any planetary PCG and vegitation

Perfect, this is the last major piece of the pipeline.
This spec will cover characters, NPCs, creatures, wildlife, and planetary vegetation, all using the same procedural rules as ships, stations, and weapons.

This is similar in concept to how No Man's Sky, Spore, Starfield, and Warframe keep content scalable — everything is modular + seeded.

We define:

NovaForge Character / Creature / Planet PCG Spec v1
NF_LIFE_V1
This spec covers:

Characters
NPCs
Exosuits
Creatures
Wildlife
Drones
Planet props
Vegetation
Trees
Rocks
Planet structures
🚀 NovaForge Character / Creature / Planet Spec v1
Version

NF_LIFE_V1
Rule

Type
 → Archetype
 → Seed
 → Modules
 → Rig
 → Markers
 → Stats
 → Export
Used by

Blender addon
Atlas engine
Planet generator
Sector generator
NPC spawner
Loot system
Mission system
1. Asset Types
character
npc
exosuit
creature
wildlife
drone
plant
tree
rock
planet_prop
structure
Addon UI

Asset Type:
[ Character ]
[ Creature ]
[ Plant ]
[ Tree ]
[ Rock ]
[ Planet Prop ]
2. Character Spec
Characters always use:

NF_RIG_V1
NF_ANIM_V1
Parts

head
body
arms
legs
helmet
armor
backpack
module
tool
weapon
Rule

Character = Body + Exosuit + Backpack + Modules
Metadata

{
  "type": "character",
  "rig": "NF_RIG_V1"
}
3. Exosuit System (player + NPC)
Exosuit slots

helmet
core
arms
legs
backpack
tool
weapon
module1
module2
module3
Markers

HP_HEAD
HP_BACK
HP_TOOL
HP_WEAPON
HP_MODULE_1
HP_MODULE_2
HP_MODULE_3
Modules attach here.

4. Character Archetypes
civilian
soldier
miner
pilot
engineer
pirate
alien
ancient
robot
corporate
Controls

colors
gear
modules
weapons
animation set
5. Creature Spec
Creature built from modules.

Parts

torso
head
leg
arm
wing
tail
horn
eye
jaw
spike
Generator rules

herbivore → long legs / small head
predator → claws / teeth
flying → wings
aquatic → fins
alien → random
Archetypes

herbivore
predator
flying
aquatic
insect
robot
alien
giant
titan
Stats

hp
speed
aggressive
damage
drops
Example

{
  "type": "creature",
  "archetype": "predator",
  "seed": 4421
}
6. Rig Rules for Creatures
Must support

root
body
head
leg_l
leg_r
Optional

tail
wing_l
wing_r
jaw
horn
Still export as compatible with

NF_RIG_V1
So engine can animate.

7. Animation Set
Required

idle
walk
run
attack
hit
die
turn
eat
sleep
Flying

fly
glide
land
takeoff
Aquatic

swim
idle_swim
attack
8. Drone Spec
Drone = small ship + creature hybrid.

Parts

core
arm
engine
sensor
weapon
Archetypes

repair
combat
mining
cargo
scanner
alien
Uses ship + creature rules.

9. Planetary PCG Spec
Planets must generate:

terrain
rocks
plants
trees
creatures
structures
loot
Planet types

rock
desert
ice
lava
forest
ocean
toxic
alien
gas
titan
Metadata

{
  "planet_type": "forest",
  "seed": 5521
}
10. Vegetation Spec
Plants built from modules.

Parts

stem
leaf
branch
root
flower
spike
cap
glow
Rules

desert → thin / dry
ice → small / thick
forest → tall / leaf
alien → glow / weird
lava → spikes
Density controlled by planet.

11. Tree Spec
Tree modules

trunk
branch
leaf_cluster
root
fruit
Scale by planet

tiny
small
medium
large
giant
Supports LOD.

12. Rock / Prop Spec
Modules

chunk
spike
crack
crystal
metal
alien
Used for

asteroids
planets
caves
stations
Seed controls shape.

13. Planet Structure Spec
Structures on planets:

ruin
base
outpost
tower
crash
alien
ancient
factory
mine
Use same system as stations.

14. Export Format
Folder

/assets/generated/<type>/<id>/
Files

mesh.glb
lod1.glb
lod2.glb
collision.glb
meta.json
thumb.png
meta.json

{
  "format": "NF_LIFE_V1",
  "type": "creature",
  "seed": 4421,
  "archetype": "predator"
}
15. Blender Addon must support
Generate Character
Generate Creature
Generate Drone
Generate Plant
Generate Tree
Generate Rock
Generate Planet Prop
Generate Structure
Seed
Theme
Archetype
Export
Batch generate
16. PCG SYSTEM COMPLETE
You now have specs for:

Interior
Asset
Ship
Station
Weapon
Rig
Character
Creature
Planet
Vegetation
This is enough to generate an entire universe.

✅ Next logical step
Next should be:

NovaForge Universe / Sector / Galaxy Spec v1
This will define

sectors
factions
wars
stations
planets
anomalies
titan race
season reset

NovaForge Universe / Sector / Galaxy Spec v1

Yes!! This is the spec that ties everything together — ships, stations, planets, NPCs, interiors, PCG assets, seasons, Titan race, all of it.
This will define how NovaForge generates the entire universe from a seed, similar in concept to No Man's Sky, EVE Online, Elite Dangerous, and X4: Foundations, but designed for your Atlas engine + seasonal reset + Titan endgame.

We define:

NovaForge Universe / Sector / Galaxy Spec v1
NF_UNIVERSE_V1
This spec controls:

Galaxy generation
Sector layout
Planets
Stations
Factions
NPCs
Wars
Anomalies
Titans
Season resets
Server PCG
🚀 NovaForge Universe / Sector / Galaxy Spec v1
Version

NF_UNIVERSE_V1
Rule

UniverseSeed
 → Galaxy
 → Regions
 → Sectors
 → Systems
 → Planets
 → Stations
 → NPCs
 → Assets
 → Runtime PCG
Used by

Server
Atlas engine
Universe generator
Sector loader
Mission system
Faction system
Season reset
1. Universe Seed
Whole universe must come from seed.

UniverseSeed = int
Controls

sector layout
planet types
stations
loot
npc
factions
anomalies
titans
Example

{
  "universe_seed": 884421
}
Needed for

season reset
server sync
multiplayer
rebuild universe
2. Galaxy Layout
Galaxy is grid / graph.

Galaxy
 ├ Region
 │ ├ Sector
 │ │ ├ System
 │ │ │ ├ Planets
 │ │ │ ├ Stations
 │ │ │ ├ Anomalies
Size configurable

Small
Medium
Large
Huge
Seasonal
Example

32 sectors
128 sectors
512 sectors
1024 sectors
3. Regions
Regions group sectors.

core
inner
mid
outer
frontier
void
titan_zone
Controls

difficulty
loot tier
npc level
station size
ship size
anomalies
Example

core → safe
outer → dangerous
void → alien
titan_zone → endgame
4. Sector Spec
Sector contains gameplay area.

sector_id
seed
region
danger
resources
faction
Sector generates

asteroids
planets
stations
npc ships
missions
anomalies
Example

{
  "sector": 12,
  "region": "outer",
  "seed": 5512
}
5. System Spec
Optional sub-level.

star
planets
moons
belts
stations
Types

single star
binary
nebula
asteroid field
dead system
alien system
6. Planet Generation
Uses

NF_LIFE_V1
Planet types

rock
desert
ice
forest
lava
ocean
toxic
alien
gas
titan
Controls

vegetation
creatures
structures
resources
weather
7. Station Placement
Uses

NF_STATION_V1
Rules

safe sector → trade station
danger sector → military
mining sector → refinery
outer → pirate
void → alien
Count per sector

0–10
8. NPC / Faction System
Factions

civilian
corp
military
pirate
alien
ancient
player
rogue_ai
titan
Controls

ship types
stations
weapons
missions
war
ownership
Example

{
  "faction": "pirate"
}
9. War / Control System
Sector can have owner.

owner_faction
war_level
security_level
Effects

npc spawn
turrets
patrols
missions
prices
loot
Needed for season gameplay.

10. Anomaly System
Rare areas.

Types

rift
ancient
alien
storm
derelict
titan_gate
blackhole
void
Anomalies spawn

loot
boss
rare ships
rare modules
titan parts
Generated by seed.

11. Titan System (Endgame)
Titan zones only in outer / void.

Titan types

forge_titan
ancient_titan
void_titan
machine_titan
Rules

1 titan per universe
or few
Titan requires

huge fleet
modules
sector control
Used for season race.

12. Season Reset System
Universe resets every season.

SeasonLength = 3 months
Reset

sectors
stations
loot
titans
wars
Keep

player unlocks
cosmetics
titles
blueprints
Needed for Titan race.

13. Runtime PCG Rules
Never load full galaxy.

Load

current sector
neighbor sectors
visible planets
near ships
Everything else = seed only.

Needed for performance.

14. Save Format
/save/universe.json
Example

{
  "format": "NF_UNIVERSE_V1",
  "seed": 55121,
  "season": 3,
  "sectors": 512
}
15. Generator Pipeline
UniverseSeed
 → Galaxy
 → Regions
 → Sectors
 → Systems
 → Planets
 → Stations
 → NPCs
 → Assets
 → Interiors
All use specs you made.

16. Blender Addon Role
Addon generates assets only.

Server generates universe.

Blender → assets
Server → placement
Engine → runtime load
Correct architecture.

17. You now have FULL NovaForge spec
You defined:

Asset
Interior
Ship
Station
Weapon
Rig
Character
Creature
Planet
Universe
This is enough to build the whole game.

✅ Next step (very important now)
Next should be:

NovaForge Blender PCG Addon Spec v1
This will define exactly how the addon must work
so Copilot can rewrite it without breaking everything.


You’ve reached the Free limit for chats with attachments
Upgrade now or wait until 5:15 AM to keep using files, or chat now without files.

New chat

Claim free offer


