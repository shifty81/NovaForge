"""
Interior generation module
Generates ship interiors with rooms, corridors, and access points
Designed for FPV exploration

Module-aware rooms
------------------
When a module is fitted, a dedicated interior room is generated for that
module type (e.g. a WEAPON module adds an armory, SHIELD adds a shield
control room, etc.).  Call :func:`generate_module_room` with the module
type string to produce the appropriate room geometry.
"""

import bpy
import random
import math


# Standard human scale for FPV (in Blender units, approximately meters)
HUMAN_HEIGHT = 1.8
DOOR_HEIGHT = 2.0
DOOR_WIDTH = 1.0
CORRIDOR_WIDTH = 1.5
CORRIDOR_HEIGHT = 2.5
ROOM_HEIGHT = 3.0

# Wall thickness used by the enclosed-room helper
_WALL_THICKNESS = 0.08


# Mapping from module type to interior room metadata
MODULE_ROOM_TYPES = {
    'CARGO': {
        'room_name': 'Cargo_Hold',
        'width_factor': 0.35,
        'depth_factor': 0.25,
    },
    'WEAPON': {
        'room_name': 'Armory',
        'width_factor': 0.25,
        'depth_factor': 0.20,
    },
    'SHIELD': {
        'room_name': 'Shield_Control',
        'width_factor': 0.20,
        'depth_factor': 0.18,
    },
    'HANGAR': {
        'room_name': 'Hangar_Bay',
        'width_factor': 0.45,
        'depth_factor': 0.35,
    },
    'SENSOR': {
        'room_name': 'Sensor_Ops',
        'width_factor': 0.18,
        'depth_factor': 0.15,
    },
    'POWER': {
        'room_name': 'Power_Core',
        'width_factor': 0.22,
        'depth_factor': 0.20,
    },
}


def _prefixed_name(prefix, name):
    """Return name with project prefix applied if prefix is non-empty."""
    if prefix:
        return f"{prefix}_{name}"
    return name


def _create_enclosed_room(name, width, depth, height, position,
                          naming_prefix, door_wall='south'):
    """Create a fully enclosed room with floor, ceiling, four walls, and a
    doorway cutout on one wall.

    The doorway is created by splitting the chosen wall into two pieces with
    a gap of :data:`DOOR_WIDTH` x :data:`DOOR_HEIGHT` in the centre.

    Args:
        name: Base name for the room objects.
        width: Room width (X extent).
        depth: Room depth (Y extent).
        height: Room height (Z extent).
        position: Centre-bottom ``(x, y, z)`` of the room floor.
        naming_prefix: Project naming prefix.
        door_wall: Which wall gets the doorway — ``'south'`` (-Y),
            ``'north'`` (+Y), ``'east'`` (+X), or ``'west'`` (-X).

    Returns:
        List of created Blender objects.
    """
    objects = []
    px, py, pz = position
    hw = width / 2.0
    hd = depth / 2.0
    wt = _WALL_THICKNESS

    def _add_box(suffix, loc, sx, sy, sz):
        bpy.ops.mesh.primitive_cube_add(size=1, location=loc)
        obj = bpy.context.active_object
        obj.name = _prefixed_name(naming_prefix, f"{name}_{suffix}")
        obj.scale = (sx, sy, sz)
        bpy.ops.object.transform_apply(scale=True)
        objects.append(obj)
        return obj

    # Floor
    _add_box("Floor", (px, py, pz), width, depth, wt)

    # Ceiling
    _add_box("Ceiling", (px, py, pz + height), width, depth, wt)

    # --- Walls (each is a thin box; the door_wall is split) ---
    wall_defs = {
        'north': {
            'loc': (px, py + hd, pz + height / 2),
            'size': (width, wt, height),
            'axis': 'x',
        },
        'south': {
            'loc': (px, py - hd, pz + height / 2),
            'size': (width, wt, height),
            'axis': 'x',
        },
        'east': {
            'loc': (px + hw, py, pz + height / 2),
            'size': (wt, depth, height),
            'axis': 'y',
        },
        'west': {
            'loc': (px - hw, py, pz + height / 2),
            'size': (wt, depth, height),
            'axis': 'y',
        },
    }

    for wall_key, wd in wall_defs.items():
        lx, ly, lz = wd['loc']
        sx, sy, sz = wd['size']

        if wall_key == door_wall:
            # Split into two pieces with a gap for the door.
            # Subtract wall thickness so the door panel edges don't
            # extend past the room corner.
            if wd['axis'] == 'x':
                span = sx  # total wall span along X
                door_hw = min(DOOR_WIDTH / 2.0, span / 2.0 - wt)
                left_w = span / 2.0 - door_hw
                right_w = left_w
                if left_w > wt:
                    _add_box(f"Wall_{wall_key}_L",
                             (lx - door_hw - left_w / 2, ly, lz),
                             left_w, sy, sz)
                if right_w > wt:
                    _add_box(f"Wall_{wall_key}_R",
                             (lx + door_hw + right_w / 2, ly, lz),
                             right_w, sy, sz)
                # Lintel above door
                lintel_h = height - DOOR_HEIGHT
                if lintel_h > wt:
                    _add_box(f"Wall_{wall_key}_Lintel",
                             (lx, ly, pz + DOOR_HEIGHT + lintel_h / 2),
                             DOOR_WIDTH, sy, lintel_h)
            else:
                span = sy
                door_hd = min(DOOR_WIDTH / 2.0, span / 2.0 - wt)
                front_d = span / 2.0 - door_hd
                back_d = front_d
                if front_d > wt:
                    _add_box(f"Wall_{wall_key}_F",
                             (lx, ly - door_hd - front_d / 2, lz),
                             sx, front_d, sz)
                if back_d > wt:
                    _add_box(f"Wall_{wall_key}_B",
                             (lx, ly + door_hd + back_d / 2, lz),
                             sx, back_d, sz)
                lintel_h = height - DOOR_HEIGHT
                if lintel_h > wt:
                    _add_box(f"Wall_{wall_key}_Lintel",
                             (lx, ly, pz + DOOR_HEIGHT + lintel_h / 2),
                             sx, DOOR_WIDTH, lintel_h)
        else:
            _add_box(f"Wall_{wall_key}", (lx, ly, lz), sx, sy, sz)

    return objects


def generate_interior(ship_class='FIGHTER', scale=1.0, crew_capacity=1, naming_prefix=''):
    """
    Generate complete interior for a ship
    
    Args:
        ship_class: Type of ship
        scale: Ship scale factor
        crew_capacity: Number of crew members
        naming_prefix: Project naming prefix
    
    Returns:
        List of interior objects
    """
    interior_objects = []
    
    # Create interior collection
    collection_name = _prefixed_name(naming_prefix, f"Interior_{ship_class}")
    
    # Determine interior layout based on ship class
    if ship_class in ['SHUTTLE', 'FIGHTER']:
        # Small ships: Simple cockpit area
        interior_objects.extend(generate_cockpit_interior(scale, naming_prefix=naming_prefix))
    elif ship_class in ['CORVETTE', 'FRIGATE']:
        # Medium ships: Cockpit + small crew area
        interior_objects.extend(generate_cockpit_interior(scale, naming_prefix=naming_prefix))
        interior_objects.extend(generate_corridor(scale, length=scale * 0.5, naming_prefix=naming_prefix))
        interior_objects.extend(generate_crew_quarters(scale, bunks=crew_capacity, naming_prefix=naming_prefix))
    else:
        # Large ships: Full interior with multiple rooms
        interior_objects.extend(generate_bridge(scale, naming_prefix=naming_prefix))
        interior_objects.extend(generate_corridor_network(scale, crew_capacity, naming_prefix=naming_prefix))
        interior_objects.extend(generate_crew_quarters(scale, bunks=crew_capacity, naming_prefix=naming_prefix))
        interior_objects.extend(generate_cargo_bay(scale, naming_prefix=naming_prefix))
        interior_objects.extend(generate_engine_room(scale, naming_prefix=naming_prefix))
    
    return interior_objects


def generate_cockpit_interior(scale=1.0, naming_prefix=''):
    """
    Generate cockpit/pilot area interior as an enclosed room with console
    and pilot seat.

    Args:
        scale: Ship scale factor
        naming_prefix: Project naming prefix
    """
    objects = []

    room_w = scale * 0.35
    room_d = scale * 0.25
    room_h = ROOM_HEIGHT * 0.85
    pos = (0, scale * 0.7, -scale * 0.15)

    # Enclosed room with angled front — use 'north' doorway as the
    # window/viewport opening
    objects.extend(_create_enclosed_room(
        "Cockpit", room_w, room_d, room_h, pos,
        naming_prefix, door_wall='north'))

    px, py, pz = pos

    # Console (beveled-look wide box at the front of the room)
    bpy.ops.mesh.primitive_cube_add(
        size=1,
        location=(px, py + room_d * 0.35, pz + room_h * 0.35))
    panel = bpy.context.active_object
    panel.name = _prefixed_name(naming_prefix, "Cockpit_Console")
    panel.scale = (room_w * 0.7, 0.12, room_h * 0.22)
    bpy.ops.object.transform_apply(scale=True)
    # Bevel the console edges
    bevel = panel.modifiers.new(name="Bevel", type='BEVEL')
    bevel.width = 0.02
    bevel.segments = 2
    objects.append(panel)

    # Pilot seat — seat base + backrest
    seat_x, seat_y, seat_z = px, py - room_d * 0.1, pz
    bpy.ops.mesh.primitive_cube_add(
        size=1,
        location=(seat_x, seat_y, seat_z + 0.25))
    seat_base = bpy.context.active_object
    seat_base.name = _prefixed_name(naming_prefix, "Pilot_Seat_Base")
    seat_base.scale = (0.4, 0.4, 0.12)
    bpy.ops.object.transform_apply(scale=True)
    objects.append(seat_base)

    bpy.ops.mesh.primitive_cube_add(
        size=1,
        location=(seat_x, seat_y - 0.18, seat_z + 0.55))
    backrest = bpy.context.active_object
    backrest.name = _prefixed_name(naming_prefix, "Pilot_Seat_Back")
    backrest.scale = (0.35, 0.08, 0.35)
    bpy.ops.object.transform_apply(scale=True)
    objects.append(backrest)

    return objects


def generate_bridge(scale=1.0, naming_prefix=''):
    """
    Generate bridge for large ships — an enclosed room with command chair,
    navigation console, and helm/ops stations.

    Args:
        scale: Ship scale factor
        naming_prefix: Project naming prefix
    """
    objects = []

    room_w = scale * 0.55
    room_d = scale * 0.4
    room_h = ROOM_HEIGHT
    pos = (0, scale * 0.7, -scale * 0.15)

    objects.extend(_create_enclosed_room(
        "Bridge", room_w, room_d, room_h, pos,
        naming_prefix, door_wall='south'))

    px, py, pz = pos

    # Command chair (centre, slightly raised)
    bpy.ops.mesh.primitive_cube_add(
        size=1,
        location=(px, py, pz + 0.3))
    cmd_base = bpy.context.active_object
    cmd_base.name = _prefixed_name(naming_prefix, "Command_Chair_Base")
    cmd_base.scale = (0.5, 0.5, 0.15)
    bpy.ops.object.transform_apply(scale=True)
    objects.append(cmd_base)

    bpy.ops.mesh.primitive_cube_add(
        size=1,
        location=(px, py - 0.22, pz + 0.6))
    cmd_back = bpy.context.active_object
    cmd_back.name = _prefixed_name(naming_prefix, "Command_Chair_Back")
    cmd_back.scale = (0.45, 0.08, 0.4)
    bpy.ops.object.transform_apply(scale=True)
    objects.append(cmd_back)

    # Forward console stations (nav + ops)
    for i, (label, x_off) in enumerate(
            [("Nav_Console", -room_w * 0.28), ("Ops_Console", room_w * 0.28)]):
        bpy.ops.mesh.primitive_cube_add(
            size=1,
            location=(px + x_off, py + room_d * 0.3, pz + room_h * 0.28))
        console = bpy.context.active_object
        console.name = _prefixed_name(naming_prefix, label)
        console.scale = (room_w * 0.28, 0.15, room_h * 0.25)
        bpy.ops.object.transform_apply(scale=True)
        bevel = console.modifiers.new(name="Bevel", type='BEVEL')
        bevel.width = 0.015
        bevel.segments = 2
        objects.append(console)

    # Helm console (front centre)
    bpy.ops.mesh.primitive_cube_add(
        size=1,
        location=(px, py + room_d * 0.38, pz + room_h * 0.22))
    helm = bpy.context.active_object
    helm.name = _prefixed_name(naming_prefix, "Helm_Console")
    helm.scale = (room_w * 0.22, 0.12, room_h * 0.18)
    bpy.ops.object.transform_apply(scale=True)
    objects.append(helm)

    return objects


def generate_corridor(scale=1.0, length=5.0, start_pos=(0, 0, 0), naming_prefix=''):
    """
    Generate a corridor segment as a proper rectangular tube with floor,
    ceiling, and two side walls.

    Args:
        scale: Ship scale factor
        length: Length of corridor
        start_pos: Starting position
        naming_prefix: Project naming prefix
    """
    objects = []
    sx, sy, sz = start_pos
    wt = _WALL_THICKNESS
    cw = CORRIDOR_WIDTH
    ch = CORRIDOR_HEIGHT
    mid_y = sy + length / 2.0

    def _add_box(suffix, loc, bx, by, bz):
        bpy.ops.mesh.primitive_cube_add(size=1, location=loc)
        obj = bpy.context.active_object
        obj.name = _prefixed_name(naming_prefix, f"Corridor_{suffix}")
        obj.scale = (bx, by, bz)
        bpy.ops.object.transform_apply(scale=True)
        objects.append(obj)

    # Floor
    _add_box("Floor", (sx, mid_y, sz), cw, length, wt)

    # Ceiling
    _add_box("Ceiling", (sx, mid_y, sz + ch), cw, length, wt)

    # Left wall
    _add_box("Wall_Left",
             (sx - cw / 2.0, mid_y, sz + ch / 2.0),
             wt, length, ch)

    # Right wall
    _add_box("Wall_Right",
             (sx + cw / 2.0, mid_y, sz + ch / 2.0),
             wt, length, ch)

    return objects


def generate_corridor_network(scale=1.0, crew_capacity=10, naming_prefix=''):
    """
    Generate network of corridors connecting ship areas
    
    Args:
        scale: Ship scale factor
        crew_capacity: Number of crew (affects corridor count)
        naming_prefix: Project naming prefix
    """
    objects = []
    
    # Main corridor running along ship center
    main_length = scale * 0.6
    objects.extend(generate_corridor(scale, main_length, (0, 0, -scale * 0.15), naming_prefix=naming_prefix))
    
    # Add side corridors for larger ships
    if crew_capacity > 20:
        # Side corridors
        side_length = scale * 0.3
        objects.extend(generate_corridor(scale, side_length, (CORRIDOR_WIDTH, scale * 0.2, -scale * 0.15), naming_prefix=naming_prefix))
        objects.extend(generate_corridor(scale, side_length, (-CORRIDOR_WIDTH, scale * 0.2, -scale * 0.15), naming_prefix=naming_prefix))
    
    return objects


def generate_crew_quarters(scale=1.0, bunks=4, naming_prefix=''):
    """
    Generate crew quarters as an enclosed room with stacked bunk beds.

    Args:
        scale: Ship scale factor
        bunks: Number of bunks to create
        naming_prefix: Project naming prefix
    """
    objects = []

    room_width = max(3.0, bunks * 0.8)
    room_depth = 4.0
    room_h = ROOM_HEIGHT
    pos = (scale * 0.3, -scale * 0.3, -scale * 0.15)

    objects.extend(_create_enclosed_room(
        "Quarters", room_width, room_depth, room_h, pos,
        naming_prefix, door_wall='north'))

    px, py, pz = pos

    # Create stacked bunk beds along the walls
    bunk_spacing = room_width / max(bunks, 1)
    for i in range(bunks):
        x_pos = px - room_width / 2 + (i + 0.5) * bunk_spacing
        # Lower bunk
        bpy.ops.mesh.primitive_cube_add(
            size=1,
            location=(x_pos, py + room_depth * 0.25, pz + 0.25))
        lower = bpy.context.active_object
        lower.name = _prefixed_name(naming_prefix, f"Bunk_Lower_{i+1}")
        lower.scale = (0.75, 1.9, 0.1)
        bpy.ops.object.transform_apply(scale=True)
        objects.append(lower)

        # Upper bunk
        bpy.ops.mesh.primitive_cube_add(
            size=1,
            location=(x_pos, py + room_depth * 0.25, pz + 1.35))
        upper = bpy.context.active_object
        upper.name = _prefixed_name(naming_prefix, f"Bunk_Upper_{i+1}")
        upper.scale = (0.75, 1.9, 0.1)
        bpy.ops.object.transform_apply(scale=True)
        objects.append(upper)

        # Vertical supports for each bunk frame
        for sx_off in (-0.32, 0.32):
            bpy.ops.mesh.primitive_cube_add(
                size=1,
                location=(x_pos + sx_off, py + room_depth * 0.25, pz + 0.8))
            post = bpy.context.active_object
            post.name = _prefixed_name(naming_prefix, f"Bunk_Post_{i+1}")
            post.scale = (0.04, 0.04, 1.2)
            bpy.ops.object.transform_apply(scale=True)
            objects.append(post)

    return objects


def generate_cargo_bay(scale=1.0, naming_prefix=''):
    """
    Generate cargo bay area as a large enclosed room with cargo containers.

    Args:
        scale: Ship scale factor
        naming_prefix: Project naming prefix
    """
    objects = []

    bay_width = scale * 0.5
    bay_depth = scale * 0.4
    bay_height = ROOM_HEIGHT * 1.5
    pos = (0, -scale * 0.5, -scale * 0.15)

    objects.extend(_create_enclosed_room(
        "Cargo_Bay", bay_width, bay_depth, bay_height, pos,
        naming_prefix, door_wall='north'))

    px, py, pz = pos

    # Cargo containers — stacked crates arranged in a grid with gaps
    container_w = 0.7
    container_d = 0.7
    container_h = 0.9
    cols = 3
    rows = 2
    crate_spacing = container_w * 1.2  # gap between crate centres
    # Centre the grid within the bay
    x_start = px - (cols - 1) * crate_spacing * 0.5
    y_start = py - bay_depth * 0.25

    for i in range(cols):
        for j in range(rows):
            cx = x_start + i * crate_spacing
            cy = y_start + j * container_d * 1.2
            bpy.ops.mesh.primitive_cube_add(
                size=1,
                location=(cx, cy, pz + container_h * 0.5 + _WALL_THICKNESS))
            crate = bpy.context.active_object
            crate.name = _prefixed_name(naming_prefix,
                                        f"Cargo_Container_{i}_{j}")
            crate.scale = (container_w, container_d, container_h)
            bpy.ops.object.transform_apply(scale=True)
            objects.append(crate)

    return objects


def generate_engine_room(scale=1.0, naming_prefix=''):
    """
    Generate engine room as an enclosed room with reactor cylinder and
    pipe details.

    Args:
        scale: Ship scale factor
        naming_prefix: Project naming prefix
    """
    objects = []

    room_width = scale * 0.4
    room_depth = scale * 0.3
    room_h = ROOM_HEIGHT
    pos = (0, -scale * 0.8, -scale * 0.15)

    objects.extend(_create_enclosed_room(
        "Engine_Room", room_width, room_depth, room_h, pos,
        naming_prefix, door_wall='north'))

    px, py, pz = pos

    # Reactor core — central glowing cylinder
    core_radius = min(room_width, room_depth) * 0.22
    core_height = room_h * 0.8
    core_pos = (px, py, pz + core_height / 2.0 + _WALL_THICKNESS)
    bpy.ops.mesh.primitive_cylinder_add(
        radius=core_radius,
        depth=core_height,
        location=core_pos)
    core = bpy.context.active_object
    core.name = _prefixed_name(naming_prefix, "Reactor_Core")
    objects.append(core)

    # Glowing material for core
    mat = bpy.data.materials.new(name="Reactor_Glow")
    mat.use_nodes = True
    nodes = mat.node_tree.nodes
    emission = nodes.new(type='ShaderNodeEmission')
    emission.inputs['Color'].default_value = (0.8, 0.3, 0.1, 1.0)
    emission.inputs['Strength'].default_value = 3.0
    output = nodes.get('Material Output')
    mat.node_tree.links.new(emission.outputs['Emission'],
                            output.inputs['Surface'])
    core.data.materials.append(mat)

    # Pipe details — horizontal cylinders running along walls
    pipe_r = 0.04
    for side in (-1, 1):
        for h_frac in (0.3, 0.6):
            pipe_y = py
            pipe_x = px + side * room_width * 0.42
            pipe_z = pz + room_h * h_frac
            bpy.ops.mesh.primitive_cylinder_add(
                radius=pipe_r,
                depth=room_depth * 0.8,
                location=(pipe_x, pipe_y, pipe_z))
            pipe = bpy.context.active_object
            pipe.name = _prefixed_name(naming_prefix, "Eng_Pipe")
            pipe.rotation_euler = (math.radians(90), 0, 0)
            bpy.ops.object.transform_apply(rotation=True)
            objects.append(pipe)

    return objects


def generate_doorway(position=(0, 0, 0), rotation=(0, 0, 0), naming_prefix=''):
    """
    Generate a doorway/access point
    
    Args:
        position: Doorway position
        rotation: Doorway rotation
        naming_prefix: Project naming prefix
    """
    bpy.ops.mesh.primitive_cube_add(
        size=1,
        location=position
    )
    doorway = bpy.context.active_object
    doorway.name = _prefixed_name(naming_prefix, "Doorway")
    doorway.scale = (DOOR_WIDTH, 0.1, DOOR_HEIGHT)
    doorway.rotation_euler = rotation
    bpy.ops.object.transform_apply(scale=True, rotation=True)
    
    return doorway


# ---------------------------------------------------------------------------
# Module-specific interior rooms
# ---------------------------------------------------------------------------


def generate_module_rooms(fitted_module_types, scale=1.0, naming_prefix=''):
    """Generate interior rooms for every unique fitted module type.

    Args:
        fitted_module_types: Iterable of module type strings
            (e.g. ``['WEAPON', 'SHIELD']``).
        scale: Ship scale factor.
        naming_prefix: Project naming prefix.

    Returns:
        List of Blender objects comprising all generated rooms.
    """
    all_objects = []
    type_list = list(fitted_module_types)
    for idx, module_type in enumerate(type_list):
        room_objects = generate_module_room(
            module_type, scale, idx, len(type_list),
            naming_prefix=naming_prefix,
        )
        all_objects.extend(room_objects)
    return all_objects


def generate_module_room(module_type, scale=1.0, index=0, total=1,
                         naming_prefix=''):
    """Generate a single interior room for *module_type*.

    The room is placed along the port (negative-X) side of the ship so it
    doesn't overlap with the main corridor or standard rooms.  Multiple
    module rooms are spread along the Y axis.

    Args:
        module_type: One of the keys in :data:`MODULE_ROOM_TYPES`.
        scale: Ship scale factor.
        index: Room index (for positioning among peers).
        total: Total number of module rooms being generated.
        naming_prefix: Project naming prefix.

    Returns:
        List of Blender objects for the room.
    """
    room_info = MODULE_ROOM_TYPES.get(module_type)
    if room_info is None:
        return []

    room_width = scale * room_info['width_factor']
    room_depth = scale * room_info['depth_factor']

    # Distribute rooms along port side
    y_spread = scale * 0.5
    y_pos = -y_spread / 2 + (index + 0.5) * y_spread / max(total, 1)
    x_pos = -scale * 0.3  # port side

    base_pos = (x_pos, y_pos, -scale * 0.15)
    room_name = room_info['room_name']

    # Dispatch to specialised room builders
    if module_type == 'WEAPON':
        return _generate_armory(base_pos, room_width, room_depth, scale,
                                naming_prefix)
    elif module_type == 'SHIELD':
        return _generate_shield_control(base_pos, room_width, room_depth,
                                        scale, naming_prefix)
    elif module_type == 'SENSOR':
        return _generate_sensor_ops(base_pos, room_width, room_depth, scale,
                                    naming_prefix)
    elif module_type == 'POWER':
        return _generate_power_core_room(base_pos, room_width, room_depth,
                                         scale, naming_prefix)
    elif module_type == 'HANGAR':
        return _generate_hangar_bay_interior(base_pos, room_width, room_depth,
                                             scale, naming_prefix)
    else:
        # CARGO and any unknown types get a generic room
        return _generate_generic_module_room(base_pos, room_width, room_depth,
                                             room_name, scale, naming_prefix)


# -- Armory (WEAPON) --------------------------------------------------------

def _generate_armory(pos, width, depth, scale, naming_prefix):
    objects = []

    # Floor
    bpy.ops.mesh.primitive_cube_add(size=1, location=pos)
    floor = bpy.context.active_object
    floor.name = _prefixed_name(naming_prefix, "Armory_Floor")
    floor.scale = (width, depth, 0.05)
    bpy.ops.object.transform_apply(scale=True)
    objects.append(floor)

    # Weapon rack (a series of thin vertical cubes)
    rack_x = pos[0] - width * 0.4
    for i in range(3):
        bpy.ops.mesh.primitive_cube_add(
            size=1,
            location=(rack_x, pos[1] - depth * 0.3 + i * depth * 0.3,
                      pos[2] + ROOM_HEIGHT * 0.4),
        )
        rack = bpy.context.active_object
        rack.name = _prefixed_name(naming_prefix, f"Weapon_Rack_{i+1}")
        rack.scale = (0.15, 0.6, ROOM_HEIGHT * 0.7)
        bpy.ops.object.transform_apply(scale=True)
        objects.append(rack)

    return objects


# -- Shield Control (SHIELD) ------------------------------------------------

def _generate_shield_control(pos, width, depth, scale, naming_prefix):
    objects = []

    # Floor
    bpy.ops.mesh.primitive_cube_add(size=1, location=pos)
    floor = bpy.context.active_object
    floor.name = _prefixed_name(naming_prefix, "Shield_Control_Floor")
    floor.scale = (width, depth, 0.05)
    bpy.ops.object.transform_apply(scale=True)
    objects.append(floor)

    # Central holographic emitter (sphere with emissive material)
    emitter_pos = (pos[0], pos[1], pos[2] + ROOM_HEIGHT * 0.5)
    bpy.ops.mesh.primitive_uv_sphere_add(radius=width * 0.2,
                                          location=emitter_pos)
    emitter = bpy.context.active_object
    emitter.name = _prefixed_name(naming_prefix, "Shield_Emitter_Holo")
    mat = bpy.data.materials.new(name="Shield_Holo_Mat")
    mat.use_nodes = True
    nodes = mat.node_tree.nodes
    emission = nodes.new(type='ShaderNodeEmission')
    emission.inputs['Color'].default_value = (0.2, 0.5, 1.0, 1.0)
    emission.inputs['Strength'].default_value = 2.5
    output = nodes.get('Material Output')
    mat.node_tree.links.new(emission.outputs['Emission'], output.inputs['Surface'])
    emitter.data.materials.append(mat)
    objects.append(emitter)

    return objects


# -- Sensor Ops (SENSOR) ----------------------------------------------------

def _generate_sensor_ops(pos, width, depth, scale, naming_prefix):
    objects = []

    # Floor
    bpy.ops.mesh.primitive_cube_add(size=1, location=pos)
    floor = bpy.context.active_object
    floor.name = _prefixed_name(naming_prefix, "Sensor_Ops_Floor")
    floor.scale = (width, depth, 0.05)
    bpy.ops.object.transform_apply(scale=True)
    objects.append(floor)

    # Console desk
    bpy.ops.mesh.primitive_cube_add(
        size=1,
        location=(pos[0], pos[1] + depth * 0.3, pos[2] + ROOM_HEIGHT * 0.25),
    )
    console = bpy.context.active_object
    console.name = _prefixed_name(naming_prefix, "Sensor_Console")
    console.scale = (width * 0.6, 0.15, ROOM_HEIGHT * 0.3)
    bpy.ops.object.transform_apply(scale=True)
    objects.append(console)

    return objects


# -- Power Core Room (POWER) ------------------------------------------------

def _generate_power_core_room(pos, width, depth, scale, naming_prefix):
    objects = []

    # Floor
    bpy.ops.mesh.primitive_cube_add(size=1, location=pos)
    floor = bpy.context.active_object
    floor.name = _prefixed_name(naming_prefix, "Power_Core_Floor")
    floor.scale = (width, depth, 0.05)
    bpy.ops.object.transform_apply(scale=True)
    objects.append(floor)

    # Power core cylinder with glow
    core_pos = (pos[0], pos[1], pos[2] + ROOM_HEIGHT * 0.45)
    bpy.ops.mesh.primitive_cylinder_add(
        radius=width * 0.25, depth=ROOM_HEIGHT * 0.8, location=core_pos,
    )
    core = bpy.context.active_object
    core.name = _prefixed_name(naming_prefix, "Power_Core_Unit")
    mat = bpy.data.materials.new(name="Power_Core_Glow")
    mat.use_nodes = True
    nodes = mat.node_tree.nodes
    emission = nodes.new(type='ShaderNodeEmission')
    emission.inputs['Color'].default_value = (0.9, 0.6, 0.1, 1.0)
    emission.inputs['Strength'].default_value = 3.5
    output = nodes.get('Material Output')
    mat.node_tree.links.new(emission.outputs['Emission'], output.inputs['Surface'])
    core.data.materials.append(mat)
    objects.append(core)

    return objects


# -- Hangar Bay Interior (HANGAR) -------------------------------------------

def _generate_hangar_bay_interior(pos, width, depth, scale, naming_prefix):
    objects = []

    bay_height = ROOM_HEIGHT * 1.8

    # Floor
    bpy.ops.mesh.primitive_cube_add(size=1, location=pos)
    floor = bpy.context.active_object
    floor.name = _prefixed_name(naming_prefix, "Hangar_Interior_Floor")
    floor.scale = (width, depth, 0.1)
    bpy.ops.object.transform_apply(scale=True)
    objects.append(floor)

    # Landing pad marker
    bpy.ops.mesh.primitive_cylinder_add(
        radius=width * 0.3, depth=0.05,
        location=(pos[0], pos[1], pos[2] + 0.06),
    )
    pad = bpy.context.active_object
    pad.name = _prefixed_name(naming_prefix, "Landing_Pad")
    objects.append(pad)

    return objects


# -- Generic Module Room (fallback) -----------------------------------------

def _generate_generic_module_room(pos, width, depth, room_name, scale,
                                  naming_prefix):
    objects = []

    bpy.ops.mesh.primitive_cube_add(size=1, location=pos)
    floor = bpy.context.active_object
    floor.name = _prefixed_name(naming_prefix, f"{room_name}_Floor")
    floor.scale = (width, depth, 0.05)
    bpy.ops.object.transform_apply(scale=True)
    objects.append(floor)

    return objects


def register():
    """Register this module"""
    pass


def unregister():
    """Unregister this module"""
    pass
