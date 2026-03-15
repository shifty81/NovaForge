"""
Greeble / detail-pass system for the Blender Spaceship Generator addon.

Adds surface details to generated hull geometry — panels, vents, pipes,
antennas, sensor domes, and wiring conduits — giving the ship the
characteristic *busy* look of sci-fi vessels.

Greeble placement is seed-deterministic so the same ship will always
receive the same detail layout.

Usage from the generate operator::

    greeble.apply_greebles(hull_obj, seed=42, density=0.5)
"""

import math
import random

import bpy
from mathutils import Vector


# ---------------------------------------------------------------------------
# Constants
# ---------------------------------------------------------------------------

# Each greeble type describes what primitive to place and how to size it.
GREEBLE_TYPES = {
    'PANEL': {
        'description': 'Recessed hull panel',
        'primitive': 'CUBE',
        'scale': (0.4, 0.4, 0.02),
        'weight': 3,  # relative spawn probability
    },
    'VENT': {
        'description': 'Exhaust or intake vent grille',
        'primitive': 'CUBE',
        'scale': (0.15, 0.4, 0.05),
        'weight': 2,
    },
    'PIPE': {
        'description': 'Exposed conduit pipe',
        'primitive': 'CYLINDER',
        'scale': (0.03, 0.03, 0.5),
        'weight': 2,
    },
    'ANTENNA': {
        'description': 'Communication antenna mast',
        'primitive': 'CYLINDER',
        'scale': (0.01, 0.01, 0.6),
        'weight': 1,
    },
    'SENSOR_DOME': {
        'description': 'Sensor or radar dome',
        'primitive': 'UV_SPHERE',
        'scale': (0.1, 0.1, 0.06),
        'weight': 1,
    },
    'BOX': {
        'description': 'Equipment box',
        'primitive': 'CUBE',
        'scale': (0.12, 0.12, 0.08),
        'weight': 2,
    },
    'CONDUIT': {
        'description': 'Wiring conduit bundle',
        'primitive': 'CYLINDER',
        'scale': (0.02, 0.02, 0.3),
        'weight': 1,
    },
}

# Weighted list for random selection
_WEIGHTED_TYPES = []
for _gtype, _gdef in GREEBLE_TYPES.items():
    _WEIGHTED_TYPES.extend([_gtype] * _gdef['weight'])


# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------

def _prefixed_name(prefix, name):
    """Return *name* with project prefix applied if *prefix* is non-empty."""
    if prefix:
        return f"{prefix}_{name}"
    return name


def _create_primitive(primitive_type, name, scale, location):
    """Create a small mesh primitive and return the object."""
    if primitive_type == 'CUBE':
        bpy.ops.mesh.primitive_cube_add(size=1.0, location=location)
    elif primitive_type == 'CYLINDER':
        bpy.ops.mesh.primitive_cylinder_add(radius=0.5, depth=1.0,
                                            vertices=8, location=location)
    elif primitive_type == 'UV_SPHERE':
        bpy.ops.mesh.primitive_uv_sphere_add(radius=0.5, segments=8,
                                             ring_count=6, location=location)
    else:
        bpy.ops.mesh.primitive_cube_add(size=1.0, location=location)

    obj = bpy.context.active_object
    obj.name = name
    obj.scale = scale
    return obj


def _sample_hull_surface(hull_obj, rng, count):
    """Return *count* approximate surface positions on *hull_obj*.

    Uses a simple bounding-box jitter approach — positions are sampled
    randomly within the bounding box and snapped toward the nearest face.
    For procedural greebles this is visually sufficient without requiring
    expensive raycasts.

    Args:
        hull_obj: Hull Blender object.
        rng: :class:`random.Random` instance for determinism.
        count: Number of positions to sample.

    Returns:
        List of ``(x, y, z)`` tuples in world space.
    """
    dims = hull_obj.dimensions
    half = Vector((dims.x * 0.5, dims.y * 0.5, dims.z * 0.5))
    origin = hull_obj.matrix_world.translation

    positions = []
    for _ in range(count):
        # Random point on the bounding-box *surface* (one axis pinned to
        # either min or max).
        axis = rng.randint(0, 2)
        sign = rng.choice((-1, 1))
        pt = [rng.uniform(-half[i], half[i]) for i in range(3)]
        pt[axis] = half[axis] * sign  # pin to surface face
        positions.append((
            origin.x + pt[0],
            origin.y + pt[1],
            origin.z + pt[2],
        ))

    return positions


# ---------------------------------------------------------------------------
# Public API
# ---------------------------------------------------------------------------

def apply_greebles(hull_obj, seed=1, density=0.5, naming_prefix=''):
    """Apply surface greebles to *hull_obj*.

    Args:
        hull_obj: Top-level hull Blender object.
        seed: Random seed for deterministic placement.
        density: Float in ``[0.0, 1.0]`` controlling how many greebles to
                 place.  ``0.0`` means none; ``1.0`` means maximum coverage.
        naming_prefix: Project naming prefix.

    Returns:
        List of created greeble Blender objects.
    """
    if hull_obj is None:
        return []

    density = max(0.0, min(1.0, density))
    if density == 0.0:
        return []

    rng = random.Random(seed)

    # Approximate count from hull volume and density
    dims = hull_obj.dimensions
    surface_area = 2.0 * (dims.x * dims.y + dims.y * dims.z + dims.x * dims.z)
    count = max(1, int(surface_area * density * 2.0))
    count = min(count, 200)  # hard cap for performance

    positions = _sample_hull_surface(hull_obj, rng, count)

    greebles = []
    for i, pos in enumerate(positions):
        gtype = rng.choice(_WEIGHTED_TYPES)
        gdef = GREEBLE_TYPES[gtype]

        base_scale = gdef['scale']
        # Vary scale slightly
        jitter = rng.uniform(0.7, 1.3)
        scale = (base_scale[0] * jitter, base_scale[1] * jitter,
                 base_scale[2] * jitter)

        obj_name = _prefixed_name(naming_prefix, f"Greeble_{gtype}_{i}")
        obj = _create_primitive(gdef['primitive'], obj_name, scale, pos)

        # Random rotation for visual variety
        obj.rotation_euler = (
            rng.uniform(0, math.pi * 2),
            rng.uniform(0, math.pi * 2),
            rng.uniform(0, math.pi * 2),
        )

        obj.parent = hull_obj
        greebles.append(obj)

    return greebles


def get_greeble_types():
    """Return the greeble type definitions dict.

    Useful for external tooling or UI enumeration.
    """
    return dict(GREEBLE_TYPES)


def get_greeble_count(density, hull_obj):
    """Estimate how many greebles *density* would produce for *hull_obj*.

    Args:
        density: Float ``[0.0, 1.0]``.
        hull_obj: Hull object (needs ``dimensions``).

    Returns:
        Estimated integer count (capped at 200).
    """
    if hull_obj is None or density <= 0:
        return 0
    dims = hull_obj.dimensions
    surface_area = 2.0 * (dims.x * dims.y + dims.y * dims.z + dims.x * dims.z)
    count = max(1, int(surface_area * max(0.0, min(1.0, density)) * 2.0))
    return min(count, 200)


# ---------------------------------------------------------------------------
# Registration
# ---------------------------------------------------------------------------

def register():
    """Register this module."""
    pass


def unregister():
    """Unregister this module."""
    pass
