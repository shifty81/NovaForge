"""
Animation system module for the Blender Spaceship Generator addon.

Sets up animation data (keyframes, action clips) for ship components
that move.  Turrets, drone bay doors, landing gear, and sensor
arrays each receive preset motion cycles stored as reusable Actions.

Frame-rate assumptions default to 24 fps.  All durations in the preset
table are expressed in frames at that rate.
"""

import math

import bpy


# ---------------------------------------------------------------------------
# Constants
# ---------------------------------------------------------------------------

FRAME_RATE = 24

ANIMATION_PRESETS = {
    'TURRET_ROTATE': {
        'description': 'Turret yaw rotation cycle',
        'property': 'rotation_euler',
        'axis': 2,  # Z axis
        'range': (-math.pi, math.pi),
        'duration': 120,  # frames for full cycle
    },
    'BAY_DOOR_OPEN': {
        'description': 'Bay door open/close cycle',
        'property': 'location',
        'axis': 2,  # Z axis
        'range': (0.0, -1.0),  # opens downward, scaled by object size
        'duration': 30,
    },
    'LANDING_GEAR_EXTEND': {
        'description': 'Landing gear extension',
        'property': 'location',
        'axis': 2,  # Z axis
        'range': (0.0, -1.5),  # extends downward, scaled by ship scale
        'duration': 45,
    },
    'RADAR_SPIN': {
        'description': 'Sensor/radar continuous rotation',
        'property': 'rotation_euler',
        'axis': 2,  # Z axis
        'range': (0.0, 2 * math.pi),
        'duration': 60,
    },
    'ENGINE_PULSE': {
        'description': 'Engine glow pulsing',
        'property': 'energy',  # light energy
        'range': (3.0, 5.0),
        'duration': 20,
    },
}


# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------

def _prefixed_name(prefix, name):
    """Return *name* with project prefix applied if *prefix* is non-empty."""
    if prefix:
        return f"{prefix}_{name}"
    return name


def _ensure_animation_data(obj):
    """Guarantee that *obj* has an animation_data block and return it."""
    if obj.animation_data is None:
        obj.animation_data_create()
    return obj.animation_data


# ---------------------------------------------------------------------------
# Per-component animation builders
# ---------------------------------------------------------------------------

def create_turret_animation(turret_obj, tracking_speed=30.0, naming_prefix=''):
    """Create a Z-axis rotation animation for a turret object.

    The full-rotation cycle length is derived from *tracking_speed* (degrees
    per second at :data:`FRAME_RATE` fps).  If the turret carries a
    ``rotation_limits`` custom property the sweep is clamped accordingly;
    otherwise a full 360° cycle is used.

    Args:
        turret_obj: Blender object with ``turret_index`` custom property.
        tracking_speed: Rotation rate in degrees per second.
        naming_prefix: Project naming prefix.

    Returns:
        The created :class:`bpy.types.Action`, or *None* if the object is
        invalid.
    """
    if turret_obj is None:
        return None

    index = turret_obj.get("turret_index", 0)

    # Determine sweep from rotation_limits if available.
    yaw_limit = 360.0
    limits_str = turret_obj.get("rotation_limits", "")
    if limits_str:
        for part in str(limits_str).split(","):
            part = part.strip()
            if part.lower().startswith("yaw:"):
                try:
                    yaw_limit = float(part.split(":")[1])
                except (ValueError, IndexError):
                    pass

    sweep_rad = math.radians(yaw_limit)

    # Frames for a full sweep at the given tracking speed.
    if tracking_speed <= 0:
        tracking_speed = 30.0
    frames_for_sweep = int((yaw_limit / tracking_speed) * FRAME_RATE)
    frames_for_sweep = max(frames_for_sweep, 2)

    action_name = _prefixed_name(naming_prefix, f"Turret_{index}_Rotate")
    action = bpy.data.actions.new(name=action_name)
    action.use_fake_user = True

    # Animate rotation_euler on the Z axis (index 2).
    fc = action.fcurves.new(data_path="rotation_euler", index=2)
    fc.keyframe_points.add(2)
    fc.keyframe_points[0].co = (1.0, 0.0)
    fc.keyframe_points[0].interpolation = 'LINEAR'
    fc.keyframe_points[1].co = (float(frames_for_sweep), sweep_rad)
    fc.keyframe_points[1].interpolation = 'LINEAR'

    anim_data = _ensure_animation_data(turret_obj)
    anim_data.action = action

    return action


def create_bay_door_animation(bay_obj, scale=1.0, naming_prefix=''):
    """Create an open/close animation on a drone bay object's Z location.

    Keyframes: frame 1 = closed (Z 0), frame 30 = open (Z down by
    ``scale * 0.5``), frame 60 = closed (Z 0).

    Args:
        bay_obj: Blender object with ``drone_bay_index`` custom property.
        scale: Ship scale factor applied to opening distance.
        naming_prefix: Project naming prefix.

    Returns:
        The created :class:`bpy.types.Action`, or *None* if the object is
        invalid.
    """
    if bay_obj is None:
        return None

    index = bay_obj.get("drone_bay_index", 0)
    preset = ANIMATION_PRESETS['BAY_DOOR_OPEN']
    duration = preset['duration']
    open_distance = preset['range'][1] * scale * 0.5

    action_name = _prefixed_name(naming_prefix, f"BayDoor_{index}_OpenClose")
    action = bpy.data.actions.new(name=action_name)
    action.use_fake_user = True

    fc = action.fcurves.new(data_path="location", index=2)
    fc.keyframe_points.add(3)
    fc.keyframe_points[0].co = (1.0, 0.0)
    fc.keyframe_points[0].interpolation = 'LINEAR'
    fc.keyframe_points[1].co = (float(duration), open_distance)
    fc.keyframe_points[1].interpolation = 'LINEAR'
    fc.keyframe_points[2].co = (float(duration * 2), 0.0)
    fc.keyframe_points[2].interpolation = 'LINEAR'

    anim_data = _ensure_animation_data(bay_obj)
    anim_data.action = action

    return action


def create_landing_gear(hull_obj, scale=1.0, naming_prefix=''):
    """Create landing gear objects with extend/retract animations.

    Three gear struts are placed beneath the hull and each receives an
    independent action that moves them along the Z axis.

    Args:
        hull_obj: Parent hull object.
        scale: Ship scale factor.
        naming_prefix: Project naming prefix.

    Returns:
        List of ``(gear_object, action)`` tuples.
    """
    if hull_obj is None:
        return []

    preset = ANIMATION_PRESETS['LANDING_GEAR_EXTEND']
    duration = preset['duration']
    extend_dist = preset['range'][1] * scale

    # Placement offsets relative to hull origin: (x, y, z).
    placements = [
        (0.0, scale * 0.8, 0.0),        # centre-front
        (-scale * 0.5, -scale * 0.6, 0.0),  # rear-left
        (scale * 0.5, -scale * 0.6, 0.0),   # rear-right
    ]

    results = []
    for i, offset in enumerate(placements):
        bpy.ops.mesh.primitive_cube_add(size=scale * 0.15, location=offset)
        gear_obj = bpy.context.active_object
        gear_obj.name = _prefixed_name(naming_prefix, f"LandingGear_{i}")
        gear_obj.parent = hull_obj

        action_name = _prefixed_name(
            naming_prefix, f"LandingGear_{i}_ExtendRetract"
        )
        action = bpy.data.actions.new(name=action_name)
        action.use_fake_user = True

        fc = action.fcurves.new(data_path="location", index=2)
        fc.keyframe_points.add(3)
        fc.keyframe_points[0].co = (1.0, 0.0)
        fc.keyframe_points[0].interpolation = 'LINEAR'
        fc.keyframe_points[1].co = (float(duration), extend_dist)
        fc.keyframe_points[1].interpolation = 'LINEAR'
        fc.keyframe_points[2].co = (float(duration * 2), 0.0)
        fc.keyframe_points[2].interpolation = 'LINEAR'

        anim_data = _ensure_animation_data(gear_obj)
        anim_data.action = action

        results.append((gear_obj, action))

    return results


def create_radar_spin_animation(sensor_obj, naming_prefix=''):
    """Create a continuous Z-axis rotation animation for a sensor object.

    The animation covers one full 360° revolution over the duration defined
    in :data:`ANIMATION_PRESETS` ``'RADAR_SPIN'``.

    Args:
        sensor_obj: Blender sensor/radar object.
        naming_prefix: Project naming prefix.

    Returns:
        The created :class:`bpy.types.Action`, or *None* if the object is
        invalid.
    """
    if sensor_obj is None:
        return None

    preset = ANIMATION_PRESETS['RADAR_SPIN']
    duration = preset['duration']
    end_angle = preset['range'][1]

    action_name = _prefixed_name(naming_prefix, "Radar_Spin")
    action = bpy.data.actions.new(name=action_name)
    action.use_fake_user = True

    fc = action.fcurves.new(data_path="rotation_euler", index=2)
    fc.keyframe_points.add(2)
    fc.keyframe_points[0].co = (1.0, 0.0)
    fc.keyframe_points[0].interpolation = 'LINEAR'
    fc.keyframe_points[1].co = (float(duration), end_angle)
    fc.keyframe_points[1].interpolation = 'LINEAR'

    # Make the cycle repeat seamlessly.
    mod = fc.modifiers.new(type='CYCLES')
    mod.mode_before = 'REPEAT'
    mod.mode_after = 'REPEAT'

    anim_data = _ensure_animation_data(sensor_obj)
    anim_data.action = action

    return action


# ---------------------------------------------------------------------------
# High-level entry point
# ---------------------------------------------------------------------------

def setup_ship_animations(hull_obj, scale=1.0, naming_prefix=''):
    """Scan children of *hull_obj* and set up animations for movable parts.

    Turrets (objects carrying ``turret_index``), drone bays
    (``drone_bay_index``), and sensor arrays (name contains ``'Sensor'``)
    are each given appropriate animation actions.

    Args:
        hull_obj: Top-level hull object whose children are inspected.
        scale: Ship scale factor forwarded to animation builders.
        naming_prefix: Project naming prefix.

    Returns:
        Dict mapping ``{object_name: action_name}`` for every animation
        created.  Returns an empty dict when *hull_obj* is *None* or has
        no animatable children.
    """
    if hull_obj is None:
        return {}

    animations = {}

    for child in hull_obj.children:
        # Turret rotation
        if "turret_index" in child:
            speed = child.get("tracking_speed", 30.0)
            action = create_turret_animation(
                child, tracking_speed=speed, naming_prefix=naming_prefix
            )
            if action is not None:
                animations[child.name] = action.name

        # Drone bay door
        if "drone_bay_index" in child:
            action = create_bay_door_animation(
                child, scale=scale, naming_prefix=naming_prefix
            )
            if action is not None:
                animations[child.name] = action.name

        # Sensor / radar spin
        if "Sensor" in child.name:
            action = create_radar_spin_animation(
                child, naming_prefix=naming_prefix
            )
            if action is not None:
                animations[child.name] = action.name

    return animations


# ---------------------------------------------------------------------------
# Registration
# ---------------------------------------------------------------------------

def register():
    """Register this module."""
    pass


def unregister():
    """Unregister this module."""
    pass
