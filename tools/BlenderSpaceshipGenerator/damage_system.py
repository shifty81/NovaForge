"""
Damage propagation, structural integrity, and salvage system.

Implements the runtime damage rules described in ENGINE_INTEGRATION.md §11–§12:

- Each brick has HP; when HP reaches 0 the brick is destroyed.
- Destroying a brick removes its hull influence and triggers a structural
  integrity re-evaluation of its neighbours.
- Bricks that lose their connection to the spine are marked unsupported and
  eventually detach as debris.
- Destroyed bricks may drop salvage entities that retain their type and a
  random condition value.

This module is a pure-Python reference implementation that works with Ship
DNA brick lists.  It does **not** depend on ``bpy`` so it can be tested
outside Blender and ported to C++ for the Atlas engine.
"""

import math
import random
from collections import deque

# ---------------------------------------------------------------------------
# Hull influence weights (ENGINE_INTEGRATION.md §17)
# ---------------------------------------------------------------------------

HULL_WEIGHTS = {
    'REACTOR_CORE': 3.0,
    'ARMOR_BLOCK': 2.0,
    'ENGINE_BLOCK': 1.5,
    'CAPACITOR': 1.2,
    'HULL_PLATE': 1.0,
    'HULL_WEDGE': 1.0,
    'HULL_CORNER': 1.0,
    'HARDPOINT_MOUNT': 0.8,
    'POWER_BUS': 0.7,
    'STRUCTURAL_SPINE': 0.7,
    'DOCKING_CLAMP': 0.5,
    'SHIELD_EMITTER': 0.5,
    'THRUSTER': 0.5,
    'SENSOR_MAST': 0.3,
    'ANTENNA_DISH': 0.2,
    'PANEL': 0.2,
    'VENT': 0.2,
    'PIPE': 0.2,
}

# Default hit points per brick type
BRICK_HP = {
    'REACTOR_CORE': 500,
    'POWER_BUS': 200,
    'STRUCTURAL_SPINE': 400,
    'HULL_PLATE': 150,
    'HULL_WEDGE': 150,
    'HULL_CORNER': 100,
    'ARMOR_BLOCK': 300,
    'ENGINE_BLOCK': 250,
    'THRUSTER': 80,
    'CAPACITOR': 180,
    'SHIELD_EMITTER': 120,
    'HARDPOINT_MOUNT': 100,
    'DOCKING_CLAMP': 120,
    'SENSOR_MAST': 60,
    'ANTENNA_DISH': 40,
    'PANEL': 30,
    'VENT': 30,
    'PIPE': 50,
}

# Brick mass in kg (used for debris physics)
BRICK_MASS = {
    'REACTOR_CORE': 5000.0,
    'POWER_BUS': 800.0,
    'STRUCTURAL_SPINE': 2000.0,
    'HULL_PLATE': 400.0,
    'HULL_WEDGE': 400.0,
    'HULL_CORNER': 200.0,
    'ARMOR_BLOCK': 1200.0,
    'ENGINE_BLOCK': 1500.0,
    'THRUSTER': 100.0,
    'CAPACITOR': 600.0,
    'SHIELD_EMITTER': 150.0,
    'HARDPOINT_MOUNT': 120.0,
    'DOCKING_CLAMP': 300.0,
    'SENSOR_MAST': 80.0,
    'ANTENNA_DISH': 50.0,
    'PANEL': 20.0,
    'VENT': 25.0,
    'PIPE': 60.0,
}

# Salvage drop chances by brick category
SALVAGE_DROP_CHANCE = {
    'CORE': 0.9,
    'HULL': 0.4,
    'FUNCTION': 0.7,
    'UTILITY': 0.6,
    'DETAIL': 0.2,
}

# Brick-type -> category mapping (mirrors brick_system.BRICK_CATEGORIES)
_BRICK_CATEGORY = {}
_CATEGORIES_MAP = {
    'CORE': ['REACTOR_CORE', 'POWER_BUS', 'STRUCTURAL_SPINE'],
    'HULL': ['HULL_PLATE', 'HULL_WEDGE', 'HULL_CORNER', 'ARMOR_BLOCK'],
    'FUNCTION': ['ENGINE_BLOCK', 'THRUSTER', 'CAPACITOR', 'SHIELD_EMITTER'],
    'UTILITY': ['HARDPOINT_MOUNT', 'DOCKING_CLAMP', 'SENSOR_MAST',
                'ANTENNA_DISH'],
    'DETAIL': ['PANEL', 'VENT', 'PIPE'],
}
for _cat, _types in _CATEGORIES_MAP.items():
    for _t in _types:
        _BRICK_CATEGORY[_t] = _cat

# Structural integrity timeout (frames) before unsupported section detaches
DETACH_TIMEOUT = 60

# Salvage condition range (fraction of max HP retained by recovered bricks)
SALVAGE_CONDITION_MIN = 0.1
SALVAGE_CONDITION_MAX = 0.8

# Visual feedback states
DAMAGE_STATES = {
    'NOMINAL': 'Normal operation',
    'POWER_LOSS': 'Hull darkens — no power connection',
    'STRUCTURAL_STRESS': 'Cracks / glow lines — load exceeded',
    'HULL_THINNING': 'Visible dents — low HP',
    'DETACHMENT': 'Chunk separation + debris',
}

# ---------------------------------------------------------------------------
# Brick entity
# ---------------------------------------------------------------------------


class BrickEntity:
    """Runtime representation of a single brick in the ECS."""

    __slots__ = (
        'id', 'brick_type', 'pos', 'hp', 'max_hp', 'mass',
        'hull_weight', 'connected_to_spine', 'parent_id',
        'children_ids', 'load_count', 'archetype',
        'unsupported_timer', 'damage_state',
    )

    def __init__(self, brick_id, brick_type, pos, archetype=None):
        self.id = brick_id
        self.brick_type = brick_type
        self.pos = tuple(pos)
        self.max_hp = BRICK_HP.get(brick_type, 100)
        self.hp = self.max_hp
        self.mass = BRICK_MASS.get(brick_type, 100.0)
        self.hull_weight = HULL_WEIGHTS.get(brick_type, 0.5)
        self.connected_to_spine = False
        self.parent_id = None
        self.children_ids = []
        self.load_count = 0
        self.archetype = archetype
        self.unsupported_timer = 0
        self.damage_state = 'NOMINAL'

    @property
    def alive(self):
        return self.hp > 0

    @property
    def hp_fraction(self):
        return self.hp / max(self.max_hp, 1)


# ---------------------------------------------------------------------------
# Salvage entity
# ---------------------------------------------------------------------------


class SalvageEntity:
    """Represents a salvageable drop from a destroyed brick."""

    __slots__ = ('brick_type', 'pos', 'condition')

    def __init__(self, brick_type, pos, condition):
        self.brick_type = brick_type
        self.pos = tuple(pos)
        self.condition = condition

    def __repr__(self):
        return (f"SalvageEntity(type={self.brick_type}, "
                f"pos={self.pos}, condition={self.condition})")


# ---------------------------------------------------------------------------
# Ship damage state
# ---------------------------------------------------------------------------

# Neighbour offsets (6-connected grid)
_NEIGHBOUR_OFFSETS = [
    (1, 0, 0), (-1, 0, 0),
    (0, 1, 0), (0, -1, 0),
    (0, 0, 1), (0, 0, -1),
]


class ShipDamageState:
    """Manages the damage, structural integrity, and salvage state of a ship.

    Initialise from Ship DNA::

        state = ShipDamageState.from_ship_dna(dna)
        events = state.apply_damage(brick_id, 120)
        detached = state.tick()
    """

    def __init__(self, grid_size=1.0, seed=0):
        self.grid_size = grid_size
        self.bricks = {}       # brick_id -> BrickEntity
        self.grid = {}         # cell (gx,gy,gz) -> brick_id
        self.salvage = []      # list of SalvageEntity
        self.detached_ids = set()
        self._rng = random.Random(seed)

    # -- construction -------------------------------------------------------

    @classmethod
    def from_ship_dna(cls, dna):
        """Build a :class:`ShipDamageState` from a Ship DNA dict."""
        grid_size = dna.get('grid_size', 1.0)
        seed = dna.get('seed', 0)
        state = cls(grid_size=grid_size, seed=seed)
        for idx, brick_data in enumerate(dna.get('bricks', [])):
            brick_type = brick_data['type']
            pos = tuple(brick_data['pos'])
            archetype = brick_data.get('archetype')
            entity = BrickEntity(idx, brick_type, pos, archetype)
            state.bricks[idx] = entity
            cell = state._pos_to_cell(pos)
            state.grid[cell] = idx
        state.rebuild_connections()
        return state

    def _pos_to_cell(self, pos):
        gs = self.grid_size if self.grid_size else 1.0
        return (round(pos[0] / gs), round(pos[1] / gs), round(pos[2] / gs))

    # -- connectivity -------------------------------------------------------

    def rebuild_connections(self):
        """Recompute spine connectivity for all bricks via BFS."""
        # Reset
        for b in self.bricks.values():
            b.connected_to_spine = False
            b.parent_id = None
            b.children_ids = []
            b.load_count = 0

        # Find spine cells
        spine_ids = [bid for bid, b in self.bricks.items()
                     if b.brick_type == 'STRUCTURAL_SPINE']
        for sid in spine_ids:
            self.bricks[sid].connected_to_spine = True

        # BFS from spine
        visited = set(spine_ids)
        queue = deque(spine_ids)
        while queue:
            current_id = queue.popleft()
            current = self.bricks[current_id]
            cell = self._pos_to_cell(current.pos)
            for offset in _NEIGHBOUR_OFFSETS:
                ncell = (cell[0] + offset[0],
                         cell[1] + offset[1],
                         cell[2] + offset[2])
                nid = self.grid.get(ncell)
                if nid is not None and nid not in visited and nid in self.bricks:
                    nb = self.bricks[nid]
                    if not nb.connected_to_spine:
                        nb.connected_to_spine = True
                        nb.parent_id = current_id
                        self.bricks[current_id].children_ids.append(nid)
                        visited.add(nid)
                        queue.append(nid)

        # Compute load counts (number of descendants)
        for bid in self.bricks:
            self.bricks[bid].load_count = self._count_descendants(bid)

        # Update damage states for disconnected bricks
        for b in self.bricks.values():
            if not b.connected_to_spine and b.damage_state == 'NOMINAL':
                b.damage_state = 'POWER_LOSS'

    def _count_descendants(self, brick_id):
        """Count all transitive children of *brick_id*."""
        entity = self.bricks.get(brick_id)
        if entity is None:
            return 0
        count = 0
        for cid in entity.children_ids:
            count += 1 + self._count_descendants(cid)
        return count

    def get_unsupported_bricks(self):
        """Return list of brick IDs not connected to the spine."""
        return [b.id for b in self.bricks.values()
                if not b.connected_to_spine]

    # -- damage -------------------------------------------------------------

    def apply_damage(self, brick_id, amount):
        """Apply *amount* damage to a brick.  Returns list of events."""
        entity = self.bricks.get(brick_id)
        if entity is None or not entity.alive:
            return []

        events = []
        entity.hp = max(0, entity.hp - amount)

        # Update visual state based on HP fraction
        if entity.hp > 0:
            frac = entity.hp_fraction
            if frac < 0.3:
                entity.damage_state = 'HULL_THINNING'
                events.append(('hull_thinning', brick_id))
            elif frac < 0.6:
                entity.damage_state = 'STRUCTURAL_STRESS'
                events.append(('structural_stress', brick_id))
        else:
            # Brick destroyed
            events.extend(self._destroy_brick(brick_id))

        return events

    def _destroy_brick(self, brick_id):
        """Remove a destroyed brick and cascade structural checks."""
        events = [('brick_destroyed', brick_id)]
        entity = self.bricks[brick_id]

        # Roll salvage
        salvage = self._roll_salvage(entity)
        if salvage is not None:
            self.salvage.append(salvage)
            events.append(('salvage_dropped', brick_id, salvage))

        # Remove from grid
        cell = self._pos_to_cell(entity.pos)
        if cell in self.grid and self.grid[cell] == brick_id:
            del self.grid[cell]

        # Remove from bricks dict
        del self.bricks[brick_id]

        # Rebuild connectivity to find newly unsupported sections
        self.rebuild_connections()

        unsupported = self.get_unsupported_bricks()
        if unsupported:
            events.append(('unsupported_bricks', unsupported))

        return events

    def _roll_salvage(self, entity):
        """Randomly decide whether the destroyed brick drops salvage."""
        category = _BRICK_CATEGORY.get(entity.brick_type, 'DETAIL')
        chance = SALVAGE_DROP_CHANCE.get(category, 0.2)
        if self._rng.random() < chance:
            condition = round(self._rng.uniform(
                SALVAGE_CONDITION_MIN, SALVAGE_CONDITION_MAX), 2)
            return SalvageEntity(entity.brick_type, entity.pos, condition)
        return None

    # -- tick (frame update) ------------------------------------------------

    def tick(self):
        """Advance one frame.  Handles detachment of unsupported sections."""
        detached_this_frame = []
        for b in list(self.bricks.values()):
            if not b.connected_to_spine:
                b.unsupported_timer += 1
                if b.unsupported_timer >= DETACH_TIMEOUT:
                    b.damage_state = 'DETACHMENT'
                    detached_this_frame.append(b.id)

        for bid in detached_this_frame:
            self.detached_ids.add(bid)
            entity = self.bricks.pop(bid, None)
            if entity is not None:
                cell = self._pos_to_cell(entity.pos)
                if cell in self.grid and self.grid[cell] == bid:
                    del self.grid[cell]

        return detached_this_frame

    # -- queries ------------------------------------------------------------

    def total_hull_weight(self):
        """Sum of all alive brick hull influence weights."""
        return sum(b.hull_weight for b in self.bricks.values())

    def total_mass(self):
        """Sum of all alive brick masses."""
        return sum(b.mass for b in self.bricks.values())

    def alive_count(self):
        """Number of bricks still alive."""
        return len(self.bricks)

    def get_damage_summary(self):
        """Return a dict mapping damage state names to brick counts."""
        summary = {state: 0 for state in DAMAGE_STATES}
        for b in self.bricks.values():
            summary[b.damage_state] = summary.get(b.damage_state, 0) + 1
        return summary


# ---------------------------------------------------------------------------
# Blender registration stubs
# ---------------------------------------------------------------------------


def register():
    """Register this module."""
    pass


def unregister():
    """Unregister this module."""
    pass
