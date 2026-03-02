"""
Collision mesh generator for game engine export

Generates simplified collision geometry from hull meshes so that game
engines can use lightweight shapes for physics simulation instead of the
full-detail render mesh.

Supported collision types:
  CONVEX_HULL   – single convex hull wrapping the entire ship
  BOX           – oriented bounding box
  MULTI_CONVEX  – decomposed into multiple convex parts
"""

import bpy
import math
import bmesh
from mathutils import Vector


# ---------------------------------------------------------------------------
# Constants
# ---------------------------------------------------------------------------

COLLISION_TYPES = {
    'CONVEX_HULL': 'Single convex hull wrapping entire ship',
    'BOX': 'Oriented bounding box',
    'MULTI_CONVEX': 'Decomposed into multiple convex parts',
}

# Smaller ships use a single convex hull; medium ships do as well.
# Large ships use multi-convex decomposition for better accuracy.
DEFAULT_COLLISION_TYPE = {
    'SHUTTLE':       'BOX',
    'FIGHTER':       'CONVEX_HULL',
    'CORVETTE':      'CONVEX_HULL',
    'FRIGATE':       'CONVEX_HULL',
    'DESTROYER':     'CONVEX_HULL',
    'CRUISER':       'CONVEX_HULL',
    'BATTLECRUISER': 'CONVEX_HULL',
    'BATTLESHIP':    'MULTI_CONVEX',
    'CARRIER':       'MULTI_CONVEX',
    'DREADNOUGHT':   'MULTI_CONVEX',
    'CAPITAL':       'MULTI_CONVEX',
    'TITAN':         'MULTI_CONVEX',
    'INDUSTRIAL':    'CONVEX_HULL',
    'MINING_BARGE':  'CONVEX_HULL',
    'EXHUMER':       'CONVEX_HULL',
    'EXPLORER':      'CONVEX_HULL',
    'HAULER':        'CONVEX_HULL',
    'EXOTIC':        'CONVEX_HULL',
}

# Number of convex parts for MULTI_CONVEX, keyed by scale thresholds.
_MULTI_CONVEX_PARTS = {
    20.0: 2,   # scale < 20  -> 2 parts
    35.0: 3,   # scale < 35  -> 3 parts
    math.inf: 4,  # scale >= 35 -> 4 parts
}

# Minimum dimension for box collision to avoid degenerate scales.
_MIN_BOX_DIMENSION = 1e-4

# Octree depth for the Remesh modifier used in convex hull simplification.
_CONVEX_REMESH_DEPTH = 3

# Ship scales (mirrored from ship_generator.SHIP_CONFIGS for look-up).
_SHIP_SCALES = {
    'FRIGATE': 5.0,      'DESTROYER': 8.0,
    'CRUISER': 12.0,     'BATTLECRUISER': 15.0,
    'BATTLESHIP': 18.0,  'CARRIER': 25.0,
    'DREADNOUGHT': 30.0, 'TITAN': 50.0,
    'INDUSTRIAL': 6.0,   'MINING_BARGE': 4.0,
    'EXHUMER': 5.0,
}


# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------

def _prefixed_name(prefix, name):
    """Return name with project prefix applied if prefix is non-empty."""
    if prefix:
        return f"{prefix}_{name}"
    return name


def _duplicate_mesh_object(src_obj, name):
    """Create a duplicate of *src_obj* with an independent mesh data-block."""
    new_mesh = src_obj.data.copy()
    new_obj = src_obj.copy()
    new_obj.data = new_mesh
    new_obj.name = name
    new_mesh.name = name
    return new_obj


def _link_to_collection(obj, reference_obj):
    """Link *obj* to the same collection(s) that *reference_obj* lives in."""
    linked = False
    for col in reference_obj.users_collection:
        col.objects.link(obj)
        linked = True
    if not linked:
        bpy.context.collection.objects.link(obj)


def _set_collision_properties(obj, collision_type, source_name):
    """Stamp standard custom properties on a collision object."""
    obj["collision_type"] = collision_type
    obj["collision_source"] = source_name
    obj["is_collision_mesh"] = 1


def _setup_display(obj):
    """Set the object to wireframe display so it is visible but unobtrusive."""
    obj.display_type = 'WIRE'


def _apply_decimate(obj, ratio):
    """Add and apply a Decimate modifier with the given *ratio*."""
    mod = obj.modifiers.new(name="CollisionDecimate", type='DECIMATE')
    mod.ratio = ratio
    ctx = {'object': obj}
    with bpy.context.temp_override(**ctx):
        bpy.ops.object.modifier_apply(modifier=mod.name)


def _parts_for_scale(scale):
    """Return the number of convex parts to generate for a given ship scale."""
    for threshold, parts in sorted(_MULTI_CONVEX_PARTS.items()):
        if scale < threshold:
            return parts
    return 4


# ---------------------------------------------------------------------------
# Public helpers
# ---------------------------------------------------------------------------

def get_collision_type(ship_class):
    """Return the default collision type string for *ship_class*.

    Falls back to ``'CONVEX_HULL'`` for unknown classes.
    """
    return DEFAULT_COLLISION_TYPE.get(ship_class, 'CONVEX_HULL')


def generate_box_collision(hull_obj, naming_prefix=''):
    """Create an oriented bounding-box collision mesh around *hull_obj*.

    Returns the newly created box object, or ``None`` if *hull_obj* is
    invalid or has no geometry.
    """
    if hull_obj is None or hull_obj.type != 'MESH':
        return None
    if len(hull_obj.data.vertices) == 0:
        return None

    # Compute bounding box dimensions in local space.
    bb = hull_obj.bound_box
    xs = [v[0] for v in bb]
    ys = [v[1] for v in bb]
    zs = [v[2] for v in bb]
    size_x = max(xs) - min(xs)
    size_y = max(ys) - min(ys)
    size_z = max(zs) - min(zs)
    center_x = (max(xs) + min(xs)) / 2
    center_y = (max(ys) + min(ys)) / 2
    center_z = (max(zs) + min(zs)) / 2

    size_x = max(size_x, _MIN_BOX_DIMENSION)
    size_y = max(size_y, _MIN_BOX_DIMENSION)
    size_z = max(size_z, _MIN_BOX_DIMENSION)

    name = _prefixed_name(naming_prefix, "Collision_BOX")
    bpy.ops.mesh.primitive_cube_add(
        size=1.0,
        location=(center_x, center_y, center_z),
    )
    box = bpy.context.active_object
    box.name = name
    box.scale = (size_x, size_y, size_z)
    box.matrix_world = hull_obj.matrix_world @ box.matrix_world

    _set_collision_properties(box, 'BOX', hull_obj.name)
    _setup_display(box)

    box.parent = hull_obj

    return box


def generate_convex_hull_collision(hull_obj, naming_prefix='',
                                   decimate_ratio=0.15):
    """Create a simplified convex hull collision mesh from *hull_obj*.

    Returns the collision object, or ``None`` on failure.
    """
    if hull_obj is None or hull_obj.type != 'MESH':
        return None
    if len(hull_obj.data.vertices) == 0:
        return None

    name = _prefixed_name(naming_prefix, "Collision_CONVEX_HULL")
    col_obj = _duplicate_mesh_object(hull_obj, name)
    _link_to_collection(col_obj, hull_obj)

    # Decimate to reduce polygon count.
    _apply_decimate(col_obj, decimate_ratio)

    # Remesh in BLOCKS mode for a simple convex approximation.
    mod = col_obj.modifiers.new(name="CollisionRemesh", type='REMESH')
    mod.mode = 'BLOCKS'
    mod.octree_depth = _CONVEX_REMESH_DEPTH
    ctx = {'object': col_obj}
    with bpy.context.temp_override(**ctx):
        bpy.ops.object.modifier_apply(modifier=mod.name)

    col_obj.matrix_world = hull_obj.matrix_world.copy()
    _set_collision_properties(col_obj, 'CONVEX_HULL', hull_obj.name)
    _setup_display(col_obj)

    col_obj.parent = hull_obj

    return col_obj


# ---------------------------------------------------------------------------
# Multi-convex helpers
# ---------------------------------------------------------------------------

def _generate_multi_convex(hull_obj, naming_prefix='', num_parts=2):
    """Split *hull_obj* into *num_parts* convex collision pieces along Y.

    Returns a list of collision objects, or an empty list on failure.
    """
    if hull_obj is None or hull_obj.type != 'MESH':
        return []
    if len(hull_obj.data.vertices) == 0:
        return []

    ys = [v.co.y for v in hull_obj.data.vertices]
    y_min, y_max = min(ys), max(ys)
    span = y_max - y_min
    if span <= 0:
        return []

    step = span / num_parts
    parts = []

    for i in range(num_parts):
        part_name = _prefixed_name(
            naming_prefix, f"Collision_MULTI_CONVEX_{i}"
        )
        part_obj = _duplicate_mesh_object(hull_obj, part_name)
        _link_to_collection(part_obj, hull_obj)

        # Decimate first.
        _apply_decimate(part_obj, 0.2)

        # Bisect to keep only this section's slice along Y.
        bm = bmesh.new()
        bm.from_mesh(part_obj.data)

        lower = y_min + i * step
        upper = y_min + (i + 1) * step

        # Cut away geometry below *lower* (keep above).
        if i > 0:
            geom = bm.verts[:] + bm.edges[:] + bm.faces[:]
            bmesh.ops.bisect_plane(
                bm, geom=geom,
                plane_co=(0.0, lower, 0.0),
                plane_no=(0.0, -1.0, 0.0),
                clear_inner=True,
            )

        # Cut away geometry above *upper* (keep below).
        if i < num_parts - 1:
            geom = bm.verts[:] + bm.edges[:] + bm.faces[:]
            bmesh.ops.bisect_plane(
                bm, geom=geom,
                plane_co=(0.0, upper, 0.0),
                plane_no=(0.0, 1.0, 0.0),
                clear_inner=True,
            )

        bm.to_mesh(part_obj.data)
        bm.free()

        part_obj.matrix_world = hull_obj.matrix_world.copy()
        _set_collision_properties(part_obj, 'MULTI_CONVEX', hull_obj.name)
        _setup_display(part_obj)

        part_obj.parent = hull_obj
        parts.append(part_obj)

    return parts


# ---------------------------------------------------------------------------
# Main entry point
# ---------------------------------------------------------------------------

def generate_collision_mesh(hull_obj, collision_type=None,
                            ship_class='FRIGATE', naming_prefix=''):
    """Generate a simplified collision mesh for *hull_obj*.

    Parameters
    ----------
    hull_obj : bpy.types.Object
        The source hull mesh object.
    collision_type : str or None
        One of the keys in :data:`COLLISION_TYPES`.  When ``None`` the
        default for *ship_class* is used.
    ship_class : str
        Ship class key (e.g. ``'FRIGATE'``, ``'TITAN'``).
    naming_prefix : str
        Optional prefix prepended to generated object names.

    Returns
    -------
    bpy.types.Object or list[bpy.types.Object] or None
        The generated collision object(s).  ``MULTI_CONVEX`` returns a
        list; other types return a single object.  ``None`` on failure.
    """
    if hull_obj is None or hull_obj.type != 'MESH':
        return None

    if collision_type is None:
        collision_type = get_collision_type(ship_class)

    if collision_type not in COLLISION_TYPES:
        collision_type = 'CONVEX_HULL'

    if collision_type == 'BOX':
        return generate_box_collision(hull_obj, naming_prefix)

    if collision_type == 'CONVEX_HULL':
        return generate_convex_hull_collision(hull_obj, naming_prefix)

    if collision_type == 'MULTI_CONVEX':
        scale = _SHIP_SCALES.get(ship_class, 1.0)
        num_parts = _parts_for_scale(scale)
        return _generate_multi_convex(hull_obj, naming_prefix, num_parts)

    return None


# ---------------------------------------------------------------------------
# Registration
# ---------------------------------------------------------------------------

def register():
    """Register this module."""
    pass


def unregister():
    """Unregister this module."""
    pass
