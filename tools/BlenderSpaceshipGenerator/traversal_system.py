"""
Traversal system for ship interior navigation.

Generates doors, ramps, ladders, elevators, and airlocks that connect
the rooms / modules inside a ship.  Provides A* pathfinding to validate
that every node is reachable from the cockpit and to compute crew
movement costs.

Architecture
~~~~~~~~~~~~
* **TraversalNode** – a single navigation point (door, ramp, ladder, etc.).
* **TraversalGraph** – directed weighted graph of nodes with A*, connectivity
  validation, and Blender visualisation helpers.
* ``generate_central_elevator`` – creates an elevator shaft spanning all decks.
* ``generate_traversal_for_grid`` – auto-generates traversal nodes from a
  :class:`slot_grid.SlotGrid` module layout.

All computation uses Python stdlib + ``mathutils`` only (no numpy).
"""

import heapq
import math

import bpy
import bmesh
from mathutils import Vector

# ---------------------------------------------------------------------------
# Node types
# ---------------------------------------------------------------------------

NODE_TYPES = [
    "DOOR",
    "RAMP",
    "LADDER",
    "ELEVATOR",
    "AIRLOCK",
    "CORRIDOR",
    "PLATFORM",
]

# Default traversal cost per node type (seconds)
NODE_COSTS = {
    "DOOR": 1.0,
    "RAMP": 2.0,
    "LADDER": 3.0,
    "ELEVATOR": 1.5,
    "AIRLOCK": 5.0,
    "CORRIDOR": 0.5,
    "PLATFORM": 0.5,
}


# ---------------------------------------------------------------------------
# TraversalNode
# ---------------------------------------------------------------------------

class TraversalNode:
    """A single navigation point inside a ship.

    Parameters
    ----------
    position : tuple[float, float, float]
        World-space or grid-space coordinate.
    node_type : str
        One of :data:`NODE_TYPES`.
    node_id : str
        Unique identifier.
    """

    def __init__(self, position=(0, 0, 0), node_type="CORRIDOR", node_id=""):
        self.position = tuple(position)
        self.node_type = node_type if node_type in NODE_TYPES else "CORRIDOR"
        self.node_id = node_id
        self.traversal_cost = NODE_COSTS.get(self.node_type, 1.0)
        self.connections: list["TraversalNode"] = []

    def __repr__(self):
        return (f"<TraversalNode id={self.node_id!r} "
                f"type={self.node_type} pos={self.position}>")


# ---------------------------------------------------------------------------
# TraversalGraph
# ---------------------------------------------------------------------------

class TraversalGraph:
    """Directed weighted graph of traversal nodes.

    Supports A* pathfinding and full-connectivity validation.
    """

    def __init__(self):
        self._nodes: dict[str, TraversalNode] = {}
        self._next_id = 0

    # -- mutation -----------------------------------------------------------

    def add_node(self, position, node_type="CORRIDOR", node_id=None):
        """Add a node and return it.  If *node_id* is ``None`` an ID is
        auto-generated."""
        if node_id is None:
            node_id = f"tn_{self._next_id}"
            self._next_id += 1
        node = TraversalNode(position=position, node_type=node_type,
                             node_id=node_id)
        self._nodes[node_id] = node
        return node

    def get_node(self, node_id):
        return self._nodes.get(node_id)

    def connect(self, id_a, id_b, bidirectional=True):
        """Create a connection between two nodes.  Returns True on success."""
        a = self._nodes.get(id_a)
        b = self._nodes.get(id_b)
        if a is None or b is None:
            return False
        if b not in a.connections:
            a.connections.append(b)
        if bidirectional and a not in b.connections:
            b.connections.append(a)
        return True

    @property
    def nodes(self):
        return list(self._nodes.values())

    # -- A* pathfinding -----------------------------------------------------

    @staticmethod
    def _heuristic(a, b):
        """Euclidean distance heuristic."""
        return math.sqrt(sum((ai - bi) ** 2 for ai, bi in zip(a.position, b.position)))

    def find_path(self, start_id, goal_id):
        """Return ``(cost, [node_id, ...])`` for the cheapest path,
        or ``(float('inf'), [])`` if unreachable."""
        start = self._nodes.get(start_id)
        goal = self._nodes.get(goal_id)
        if start is None or goal is None:
            return (float("inf"), [])

        open_set: list[tuple[float, str]] = []
        heapq.heappush(open_set, (0.0, start.node_id))
        came_from: dict[str, str] = {}
        g_score: dict[str, float] = {start.node_id: 0.0}
        f_score: dict[str, float] = {start.node_id: self._heuristic(start, goal)}

        while open_set:
            _, current_id = heapq.heappop(open_set)
            if current_id == goal_id:
                # Reconstruct path
                path = [current_id]
                while current_id in came_from:
                    current_id = came_from[current_id]
                    path.append(current_id)
                path.reverse()
                return (g_score[goal.node_id], path)

            current = self._nodes[current_id]
            for neighbour in current.connections:
                nid = neighbour.node_id
                tentative = g_score[current_id] + neighbour.traversal_cost
                if tentative < g_score.get(nid, float("inf")):
                    came_from[nid] = current_id
                    g_score[nid] = tentative
                    f_score[nid] = tentative + self._heuristic(neighbour, goal)
                    heapq.heappush(open_set, (f_score[nid], nid))

        return (float("inf"), [])

    # -- connectivity validation --------------------------------------------

    def validate_connectivity(self, root_id=None):
        """Check that every node is reachable from *root_id* (default: first
        node added).

        Returns ``(ok, unreachable_ids)``.
        """
        if not self._nodes:
            return (True, [])
        if root_id is None:
            root_id = next(iter(self._nodes))
        visited: set[str] = set()
        stack = [root_id]
        while stack:
            nid = stack.pop()
            if nid in visited:
                continue
            visited.add(nid)
            node = self._nodes.get(nid)
            if node:
                for nb in node.connections:
                    if nb.node_id not in visited:
                        stack.append(nb.node_id)
        unreachable = [nid for nid in self._nodes if nid not in visited]
        return (len(unreachable) == 0, unreachable)


# ---------------------------------------------------------------------------
# Generation helpers
# ---------------------------------------------------------------------------

def generate_central_elevator(spine_positions, num_decks):
    """Generate an elevator shaft spanning *num_decks* along the given spine.

    Parameters
    ----------
    spine_positions : list[tuple[float, float, float]]
        One position per deck, ordered bottom-to-top.
    num_decks : int
        Number of decks (should match ``len(spine_positions)``).

    Returns
    -------
    TraversalGraph
        Graph with one ELEVATOR node per deck, connected vertically.
    """
    graph = TraversalGraph()
    decks = spine_positions[:num_decks]
    prev_id = None
    for i, pos in enumerate(decks):
        node = graph.add_node(position=pos, node_type="ELEVATOR",
                              node_id=f"elev_deck_{i}")
        if prev_id is not None:
            graph.connect(prev_id, node.node_id)
        prev_id = node.node_id
    return graph


def generate_traversal_for_grid(slot_grid):
    """Auto-generate traversal nodes for a :class:`slot_grid.SlotGrid`.

    Places a CORRIDOR node at every occupied cell and a DOOR node at every
    boundary between two different modules.  Ladders are placed between
    vertically adjacent cells in different modules.

    Parameters
    ----------
    slot_grid : slot_grid.SlotGrid
        A populated slot grid.

    Returns
    -------
    TraversalGraph
    """
    graph = TraversalGraph()
    cell_to_node: dict[tuple[int, int, int], str] = {}

    # Create corridor node for each cell
    for slot in slot_grid.all_slots():
        nid = f"cell_{slot.position[0]}_{slot.position[1]}_{slot.position[2]}"
        graph.add_node(position=slot.position, node_type="CORRIDOR", node_id=nid)
        cell_to_node[slot.position] = nid

    # Connect neighbours
    for slot in slot_grid.all_slots():
        x, y, z = slot.position
        for dx, dy, dz in [(1, 0, 0), (-1, 0, 0),
                            (0, 1, 0), (0, -1, 0),
                            (0, 0, 1), (0, 0, -1)]:
            nb_pos = (x + dx, y + dy, z + dz)
            nb_id = cell_to_node.get(nb_pos)
            if nb_id is None:
                continue
            src_id = cell_to_node[slot.position]

            # Different module → add a door or ladder
            nb_slot = None
            for s in slot_grid.all_slots():
                if s.position == nb_pos:
                    nb_slot = s
                    break
            if nb_slot and nb_slot.module_id != slot.module_id:
                # Vertical → ladder, horizontal → door
                ntype = "LADDER" if dz != 0 else "DOOR"
                prefix = "ladder" if dz != 0 else "door"
                transition_id = f"{prefix}_{min(src_id, nb_id)}_{max(src_id, nb_id)}"
                if graph.get_node(transition_id) is None:
                    graph.add_node(
                        position=((x + nb_pos[0]) / 2,
                                  (y + nb_pos[1]) / 2,
                                  (z + nb_pos[2]) / 2),
                        node_type=ntype,
                        node_id=transition_id,
                    )
                    graph.connect(src_id, transition_id)
                    graph.connect(transition_id, nb_id)
            else:
                graph.connect(src_id, nb_id)

    return graph


# ---------------------------------------------------------------------------
# Blender operator
# ---------------------------------------------------------------------------

class TRAVERSAL_OT_generate(bpy.types.Operator):
    """Generate and visualize a traversal network for the current ship"""
    bl_idname = "mesh.traversal_generate"
    bl_label = "Traversal: Generate Network"
    bl_options = {'REGISTER', 'UNDO'}

    def execute(self, context):
        # Build a small demo elevator + floor graph
        num_decks = 4
        spine = [(0, 0, float(i) * 2.0) for i in range(num_decks)]
        graph = generate_central_elevator(spine, num_decks)

        # Add some corridor nodes on each deck
        for deck in range(num_decks):
            for offset in range(1, 4):
                nid = graph.add_node(
                    position=(float(offset), 0, float(deck) * 2.0),
                    node_type="CORRIDOR",
                ).node_id
                graph.connect(f"elev_deck_{deck}", nid)

        ok, unreachable = graph.validate_connectivity("elev_deck_0")

        # Visualise nodes as empties and connections as edges
        col_name = "Traversal_Preview"
        col = bpy.data.collections.get(col_name)
        if col is None:
            col = bpy.data.collections.new(col_name)
            bpy.context.scene.collection.children.link(col)

        node_objs: dict[str, bpy.types.Object] = {}
        for node in graph.nodes:
            empty = bpy.data.objects.new(f"nav_{node.node_id}", None)
            empty.empty_display_type = 'SPHERE'
            empty.empty_display_size = 0.3
            empty.location = Vector(node.position)
            col.objects.link(empty)
            node_objs[node.node_id] = empty

        # Draw edges as a single mesh of line segments
        bm = bmesh.new()
        seen_edges: set[tuple[str, str]] = set()
        for node in graph.nodes:
            for nb in node.connections:
                edge_key = (min(node.node_id, nb.node_id),
                            max(node.node_id, nb.node_id))
                if edge_key in seen_edges:
                    continue
                seen_edges.add(edge_key)
                v1 = bm.verts.new(Vector(node.position))
                v2 = bm.verts.new(Vector(nb.position))
                bm.edges.new((v1, v2))
        mesh = bpy.data.meshes.new("TraversalEdges")
        bm.to_mesh(mesh)
        bm.free()
        edge_obj = bpy.data.objects.new("TraversalEdges", mesh)
        col.objects.link(edge_obj)

        status = "fully connected" if ok else f"{len(unreachable)} unreachable"
        self.report({'INFO'},
                    f"Traversal: {len(graph.nodes)} nodes, {len(seen_edges)} edges "
                    f"({status})")
        return {'FINISHED'}


# ---------------------------------------------------------------------------
# Registration
# ---------------------------------------------------------------------------

_classes = (
    TRAVERSAL_OT_generate,
)


def register():
    for cls in _classes:
        bpy.utils.register_class(cls)


def unregister():
    for cls in reversed(_classes):
        bpy.utils.unregister_class(cls)
