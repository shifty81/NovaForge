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
    Generate cockpit/pilot area interior
    
    Args:
        scale: Ship scale factor
        naming_prefix: Project naming prefix
    """
    objects = []
    
    # Create cockpit floor
    bpy.ops.mesh.primitive_cube_add(
        size=1,
        location=(0, scale * 0.7, -scale * 0.15)
    )
    floor = bpy.context.active_object
    floor.name = _prefixed_name(naming_prefix, "Cockpit_Floor")
    floor.scale = (scale * 0.4, scale * 0.3, 0.05)
    bpy.ops.object.transform_apply(scale=True)
    objects.append(floor)
    
    # Create pilot seat
    bpy.ops.mesh.primitive_cube_add(
        size=0.5,
        location=(0, scale * 0.65, -scale * 0.1)
    )
    seat = bpy.context.active_object
    seat.name = _prefixed_name(naming_prefix, "Pilot_Seat")
    objects.append(seat)
    
    # Create control panel
    bpy.ops.mesh.primitive_cube_add(
        size=1,
        location=(0, scale * 0.8, -scale * 0.05)
    )
    panel = bpy.context.active_object
    panel.name = _prefixed_name(naming_prefix, "Control_Panel")
    panel.scale = (scale * 0.3, 0.1, scale * 0.2)
    bpy.ops.object.transform_apply(scale=True)
    objects.append(panel)
    
    return objects


def generate_bridge(scale=1.0, naming_prefix=''):
    """
    Generate bridge for large ships
    
    Args:
        scale: Ship scale factor
        naming_prefix: Project naming prefix
    """
    objects = []
    
    # Bridge floor
    bpy.ops.mesh.primitive_cube_add(
        size=1,
        location=(0, scale * 0.7, -scale * 0.15)
    )
    floor = bpy.context.active_object
    floor.name = _prefixed_name(naming_prefix, "Bridge_Floor")
    floor.scale = (scale * 0.6, scale * 0.4, 0.05)
    bpy.ops.object.transform_apply(scale=True)
    objects.append(floor)
    
    # Bridge walls
    wall_height = ROOM_HEIGHT
    
    # Front wall
    bpy.ops.mesh.primitive_cube_add(
        size=1,
        location=(0, scale * 0.9, -scale * 0.15 + wall_height / 2)
    )
    front_wall = bpy.context.active_object
    front_wall.name = _prefixed_name(naming_prefix, "Bridge_Wall_Front")
    front_wall.scale = (scale * 0.6, 0.1, wall_height)
    bpy.ops.object.transform_apply(scale=True)
    objects.append(front_wall)
    
    # Command chair
    bpy.ops.mesh.primitive_cube_add(
        size=0.7,
        location=(0, scale * 0.7, -scale * 0.1)
    )
    command_chair = bpy.context.active_object
    command_chair.name = _prefixed_name(naming_prefix, "Command_Chair")
    objects.append(command_chair)
    
    # Navigation console
    bpy.ops.mesh.primitive_cube_add(
        size=1,
        location=(scale * 0.2, scale * 0.75, -scale * 0.05)
    )
    nav_console = bpy.context.active_object
    nav_console.name = _prefixed_name(naming_prefix, "Nav_Console")
    nav_console.scale = (0.4, 0.4, 0.6)
    bpy.ops.object.transform_apply(scale=True)
    objects.append(nav_console)
    
    return objects


def generate_corridor(scale=1.0, length=5.0, start_pos=(0, 0, 0), naming_prefix=''):
    """
    Generate a corridor segment
    
    Args:
        scale: Ship scale factor
        length: Length of corridor
        start_pos: Starting position
        naming_prefix: Project naming prefix
    """
    objects = []
    
    # Corridor floor
    bpy.ops.mesh.primitive_cube_add(
        size=1,
        location=(start_pos[0], start_pos[1] + length / 2, start_pos[2])
    )
    floor = bpy.context.active_object
    floor.name = _prefixed_name(naming_prefix, "Corridor_Floor")
    floor.scale = (CORRIDOR_WIDTH, length, 0.05)
    bpy.ops.object.transform_apply(scale=True)
    objects.append(floor)
    
    # Corridor ceiling
    bpy.ops.mesh.primitive_cube_add(
        size=1,
        location=(start_pos[0], start_pos[1] + length / 2, start_pos[2] + CORRIDOR_HEIGHT)
    )
    ceiling = bpy.context.active_object
    ceiling.name = _prefixed_name(naming_prefix, "Corridor_Ceiling")
    ceiling.scale = (CORRIDOR_WIDTH, length, 0.05)
    bpy.ops.object.transform_apply(scale=True)
    objects.append(ceiling)
    
    # Corridor walls
    # Left wall
    bpy.ops.mesh.primitive_cube_add(
        size=1,
        location=(start_pos[0] - CORRIDOR_WIDTH / 2, start_pos[1] + length / 2, start_pos[2] + CORRIDOR_HEIGHT / 2)
    )
    left_wall = bpy.context.active_object
    left_wall.name = _prefixed_name(naming_prefix, "Corridor_Wall_Left")
    left_wall.scale = (0.1, length, CORRIDOR_HEIGHT)
    bpy.ops.object.transform_apply(scale=True)
    objects.append(left_wall)
    
    # Right wall
    bpy.ops.mesh.primitive_cube_add(
        size=1,
        location=(start_pos[0] + CORRIDOR_WIDTH / 2, start_pos[1] + length / 2, start_pos[2] + CORRIDOR_HEIGHT / 2)
    )
    right_wall = bpy.context.active_object
    right_wall.name = _prefixed_name(naming_prefix, "Corridor_Wall_Right")
    right_wall.scale = (0.1, length, CORRIDOR_HEIGHT)
    bpy.ops.object.transform_apply(scale=True)
    objects.append(right_wall)
    
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
    Generate crew quarters with bunks
    
    Args:
        scale: Ship scale factor
        bunks: Number of bunks to create
        naming_prefix: Project naming prefix
    """
    objects = []
    
    # Room floor
    room_width = max(3.0, bunks * 0.8)
    room_depth = 4.0
    
    bpy.ops.mesh.primitive_cube_add(
        size=1,
        location=(scale * 0.3, -scale * 0.3, -scale * 0.15)
    )
    floor = bpy.context.active_object
    floor.name = _prefixed_name(naming_prefix, "Quarters_Floor")
    floor.scale = (room_width, room_depth, 0.05)
    bpy.ops.object.transform_apply(scale=True)
    objects.append(floor)
    
    # Create bunks
    bunk_spacing = room_width / bunks if bunks > 0 else 1.0
    for i in range(bunks):
        x_pos = scale * 0.3 - room_width / 2 + (i + 0.5) * bunk_spacing
        bpy.ops.mesh.primitive_cube_add(
            size=1,
            location=(x_pos, -scale * 0.3 + room_depth / 2 - 0.5, -scale * 0.1)
        )
        bunk = bpy.context.active_object
        bunk.name = _prefixed_name(naming_prefix, f"Bunk_{i+1}")
        bunk.scale = (0.8, 2.0, 0.3)
        bpy.ops.object.transform_apply(scale=True)
        objects.append(bunk)
    
    return objects


def generate_cargo_bay(scale=1.0, naming_prefix=''):
    """
    Generate cargo bay area
    
    Args:
        scale: Ship scale factor
        naming_prefix: Project naming prefix
    """
    objects = []
    
    # Large open area for cargo
    bay_width = scale * 0.5
    bay_depth = scale * 0.4
    bay_height = ROOM_HEIGHT * 1.5
    
    bpy.ops.mesh.primitive_cube_add(
        size=1,
        location=(0, -scale * 0.5, -scale * 0.15)
    )
    floor = bpy.context.active_object
    floor.name = _prefixed_name(naming_prefix, "Cargo_Bay_Floor")
    floor.scale = (bay_width, bay_depth, 0.1)
    bpy.ops.object.transform_apply(scale=True)
    objects.append(floor)
    
    # Add cargo containers
    for i in range(3):
        for j in range(2):
            x_pos = -bay_width / 3 + i * bay_width / 3
            y_pos = -scale * 0.5 - bay_depth / 4 + j * bay_depth / 2
            bpy.ops.mesh.primitive_cube_add(
                size=1,
                location=(x_pos, y_pos, -scale * 0.1)
            )
            container = bpy.context.active_object
            container.name = _prefixed_name(naming_prefix, f"Cargo_Container_{i}_{j}")
            container.scale = (0.8, 0.8, 1.0)
            bpy.ops.object.transform_apply(scale=True)
            objects.append(container)
    
    return objects


def generate_engine_room(scale=1.0, naming_prefix=''):
    """
    Generate engine room with machinery
    
    Args:
        scale: Ship scale factor
        naming_prefix: Project naming prefix
    """
    objects = []
    
    # Engine room floor
    room_width = scale * 0.4
    room_depth = scale * 0.3
    
    bpy.ops.mesh.primitive_cube_add(
        size=1,
        location=(0, -scale * 0.8, -scale * 0.15)
    )
    floor = bpy.context.active_object
    floor.name = _prefixed_name(naming_prefix, "Engine_Room_Floor")
    floor.scale = (room_width, room_depth, 0.05)
    bpy.ops.object.transform_apply(scale=True)
    objects.append(floor)
    
    # Reactor core (central)
    bpy.ops.mesh.primitive_cylinder_add(
        radius=scale * 0.1,
        depth=ROOM_HEIGHT,
        location=(0, -scale * 0.8, -scale * 0.15 + ROOM_HEIGHT / 2)
    )
    core = bpy.context.active_object
    core.name = _prefixed_name(naming_prefix, "Reactor_Core")
    objects.append(core)
    
    # Add glowing material to core
    mat = bpy.data.materials.new(name="Reactor_Glow")
    mat.use_nodes = True
    nodes = mat.node_tree.nodes
    emission = nodes.new(type='ShaderNodeEmission')
    emission.inputs['Color'].default_value = (0.8, 0.3, 0.1, 1.0)  # Orange glow
    emission.inputs['Strength'].default_value = 3.0
    output = nodes.get('Material Output')
    mat.node_tree.links.new(emission.outputs['Emission'], output.inputs['Surface'])
    core.data.materials.append(mat)
    
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
