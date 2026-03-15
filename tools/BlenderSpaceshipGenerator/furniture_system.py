"""
Furniture system module for the Blender Spaceship Generator addon.

Places room-appropriate furniture inside generated ship interiors —
tables, chairs, consoles, storage lockers, beds, monitors, and
workbenches — giving rooms a lived-in, functional appearance.

Furniture placement is seed-deterministic so the same ship always
receives the same layout.  Items are matched to room type via
:data:`ROOM_FURNITURE_MAP` and scaled relative to the ship's overall
size.

Usage from the generate operator::

    furniture.populate_ship_furniture(hull_obj, scale=scale, seed=42)
"""

import math
import random

import bpy


# ---------------------------------------------------------------------------
# Constants
# ---------------------------------------------------------------------------

# Human-scale reference dimensions (metres)
HUMAN_HEIGHT = 1.8
CHAIR_HEIGHT = 0.45
TABLE_HEIGHT = 0.75
CONSOLE_HEIGHT = 1.1
LOCKER_HEIGHT = 1.8
MONITOR_HEIGHT = 0.4
BED_HEIGHT = 0.4
WORKBENCH_HEIGHT = 0.9

# Furniture item definitions — each maps to a primitive + default scale.
FURNITURE_TYPES = {
    'CHAIR': {
        'description': 'Crew seat or chair',
        'primitive': 'CUBE',
        'scale': (0.4, 0.4, CHAIR_HEIGHT),
    },
    'TABLE': {
        'description': 'Work or dining table',
        'primitive': 'CUBE',
        'scale': (1.0, 0.6, TABLE_HEIGHT),
    },
    'CONSOLE': {
        'description': 'Control console with displays',
        'primitive': 'CUBE',
        'scale': (0.8, 0.3, CONSOLE_HEIGHT),
    },
    'LOCKER': {
        'description': 'Equipment storage locker',
        'primitive': 'CUBE',
        'scale': (0.5, 0.4, LOCKER_HEIGHT),
    },
    'BED': {
        'description': 'Crew bunk or bed',
        'primitive': 'CUBE',
        'scale': (0.8, 2.0, BED_HEIGHT),
    },
    'MONITOR': {
        'description': 'Wall-mounted display or readout',
        'primitive': 'CUBE',
        'scale': (0.6, 0.04, MONITOR_HEIGHT),
    },
    'WORKBENCH': {
        'description': 'Engineering workbench',
        'primitive': 'CUBE',
        'scale': (1.2, 0.6, WORKBENCH_HEIGHT),
    },
}

# Which furniture items belong in which room types, with weights for
# random selection.  Keys are substrings matched against room-object
# names (lower-cased).
ROOM_FURNITURE_MAP = {
    'cockpit': [
        ('CHAIR', 3),
        ('CONSOLE', 4),
        ('MONITOR', 2),
    ],
    'bridge': [
        ('CHAIR', 4),
        ('CONSOLE', 5),
        ('MONITOR', 3),
        ('TABLE', 1),
    ],
    'corridor': [
        ('LOCKER', 2),
        ('MONITOR', 1),
    ],
    'quarters': [
        ('BED', 4),
        ('LOCKER', 3),
        ('TABLE', 1),
        ('CHAIR', 1),
    ],
    'cargo': [
        ('LOCKER', 4),
        ('WORKBENCH', 2),
    ],
    'engine': [
        ('CONSOLE', 3),
        ('WORKBENCH', 3),
        ('MONITOR', 2),
        ('LOCKER', 1),
    ],
    'reactor': [
        ('CONSOLE', 3),
        ('MONITOR', 2),
    ],
    'hangar': [
        ('WORKBENCH', 3),
        ('LOCKER', 3),
        ('CONSOLE', 1),
    ],
    'weapon': [
        ('LOCKER', 4),
        ('CONSOLE', 2),
        ('WORKBENCH', 1),
    ],
    'sensor': [
        ('CONSOLE', 4),
        ('CHAIR', 2),
        ('MONITOR', 3),
    ],
    'power': [
        ('CONSOLE', 3),
        ('MONITOR', 2),
        ('WORKBENCH', 1),
    ],
    'shield': [
        ('CONSOLE', 4),
        ('CHAIR', 2),
        ('MONITOR', 2),
    ],
}

# Hard cap on total furniture items per ship for performance.
MAX_FURNITURE_ITEMS = 150


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


def _create_furniture_piece(name, scale, location):
    """Create a cube primitive representing a piece of furniture."""
    bpy.ops.mesh.primitive_cube_add(size=1.0, location=location)
    obj = bpy.context.active_object
    obj.name = name
    obj.scale = scale
    bpy.ops.object.transform_apply(scale=True)
    return obj


def _build_weighted_list(items):
    """Expand ``[(type, weight), ...]`` into a flat list for ``rng.choice``."""
    result = []
    for ftype, weight in items:
        result.extend([ftype] * weight)
    return result


# ---------------------------------------------------------------------------
# Room furniture placement
# ---------------------------------------------------------------------------

def _place_furniture_in_room(room_obj, room_key, rng, scale, naming_prefix,
                             counter):
    """Place furniture items inside a single detected room.

    Args:
        room_obj: Blender object representing the room.
        room_key: Key into :data:`ROOM_FURNITURE_MAP`.
        rng: :class:`random.Random` instance for determinism.
        scale: Ship scale factor.
        naming_prefix: Project naming prefix.
        counter: Mutable ``[int]`` tracking total items created so far.

    Returns:
        List of created furniture objects.
    """
    mapping = ROOM_FURNITURE_MAP.get(room_key)
    if not mapping:
        return []

    weighted = _build_weighted_list(mapping)
    if not weighted:
        return []

    # Approximate room bounds from the room object's dimensions.
    dims = room_obj.dimensions
    room_width = max(dims.x, 0.5) * 0.7   # leave margin from walls
    room_depth = max(dims.y, 0.5) * 0.7
    origin = room_obj.matrix_world.translation.copy()

    # Decide how many items to place (2-6 depending on room area).
    area = room_width * room_depth
    item_count = max(2, min(6, int(area * 1.5)))

    pieces = []
    for i in range(item_count):
        if counter[0] >= MAX_FURNITURE_ITEMS:
            break

        ftype = rng.choice(weighted)
        fdef = FURNITURE_TYPES[ftype]
        base_scale = fdef['scale']

        # Apply slight random jitter to scale
        jitter = rng.uniform(0.85, 1.15)
        piece_scale = (
            base_scale[0] * jitter * scale,
            base_scale[1] * jitter * scale,
            base_scale[2] * jitter * scale,
        )

        # Random position inside the room footprint
        x = origin.x + rng.uniform(-room_width * 0.4, room_width * 0.4)
        y = origin.y + rng.uniform(-room_depth * 0.4, room_depth * 0.4)
        z = origin.z + (base_scale[2] * jitter * scale) * 0.5  # sit on floor

        obj_name = _prefixed_name(
            naming_prefix,
            f"Furn_{ftype}_{counter[0]}",
        )
        obj = _create_furniture_piece(obj_name, piece_scale, (x, y, z))
        obj.rotation_euler.z = rng.uniform(0, math.pi * 2)
        obj.parent = room_obj
        pieces.append(obj)
        counter[0] += 1

    return pieces


# ---------------------------------------------------------------------------
# Public API
# ---------------------------------------------------------------------------

def populate_ship_furniture(hull_obj, scale=1.0, seed=1, naming_prefix=''):
    """Place furniture in all detected interior rooms of *hull_obj*.

    Rooms are discovered by scanning the children of *hull_obj* for
    objects whose name contains a key from :data:`ROOM_FURNITURE_MAP`.

    Args:
        hull_obj: Top-level hull Blender object.
        scale: Ship scale factor — affects furniture dimensions.
        seed: Random seed for deterministic placement.
        naming_prefix: Project naming prefix.

    Returns:
        List of all created furniture objects.
    """
    if hull_obj is None:
        return []

    rng = random.Random(seed)
    counter = [0]  # mutable counter for global cap

    all_pieces = []

    def _scan(obj):
        name_lower = obj.name.lower()
        for room_key in ROOM_FURNITURE_MAP:
            if room_key in name_lower:
                pieces = _place_furniture_in_room(
                    obj, room_key, rng, scale, naming_prefix, counter,
                )
                all_pieces.extend(pieces)
                break  # one match per object

        for child in obj.children:
            _scan(child)

    _scan(hull_obj)

    # Organise into a sub-collection
    if all_pieces:
        col_name = _prefixed_name(naming_prefix, "Furniture") or "Furniture"
        furn_col = _get_or_create_collection(col_name)
        for obj in all_pieces:
            if obj.name not in furn_col.objects:
                furn_col.objects.link(obj)

    return all_pieces


def get_furniture_types():
    """Return the furniture type definitions dict.

    Useful for external tooling or UI enumeration.
    """
    return dict(FURNITURE_TYPES)


def get_room_furniture_map():
    """Return the room-to-furniture mapping dict.

    Useful for external tooling or documentation.
    """
    return dict(ROOM_FURNITURE_MAP)


# ---------------------------------------------------------------------------
# Registration
# ---------------------------------------------------------------------------

def register():
    """Register this module."""
    pass


def unregister():
    """Unregister this module."""
    pass
