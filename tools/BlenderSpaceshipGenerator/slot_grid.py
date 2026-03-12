"""
Module slot grid system for ship interior layout.

Implements a 3-D grid of module slots with an XS → XXL sizing hierarchy.
Ships have tier-dependent limits on the maximum module size they can fit
and the total number of slots available.  The system supports:

* **ModuleSlot** – a single slot in the grid (position, size, occupancy).
* **SlotGrid** – the grid itself with placement, removal, collision
  detection, adaptive gap-filling, and traversal-graph extraction.
* ``generate_slot_grid`` – high-level helper that creates a slot grid
  for a given ship class, tier, and seed.

Module sizes
~~~~~~~~~~~~
.. code-block:: python

    MODULE_SIZES = {"XS": 1, "S": 2, "M": 6, "L": 10, "XL": 20, "XXL": 40}

Each size value represents the number of unit cells the module occupies.

Tier limits
~~~~~~~~~~~
Each ship class / tier combination imposes:
* ``max_module_size`` – largest size category that will physically fit.
* ``max_slots`` – total number of module slots the hull can house.

All computation uses Python stdlib + ``mathutils`` only (no numpy).
"""

import random

import bpy
import bmesh
from mathutils import Vector

# ---------------------------------------------------------------------------
# Module size definitions (unit cells per module)
# ---------------------------------------------------------------------------

MODULE_SIZES = {
    "XS": 1,
    "S": 2,
    "M": 6,
    "L": 10,
    "XL": 20,
    "XXL": 40,
}

# Ordered from smallest to largest for adaptive fill
_SIZE_ORDER = ["XS", "S", "M", "L", "XL", "XXL"]

# ---------------------------------------------------------------------------
# Ship class tier limits
# ---------------------------------------------------------------------------

TIER_LIMITS = {
    "SHUTTLE":       {"max_module_size": "S",   "max_slots": 4},
    "FIGHTER":       {"max_module_size": "S",   "max_slots": 6},
    "CORVETTE":      {"max_module_size": "M",   "max_slots": 10},
    "FRIGATE":       {"max_module_size": "M",   "max_slots": 16},
    "DESTROYER":     {"max_module_size": "L",   "max_slots": 24},
    "CRUISER":       {"max_module_size": "L",   "max_slots": 32},
    "BATTLECRUISER": {"max_module_size": "XL",  "max_slots": 40},
    "BATTLESHIP":    {"max_module_size": "XL",  "max_slots": 48},
    "CARRIER":       {"max_module_size": "XXL", "max_slots": 56},
    "DREADNOUGHT":   {"max_module_size": "XXL", "max_slots": 64},
    "CAPITAL":       {"max_module_size": "XXL", "max_slots": 80},
    "TITAN":         {"max_module_size": "XXL", "max_slots": 100},
    "INDUSTRIAL":    {"max_module_size": "L",   "max_slots": 20},
    "MINING_BARGE":  {"max_module_size": "M",   "max_slots": 12},
    "EXHUMER":       {"max_module_size": "L",   "max_slots": 18},
    "EXPLORER":      {"max_module_size": "M",   "max_slots": 10},
    "HAULER":        {"max_module_size": "L",   "max_slots": 24},
    "EXOTIC":        {"max_module_size": "M",   "max_slots": 8},
}

# Module type catalogue — each type has a preferred size range
MODULE_TYPES = {
    "REACTOR":        {"min_size": "M",  "max_size": "XXL"},
    "ENGINE":         {"min_size": "S",  "max_size": "XL"},
    "SHIELD":         {"min_size": "S",  "max_size": "L"},
    "WEAPON_TURRET":  {"min_size": "XS", "max_size": "L"},
    "MISSILE_BAY":    {"min_size": "S",  "max_size": "XL"},
    "DRONE_BAY":      {"min_size": "M",  "max_size": "XL"},
    "CARGO_HOLD":     {"min_size": "S",  "max_size": "XXL"},
    "CREW_QUARTERS":  {"min_size": "XS", "max_size": "L"},
    "BRIDGE":         {"min_size": "S",  "max_size": "M"},
    "SENSOR_ARRAY":   {"min_size": "XS", "max_size": "M"},
    "REFINERY":       {"min_size": "M",  "max_size": "XL"},
    "HANGAR":         {"min_size": "L",  "max_size": "XXL"},
    "FACTORY":        {"min_size": "M",  "max_size": "XL"},
    "STORAGE":        {"min_size": "XS", "max_size": "L"},
    "ELEVATOR":       {"min_size": "XS", "max_size": "S"},
    "CORRIDOR":       {"min_size": "XS", "max_size": "S"},
}


# ---------------------------------------------------------------------------
# ModuleSlot
# ---------------------------------------------------------------------------

class ModuleSlot:
    """A single slot in the module grid.

    Parameters
    ----------
    position : tuple[int, int, int]
        Grid coordinate.
    size : str
        One of the keys in :data:`MODULE_SIZES`.
    """

    def __init__(self, position=(0, 0, 0), size="XS"):
        self.position = tuple(position)
        self.size = size if size in MODULE_SIZES else "XS"
        self.module_type = ""
        self.occupied = False
        self.module_id = ""
        self.neighbors: list["ModuleSlot"] = []

    @property
    def cell_count(self):
        return MODULE_SIZES.get(self.size, 1)

    def __repr__(self):
        status = "occupied" if self.occupied else "empty"
        return f"<ModuleSlot pos={self.position} size={self.size} {status}>"


# ---------------------------------------------------------------------------
# SlotGrid
# ---------------------------------------------------------------------------

class SlotGrid:
    """3-D grid of module slots.

    Parameters
    ----------
    dims : tuple[int, int, int]
        Grid dimensions (number of unit cells along each axis).
    """

    def __init__(self, dims=(8, 8, 4)):
        self.dims = dims
        self._slots: dict[tuple[int, int, int], ModuleSlot] = {}
        self._modules: dict[str, list[ModuleSlot]] = {}
        self._next_id = 0

    # -- helpers ------------------------------------------------------------

    def _cells_for(self, origin, size_label):
        """Return list of grid cells a module of *size_label* placed at
        *origin* would occupy (simple 1-D expansion along X)."""
        count = MODULE_SIZES.get(size_label, 1)
        ox, oy, oz = origin
        cells = []
        for i in range(count):
            cells.append((ox + i, oy, oz))
        return cells

    def _in_bounds(self, cells):
        dx, dy, dz = self.dims
        for x, y, z in cells:
            if x < 0 or x >= dx or y < 0 or y >= dy or z < 0 or z >= dz:
                return False
        return True

    # -- public API ---------------------------------------------------------

    def can_fit(self, origin, size_label):
        """Return True if a module of *size_label* fits at *origin*."""
        cells = self._cells_for(origin, size_label)
        if not self._in_bounds(cells):
            return False
        for c in cells:
            if c in self._slots and self._slots[c].occupied:
                return False
        return True

    def place_module(self, origin, size_label, module_type=""):
        """Place a module and return its module ID, or ``""`` on failure."""
        if not self.can_fit(origin, size_label):
            return ""
        cells = self._cells_for(origin, size_label)
        mid = f"mod_{self._next_id}"
        self._next_id += 1
        slot_list = []
        for c in cells:
            slot = ModuleSlot(position=c, size=size_label)
            slot.occupied = True
            slot.module_type = module_type
            slot.module_id = mid
            self._slots[c] = slot
            slot_list.append(slot)
        # Link neighbours within the same module
        for s in slot_list:
            s.neighbors = [o for o in slot_list if o is not s]
        self._modules[mid] = slot_list
        return mid

    def remove_module(self, module_id):
        """Remove a placed module by its ID.  Returns True on success."""
        slots = self._modules.pop(module_id, None)
        if slots is None:
            return False
        for s in slots:
            s.occupied = False
            s.module_type = ""
            s.module_id = ""
            s.neighbors = []
            self._slots.pop(s.position, None)
        return True

    def adaptive_fill(self, rng=None):
        """Fill unoccupied cells with the largest modules that fit,
        working from largest to smallest size category."""
        if rng is None:
            rng = random.Random()
        dx, dy, dz = self.dims
        for size_label in reversed(_SIZE_ORDER):
            for iz in range(dz):
                for iy in range(dy):
                    for ix in range(dx):
                        if self.can_fit((ix, iy, iz), size_label):
                            self.place_module((ix, iy, iz), size_label,
                                              module_type="AUTO_FILL")

    def get_traversal_graph(self):
        """Return a dict mapping module IDs to sets of adjacent module IDs.

        Two modules are adjacent if they have slots in neighbouring grid cells.
        """
        graph: dict[str, set[str]] = {}
        for mid, slots in self._modules.items():
            graph.setdefault(mid, set())
            for slot in slots:
                x, y, z = slot.position
                for dx, dy, dz in [(1, 0, 0), (-1, 0, 0),
                                    (0, 1, 0), (0, -1, 0),
                                    (0, 0, 1), (0, 0, -1)]:
                    nb = (x + dx, y + dy, z + dz)
                    nb_slot = self._slots.get(nb)
                    if nb_slot and nb_slot.module_id and nb_slot.module_id != mid:
                        graph[mid].add(nb_slot.module_id)
        return graph

    @property
    def slot_count(self):
        """Number of occupied slots (counted as modules, not cells)."""
        return len(self._modules)

    def all_slots(self):
        """Iterate over all placed module slots."""
        return list(self._slots.values())


# ---------------------------------------------------------------------------
# High-level generator
# ---------------------------------------------------------------------------

def generate_slot_grid(seed, ship_class, tier=1):
    """Create and populate a :class:`SlotGrid` for the given ship class.

    Parameters
    ----------
    seed : int
        Random seed for reproducibility.
    ship_class : str
        Ship class key (e.g. ``"FRIGATE"``).
    tier : int
        Tech tier (currently unused but reserved for future progression).

    Returns
    -------
    SlotGrid
        A populated grid.
    """
    rng = random.Random(seed)
    limits = TIER_LIMITS.get(ship_class, TIER_LIMITS["FRIGATE"])
    max_slots = limits["max_slots"]
    max_size = limits["max_module_size"]
    max_size_idx = _SIZE_ORDER.index(max_size)

    # Grid dimensions: enough to house max_slots worth of XS cells
    base_dim = int(max_slots ** 0.5) + 2
    dim_x = max(4, base_dim)
    dim_y = max(4, base_dim)
    dim_z = max(2, max_slots // (dim_x * dim_y) + 1)
    grid = SlotGrid(dims=(dim_x, dim_y, dim_z))

    placed = 0
    attempts = 0
    while placed < max_slots and attempts < max_slots * 10:
        attempts += 1
        size_idx = rng.randint(0, max_size_idx)
        size_label = _SIZE_ORDER[size_idx]
        x = rng.randint(0, dim_x - 1)
        y = rng.randint(0, dim_y - 1)
        z = rng.randint(0, dim_z - 1)
        mid = grid.place_module((x, y, z), size_label,
                                module_type=rng.choice(list(MODULE_TYPES.keys())))
        if mid:
            placed += 1

    return grid


# ---------------------------------------------------------------------------
# Blender operator
# ---------------------------------------------------------------------------

class SLOTGRID_OT_generate(bpy.types.Operator):
    """Generate and visualize a module slot grid"""
    bl_idname = "mesh.slotgrid_generate"
    bl_label = "SlotGrid: Generate"
    bl_options = {'REGISTER', 'UNDO'}

    def execute(self, context):
        props = context.scene.spaceship_props
        seed = props.seed if hasattr(props, "seed") else 1
        ship_class = props.ship_class if hasattr(props, "ship_class") else "FRIGATE"

        grid = generate_slot_grid(seed=seed, ship_class=ship_class)

        col_name = "SlotGrid_Preview"
        col = bpy.data.collections.get(col_name)
        if col is None:
            col = bpy.data.collections.new(col_name)
            bpy.context.scene.collection.children.link(col)

        for slot in grid.all_slots():
            bm = bmesh.new()
            bmesh.ops.create_cube(bm, size=0.9)
            mesh = bpy.data.meshes.new(f"slot_{slot.position}")
            bm.to_mesh(mesh)
            bm.free()
            obj = bpy.data.objects.new(f"slot_{slot.position}", mesh)
            obj.location = Vector(slot.position)

            mat_name = f"slot_{slot.size}"
            mat = bpy.data.materials.get(mat_name)
            if mat is None:
                mat = bpy.data.materials.new(mat_name)
                mat.use_nodes = False
                idx = _SIZE_ORDER.index(slot.size) if slot.size in _SIZE_ORDER else 0
                t = idx / max(len(_SIZE_ORDER) - 1, 1)
                mat.diffuse_color = (t, 0.4, 1.0 - t, 0.7)
            obj.data.materials.append(mat)
            col.objects.link(obj)

        self.report({'INFO'},
                    f"Generated slot grid: {grid.slot_count} modules, "
                    f"{len(grid.all_slots())} cells")
        return {'FINISHED'}


# ---------------------------------------------------------------------------
# Registration
# ---------------------------------------------------------------------------

_classes = (
    SLOTGRID_OT_generate,
)


def register():
    for cls in _classes:
        bpy.utils.register_class(cls)


def unregister():
    for cls in reversed(_classes):
        bpy.utils.unregister_class(cls)
