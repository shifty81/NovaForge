"""
Fleet logistics system — drone ports, storage, manufacturing, and resource flow.

Implements the "mobile base" resource pipeline where a fleet of ships acts
as an autonomous economic unit.  Drone ports move resources between storage
blocks and factories.  Factories consume input materials and produce output
goods according to recipes.  The system ticks forward in time, processing
queues and scheduling drone deliveries.

Architecture
~~~~~~~~~~~~
* **DronePort** – input / output / bidirectional link between storage and
  the outside world (mining drones, trade shuttles, etc.).
* **StorageBlock** – finite-capacity item container.
* **FactoryBlock** – recipe-driven manufacturing node.
* **ResourceGraph** – the connectivity graph that owns all blocks and
  provides ``tick(delta_time)`` for simulation.
* **DroneScheduler** – schedules visual drone animations or instant resource
  transfers depending on context.

All computation uses Python stdlib only (no numpy, no bpy required for
the simulation core).  Blender operators are provided for visualisation.
"""

import bpy
import bmesh
from mathutils import Vector

# ---------------------------------------------------------------------------
# Port / block types
# ---------------------------------------------------------------------------

PORT_TYPES = ["INPUT", "OUTPUT", "BIDIRECTIONAL"]

# ---------------------------------------------------------------------------
# Recipes (input → output)
# ---------------------------------------------------------------------------

RECIPES = {
    "REFINE_ORE": {
        "inputs": {"raw_ore": 10},
        "outputs": {"refined_metal": 5},
        "time": 5.0,
        "power": 20.0,
    },
    "MANUFACTURE_PARTS": {
        "inputs": {"refined_metal": 5},
        "outputs": {"ship_parts": 2},
        "time": 8.0,
        "power": 30.0,
    },
    "PROCESS_FUEL": {
        "inputs": {"raw_ore": 4},
        "outputs": {"fuel": 8},
        "time": 3.0,
        "power": 15.0,
    },
    "ASSEMBLE_DRONES": {
        "inputs": {"ship_parts": 3, "refined_metal": 2},
        "outputs": {"drone": 1},
        "time": 12.0,
        "power": 40.0,
    },
    "PRODUCE_AMMO": {
        "inputs": {"refined_metal": 2},
        "outputs": {"ammo": 20},
        "time": 4.0,
        "power": 10.0,
    },
}


# ---------------------------------------------------------------------------
# DronePort
# ---------------------------------------------------------------------------

class DronePort:
    """Connection point for drone-based resource transfer.

    Parameters
    ----------
    port_id : str
        Unique identifier.
    port_type : str
        ``"INPUT"``, ``"OUTPUT"``, or ``"BIDIRECTIONAL"``.
    max_throughput : float
        Maximum items per second this port can handle.
    """

    def __init__(self, port_id="", port_type="BIDIRECTIONAL",
                 max_throughput=10.0):
        self.port_id = port_id
        self.port_type = port_type if port_type in PORT_TYPES else "BIDIRECTIONAL"
        self.max_throughput = max_throughput
        self.active = True
        self.connected_storage: "StorageBlock | None" = None

    def can_input(self):
        return self.active and self.port_type in ("INPUT", "BIDIRECTIONAL")

    def can_output(self):
        return self.active and self.port_type in ("OUTPUT", "BIDIRECTIONAL")


# ---------------------------------------------------------------------------
# StorageBlock
# ---------------------------------------------------------------------------

class StorageBlock:
    """Finite-capacity item container.

    Parameters
    ----------
    storage_id : str
        Unique identifier.
    capacity : float
        Maximum total item count.
    """

    def __init__(self, storage_id="", capacity=100.0):
        self.storage_id = storage_id
        self.capacity = capacity
        self.items: dict[str, float] = {}
        self.connected_ports: list[DronePort] = []
        self.max_throughput = 50.0

    @property
    def used(self):
        return sum(self.items.values())

    @property
    def free(self):
        return max(0.0, self.capacity - self.used)

    def add_item(self, item, amount):
        """Add *amount* of *item*.  Returns actually added quantity."""
        can_add = min(amount, self.free)
        if can_add <= 0:
            return 0.0
        self.items[item] = self.items.get(item, 0.0) + can_add
        return can_add

    def remove_item(self, item, amount):
        """Remove up to *amount* of *item*.  Returns actually removed qty."""
        have = self.items.get(item, 0.0)
        removed = min(amount, have)
        if removed <= 0:
            return 0.0
        self.items[item] = have - removed
        if self.items[item] <= 0:
            del self.items[item]
        return removed

    def has_items(self, requirements):
        """Return True if all *requirements* ``{item: qty}`` are satisfied."""
        for item, qty in requirements.items():
            if self.items.get(item, 0.0) < qty:
                return False
        return True


# ---------------------------------------------------------------------------
# FactoryBlock
# ---------------------------------------------------------------------------

class FactoryBlock:
    """Recipe-driven manufacturing node.

    Parameters
    ----------
    factory_id : str
        Unique identifier.
    recipe : str
        Key into :data:`RECIPES`.
    """

    def __init__(self, factory_id="", recipe="REFINE_ORE"):
        self.factory_id = factory_id
        self.recipe = recipe
        self.input_storage: StorageBlock | None = None
        self.output_storage: StorageBlock | None = None
        self.speed = 1.0
        self.power_required = RECIPES.get(recipe, {}).get("power", 0.0)
        self.queue: list[dict] = []
        self._progress = 0.0
        self._active_job = False

    def start_job(self):
        """Try to start a manufacturing job.  Returns True on success."""
        recipe = RECIPES.get(self.recipe)
        if recipe is None:
            return False
        if self.input_storage is None or self.output_storage is None:
            return False
        if not self.input_storage.has_items(recipe["inputs"]):
            return False
        # Consume inputs
        for item, qty in recipe["inputs"].items():
            self.input_storage.remove_item(item, qty)
        self._progress = 0.0
        self._active_job = True
        return True

    def tick(self, delta_time):
        """Advance manufacturing by *delta_time* seconds.

        Returns True if a job completed this tick.
        """
        if not self._active_job:
            # Try starting a new job automatically
            if not self.start_job():
                return False

        recipe = RECIPES.get(self.recipe)
        if recipe is None:
            self._active_job = False
            return False

        self._progress += delta_time * self.speed
        if self._progress >= recipe["time"]:
            # Produce outputs
            if self.output_storage:
                for item, qty in recipe["outputs"].items():
                    self.output_storage.add_item(item, qty)
            self._active_job = False
            self._progress = 0.0
            return True
        return False


# ---------------------------------------------------------------------------
# DroneScheduler
# ---------------------------------------------------------------------------

class DroneScheduler:
    """Manages drone delivery scheduling.

    In a game context drones would be animated flying between ports.
    For simulation purposes, transfers happen instantly when scheduled.
    """

    def __init__(self):
        self._pending: list[dict] = []

    def schedule_drone(self, source_storage, dest_storage, item, amount):
        """Queue a drone delivery.  Returns the amount actually queued."""
        available = source_storage.items.get(item, 0.0)
        actual = min(amount, available, dest_storage.free)
        if actual <= 0:
            return 0.0
        self._pending.append({
            "source": source_storage,
            "dest": dest_storage,
            "item": item,
            "amount": actual,
        })
        return actual

    def process(self):
        """Execute all pending deliveries.  Returns count of completed transfers."""
        count = 0
        for job in self._pending:
            removed = job["source"].remove_item(job["item"], job["amount"])
            if removed > 0:
                job["dest"].add_item(job["item"], removed)
                count += 1
        self._pending.clear()
        return count


# ---------------------------------------------------------------------------
# ResourceGraph
# ---------------------------------------------------------------------------

class ResourceGraph:
    """Top-level resource flow graph.

    Owns all ports, storage blocks, and factories.  Call ``tick(dt)`` each
    frame or simulation step to advance manufacturing and drone deliveries.
    """

    def __init__(self):
        self.ports: dict[str, DronePort] = {}
        self.storages: dict[str, StorageBlock] = {}
        self.factories: dict[str, FactoryBlock] = {}
        self.scheduler = DroneScheduler()

    def add_port(self, port):
        self.ports[port.port_id] = port

    def add_storage(self, storage):
        self.storages[storage.storage_id] = storage

    def add_factory(self, factory):
        self.factories[factory.factory_id] = factory

    def transfer(self, src_storage_id, dst_storage_id, item, amount):
        """Instant transfer between two storage blocks.  Returns transferred qty."""
        src = self.storages.get(src_storage_id)
        dst = self.storages.get(dst_storage_id)
        if src is None or dst is None:
            return 0.0
        removed = src.remove_item(item, amount)
        if removed > 0:
            added = dst.add_item(item, removed)
            if added < removed:
                src.add_item(item, removed - added)
            return added
        return 0.0

    def schedule_drone(self, src_storage_id, dst_storage_id, item, amount):
        """Schedule a drone delivery.  Returns queued amount."""
        src = self.storages.get(src_storage_id)
        dst = self.storages.get(dst_storage_id)
        if src is None or dst is None:
            return 0.0
        return self.scheduler.schedule_drone(src, dst, item, amount)

    def process_manufacturing(self, delta_time):
        """Tick all factories.  Returns number of completed jobs."""
        completed = 0
        for factory in self.factories.values():
            if factory.tick(delta_time):
                completed += 1
        return completed

    def tick(self, delta_time):
        """Advance the full resource pipeline by *delta_time* seconds.

        1. Process manufacturing.
        2. Execute pending drone deliveries.

        Returns ``(completed_jobs, completed_deliveries)``.
        """
        jobs = self.process_manufacturing(delta_time)
        deliveries = self.scheduler.process()
        return (jobs, deliveries)


# ---------------------------------------------------------------------------
# Blender operator
# ---------------------------------------------------------------------------

class FLEET_OT_visualize_logistics(bpy.types.Operator):
    """Visualize a demo resource flow graph"""
    bl_idname = "mesh.fleet_visualize_logistics"
    bl_label = "Fleet: Visualize Logistics"
    bl_options = {'REGISTER', 'UNDO'}

    def execute(self, context):
        # Build a small demo graph
        rg = ResourceGraph()

        s_ore = StorageBlock(storage_id="ore_hold", capacity=200)
        s_ore.add_item("raw_ore", 100)
        s_metal = StorageBlock(storage_id="metal_hold", capacity=200)
        s_parts = StorageBlock(storage_id="parts_hold", capacity=100)

        rg.add_storage(s_ore)
        rg.add_storage(s_metal)
        rg.add_storage(s_parts)

        f_refinery = FactoryBlock(factory_id="refinery_1", recipe="REFINE_ORE")
        f_refinery.input_storage = s_ore
        f_refinery.output_storage = s_metal
        rg.add_factory(f_refinery)

        f_mfg = FactoryBlock(factory_id="mfg_1", recipe="MANUFACTURE_PARTS")
        f_mfg.input_storage = s_metal
        f_mfg.output_storage = s_parts
        rg.add_factory(f_mfg)

        # Simulate a few ticks
        for _ in range(20):
            rg.tick(1.0)

        # Visualise as empties
        col_name = "Fleet_Logistics_Preview"
        col = bpy.data.collections.get(col_name)
        if col is None:
            col = bpy.data.collections.new(col_name)
            bpy.context.scene.collection.children.link(col)

        positions = {
            "ore_hold": Vector((0, 0, 0)),
            "metal_hold": Vector((3, 0, 0)),
            "parts_hold": Vector((6, 0, 0)),
            "refinery_1": Vector((1.5, 2, 0)),
            "mfg_1": Vector((4.5, 2, 0)),
        }
        for sid, storage in rg.storages.items():
            empty = bpy.data.objects.new(f"storage_{sid}", None)
            empty.empty_display_type = 'CUBE'
            empty.empty_display_size = 0.5
            empty.location = positions.get(sid, Vector((0, 0, 0)))
            col.objects.link(empty)

        for fid, factory in rg.factories.items():
            empty = bpy.data.objects.new(f"factory_{fid}", None)
            empty.empty_display_type = 'CONE'
            empty.empty_display_size = 0.5
            empty.location = positions.get(fid, Vector((0, 0, 0)))
            col.objects.link(empty)

        # Draw flow edges
        bm = bmesh.new()
        edges_data = [
            ("ore_hold", "refinery_1"),
            ("refinery_1", "metal_hold"),
            ("metal_hold", "mfg_1"),
            ("mfg_1", "parts_hold"),
        ]
        for src, dst in edges_data:
            v1 = bm.verts.new(positions.get(src, Vector((0, 0, 0))))
            v2 = bm.verts.new(positions.get(dst, Vector((0, 0, 0))))
            bm.edges.new((v1, v2))
        mesh = bpy.data.meshes.new("FleetFlowEdges")
        bm.to_mesh(mesh)
        bm.free()
        edge_obj = bpy.data.objects.new("FleetFlowEdges", mesh)
        col.objects.link(edge_obj)

        # Report state
        ore_left = s_ore.items.get("raw_ore", 0)
        metal = s_metal.items.get("refined_metal", 0)
        parts = s_parts.items.get("ship_parts", 0)
        self.report({'INFO'},
                    f"Fleet logistics: ore={ore_left:.0f} metal={metal:.0f} "
                    f"parts={parts:.0f}")
        return {'FINISHED'}


# ---------------------------------------------------------------------------
# Registration
# ---------------------------------------------------------------------------

_classes = (
    FLEET_OT_visualize_logistics,
)


def register():
    for cls in _classes:
        bpy.utils.register_class(cls)


def unregister():
    for cls in reversed(_classes):
        bpy.utils.unregister_class(cls)
