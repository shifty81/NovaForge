"""
NovaForge PCG Pipeline — Character Generator

Generates procedural character metadata **and mesh geometry data**
matching the NovaForge race and body type system defined in
``character_mesh_system.h``.

The mesh data is a simplified humanoid body consisting of labelled body
parts (head, torso, limbs) described as vertex/face arrays ready for
import into Blender or any mesh engine.  Proportions are varied
deterministically based on race, body type, and seed.
"""

import math
import random

RACES = ["TerranDescendant", "SynthBorn", "PureAlien", "HybridEvolutionary"]

BODY_TYPES = ["Organic", "Augmented", "Cybernetic", "FullSynth", "MechFrame"]

CYBER_LIMB_SLOTS = [
    "left_arm", "right_arm", "left_leg", "right_leg",
    "torso_core", "spine",
]

# Base proportions (metres) for a standard humanoid character.
_BASE_HEIGHT = 1.80
_BASE_PROPORTIONS = {
    "head":      {"radius": 0.10, "height": 0.24},
    "torso":     {"width": 0.36, "depth": 0.20, "height": 0.52},
    "upper_arm": {"radius": 0.045, "length": 0.30},
    "lower_arm": {"radius": 0.038, "length": 0.26},
    "upper_leg": {"radius": 0.065, "length": 0.42},
    "lower_leg": {"radius": 0.050, "length": 0.40},
}

# Race-specific proportion multipliers.
_RACE_MODIFIERS = {
    "TerranDescendant":    {"height": 1.00, "bulk": 1.00},
    "SynthBorn":           {"height": 1.05, "bulk": 0.90},
    "PureAlien":           {"height": 1.12, "bulk": 0.85},
    "HybridEvolutionary":  {"height": 0.95, "bulk": 1.10},
}

# Body-type bulk modifiers.
_BODY_TYPE_MODIFIERS = {
    "Organic":    1.00,
    "Augmented":  1.05,
    "Cybernetic": 1.10,
    "FullSynth":  1.15,
    "MechFrame":  1.25,
}

# Number of subdivisions around each cylindrical limb segment.
_LIMB_SEGMENTS = 8


# ---------------------------------------------------------------------------
# Mesh primitive helpers
# ---------------------------------------------------------------------------

def _make_box(cx, cy, cz, sx, sy, sz, vertex_offset=0):
    """Return ``(vertices, faces)`` for an axis-aligned box.

    *cx/cy/cz* — centre, *sx/sy/sz* — half-extents.
    """
    verts = [
        (cx - sx, cy - sy, cz - sz),
        (cx + sx, cy - sy, cz - sz),
        (cx + sx, cy + sy, cz - sz),
        (cx - sx, cy + sy, cz - sz),
        (cx - sx, cy - sy, cz + sz),
        (cx + sx, cy - sy, cz + sz),
        (cx + sx, cy + sy, cz + sz),
        (cx - sx, cy + sy, cz + sz),
    ]
    o = vertex_offset
    faces = [
        (o + 0, o + 1, o + 2, o + 3),
        (o + 4, o + 7, o + 6, o + 5),
        (o + 0, o + 4, o + 5, o + 1),
        (o + 2, o + 6, o + 7, o + 3),
        (o + 0, o + 3, o + 7, o + 4),
        (o + 1, o + 5, o + 6, o + 2),
    ]
    return verts, faces


def _make_cylinder(cx, cy, cz_bottom, radius, height, segments=_LIMB_SEGMENTS,
                   vertex_offset=0):
    """Return ``(vertices, faces)`` for a vertical cylinder."""
    verts = []
    angle_step = 2.0 * math.pi / segments
    for ring in range(2):  # bottom then top
        z = cz_bottom + ring * height
        for i in range(segments):
            angle = i * angle_step
            x = cx + radius * math.cos(angle)
            y = cy + radius * math.sin(angle)
            verts.append((round(x, 5), round(y, 5), round(z, 5)))

    o = vertex_offset
    faces = []
    for i in range(segments):
        j = (i + 1) % segments
        faces.append((o + i, o + j, o + segments + j, o + segments + i))

    # Cap faces
    bottom_cap = tuple(o + i for i in range(segments))
    top_cap = tuple(o + segments + i for i in reversed(range(segments)))
    faces.append(bottom_cap)
    faces.append(top_cap)

    return verts, faces


def _make_sphere(cx, cy, cz, radius, rings=4, segments=_LIMB_SEGMENTS,
                 vertex_offset=0):
    """Return ``(vertices, faces)`` for a UV sphere."""
    verts = []
    # Bottom pole
    verts.append((cx, cy, cz - radius))
    # Intermediate rings
    for r in range(1, rings):
        phi = math.pi * r / rings
        z = cz - radius * math.cos(phi)
        ring_r = radius * math.sin(phi)
        for s in range(segments):
            theta = 2.0 * math.pi * s / segments
            x = cx + ring_r * math.cos(theta)
            y = cy + ring_r * math.sin(theta)
            verts.append((round(x, 5), round(y, 5), round(z, 5)))
    # Top pole
    verts.append((cx, cy, cz + radius))

    o = vertex_offset
    faces = []
    # Bottom fan
    for s in range(segments):
        j = (s + 1) % segments
        faces.append((o, o + 1 + s, o + 1 + j))

    # Middle quads
    for r in range(rings - 2):
        base = o + 1 + r * segments
        for s in range(segments):
            j = (s + 1) % segments
            faces.append((
                base + s, base + segments + s,
                base + segments + j, base + j,
            ))

    # Top fan
    top_idx = o + len(verts) - 1
    base = o + 1 + (rings - 2) * segments
    for s in range(segments):
        j = (s + 1) % segments
        faces.append((top_idx, base + j, base + s))

    return verts, faces


# ---------------------------------------------------------------------------
# Humanoid mesh assembly
# ---------------------------------------------------------------------------

def _build_body_mesh(race, body_type, rng):
    """Assemble a humanoid mesh from procedural body parts.

    Returns a dict ``{part_name: {"vertices": [...], "faces": [...]}}``.
    """
    race_mod = _RACE_MODIFIERS.get(race, {"height": 1.0, "bulk": 1.0})
    bt_bulk = _BODY_TYPE_MODIFIERS.get(body_type, 1.0)

    h_scale = race_mod["height"] * rng.uniform(0.95, 1.05)
    b_scale = race_mod["bulk"] * bt_bulk * rng.uniform(0.95, 1.05)

    bp = {}
    for key, vals in _BASE_PROPORTIONS.items():
        bp[key] = {}
        for k, v in vals.items():
            if k in ("radius", "width", "depth"):
                bp[key][k] = v * b_scale
            else:
                bp[key][k] = v * h_scale

    parts = {}

    # --- Head (sphere) ---
    head_cz = (bp["torso"]["height"] + bp["upper_leg"]["length"]
               + bp["lower_leg"]["length"] + bp["head"]["radius"])
    v, f = _make_sphere(0, 0, head_cz, bp["head"]["radius"])
    parts["head"] = {"vertices": v, "faces": f}

    # --- Torso (box) ---
    torso_bottom = bp["upper_leg"]["length"] + bp["lower_leg"]["length"]
    torso_cz = torso_bottom + bp["torso"]["height"] / 2
    v, f = _make_box(
        0, 0, torso_cz,
        bp["torso"]["width"] / 2,
        bp["torso"]["depth"] / 2,
        bp["torso"]["height"] / 2,
    )
    parts["torso"] = {"vertices": v, "faces": f}

    # --- Arms (cylinders × 2 per side) ---
    shoulder_z = torso_bottom + bp["torso"]["height"] * 0.92
    for side, sign in [("left", -1), ("right", 1)]:
        arm_x = sign * (bp["torso"]["width"] / 2 + bp["upper_arm"]["radius"])

        # Upper arm
        v, f = _make_cylinder(
            arm_x, 0,
            shoulder_z - bp["upper_arm"]["length"],
            bp["upper_arm"]["radius"],
            bp["upper_arm"]["length"],
        )
        parts[f"{side}_upper_arm"] = {"vertices": v, "faces": f}

        # Lower arm
        elbow_z = shoulder_z - bp["upper_arm"]["length"]
        v, f = _make_cylinder(
            arm_x, 0,
            elbow_z - bp["lower_arm"]["length"],
            bp["lower_arm"]["radius"],
            bp["lower_arm"]["length"],
        )
        parts[f"{side}_lower_arm"] = {"vertices": v, "faces": f}

    # --- Legs (cylinders × 2 per side) ---
    hip_z = bp["upper_leg"]["length"] + bp["lower_leg"]["length"]
    for side, sign in [("left", -1), ("right", 1)]:
        leg_x = sign * bp["torso"]["width"] * 0.25

        # Upper leg
        v, f = _make_cylinder(
            leg_x, 0,
            bp["lower_leg"]["length"],
            bp["upper_leg"]["radius"],
            bp["upper_leg"]["length"],
        )
        parts[f"{side}_upper_leg"] = {"vertices": v, "faces": f}

        # Lower leg
        v, f = _make_cylinder(
            leg_x, 0,
            0,
            bp["lower_leg"]["radius"],
            bp["lower_leg"]["length"],
        )
        parts[f"{side}_lower_leg"] = {"vertices": v, "faces": f}

    return parts


# ---------------------------------------------------------------------------
# Public API
# ---------------------------------------------------------------------------

def generate_character(seed, char_id):
    """Generate character metadata and mesh geometry data.

    Args:
        seed: Deterministic seed.
        char_id: Unique identifier string.

    Returns:
        dict with character metadata **and** ``mesh_parts`` containing
        per-body-part vertex/face arrays.
    """
    rng = random.Random(seed)

    race = rng.choice(RACES)
    body_type = rng.choice(BODY_TYPES)

    # Cyber limbs — only for appropriate body types
    cyber_limbs = []
    if body_type in ("Augmented", "Cybernetic", "FullSynth"):
        num_limbs = rng.randint(1, len(CYBER_LIMB_SLOTS))
        cyber_limbs = rng.sample(CYBER_LIMB_SLOTS, k=num_limbs)

    mesh_parts = _build_body_mesh(race, body_type, rng)

    return {
        "char_id": char_id,
        "seed": seed,
        "race": race,
        "body_type": body_type,
        "cyber_limbs": cyber_limbs,
        "mesh_parts": mesh_parts,
    }
