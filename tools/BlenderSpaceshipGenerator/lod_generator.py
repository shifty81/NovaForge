"""
LOD (Level of Detail) mesh generator for game engine export

Generates decimated copies of hull meshes at multiple detail levels
so that game engines can swap geometry based on camera distance.

LOD levels:
  LOD0 – full resolution (original mesh)
  LOD1 – medium resolution (50% polygons)
  LOD2 – low resolution (25% polygons)
  LOD3 – minimal resolution (10% polygons)
"""

import bpy


# ---------------------------------------------------------------------------
# Constants
# ---------------------------------------------------------------------------

LOD_LEVELS = {
    0: {'ratio': 1.0, 'name': 'LOD0'},
    1: {'ratio': 0.5, 'name': 'LOD1'},
    2: {'ratio': 0.25, 'name': 'LOD2'},
    3: {'ratio': 0.1, 'name': 'LOD3'},
}

# Switch-distances (metres) per ship class.  Larger vessels stay visible at
# greater range so their thresholds are proportionally bigger.
LOD_DISTANCES = {
    'SHUTTLE':        [0, 100,  300,   800],
    'FIGHTER':        [0, 110,  330,   900],
    'CORVETTE':       [0, 130,  400,  1000],
    'FRIGATE':        [0, 150,  450,  1200],
    'DESTROYER':      [0, 200,  600,  1800],
    'CRUISER':        [0, 250,  800,  2500],
    'BATTLECRUISER':  [0, 300, 1000,  3000],
    'BATTLESHIP':     [0, 350, 1200,  4000],
    'CARRIER':        [0, 400, 1500,  5000],
    'DREADNOUGHT':    [0, 450, 1800,  6000],
    'CAPITAL':        [0, 480, 1900,  7000],
    'TITAN':          [0, 500, 2000,  8000],
    'INDUSTRIAL':     [0, 150,  450,  1200],
    'MINING_BARGE':   [0, 120,  350,  1000],
    'EXHUMER':        [0, 140,  420,  1100],
    'EXPLORER':       [0, 110,  330,   900],
    'HAULER':         [0, 150,  450,  1200],
    'EXOTIC':         [0, 120,  360,   950],
}


# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------

def _prefixed_name(prefix, name):
    """Return name with project prefix applied if prefix is non-empty."""
    if prefix:
        return f"{prefix}_{name}"
    return name


def get_lod_distances(ship_class):
    """Return the list of LOD switch-distances for *ship_class*.

    Falls back to FRIGATE distances when the class is not recognised.

    Args:
        ship_class: Ship class key (e.g. ``'CRUISER'``).

    Returns:
        list[float]: Four distance values, one per LOD level.
    """
    return LOD_DISTANCES.get(ship_class, LOD_DISTANCES['FRIGATE'])


def apply_decimate(obj, ratio):
    """Add a Decimate modifier to *obj* and set its ratio.

    Args:
        obj:   Blender mesh object to decimate.
        ratio: Decimation ratio in the range ``(0, 1]``.

    Returns:
        bpy.types.Modifier: The newly created modifier.
    """
    modifier = obj.modifiers.new(name="LOD_Decimate", type='DECIMATE')
    modifier.ratio = ratio
    return modifier


# ---------------------------------------------------------------------------
# Main entry point
# ---------------------------------------------------------------------------

def generate_lods(hull_obj, ship_class='FRIGATE', naming_prefix=''):
    """Generate LOD meshes for a hull object.

    Duplicates *hull_obj* for each reduced LOD level (LOD1-LOD3), applies a
    Decimate modifier at the appropriate ratio, and tags every object
    (including the original) with custom properties consumed by exporters.

    Args:
        hull_obj:       The source mesh object (treated as LOD0).
        ship_class:     Ship class key used to look up switch-distances.
        naming_prefix:  Optional project prefix applied to created objects.

    Returns:
        list[bpy.types.Object]: The newly created LOD objects (LOD1-LOD3).
            Returns an empty list when *hull_obj* is ``None`` or has no
            mesh data.
    """
    if hull_obj is None or hull_obj.type != 'MESH':
        return []

    if hull_obj.data is None or len(hull_obj.data.vertices) == 0:
        return []

    distances = get_lod_distances(ship_class)

    # Tag the original mesh as LOD0.
    hull_obj["lod_level"] = 0
    hull_obj["lod_ratio"] = LOD_LEVELS[0]['ratio']
    hull_obj["lod_distance"] = float(distances[0])

    # Strip Blender's ".001" duplicate suffix to derive a clean base name.
    base_name = hull_obj.name.split('.')[0]

    created = []
    for level in (1, 2, 3):
        cfg = LOD_LEVELS[level]

        # Duplicate mesh data so each LOD is independent.
        new_mesh = hull_obj.data.copy()
        lod_name = _prefixed_name(naming_prefix, f"{base_name}_{cfg['name']}")
        new_obj = bpy.data.objects.new(lod_name, new_mesh)

        # Place in the same collection(s) as the source object.
        for col in hull_obj.users_collection:
            col.objects.link(new_obj)

        # Copy transform from the original.
        new_obj.matrix_world = hull_obj.matrix_world.copy()

        # Apply decimation.
        apply_decimate(new_obj, cfg['ratio'])

        # Store custom properties for exporters / game-engine pipelines.
        new_obj["lod_level"] = level
        new_obj["lod_ratio"] = cfg['ratio']
        new_obj["lod_distance"] = float(distances[level])

        created.append(new_obj)

    return created


# ---------------------------------------------------------------------------
# Registration
# ---------------------------------------------------------------------------

def register():
    """Register this module."""
    pass


def unregister():
    """Unregister this module."""
    pass
