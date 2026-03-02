"""
Render setup for NovaForge asset catalog renders.

Configures camera, lighting, and render settings to produce consistent
catalog images of generated ships for the NovaForge asset pipeline.

Output formats:
  - Catalog render (3/4 view, PNG with transparency)
  - Thumbnail (smaller, front view)
"""

import bpy
import math


# Default render settings for catalog images
CATALOG_RESOLUTION = (1920, 1080)
THUMBNAIL_RESOLUTION = (512, 512)


def setup_catalog_render(ship_object, resolution=None, samples=64):
    """Configure the scene for a catalog render of *ship_object*.

    Places a camera at a 3/4 view angle, adds three-point lighting,
    and sets render output to PNG with transparent background.

    Args:
        ship_object: The root ship object to frame.
        resolution: (width, height) tuple, defaults to CATALOG_RESOLUTION.
        samples: Render sample count (Cycles) or max samples (Eevee).

    Returns:
        The camera object.
    """
    if resolution is None:
        resolution = CATALOG_RESOLUTION

    scene = bpy.context.scene

    # --- Render settings ---
    scene.render.resolution_x = resolution[0]
    scene.render.resolution_y = resolution[1]
    scene.render.film_transparent = True
    scene.render.image_settings.file_format = 'PNG'
    scene.render.image_settings.color_mode = 'RGBA'

    # Use Eevee for speed (Cycles fallback if preferred)
    scene.render.engine = 'BLENDER_EEVEE_NEXT' if hasattr(
        bpy.types, 'RenderSettings') else 'BLENDER_EEVEE'
    if hasattr(scene, 'eevee'):
        scene.eevee.taa_render_samples = samples

    # --- Camera ---
    cam = _get_or_create_camera("CatalogCamera")
    scene.camera = cam

    # Position at 3/4 view (above-right-front)
    dims = ship_object.dimensions
    max_dim = max(dims.x, dims.y, dims.z, 1.0)
    distance = max_dim * 2.5
    cam.location = (
        distance * 0.7,
        -distance * 1.0,
        distance * 0.5,
    )

    # Point at ship centre
    _track_to(cam, ship_object)

    # --- Three-point lighting ---
    _setup_three_point_lighting(ship_object, max_dim)

    return cam


def setup_thumbnail_render(ship_object, resolution=None, samples=32):
    """Configure a front-facing thumbnail render.

    Args:
        ship_object: The root ship object to frame.
        resolution: (width, height), defaults to THUMBNAIL_RESOLUTION.
        samples: Render sample count.

    Returns:
        The camera object.
    """
    if resolution is None:
        resolution = THUMBNAIL_RESOLUTION

    scene = bpy.context.scene
    scene.render.resolution_x = resolution[0]
    scene.render.resolution_y = resolution[1]
    scene.render.film_transparent = True
    scene.render.image_settings.file_format = 'PNG'
    scene.render.image_settings.color_mode = 'RGBA'

    cam = _get_or_create_camera("ThumbnailCamera")
    scene.camera = cam

    dims = ship_object.dimensions
    max_dim = max(dims.x, dims.y, dims.z, 1.0)
    distance = max_dim * 2.2
    cam.location = (distance * 0.3, -distance, distance * 0.3)
    _track_to(cam, ship_object)

    return cam


def render_to_file(filepath):
    """Render the current scene to *filepath*.

    Args:
        filepath: Output file path (without extension; .png appended by Blender).
    """
    bpy.context.scene.render.filepath = filepath
    bpy.ops.render.render(write_still=True)


# ------------------------------------------------------------------
# Internal helpers
# ------------------------------------------------------------------

def _get_or_create_camera(name):
    """Return an existing camera named *name* or create a new one."""
    if name in bpy.data.objects:
        return bpy.data.objects[name]
    cam_data = bpy.data.cameras.new(name)
    cam_obj = bpy.data.objects.new(name, cam_data)
    bpy.context.scene.collection.objects.link(cam_obj)
    return cam_obj


def _track_to(cam, target):
    """Add a Track-To constraint so *cam* always faces *target*."""
    # Remove existing Track To
    for c in cam.constraints:
        if c.type == 'TRACK_TO':
            cam.constraints.remove(c)
    constraint = cam.constraints.new(type='TRACK_TO')
    constraint.target = target
    constraint.track_axis = 'TRACK_NEGATIVE_Z'
    constraint.up_axis = 'UP_Y'


def _setup_three_point_lighting(target, scale):
    """Create key, fill, and rim lights around *target*."""
    # Key light (main, warm)
    _get_or_create_light(
        "Key_Light",
        light_type='SUN',
        energy=3.0,
        color=(1.0, 0.95, 0.9),
        location=(scale * 2, -scale * 2, scale * 3),
    )
    # Fill light (soft, cool)
    _get_or_create_light(
        "Fill_Light",
        light_type='SUN',
        energy=1.0,
        color=(0.85, 0.9, 1.0),
        location=(-scale * 2, -scale, scale * 1.5),
    )
    # Rim light (back, bright)
    _get_or_create_light(
        "Rim_Light",
        light_type='SUN',
        energy=2.0,
        color=(0.9, 0.95, 1.0),
        location=(0, scale * 3, scale * 1),
    )


def _get_or_create_light(name, light_type='SUN', energy=1.0, color=(1, 1, 1),
                         location=(0, 0, 0)):
    """Return an existing light or create a new one."""
    if name in bpy.data.objects:
        obj = bpy.data.objects[name]
        obj.location = location
        obj.data.energy = energy
        obj.data.color = color
        return obj
    light_data = bpy.data.lights.new(name, type=light_type)
    light_data.energy = energy
    light_data.color = color
    light_obj = bpy.data.objects.new(name, light_data)
    light_obj.location = location
    bpy.context.scene.collection.objects.link(light_obj)
    return light_obj


def register():
    """Register this module."""
    pass


def unregister():
    """Unregister this module."""
    pass
