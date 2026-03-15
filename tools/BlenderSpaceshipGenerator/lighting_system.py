"""
Lighting system module for the Blender Spaceship Generator addon.

Creates interior and exterior lighting for generated ships:
  - **Interior**: point lights placed inside rooms (cockpit, bridge, corridors,
    engine room, cargo bay) with warm or cool colour temperatures.
  - **Exterior**: engine glow lights, running lights (nav red/green/white),
    and optional flood lights for larger ships.

All lights are parented to the hull and collected into a ``Lights``
sub-collection for easy toggling.

Usage from the generate operator::

    lighting.setup_ship_lighting(hull_obj, scale=scale, generate_interior=True)
"""

import math
import random

import bpy


# ---------------------------------------------------------------------------
# Constants
# ---------------------------------------------------------------------------

# Colour temperatures as RGB tuples
WARM_WHITE = (1.0, 0.92, 0.82)
COOL_WHITE = (0.85, 0.92, 1.0)
ENGINE_BLUE = (0.4, 0.6, 1.0)
NAV_RED = (1.0, 0.1, 0.05)
NAV_GREEN = (0.05, 1.0, 0.1)
NAV_WHITE = (1.0, 1.0, 1.0)

# Default light energies (Watts) — scaled by ship size at creation time
DEFAULT_INTERIOR_ENERGY = 10.0
DEFAULT_ENGINE_GLOW_ENERGY = 50.0
DEFAULT_RUNNING_LIGHT_ENERGY = 5.0

# Interior room name patterns and their preferred colour
ROOM_LIGHT_MAP = {
    'cockpit': WARM_WHITE,
    'bridge': WARM_WHITE,
    'corridor': COOL_WHITE,
    'quarters': WARM_WHITE,
    'cargo': COOL_WHITE,
    'engine': ENGINE_BLUE,
    'reactor': ENGINE_BLUE,
    'hangar': COOL_WHITE,
    'weapon': NAV_RED,
    'sensor': COOL_WHITE,
    'power': ENGINE_BLUE,
    'shield': COOL_WHITE,
}


# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------

def _prefixed_name(prefix, name):
    """Return *name* with project prefix applied if *prefix* is non-empty."""
    if prefix:
        return f"{prefix}_{name}"
    return name


def _get_or_create_collection(name, parent_collection=None):
    """Return the existing Blender collection *name* or create a new one."""
    if name in bpy.data.collections:
        return bpy.data.collections[name]
    col = bpy.data.collections.new(name)
    if parent_collection is not None:
        parent_collection.children.link(col)
    else:
        bpy.context.scene.collection.children.link(col)
    return col


def _create_point_light(name, energy, color, location, radius=0.1):
    """Create a point light, add it to the scene, and return the object."""
    light_data = bpy.data.lights.new(name=name, type='POINT')
    light_data.energy = energy
    light_data.color = color
    light_data.shadow_soft_size = radius
    light_obj = bpy.data.objects.new(name, light_data)
    light_obj.location = location
    bpy.context.scene.collection.objects.link(light_obj)
    return light_obj


# ---------------------------------------------------------------------------
# Interior lighting
# ---------------------------------------------------------------------------

def create_interior_lights(hull_obj, scale=1.0, naming_prefix=''):
    """Place point lights inside detected interior rooms.

    Rooms are discovered by scanning the children (and children of children)
    of *hull_obj* for objects whose name contains a key from
    :data:`ROOM_LIGHT_MAP`.  Each matched room receives a centred point
    light with colour appropriate to the room type.

    Args:
        hull_obj: Top-level hull Blender object.
        scale: Ship scale factor — affects light energy.
        naming_prefix: Project naming prefix.

    Returns:
        List of created light objects.
    """
    if hull_obj is None:
        return []

    lights = []
    energy = DEFAULT_INTERIOR_ENERGY * max(scale, 0.5)

    def _scan(obj):
        name_lower = obj.name.lower()
        for room_key, color in ROOM_LIGHT_MAP.items():
            if room_key in name_lower:
                # Place light at the object's world centre, raised slightly
                loc = obj.matrix_world.translation.copy()
                loc.z += 0.3 * scale  # offset toward ceiling

                light_name = _prefixed_name(
                    naming_prefix,
                    f"IntLight_{room_key}_{len(lights)}",
                )
                light_obj = _create_point_light(
                    light_name, energy, color, loc, radius=0.05 * scale,
                )
                light_obj.parent = hull_obj
                lights.append(light_obj)
                break  # one light per room object

        for child in obj.children:
            _scan(child)

    _scan(hull_obj)
    return lights


# ---------------------------------------------------------------------------
# Engine glow
# ---------------------------------------------------------------------------

def create_engine_glow_lights(hull_obj, scale=1.0, naming_prefix=''):
    """Create blue glow lights at engine nozzle positions.

    Engine children are identified by name containing ``'Engine'`` or
    ``'Nozzle'``, or by carrying an ``engine_archetype`` custom property.

    Args:
        hull_obj: Top-level hull object.
        scale: Ship scale factor.
        naming_prefix: Project naming prefix.

    Returns:
        List of created engine glow light objects.
    """
    if hull_obj is None:
        return []

    lights = []
    energy = DEFAULT_ENGINE_GLOW_ENERGY * max(scale, 0.5)

    for child in hull_obj.children_recursive:
        name_lower = child.name.lower()
        is_engine = (
            'engine' in name_lower
            or 'nozzle' in name_lower
            or 'engine_archetype' in child
        )
        if not is_engine:
            continue

        loc = child.matrix_world.translation.copy()
        # Push the glow behind the engine (negative Y in ship-local space)
        loc.y -= 0.3 * scale

        light_name = _prefixed_name(
            naming_prefix,
            f"EngineGlow_{len(lights)}",
        )
        light_obj = _create_point_light(
            light_name, energy, ENGINE_BLUE, loc, radius=0.2 * scale,
        )
        light_obj.parent = hull_obj
        lights.append(light_obj)

    return lights


# ---------------------------------------------------------------------------
# Navigation / running lights
# ---------------------------------------------------------------------------

def create_running_lights(hull_obj, scale=1.0, naming_prefix=''):
    """Create standard navigation running lights on the hull.

    Placement follows real-world aviation convention:
      - **Port** (left / -X): red
      - **Starboard** (right / +X): green
      - **Tail** (rear / -Y): white

    Args:
        hull_obj: Top-level hull object.
        scale: Ship scale factor.
        naming_prefix: Project naming prefix.

    Returns:
        List of created running-light objects.
    """
    if hull_obj is None:
        return []

    dims = hull_obj.dimensions
    half_x = dims.x * 0.5
    half_y = dims.y * 0.5

    placements = [
        ('NavRed',   NAV_RED,   (-half_x, 0.0, 0.0)),
        ('NavGreen', NAV_GREEN, (half_x, 0.0, 0.0)),
        ('NavWhite', NAV_WHITE, (0.0, -half_y, 0.0)),
    ]

    energy = DEFAULT_RUNNING_LIGHT_ENERGY * max(scale, 0.5)
    lights = []

    for suffix, color, offset in placements:
        light_name = _prefixed_name(naming_prefix, suffix)
        light_obj = _create_point_light(
            light_name, energy, color, offset, radius=0.02 * scale,
        )
        light_obj.parent = hull_obj
        lights.append(light_obj)

    return lights


# ---------------------------------------------------------------------------
# High-level entry point
# ---------------------------------------------------------------------------

def setup_ship_lighting(hull_obj, scale=1.0, generate_interior=True,
                        naming_prefix=''):
    """Create all lighting for a ship.

    Args:
        hull_obj: Top-level hull Blender object.
        scale: Ship scale factor.
        generate_interior: Whether to add interior room lights.
        naming_prefix: Project naming prefix.

    Returns:
        Dict ``{"interior": [...], "engine_glow": [...], "running": [...]}``
        listing all created light objects.
    """
    if hull_obj is None:
        return {"interior": [], "engine_glow": [], "running": []}

    result = {
        "interior": [],
        "engine_glow": [],
        "running": [],
    }

    if generate_interior:
        result["interior"] = create_interior_lights(
            hull_obj, scale=scale, naming_prefix=naming_prefix,
        )

    result["engine_glow"] = create_engine_glow_lights(
        hull_obj, scale=scale, naming_prefix=naming_prefix,
    )

    result["running"] = create_running_lights(
        hull_obj, scale=scale, naming_prefix=naming_prefix,
    )

    # Move all lights into a sub-collection for easy toggling
    col_name = _prefixed_name(naming_prefix, "Lights") or "Lights"
    lights_col = _get_or_create_collection(col_name)
    for group in result.values():
        for obj in group:
            if obj.name not in lights_col.objects:
                lights_col.objects.link(obj)

    return result


# ---------------------------------------------------------------------------
# Registration
# ---------------------------------------------------------------------------

def register():
    """Register this module."""
    pass


def unregister():
    """Unregister this module."""
    pass
