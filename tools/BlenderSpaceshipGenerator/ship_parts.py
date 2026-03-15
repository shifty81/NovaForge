"""
Ship parts generation module
Generates individual ship components (hull, cockpit, engines, wings, weapons, turrets)

Engine generation supports three archetypes (MAIN_THRUST, MANEUVERING,
UTILITY_EXHAUST) for visual variety defined in :mod:`brick_system`.
"""

import bpy
import bmesh
import random
import math
from mathutils import Vector
from . import brick_system


# Maximum number of turret hardpoints any ship may have
MAX_TURRET_HARDPOINTS = 10


def _prefixed_name(prefix, name):
    """Return name with project prefix applied if prefix is non-empty."""
    if prefix:
        return f"{prefix}_{name}"
    return name


def create_mesh_object(name, verts, edges, faces):
    """Helper function to create a mesh object from vertices, edges, and faces"""
    mesh = bpy.data.meshes.new(name)
    mesh.from_pydata(verts, edges, faces)
    mesh.update()
    
    obj = bpy.data.objects.new(name, mesh)
    return obj


def generate_hull(segments=5, scale=1.0, complexity=1.0, symmetry=True, style='MIXED',
                  naming_prefix=''):
    """
    Generate the main hull of the spaceship using a bmesh octagonal profile
    extruded along the Y axis with nose/tail tapering.

    Args:
        segments: Number of hull segments
        scale: Overall scale factor
        complexity: Geometry complexity (0.1-3.0)
        symmetry: Use symmetrical design
        style: Design style
        naming_prefix: Project naming prefix
    """
    half_width = scale * 0.25
    half_height = scale * 0.15
    hull_length = scale

    num_sides = 8
    seg_count = max(3, segments)
    y_positions = [hull_length * (i / (seg_count - 1) - 0.5) for i in range(seg_count)]

    # Per-segment taper factor: 1.0 at centre, narrowing at nose and tail
    _REAR_TAPER_MIN = 0.35   # minimum rear cross-section factor
    _NOSE_TAPER_MIN = 0.30   # minimum nose cross-section factor

    taper_factors = []
    for i in range(seg_count):
        t = i / (seg_count - 1)  # 0 = rear, 1 = front
        # Smooth taper: full width in middle third, narrows toward ends
        if t < 0.25:
            factor = _REAR_TAPER_MIN + (1.0 - _REAR_TAPER_MIN) * (t / 0.25)
        elif t > 0.8:
            factor = _NOSE_TAPER_MIN + (1.0 - _NOSE_TAPER_MIN) * ((1.0 - t) / 0.2)
        else:
            factor = 1.0
        taper_factors.append(factor)

    mesh = bpy.data.meshes.new(_prefixed_name(naming_prefix, "Hull_Mesh"))
    bm = bmesh.new()

    ring_vert_lists = []
    for si, y in enumerate(y_positions):
        tf = taper_factors[si]
        ring_verts = []
        for vi in range(num_sides):
            angle = 2.0 * math.pi * vi / num_sides
            x = math.cos(angle) * half_width * tf
            z = math.sin(angle) * half_height * tf
            # Random organic displacement scaled by complexity
            disp = random.uniform(-0.012, 0.012) * scale * complexity
            ring_verts.append(bm.verts.new(Vector((x + disp, y, z + disp))))
        ring_vert_lists.append(ring_verts)

    bm.verts.ensure_lookup_table()

    # Connect adjacent rings with quad faces
    for si in range(seg_count - 1):
        cur = ring_vert_lists[si]
        nxt = ring_vert_lists[si + 1]
        for vi in range(num_sides):
            vi_next = (vi + 1) % num_sides
            bm.faces.new([cur[vi], nxt[vi], nxt[vi_next], cur[vi_next]])

    # Cap the rear (index 0) and front (index -1)
    rear_face = bm.faces.new(ring_vert_lists[0][::-1])
    front_face = bm.faces.new(ring_vert_lists[-1])

    bm.normal_update()
    bm.to_mesh(mesh)
    bm.free()
    mesh.update()

    hull = bpy.data.objects.new(_prefixed_name(naming_prefix, "Hull"), mesh)
    bpy.context.collection.objects.link(hull)
    bpy.context.view_layer.objects.active = hull
    hull.select_set(True)

    # Apply style-specific modifications
    if style == 'X4':
        apply_x4_style(hull, scale)
    elif style == 'ELITE':
        apply_elite_style(hull, scale)
    elif style == 'EVE':
        apply_eve_style(hull, scale)
    elif style == 'SOLARI':
        apply_solari_style(hull, scale)
    elif style == 'VEYREN':
        apply_veyren_style(hull, scale)
    elif style == 'AURELIAN':
        apply_aurelian_style(hull, scale)
    elif style == 'KELDARI':
        apply_keldari_style(hull, scale)
    elif style == 'NMS':
        apply_nms_style(hull, scale)
    else:
        apply_mixed_style(hull, scale)

    # Add smooth shading
    bpy.ops.object.shade_smooth()

    # Add subdivision surface modifier for smoother look
    modifier = hull.modifiers.new(name="Subdivision", type='SUBSURF')
    modifier.levels = 1
    modifier.render_levels = 2

    # Add edge split for hard edges
    edge_split = hull.modifiers.new(name="EdgeSplit", type='EDGE_SPLIT')
    edge_split.split_angle = math.radians(30)

    return hull


def apply_x4_style(hull, scale):
    """Apply X4 Foundations style - angular and geometric"""
    bpy.context.view_layer.objects.active = hull
    bpy.ops.object.mode_set(mode='EDIT')
    bpy.ops.mesh.bevel(offset=0.1 * scale, segments=1)
    bpy.ops.object.mode_set(mode='OBJECT')


def apply_elite_style(hull, scale):
    """Apply Elite Dangerous style - sleek and aerodynamic"""
    bpy.context.view_layer.objects.active = hull
    bpy.ops.object.mode_set(mode='EDIT')
    bpy.ops.transform.taper(value=0.7)
    bpy.ops.object.mode_set(mode='OBJECT')


def apply_eve_style(hull, scale):
    """Apply Eve Online style - organic and flowing"""
    # Add a smooth deformation for organic look
    bpy.context.view_layer.objects.active = hull
    bpy.ops.object.mode_set(mode='EDIT')
    # Add some organic variation with proportional editing concept
    bpy.ops.mesh.subdivide(number_cuts=1)
    bpy.ops.object.mode_set(mode='OBJECT')
    
    # Add a cast modifier for more organic curves
    cast_mod = hull.modifiers.new(name="Cast", type='CAST')
    cast_mod.factor = 0.3
    cast_mod.cast_type = 'SPHERE'


def apply_mixed_style(hull, scale):
    """Apply mixed style from all inspirations"""
    bpy.context.view_layer.objects.active = hull
    bpy.ops.object.mode_set(mode='EDIT')
    bpy.ops.mesh.bevel(offset=0.05 * scale, segments=2)
    bpy.ops.object.mode_set(mode='OBJECT')


def apply_solari_style(hull, scale):
    """Apply Solari faction style - golden, elegant, armor-focused"""
    bpy.context.view_layer.objects.active = hull
    bpy.ops.object.mode_set(mode='EDIT')
    bpy.ops.mesh.bevel(offset=0.08 * scale, segments=3)
    bpy.ops.object.mode_set(mode='OBJECT')

    # Add smooth curves for elegant look
    cast_mod = hull.modifiers.new(name="Cast", type='CAST')
    cast_mod.factor = 0.15
    cast_mod.cast_type = 'SPHERE'


def apply_veyren_style(hull, scale):
    """Apply Veyren faction style - angular, utilitarian, shield-focused"""
    bpy.context.view_layer.objects.active = hull
    bpy.ops.object.mode_set(mode='EDIT')
    bpy.ops.mesh.bevel(offset=0.12 * scale, segments=1)
    bpy.ops.object.mode_set(mode='OBJECT')


def apply_aurelian_style(hull, scale):
    """Apply Aurelian faction style - sleek, organic, drone-focused"""
    bpy.context.view_layer.objects.active = hull
    bpy.ops.object.mode_set(mode='EDIT')
    bpy.ops.mesh.subdivide(number_cuts=1)
    bpy.ops.object.mode_set(mode='OBJECT')

    cast_mod = hull.modifiers.new(name="Cast", type='CAST')
    cast_mod.factor = 0.25
    cast_mod.cast_type = 'SPHERE'


def apply_keldari_style(hull, scale):
    """Apply Keldari faction style - rugged, industrial, missile-focused"""
    bpy.context.view_layer.objects.active = hull
    bpy.ops.object.mode_set(mode='EDIT')
    bpy.ops.mesh.bevel(offset=0.15 * scale, segments=1)
    bpy.ops.object.mode_set(mode='OBJECT')


def apply_nms_style(hull, scale):
    """Apply No Man's Sky style - colorful, varied, mix of smooth and angular.

    NMS ships combine rounded organic surfaces with sharp mechanical details.
    The hull gets a moderate cast towards a sphere for that rounded look plus a
    light bevel to keep panel edges visible.
    """
    bpy.context.view_layer.objects.active = hull
    bpy.ops.object.mode_set(mode='EDIT')
    bpy.ops.mesh.subdivide(number_cuts=1)
    bpy.ops.mesh.bevel(offset=0.06 * scale, segments=2)
    bpy.ops.object.mode_set(mode='OBJECT')

    # Rounded cast for organic NMS silhouette
    cast_mod = hull.modifiers.new(name="NMS_Cast", type='CAST')
    cast_mod.factor = 0.2
    cast_mod.cast_type = 'SPHERE'


def generate_cockpit(scale=1.0, position=(0, 0, 0), ship_class='FIGHTER', style='MIXED',
                     naming_prefix='', grid_size=1.0):
    """
    Generate cockpit/bridge dome canopy for the ship.

    Creates a UV-sphere dome scaled for a windshield look with a flat base
    ring so it sits flush on the hull.

    Args:
        scale: Ship scale factor
        position: Nominal position relative to hull; snapped to the nearest
            half-cell grid centre inside this function.
        ship_class: Type of ship
        style: Design style
        naming_prefix: Project naming prefix
        grid_size: Snap grid cell size from brick_system
    """
    dome_radius = scale * 0.15

    # Snap cockpit origin to the nearest half-cell centre for grid alignment
    position = brick_system.snap_to_grid_half(position, grid_size)

    # Build dome from a UV sphere, keeping only the upper hemisphere
    mesh = bpy.data.meshes.new(_prefixed_name(naming_prefix, "Cockpit_Mesh"))
    bm = bmesh.new()

    # Create a UV sphere inside bmesh
    bmesh.ops.create_uvsphere(bm, u_segments=16, v_segments=8,
                              radius=dome_radius)
    bm.verts.ensure_lookup_table()

    # Remove lower-hemisphere vertices (z < -0.01 relative to centre)
    to_delete = [v for v in bm.verts if v.co.z < -dome_radius * 0.05]
    bmesh.ops.delete(bm, geom=to_delete, context='VERTS')

    # Scale for a windshield canopy shape: wider in X, stretched in Y, squashed in Z
    for v in bm.verts:
        v.co.x *= 1.1
        v.co.y *= 1.6
        v.co.z *= 0.7

    # Create a flat base ring to sit flush on the hull
    bm.verts.ensure_lookup_table()
    boundary_edges = [e for e in bm.edges if e.is_boundary]
    if boundary_edges:
        boundary_verts = set()
        for e in boundary_edges:
            boundary_verts.update(e.verts)
        # Flatten base ring to z=0
        for v in boundary_verts:
            v.co.z = 0.0
        # Fill the base opening
        bmesh.ops.edgenet_fill(bm, edges=boundary_edges)

    bm.normal_update()
    bm.to_mesh(mesh)
    bm.free()
    mesh.update()

    cockpit = bpy.data.objects.new(
        _prefixed_name(naming_prefix, "Cockpit"), mesh)
    cockpit.location = position
    bpy.context.collection.objects.link(cockpit)
    bpy.context.view_layer.objects.active = cockpit
    cockpit.select_set(True)

    # Add smooth shading
    bpy.ops.object.shade_smooth()

    return cockpit


def generate_engines(count=2, scale=1.0, symmetry=True, style='MIXED', naming_prefix='',
                     grid_size=1.0):
    """
    Generate engine units with archetype-based variation.

    Engines are assigned one of three archetypes (MAIN_THRUST, MANEUVERING,
    UTILITY_EXHAUST) based on their index.  Main engines are larger with
    nozzle flares; maneuvering thrusters are small; utility vents are flat.

    Args:
        count: Number of engines
        scale: Ship scale factor
        symmetry: Use symmetrical placement
        style: Design style
        naming_prefix: Project naming prefix
        grid_size: Snap grid cell size from brick_system
    """
    engines = []
    base_engine_size = scale * 0.2

    # Position engines at the rear of the ship (hull spans Y from -scale*0.5 to +scale*0.5)
    rear_position = -scale * 0.45

    # Pre-compute archetype for each logical engine slot
    archetypes = [
        brick_system.select_engine_archetype(i, count)
        for i in range(count)
    ]

    if symmetry and count % 2 == 0:
        # Symmetric engine placement
        spacing = scale * 0.3
        for i in range(count // 2):
            offset = spacing * (i + 0.5)

            arch_name = archetypes[i * 2]
            arch = brick_system.get_engine_archetype(arch_name)
            engine_size = base_engine_size * arch['radius_factor']
            depth = engine_size * 2 * random.uniform(*arch['depth_range'])

            # Snap engine positions to half-cell grid centres
            left_pos = brick_system.snap_to_grid_half((offset, rear_position, 0), grid_size)
            right_pos = brick_system.snap_to_grid_half((-offset, rear_position, 0), grid_size)

            # Left engine
            bpy.ops.mesh.primitive_cylinder_add(
                radius=engine_size,
                depth=depth,
                location=left_pos
            )
            left_engine = bpy.context.active_object
            left_engine.name = _prefixed_name(naming_prefix, f"Engine_L{i+1}")
            left_engine.rotation_euler = (math.radians(90), 0, 0)
            left_engine["engine_archetype"] = arch_name
            engines.append(left_engine)

            # Add nozzle flare for main thrust engines
            if arch['has_nozzle_flare']:
                _add_nozzle_flare(left_engine, engine_size, naming_prefix)

            # Right engine
            bpy.ops.mesh.primitive_cylinder_add(
                radius=engine_size,
                depth=depth,
                location=right_pos
            )
            right_engine = bpy.context.active_object
            right_engine.name = _prefixed_name(naming_prefix, f"Engine_R{i+1}")
            right_engine.rotation_euler = (math.radians(90), 0, 0)
            right_engine["engine_archetype"] = arch_name
            engines.append(right_engine)

            if arch['has_nozzle_flare']:
                _add_nozzle_flare(right_engine, engine_size, naming_prefix)
    else:
        # Non-symmetric or odd count
        for i in range(count):
            x_offset = (i - count / 2) * scale * 0.3

            arch_name = archetypes[i]
            arch = brick_system.get_engine_archetype(arch_name)
            engine_size = base_engine_size * arch['radius_factor']
            depth = engine_size * 2 * random.uniform(*arch['depth_range'])

            # Snap engine position to half-cell grid centre
            eng_pos = brick_system.snap_to_grid_half((x_offset, rear_position, 0), grid_size)

            bpy.ops.mesh.primitive_cylinder_add(
                radius=engine_size,
                depth=depth,
                location=eng_pos
            )
            engine = bpy.context.active_object
            engine.name = _prefixed_name(naming_prefix, f"Engine_{i+1}")
            engine.rotation_euler = (math.radians(90), 0, 0)
            engine["engine_archetype"] = arch_name
            engines.append(engine)

            if arch['has_nozzle_flare']:
                _add_nozzle_flare(engine, engine_size, naming_prefix)

    # Add glow material to engines (strength varies by archetype)
    for engine in engines:
        arch_name = engine.get("engine_archetype", "MAIN_THRUST")
        arch = brick_system.get_engine_archetype(arch_name) or brick_system.ENGINE_ARCHETYPES['MAIN_THRUST']

        mat = bpy.data.materials.new(name="Engine_Glow")
        mat.use_nodes = True
        nodes = mat.node_tree.nodes

        # Add emission shader
        emission = nodes.new(type='ShaderNodeEmission')
        emission.inputs['Color'].default_value = (0.2, 0.5, 1.0, 1.0)  # Blue glow
        emission.inputs['Strength'].default_value = arch['glow_strength']

        output = nodes.get('Material Output')
        mat.node_tree.links.new(emission.outputs['Emission'], output.inputs['Surface'])

        engine.data.materials.append(mat)

    return engines


def _add_nozzle_flare(engine_obj, engine_size, naming_prefix=''):
    """Add a cone-shaped nozzle flare to a main-thrust engine."""
    loc = engine_obj.location
    bpy.ops.mesh.primitive_cone_add(
        radius1=engine_size * 1.3,
        radius2=engine_size * 0.9,
        depth=engine_size * 0.5,
        location=(loc.x, loc.y - engine_size * 1.2, loc.z)
    )
    flare = bpy.context.active_object
    flare.name = _prefixed_name(naming_prefix, f"{engine_obj.name}_Flare")
    flare.rotation_euler = (math.radians(90), 0, 0)
    flare.parent = engine_obj


def generate_wings(scale=1.0, symmetry=True, style='MIXED', naming_prefix='',
                   grid_size=1.0):
    """
    Generate swept-back wing structures with airfoil-like taper.

    Each wing is built from bmesh quads: wider and thicker at the root,
    narrower and thinner at the tip, with a swept-back leading edge.

    Args:
        scale: Ship scale factor
        symmetry: Use symmetrical design
        style: Design style
        naming_prefix: Project naming prefix
        grid_size: Snap grid cell size from brick_system
    """
    wings = []
    span = scale * 0.8
    root_chord = scale * 0.3
    tip_chord = root_chord * 0.35
    root_thickness = scale * 0.04
    tip_thickness = root_thickness * 0.3
    sweep_back = root_chord * 0.4  # leading-edge sweep offset

    def _build_wing(name, sign):
        """Build one wing on the given side (sign: +1 = right/+X, -1 = left/-X)."""
        mesh = bpy.data.meshes.new(name + "_Mesh")
        bm = bmesh.new()

        # Root profile (at X = 0, sits against the hull)
        r_le_y = root_chord * 0.5   # leading edge Y
        r_te_y = -root_chord * 0.5  # trailing edge Y
        rt = root_thickness * 0.5

        # Tip profile (at X = sign * span)
        t_le_y = r_le_y - sweep_back + tip_chord * 0.5
        t_te_y = t_le_y - tip_chord
        tt = tip_thickness * 0.5
        tip_x = sign * span

        # Trailing edge is thinner than the leading edge for an airfoil look
        te_thin = 0.5

        # Root quad (4 verts): top-LE, top-TE, bot-TE, bot-LE
        r_tl = bm.verts.new(Vector((0, r_le_y, rt)))
        r_tt = bm.verts.new(Vector((0, r_te_y, rt * te_thin)))
        r_bt = bm.verts.new(Vector((0, r_te_y, -rt * te_thin)))
        r_bl = bm.verts.new(Vector((0, r_le_y, -rt)))

        # Tip quad
        t_tl = bm.verts.new(Vector((tip_x, t_le_y, tt)))
        t_tt = bm.verts.new(Vector((tip_x, t_te_y, tt * te_thin)))
        t_bt = bm.verts.new(Vector((tip_x, t_te_y, -tt * te_thin)))
        t_bl = bm.verts.new(Vector((tip_x, t_le_y, -tt)))

        # Faces connecting root to tip (6 faces: top, bottom, LE, TE, root cap, tip cap)
        if sign > 0:
            bm.faces.new([r_tl, r_tt, t_tt, t_tl])  # top
            bm.faces.new([r_bt, r_bl, t_bl, t_bt])  # bottom
            bm.faces.new([r_tl, t_tl, t_bl, r_bl])  # leading edge
            bm.faces.new([r_tt, r_bt, t_bt, t_tt])  # trailing edge
            bm.faces.new([r_tl, r_bl, r_bt, r_tt])  # root cap
            bm.faces.new([t_tl, t_tt, t_bt, t_bl])  # tip cap
        else:
            bm.faces.new([r_tl, t_tl, t_tt, r_tt])  # top
            bm.faces.new([r_bt, t_bt, t_bl, r_bl])  # bottom
            bm.faces.new([r_tl, r_bl, t_bl, t_tl])  # leading edge
            bm.faces.new([r_tt, t_tt, t_bt, r_bt])  # trailing edge
            bm.faces.new([r_tl, r_tt, r_bt, r_bl])  # root cap
            bm.faces.new([t_tl, t_bl, t_bt, t_tt])  # tip cap

        bm.normal_update()
        bm.to_mesh(mesh)
        bm.free()
        mesh.update()

        obj = bpy.data.objects.new(name, mesh)
        bpy.context.collection.objects.link(obj)
        bpy.context.view_layer.objects.active = obj
        obj.select_set(True)
        return obj

    right_wing = _build_wing(
        _prefixed_name(naming_prefix, "Wing_Right"), 1)
    # Offset so the wing root sits at the hull edge (+X half-width), snapped to grid
    raw_x = scale * 0.25
    right_wing.location.x = brick_system.snap_to_grid_half((raw_x, 0, 0), grid_size)[0]
    wings.append(right_wing)

    if symmetry:
        left_wing = _build_wing(
            _prefixed_name(naming_prefix, "Wing_Left"), -1)
        # Mirror offset so the left-wing root sits at the hull edge (-X half-width)
        left_wing.location.x = -brick_system.snap_to_grid_half((raw_x, 0, 0), grid_size)[0]
        wings.append(left_wing)

    return wings


def generate_weapon_hardpoints(count=2, scale=1.0, symmetry=True, naming_prefix='',
                               grid_size=1.0):
    """
    Generate weapon hardpoint markers
    
    Args:
        count: Number of weapon hardpoints
        scale: Ship scale factor
        symmetry: Use symmetrical placement
        naming_prefix: Project naming prefix
        grid_size: Snap grid cell size from brick_system
    """
    hardpoints = []
    hardpoint_size = scale * 0.1

    positions = []
    if symmetry and count % 2 == 0:
        # Symmetric placement — snap each pair to grid half-centres
        for i in range(count // 2):
            y_pos = scale * 0.3 + (i * scale * 0.2)
            x_offset = scale * 0.3 + (i * scale * 0.1)
            pos_r = brick_system.snap_to_grid_half((x_offset, y_pos, -scale * 0.1), grid_size)
            pos_l = brick_system.snap_to_grid_half((-x_offset, y_pos, -scale * 0.1), grid_size)
            positions.append(pos_r)
            positions.append(pos_l)
    else:
        # Non-symmetric — snap each position to grid half-centre
        for i in range(count):
            y_pos = scale * 0.3 + (i * scale * 0.2)
            x_pos = (i - count / 2) * scale * 0.2
            positions.append(brick_system.snap_to_grid_half((x_pos, y_pos, -scale * 0.1), grid_size))

    for i, pos in enumerate(positions):
        bpy.ops.mesh.primitive_cylinder_add(
            radius=hardpoint_size,
            depth=hardpoint_size * 2,
            location=pos
        )
        hardpoint = bpy.context.active_object
        hardpoint.name = _prefixed_name(naming_prefix, f"Weapon_Hardpoint_{i+1}")
        hardpoint.rotation_euler = (math.radians(90), 0, 0)
        hardpoints.append(hardpoint)

    return hardpoints


def generate_turret_hardpoints(count=2, scale=1.0, symmetry=True, naming_prefix='',
                               grid_size=1.0):
    """
    Generate turret hardpoint fittings with visual turret geometry.

    Each turret consists of a cylindrical base, a rotation ring (torus),
    and a barrel.  Custom properties are added to each turret for engine
    mapping: ``turret_index``, ``turret_type``, ``tracking_speed`` and
    ``rotation_limits``.

    Ships may have up to 10 turret hardpoints.

    Args:
        count: Number of turret hardpoints (max 10)
        scale: Ship scale factor
        symmetry: Use symmetrical placement
        naming_prefix: Project naming prefix
        grid_size: Snap grid cell size from brick_system

    Returns:
        List of turret hardpoint root objects
    """
    count = min(count, MAX_TURRET_HARDPOINTS)
    turrets = []
    turret_size = scale * 0.12

    # Calculate positions along the dorsal (top) surface of the hull,
    # snapped to half-cell grid centres for modular alignment.
    positions = []
    if symmetry and count % 2 == 0:
        for i in range(count // 2):
            y_pos = scale * 0.2 - (i * scale * 0.25)
            x_offset = scale * 0.2 + (i * scale * 0.08)
            pos_r = brick_system.snap_to_grid_half((x_offset, y_pos, scale * 0.15), grid_size)
            pos_l = brick_system.snap_to_grid_half((-x_offset, y_pos, scale * 0.15), grid_size)
            positions.append(pos_r)
            positions.append(pos_l)
    else:
        for i in range(count):
            y_pos = scale * 0.3 - (i * scale * 0.15)
            x_pos = (i - count / 2) * scale * 0.15
            positions.append(brick_system.snap_to_grid_half((x_pos, y_pos, scale * 0.15), grid_size))

    for i, pos in enumerate(positions):
        turret_name = _prefixed_name(naming_prefix, f"Turret_Hardpoint_{i+1}")

        # --- Turret base (flat cylinder) ---
        bpy.ops.mesh.primitive_cylinder_add(
            radius=turret_size,
            depth=turret_size * 0.4,
            location=pos
        )
        base = bpy.context.active_object
        base.name = turret_name

        # --- Rotation ring (torus around the base) ---
        ring_pos = (pos[0], pos[1], pos[2] + turret_size * 0.25)
        bpy.ops.mesh.primitive_torus_add(
            major_radius=turret_size * 0.8,
            minor_radius=turret_size * 0.08,
            location=ring_pos
        )
        ring = bpy.context.active_object
        ring.name = _prefixed_name(naming_prefix, f"Turret_Ring_{i+1}")
        ring.parent = base

        # --- Barrel ---
        barrel_pos = (pos[0], pos[1] + turret_size * 0.9, pos[2] + turret_size * 0.25)
        bpy.ops.mesh.primitive_cylinder_add(
            radius=turret_size * 0.1,
            depth=turret_size * 1.6,
            location=barrel_pos
        )
        barrel = bpy.context.active_object
        barrel.name = _prefixed_name(naming_prefix, f"Turret_Barrel_{i+1}")
        barrel.rotation_euler = (math.radians(90), 0, 0)
        barrel.parent = base

        # --- Engine mapping custom properties ---
        base["turret_index"] = i + 1
        base["turret_type"] = "projectile"
        base["tracking_speed"] = 30.0
        base["rotation_limits"] = "yaw:360,pitch:90"
        base["hardpoint_size"] = turret_size

        # Apply turret material
        mat = bpy.data.materials.new(
            name=_prefixed_name(naming_prefix, f"Turret_Mat_{i+1}"))
        mat.use_nodes = True
        nodes = mat.node_tree.nodes
        bsdf = nodes.get('Principled BSDF')
        if bsdf:
            bsdf.inputs['Base Color'].default_value = (0.35, 0.35, 0.4, 1.0)
            bsdf.inputs['Metallic'].default_value = 0.9
            bsdf.inputs['Roughness'].default_value = 0.3
        base.data.materials.append(mat)
        ring.data.materials.append(mat)
        barrel.data.materials.append(mat)

        turrets.append(base)

    return turrets


def register():
    """Register this module"""
    pass


def unregister():
    """Unregister this module"""
    pass
