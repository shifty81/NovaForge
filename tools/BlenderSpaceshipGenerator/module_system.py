"""
Module system for progressive ship expansion
Allows ships to have additional modules attached.

Modules influence both the ship interior (dedicated rooms are generated
for each fitted module) and the exterior hull (visible surface features
such as antenna arrays, armor plating, and weapon ports are added).
"""

import bpy
import random
import math


# Module types
MODULE_TYPES = {
    'CARGO': {
        'name': 'Cargo Module',
        'scale_factor': 1.0,
        'shape': 'box',
    },
    'WEAPON': {
        'name': 'Weapon Module',
        'scale_factor': 0.8,
        'shape': 'cylinder',
    },
    'SHIELD': {
        'name': 'Shield Module',
        'scale_factor': 0.7,
        'shape': 'sphere',
    },
    'HANGAR': {
        'name': 'Hangar Module',
        'scale_factor': 1.5,
        'shape': 'box',
    },
    'SENSOR': {
        'name': 'Sensor Module',
        'scale_factor': 0.5,
        'shape': 'cone',
    },
    'POWER': {
        'name': 'Power Module',
        'scale_factor': 0.9,
        'shape': 'cylinder',
    },
}


# ---------------------------------------------------------------------------
# Exterior influence definitions
# ---------------------------------------------------------------------------
# Each module type declares how it visibly affects the hull exterior.
#   hull_feature  – type of surface detail added to the hull
#   accent_color  – RGBA used for the feature material
#   hull_scale    – relative size of the exterior feature (fraction of ship scale)

EXTERIOR_INFLUENCE = {
    'CARGO': {
        'hull_feature': 'container_rails',
        'accent_color': (0.45, 0.35, 0.2, 1.0),
        'hull_scale': 0.12,
    },
    'WEAPON': {
        'hull_feature': 'weapon_port',
        'accent_color': (0.6, 0.15, 0.15, 1.0),
        'hull_scale': 0.10,
    },
    'SHIELD': {
        'hull_feature': 'shield_strip',
        'accent_color': (0.2, 0.4, 0.9, 1.0),
        'hull_scale': 0.08,
    },
    'HANGAR': {
        'hull_feature': 'bay_recess',
        'accent_color': (0.5, 0.5, 0.5, 1.0),
        'hull_scale': 0.18,
    },
    'SENSOR': {
        'hull_feature': 'antenna_array',
        'accent_color': (0.3, 0.8, 0.3, 1.0),
        'hull_scale': 0.07,
    },
    'POWER': {
        'hull_feature': 'power_vent',
        'accent_color': (0.9, 0.6, 0.1, 1.0),
        'hull_scale': 0.09,
    },
}


def _prefixed_name(prefix, name):
    """Return name with project prefix applied if prefix is non-empty."""
    if prefix:
        return f"{prefix}_{name}"
    return name


def generate_modules(count=2, scale=1.0, ship_class='FIGHTER', naming_prefix=''):
    """
    Generate module attachments for the ship
    
    Args:
        count: Number of modules to generate
        scale: Ship scale factor
        ship_class: Type of ship (affects module types)
        naming_prefix: Project naming prefix
    
    Returns:
        List of module objects
    """
    modules = []
    
    # Select appropriate module types based on ship class
    available_types = get_available_modules(ship_class)
    
    # Generate modules
    for i in range(count):
        module_type = random.choice(available_types)
        module = create_module(module_type, scale, i, naming_prefix=naming_prefix)
        modules.append(module)
    
    return modules


def get_available_modules(ship_class):
    """
    Get list of available module types for a ship class
    
    Args:
        ship_class: Type of ship
    
    Returns:
        List of module type keys
    """
    if ship_class in ['SHUTTLE', 'FIGHTER']:
        # Small ships: Limited modules
        return ['WEAPON', 'SHIELD', 'SENSOR']
    elif ship_class in ['CORVETTE', 'FRIGATE']:
        # Medium ships: More variety
        return ['CARGO', 'WEAPON', 'SHIELD', 'SENSOR', 'POWER']
    else:
        # Large ships: All modules
        return list(MODULE_TYPES.keys())


def create_module(module_type, scale=1.0, index=0, naming_prefix=''):
    """
    Create a single module
    
    Args:
        module_type: Type of module to create
        scale: Ship scale factor
        index: Module index for positioning
        naming_prefix: Project naming prefix
    
    Returns:
        Module object
    """
    config = MODULE_TYPES[module_type]
    module_scale = scale * config['scale_factor']
    module_name = _prefixed_name(naming_prefix, config['name'])
    
    # Calculate position (distributed along ship)
    angle = (index / 4) * 2 * math.pi  # Distribute around ship
    radius = scale * 0.4
    x_pos = radius * math.cos(angle)
    z_pos = radius * math.sin(angle)
    y_pos = (index - 1) * scale * 0.3  # Spread along length
    
    position = (x_pos, y_pos, z_pos)
    
    # Create module based on shape
    if config['shape'] == 'box':
        module = create_box_module(position, module_scale, module_name)
    elif config['shape'] == 'cylinder':
        module = create_cylinder_module(position, module_scale, module_name)
    elif config['shape'] == 'sphere':
        module = create_sphere_module(position, module_scale, module_name)
    elif config['shape'] == 'cone':
        module = create_cone_module(position, module_scale, module_name)
    else:
        module = create_box_module(position, module_scale, module_name)
    
    # Store module type for downstream lookups (interior / exterior influence)
    module["module_type"] = module_type

    # Add module-specific details
    add_module_details(module, module_type, module_scale, naming_prefix=naming_prefix)
    
    return module


def create_box_module(position, scale, name):
    """Create a box-shaped module"""
    bpy.ops.mesh.primitive_cube_add(size=scale, location=position)
    module = bpy.context.active_object
    module.name = name
    module.scale = (1.0, 1.2, 0.8)
    bpy.ops.object.transform_apply(scale=True)
    return module


def create_cylinder_module(position, scale, name):
    """Create a cylinder-shaped module"""
    bpy.ops.mesh.primitive_cylinder_add(
        radius=scale * 0.5,
        depth=scale * 1.5,
        location=position
    )
    module = bpy.context.active_object
    module.name = name
    module.rotation_euler = (math.radians(90), 0, 0)
    bpy.ops.object.transform_apply(rotation=True)
    return module


def create_sphere_module(position, scale, name):
    """Create a sphere-shaped module"""
    bpy.ops.mesh.primitive_uv_sphere_add(
        radius=scale * 0.6,
        location=position
    )
    module = bpy.context.active_object
    module.name = name
    return module


def create_cone_module(position, scale, name):
    """Create a cone-shaped module"""
    bpy.ops.mesh.primitive_cone_add(
        radius1=scale * 0.5,
        depth=scale * 1.2,
        location=position
    )
    module = bpy.context.active_object
    module.name = name
    module.rotation_euler = (0, 0, 0)
    return module


def add_module_details(module, module_type, scale, naming_prefix=''):
    """
    Add details to module based on type
    
    Args:
        module: Module object
        module_type: Type of module
        scale: Module scale
        naming_prefix: Project naming prefix
    """
    # Add connection point indicator
    bpy.ops.mesh.primitive_cylinder_add(
        radius=scale * 0.1,
        depth=scale * 0.2,
        location=module.location
    )
    connector = bpy.context.active_object
    connector.name = _prefixed_name(naming_prefix, f"{module.name}_Connector")
    connector.parent = module
    
    # Add type-specific elements
    if module_type == 'WEAPON':
        # Add weapon barrel indicators
        add_weapon_barrels(module, scale, naming_prefix=naming_prefix)
    elif module_type == 'SENSOR':
        # Add sensor dish
        add_sensor_dish(module, scale, naming_prefix=naming_prefix)
    elif module_type == 'SHIELD':
        # Add shield emitter effect
        add_shield_emitter(module, scale, naming_prefix=naming_prefix)
    elif module_type == 'HANGAR':
        # Add hangar bay doors
        add_hangar_doors(module, scale, naming_prefix=naming_prefix)


def add_weapon_barrels(parent, scale, naming_prefix=''):
    """Add weapon barrel indicators to weapon module"""
    for i in range(2):
        offset = (i - 0.5) * scale * 0.3
        bpy.ops.mesh.primitive_cylinder_add(
            radius=scale * 0.05,
            depth=scale * 0.8,
            location=(parent.location.x + offset, parent.location.y + scale * 0.6, parent.location.z)
        )
        barrel = bpy.context.active_object
        barrel.name = _prefixed_name(naming_prefix, f"{parent.name}_Barrel_{i+1}")
        barrel.rotation_euler = (math.radians(90), 0, 0)
        barrel.parent = parent


def add_sensor_dish(parent, scale, naming_prefix=''):
    """Add sensor dish to sensor module"""
    bpy.ops.mesh.primitive_cone_add(
        radius1=scale * 0.4,
        depth=scale * 0.3,
        location=(parent.location.x, parent.location.y, parent.location.z + scale * 0.5)
    )
    dish = bpy.context.active_object
    dish.name = _prefixed_name(naming_prefix, f"{parent.name}_Dish")
    dish.rotation_euler = (math.radians(180), 0, 0)
    dish.parent = parent


def add_shield_emitter(parent, scale, naming_prefix=''):
    """Add shield emitter effect to shield module"""
    # Create emitter ring
    bpy.ops.mesh.primitive_torus_add(
        major_radius=scale * 0.5,
        minor_radius=scale * 0.05,
        location=parent.location
    )
    emitter = bpy.context.active_object
    emitter.name = _prefixed_name(naming_prefix, f"{parent.name}_Emitter")
    emitter.parent = parent
    
    # Add glowing material
    mat = bpy.data.materials.new(name="Shield_Emitter")
    mat.use_nodes = True
    nodes = mat.node_tree.nodes
    emission = nodes.new(type='ShaderNodeEmission')
    emission.inputs['Color'].default_value = (0.3, 0.5, 1.0, 1.0)  # Blue shield color
    emission.inputs['Strength'].default_value = 2.0
    output = nodes.get('Material Output')
    mat.node_tree.links.new(emission.outputs['Emission'], output.inputs['Surface'])
    emitter.data.materials.append(mat)


def add_hangar_doors(parent, scale, naming_prefix=''):
    """Add hangar bay doors to hangar module"""
    # Left door
    bpy.ops.mesh.primitive_cube_add(
        size=1,
        location=(parent.location.x - scale * 0.4, parent.location.y, parent.location.z)
    )
    left_door = bpy.context.active_object
    left_door.name = _prefixed_name(naming_prefix, f"{parent.name}_Door_Left")
    left_door.scale = (scale * 0.1, scale * 0.8, scale * 0.8)
    bpy.ops.object.transform_apply(scale=True)
    left_door.parent = parent
    
    # Right door
    bpy.ops.mesh.primitive_cube_add(
        size=1,
        location=(parent.location.x + scale * 0.4, parent.location.y, parent.location.z)
    )
    right_door = bpy.context.active_object
    right_door.name = _prefixed_name(naming_prefix, f"{parent.name}_Door_Right")
    right_door.scale = (scale * 0.1, scale * 0.8, scale * 0.8)
    bpy.ops.object.transform_apply(scale=True)
    right_door.parent = parent


# ---------------------------------------------------------------------------
# Exterior influence helpers
# ---------------------------------------------------------------------------


def get_exterior_influence(module_type):
    """Return the exterior influence dict for *module_type*, or ``None``."""
    return EXTERIOR_INFLUENCE.get(module_type)


def collect_fitted_module_types(modules):
    """Deduplicate module types from a list of placed module objects.

    Each module object is expected to carry a ``module_type`` custom property
    set during :func:`create_module`.

    Returns:
        Sorted list of unique module type strings.
    """
    seen = set()
    for mod in modules:
        mt = mod.get("module_type")
        if mt and mt in EXTERIOR_INFLUENCE:
            seen.add(mt)
    return sorted(seen)


def apply_exterior_influence(hull, fitted_types, scale=1.0, naming_prefix=''):
    """Add visible hull features for each fitted module type.

    For every unique module type present on the ship a small exterior detail
    is generated on the hull surface.  The feature type and colour are
    determined by :data:`EXTERIOR_INFLUENCE`.

    Args:
        hull: The hull Blender object (features are parented to it).
        fitted_types: Iterable of module type strings (e.g. ``['WEAPON', 'SHIELD']``).
        scale: Ship scale factor.
        naming_prefix: Project naming prefix.

    Returns:
        List of newly created feature objects.
    """
    features = []
    type_list = list(fitted_types)
    for idx, module_type in enumerate(type_list):
        influence = EXTERIOR_INFLUENCE.get(module_type)
        if influence is None:
            continue
        feature = _create_hull_feature(
            hull, module_type, influence, scale, idx, len(type_list),
            naming_prefix=naming_prefix,
        )
        if feature is not None:
            features.append(feature)
    return features


def _create_hull_feature(hull, module_type, influence, scale, index, total,
                         naming_prefix=''):
    """Create a single hull-surface feature for a module type.

    The feature is placed on the dorsal (top) surface of the hull and
    distributed along the Y axis so that multiple features don't overlap.
    """
    feature_scale = scale * influence['hull_scale']
    y_spread = scale * 0.6
    y_pos = -y_spread / 2 + (index + 0.5) * y_spread / max(total, 1)
    z_pos = scale * 0.16  # slightly above hull surface

    feature_name = _prefixed_name(
        naming_prefix,
        f"Hull_{influence['hull_feature']}_{module_type}",
    )

    feature = None
    hull_feature = influence['hull_feature']

    if hull_feature == 'container_rails':
        # Pair of rails along the hull
        bpy.ops.mesh.primitive_cube_add(
            size=1,
            location=(0, y_pos, z_pos),
        )
        feature = bpy.context.active_object
        feature.name = feature_name
        feature.scale = (scale * 0.35, feature_scale * 2, feature_scale * 0.3)
        bpy.ops.object.transform_apply(scale=True)

    elif hull_feature == 'weapon_port':
        # Recessed weapon port
        bpy.ops.mesh.primitive_cylinder_add(
            radius=feature_scale * 0.8,
            depth=feature_scale * 0.5,
            location=(0, y_pos, z_pos),
        )
        feature = bpy.context.active_object
        feature.name = feature_name

    elif hull_feature == 'shield_strip':
        # Glowing strip along hull
        bpy.ops.mesh.primitive_cube_add(
            size=1,
            location=(0, y_pos, z_pos),
        )
        feature = bpy.context.active_object
        feature.name = feature_name
        feature.scale = (scale * 0.4, feature_scale * 0.4, feature_scale * 0.15)
        bpy.ops.object.transform_apply(scale=True)

    elif hull_feature == 'bay_recess':
        # Large bay opening outline
        bpy.ops.mesh.primitive_cube_add(
            size=1,
            location=(0, y_pos, z_pos),
        )
        feature = bpy.context.active_object
        feature.name = feature_name
        feature.scale = (scale * 0.25, feature_scale * 2.5, feature_scale * 0.2)
        bpy.ops.object.transform_apply(scale=True)

    elif hull_feature == 'antenna_array':
        # Antenna mast on dorsal surface
        bpy.ops.mesh.primitive_cone_add(
            radius1=feature_scale * 0.6,
            depth=feature_scale * 2.0,
            location=(0, y_pos, z_pos + feature_scale),
        )
        feature = bpy.context.active_object
        feature.name = feature_name

    elif hull_feature == 'power_vent':
        # Exhaust vent grille
        bpy.ops.mesh.primitive_cube_add(
            size=1,
            location=(0, y_pos, z_pos),
        )
        feature = bpy.context.active_object
        feature.name = feature_name
        feature.scale = (scale * 0.15, feature_scale * 1.2, feature_scale * 0.2)
        bpy.ops.object.transform_apply(scale=True)

    if feature is None:
        return None

    # Apply accent material
    mat = bpy.data.materials.new(name=f"{feature_name}_Mat")
    mat.use_nodes = True
    nodes = mat.node_tree.nodes
    bsdf = nodes.get('Principled BSDF')
    if bsdf:
        bsdf.inputs['Base Color'].default_value = influence['accent_color']
        bsdf.inputs['Metallic'].default_value = 0.8
        bsdf.inputs['Roughness'].default_value = 0.35
    feature.data.materials.append(mat)

    # Store module type for engine mapping
    feature["source_module_type"] = module_type
    feature["hull_feature"] = influence['hull_feature']

    # Parent to hull
    feature.parent = hull

    return feature


def register():
    """Register this module"""
    pass


def unregister():
    """Unregister this module"""
    pass
