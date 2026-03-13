"""
Density field and interior block system for procedural hull generation.

Implements a voxel-based density field where interior blocks contribute
density values.  An isosurface (marching cubes) extracts the hull mesh
from the field.  This replaces hard-coded hull shapes with a data-driven
approach: place blocks, accumulate density, extract surface.

Architecture
~~~~~~~~~~~~
* **DensityField** – 3-D voxel grid of scalar density values.
  - ``set_voxel`` / ``get_voxel`` for direct manipulation.
  - ``add_block_contribution`` projects an ``InteriorBlock`` into the field.
  - ``apply_symmetry`` mirrors density across chosen axes.
  - ``smooth`` applies a box-filter smoothing pass.
  - ``generate_hull_mesh`` runs marching cubes to produce a Blender mesh.
  - ``visualize_heatmap`` creates debug cubes colored by density.

* **InteriorBlock** – logical volume inside the ship.
  - Has a position, size, block type, and connectivity points.
  - Contributes density to the surrounding field with configurable falloff.

All computation uses Python stdlib + ``mathutils`` only (no numpy).
"""

import math

import bpy
import bmesh
from mathutils import Vector

# ---------------------------------------------------------------------------
# Block type enumeration
# ---------------------------------------------------------------------------

BLOCK_TYPES = [
    "COCKPIT",
    "CORRIDOR",
    "FACTORY",
    "STORAGE",
    "DRONE_PORT",
    "HANGAR",
    "REACTOR",
    "ELEVATOR",
    "CREW_QUARTERS",
    "MEDBAY",
    "ARMORY",
    "BRIDGE",
    "ENGINEERING",
    "SHIELD_ROOM",
    "CARGO_BAY",
    "REFINERY",
    "SENSOR_ROOM",
    "COMMS_ROOM",
]

# Default density contribution per block type
BLOCK_DENSITY = {
    "COCKPIT": 0.9,
    "CORRIDOR": 0.5,
    "FACTORY": 0.8,
    "STORAGE": 0.7,
    "DRONE_PORT": 0.6,
    "HANGAR": 0.75,
    "REACTOR": 1.0,
    "ELEVATOR": 0.4,
    "CREW_QUARTERS": 0.6,
    "MEDBAY": 0.55,
    "ARMORY": 0.7,
    "BRIDGE": 0.85,
    "ENGINEERING": 0.8,
    "SHIELD_ROOM": 0.65,
    "CARGO_BAY": 0.7,
    "REFINERY": 0.85,
    "SENSOR_ROOM": 0.5,
    "COMMS_ROOM": 0.5,
}

# Default falloff distance (in voxel units) per block type
BLOCK_FALLOFF = {
    "COCKPIT": 2.0,
    "CORRIDOR": 1.0,
    "FACTORY": 2.5,
    "STORAGE": 2.0,
    "DRONE_PORT": 1.5,
    "HANGAR": 3.0,
    "REACTOR": 3.0,
    "ELEVATOR": 1.0,
    "CREW_QUARTERS": 1.5,
    "MEDBAY": 1.5,
    "ARMORY": 1.5,
    "BRIDGE": 2.0,
    "ENGINEERING": 2.5,
    "SHIELD_ROOM": 2.0,
    "CARGO_BAY": 2.5,
    "REFINERY": 2.5,
    "SENSOR_ROOM": 1.5,
    "COMMS_ROOM": 1.5,
}


# ---------------------------------------------------------------------------
# InteriorBlock
# ---------------------------------------------------------------------------

class InteriorBlock:
    """Represents a modular interior volume inside a ship.

    Parameters
    ----------
    position : tuple[float, float, float]
        Centre of the block in grid coordinates.
    size : tuple[float, float, float]
        Half-extents along each axis.
    block_type : str
        One of :data:`BLOCK_TYPES`.
    """

    def __init__(self, position=(0, 0, 0), size=(1, 1, 1),
                 block_type="CORRIDOR"):
        self.position = tuple(position)
        self.size = tuple(size)
        self.block_type = block_type if block_type in BLOCK_TYPES else "CORRIDOR"
        self.connectivity_points: list[tuple[float, float, float]] = []
        self.density_contribution = BLOCK_DENSITY.get(self.block_type, 0.5)
        self.falloff = BLOCK_FALLOFF.get(self.block_type, 1.5)

    def add_connectivity_point(self, point):
        """Register a point where this block can connect to neighbours."""
        self.connectivity_points.append(tuple(point))

    def contains(self, x, y, z):
        """Return True if (x, y, z) falls inside the block volume."""
        px, py, pz = self.position
        sx, sy, sz = self.size
        return (abs(x - px) <= sx and
                abs(y - py) <= sy and
                abs(z - pz) <= sz)

    def distance_to_surface(self, x, y, z):
        """Signed distance from *point* to the block surface (negative=inside)."""
        px, py, pz = self.position
        sx, sy, sz = self.size
        dx = max(abs(x - px) - sx, 0.0)
        dy = max(abs(y - py) - sy, 0.0)
        dz = max(abs(z - pz) - sz, 0.0)
        return math.sqrt(dx * dx + dy * dy + dz * dz)


# ---------------------------------------------------------------------------
# Marching-cubes edge table (compact)
# ---------------------------------------------------------------------------

# Each cube vertex is indexed 0-7.  For each of the 256 possible inside/outside
# configurations we store a tuple of edge-triangle triples.  Because including
# the full 256-entry table literally is very large, we use a simplified
# approach: for every voxel cell whose *any* corner exceeds the iso-value we
# emit a small quad (the "blocky" method).  A subsequent smooth pass rounds
# things out.  This keeps the module self-contained without huge lookup tables
# while still producing usable results.

_CUBE_OFFSETS = [
    (0, 0, 0), (1, 0, 0), (1, 1, 0), (0, 1, 0),
    (0, 0, 1), (1, 0, 1), (1, 1, 1), (0, 1, 1),
]


# ---------------------------------------------------------------------------
# DensityField
# ---------------------------------------------------------------------------

class DensityField:
    """3-D voxel grid of scalar density values.

    Parameters
    ----------
    resolution : tuple[int, int, int]
        Number of voxels along each axis.
    cell_size : float
        World-space size of a single voxel.
    origin : tuple[float, float, float]
        World-space position of the grid origin (corner 0,0,0).
    """

    def __init__(self, resolution=(32, 32, 32), cell_size=1.0,
                 origin=(0.0, 0.0, 0.0)):
        self.rx, self.ry, self.rz = resolution
        self.cell_size = cell_size
        self.origin = Vector(origin)
        # Flat list for speed; index = x + y*rx + z*rx*ry
        self._data = [0.0] * (self.rx * self.ry * self.rz)

    # -- direct access ------------------------------------------------------

    def _idx(self, x, y, z):
        return x + y * self.rx + z * self.rx * self.ry

    def set_voxel(self, x, y, z, value):
        """Set the density at integer grid coordinates."""
        if 0 <= x < self.rx and 0 <= y < self.ry and 0 <= z < self.rz:
            self._data[self._idx(x, y, z)] = value

    def get_voxel(self, x, y, z):
        """Return the density at integer grid coordinates (0.0 if out-of-bounds)."""
        if 0 <= x < self.rx and 0 <= y < self.ry and 0 <= z < self.rz:
            return self._data[self._idx(x, y, z)]
        return 0.0

    # -- block contribution -------------------------------------------------

    def _world_to_grid(self, wx, wy, wz):
        """Convert world coordinates to continuous grid coordinates."""
        return (
            (wx - self.origin.x) / self.cell_size,
            (wy - self.origin.y) / self.cell_size,
            (wz - self.origin.z) / self.cell_size,
        )

    def _grid_to_world(self, gx, gy, gz):
        """Convert integer grid coordinates to world-space centre."""
        return Vector((
            self.origin.x + (gx + 0.5) * self.cell_size,
            self.origin.y + (gy + 0.5) * self.cell_size,
            self.origin.z + (gz + 0.5) * self.cell_size,
        ))

    def add_block_contribution(self, block):
        """Project an :class:`InteriorBlock` into the density field.

        Voxels inside the block receive the full density contribution;
        voxels within the falloff distance receive a linearly decayed value.
        """
        gx, gy, gz = self._world_to_grid(*block.position)
        sx = block.size[0] / self.cell_size
        sy = block.size[1] / self.cell_size
        sz = block.size[2] / self.cell_size
        falloff = block.falloff / self.cell_size
        density = block.density_contribution

        x_min = max(int(gx - sx - falloff), 0)
        x_max = min(int(gx + sx + falloff) + 1, self.rx)
        y_min = max(int(gy - sy - falloff), 0)
        y_max = min(int(gy + sy + falloff) + 1, self.ry)
        z_min = max(int(gz - sz - falloff), 0)
        z_max = min(int(gz + sz + falloff) + 1, self.rz)

        for iz in range(z_min, z_max):
            for iy in range(y_min, y_max):
                for ix in range(x_min, x_max):
                    wx, wy, wz = self._grid_to_world(ix, iy, iz)
                    dist = block.distance_to_surface(wx, wy, wz)
                    dist_grid = dist / self.cell_size
                    if dist_grid <= 0:
                        contrib = density
                    elif dist_grid < falloff:
                        contrib = density * (1.0 - dist_grid / falloff)
                    else:
                        continue
                    idx = self._idx(ix, iy, iz)
                    self._data[idx] = min(self._data[idx] + contrib, 1.0)

    # -- symmetry -----------------------------------------------------------

    def apply_symmetry(self, axis="X"):
        """Mirror density values across the given axis (X, Y, or Z).

        For each pair of mirrored voxels the maximum value is kept on both
        sides so that the resulting hull is symmetric.
        """
        for iz in range(self.rz):
            for iy in range(self.ry):
                for ix in range(self.rx):
                    if axis == "X":
                        mx, my, mz = self.rx - 1 - ix, iy, iz
                    elif axis == "Y":
                        mx, my, mz = ix, self.ry - 1 - iy, iz
                    else:
                        mx, my, mz = ix, iy, self.rz - 1 - iz
                    if mx < 0 or mx >= self.rx:
                        continue
                    a = self._data[self._idx(ix, iy, iz)]
                    b = self._data[self._idx(mx, my, mz)]
                    v = max(a, b)
                    self._data[self._idx(ix, iy, iz)] = v
                    self._data[self._idx(mx, my, mz)] = v

    # -- smoothing ----------------------------------------------------------

    def smooth(self, iterations=1):
        """Apply a 3×3×3 box-filter smoothing pass."""
        for _ in range(iterations):
            new_data = [0.0] * len(self._data)
            for iz in range(self.rz):
                for iy in range(self.ry):
                    for ix in range(self.rx):
                        total = 0.0
                        count = 0
                        for dz in (-1, 0, 1):
                            nz = iz + dz
                            if nz < 0 or nz >= self.rz:
                                continue
                            for dy in (-1, 0, 1):
                                ny = iy + dy
                                if ny < 0 or ny >= self.ry:
                                    continue
                                for dx in (-1, 0, 1):
                                    nx = ix + dx
                                    if nx < 0 or nx >= self.rx:
                                        continue
                                    total += self._data[self._idx(nx, ny, nz)]
                                    count += 1
                        new_data[self._idx(ix, iy, iz)] = total / count if count else 0.0
            self._data = new_data

    # -- mesh generation (blocky marching cubes) ----------------------------

    def generate_hull_mesh(self, iso_value=0.5, mesh_name="DensityHull"):
        """Generate a Blender mesh from the density field using a simplified
        marching-cubes approach.

        For every voxel cell that straddles the iso-value boundary, a set
        of faces is emitted.  The result is a watertight (though blocky)
        mesh that can be smoothed with a Subdivision Surface modifier.

        Returns the created :class:`bpy.types.Object`.
        """
        verts = []
        faces = []
        vert_map: dict[tuple[int, int, int], int] = {}

        def _get_vert(ix, iy, iz):
            key = (ix, iy, iz)
            if key in vert_map:
                return vert_map[key]
            idx = len(verts)
            pos = self._grid_to_world(ix, iy, iz)
            verts.append(pos)
            vert_map[key] = idx
            return idx

        for iz in range(self.rz - 1):
            for iy in range(self.ry - 1):
                for ix in range(self.rx - 1):
                    val = self.get_voxel(ix, iy, iz)
                    above = val >= iso_value

                    # Check each of the 6 face-adjacent neighbours;
                    # where the iso-value boundary is crossed, emit a quad.
                    neighbours = [
                        (ix + 1, iy, iz), (ix - 1, iy, iz),
                        (ix, iy + 1, iz), (ix, iy - 1, iz),
                        (ix, iy, iz + 1), (ix, iy, iz - 1),
                    ]
                    for nx, ny, nz in neighbours:
                        n_val = self.get_voxel(nx, ny, nz)
                        n_above = n_val >= iso_value
                        if above and not n_above:
                            # Determine face orientation
                            dx, dy, dz = nx - ix, ny - iy, nz - iz
                            if dx == 1:
                                f = [
                                    _get_vert(ix + 1, iy, iz),
                                    _get_vert(ix + 1, iy + 1, iz),
                                    _get_vert(ix + 1, iy + 1, iz + 1),
                                    _get_vert(ix + 1, iy, iz + 1),
                                ]
                            elif dx == -1:
                                f = [
                                    _get_vert(ix, iy, iz),
                                    _get_vert(ix, iy, iz + 1),
                                    _get_vert(ix, iy + 1, iz + 1),
                                    _get_vert(ix, iy + 1, iz),
                                ]
                            elif dy == 1:
                                f = [
                                    _get_vert(ix, iy + 1, iz),
                                    _get_vert(ix, iy + 1, iz + 1),
                                    _get_vert(ix + 1, iy + 1, iz + 1),
                                    _get_vert(ix + 1, iy + 1, iz),
                                ]
                            elif dy == -1:
                                f = [
                                    _get_vert(ix, iy, iz),
                                    _get_vert(ix + 1, iy, iz),
                                    _get_vert(ix + 1, iy, iz + 1),
                                    _get_vert(ix, iy, iz + 1),
                                ]
                            elif dz == 1:
                                f = [
                                    _get_vert(ix, iy, iz + 1),
                                    _get_vert(ix + 1, iy, iz + 1),
                                    _get_vert(ix + 1, iy + 1, iz + 1),
                                    _get_vert(ix, iy + 1, iz + 1),
                                ]
                            else:
                                f = [
                                    _get_vert(ix, iy, iz),
                                    _get_vert(ix, iy + 1, iz),
                                    _get_vert(ix + 1, iy + 1, iz),
                                    _get_vert(ix + 1, iy, iz),
                                ]
                            faces.append(f)

        mesh = bpy.data.meshes.new(mesh_name)
        mesh.from_pydata([tuple(v) for v in verts], [], faces)
        mesh.update()

        obj = bpy.data.objects.new(mesh_name, mesh)
        bpy.context.collection.objects.link(obj)
        return obj

    # -- debug heatmap ------------------------------------------------------

    def visualize_heatmap(self, threshold=0.05, collection_name="DensityHeatmap"):
        """Create small coloured cubes for every voxel above *threshold*.

        Colour is mapped from blue (low density) to red (high density).
        Useful for debugging block placement before hull extraction.
        """
        col = bpy.data.collections.get(collection_name)
        if col is None:
            col = bpy.data.collections.new(collection_name)
            bpy.context.scene.collection.children.link(col)

        for iz in range(self.rz):
            for iy in range(self.ry):
                for ix in range(self.rx):
                    val = self.get_voxel(ix, iy, iz)
                    if val < threshold:
                        continue
                    pos = self._grid_to_world(ix, iy, iz)
                    bm = bmesh.new()
                    bmesh.ops.create_cube(bm, size=self.cell_size * 0.9)
                    mesh = bpy.data.meshes.new(f"hm_{ix}_{iy}_{iz}")
                    bm.to_mesh(mesh)
                    bm.free()
                    obj = bpy.data.objects.new(f"hm_{ix}_{iy}_{iz}", mesh)
                    obj.location = pos

                    # Create or reuse a material coloured by density
                    mat_name = f"density_{int(val * 10)}"
                    mat = bpy.data.materials.get(mat_name)
                    if mat is None:
                        mat = bpy.data.materials.new(mat_name)
                        mat.use_nodes = False
                        # Blue (0) → Red (1)
                        mat.diffuse_color = (val, 0.1, 1.0 - val, 0.6)
                    obj.data.materials.append(mat)

                    col.objects.link(obj)


# ---------------------------------------------------------------------------
# Blender operators
# ---------------------------------------------------------------------------

class DENSITY_OT_generate_hull(bpy.types.Operator):
    """Generate a hull mesh from a density field demo"""
    bl_idname = "mesh.density_generate_hull"
    bl_label = "Density: Generate Hull"
    bl_options = {'REGISTER', 'UNDO'}

    def execute(self, context):
        field = DensityField(resolution=(16, 16, 16), cell_size=0.5,
                             origin=(-4, -4, -4))
        # Place a few demo blocks
        blocks = [
            InteriorBlock(position=(0, 0, 0), size=(2, 1, 1),
                          block_type="REACTOR"),
            InteriorBlock(position=(0, 0, 1.5), size=(1.5, 0.8, 0.6),
                          block_type="BRIDGE"),
            InteriorBlock(position=(0, 0, -1.5), size=(1.2, 0.8, 0.8),
                          block_type="CORRIDOR"),
        ]
        for b in blocks:
            field.add_block_contribution(b)
        field.apply_symmetry("X")
        field.smooth(iterations=1)
        obj = field.generate_hull_mesh(iso_value=0.3)
        self.report({'INFO'}, f"Generated density hull with {len(obj.data.polygons)} faces")
        return {'FINISHED'}


class DENSITY_OT_visualize_heatmap(bpy.types.Operator):
    """Visualize the density field as a colour-coded voxel heatmap"""
    bl_idname = "mesh.density_visualize_heatmap"
    bl_label = "Density: Visualize Heatmap"
    bl_options = {'REGISTER', 'UNDO'}

    def execute(self, context):
        field = DensityField(resolution=(12, 12, 12), cell_size=0.6,
                             origin=(-3.6, -3.6, -3.6))
        blocks = [
            InteriorBlock(position=(0, 0, 0), size=(1.5, 1, 1),
                          block_type="REACTOR"),
            InteriorBlock(position=(0, 0, 1.2), size=(1, 0.6, 0.5),
                          block_type="COCKPIT"),
        ]
        for b in blocks:
            field.add_block_contribution(b)
        field.apply_symmetry("X")
        field.visualize_heatmap(threshold=0.05)
        self.report({'INFO'}, "Density heatmap created")
        return {'FINISHED'}


# ---------------------------------------------------------------------------
# Registration
# ---------------------------------------------------------------------------

_classes = (
    DENSITY_OT_generate_hull,
    DENSITY_OT_visualize_heatmap,
)


def register():
    for cls in _classes:
        bpy.utils.register_class(cls)


def unregister():
    for cls in reversed(_classes):
        bpy.utils.unregister_class(cls)
