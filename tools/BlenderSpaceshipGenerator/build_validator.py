"""
Player build-mode validation system.

Implements placement validation rules from ENGINE_INTEGRATION.md §5 and §11
(BrickPlacementSystem):

- Grid snapping enforcement
- Hardpoint compatibility checks between adjacent bricks
- Overlap detection (no two bricks in the same grid cell)
- Power connection verification
- Structural connectivity check (must connect to spine)

This module is a pure-Python reference implementation that works with
brick definitions from :mod:`brick_system`.  It does **not** depend on
``bpy`` so it can be tested outside Blender and ported to C++ for the
Atlas engine.
"""

from collections import deque

# ---------------------------------------------------------------------------
# Hardpoint compatibility matrix
# ---------------------------------------------------------------------------

# Compatible role pairs — order-independent.
COMPATIBLE_ROLES = frozenset([
    ('power', 'power'),
    ('attach', 'attach'),
    ('weapon', 'attach'),
    ('attach', 'weapon'),
    ('exhaust', 'attach'),
    ('attach', 'exhaust'),
    ('dock', 'attach'),
    ('attach', 'dock'),
    ('dock', 'dock'),
])

# ---------------------------------------------------------------------------
# Validation result
# ---------------------------------------------------------------------------


class ValidationResult:
    """Encapsulates the outcome of a placement validation."""

    __slots__ = ('valid', 'errors', 'warnings')

    def __init__(self):
        self.valid = True
        self.errors = []
        self.warnings = []

    def add_error(self, message):
        self.valid = False
        self.errors.append(message)

    def add_warning(self, message):
        self.warnings.append(message)

    def __bool__(self):
        return self.valid

    def __repr__(self):
        if self.valid:
            return f"ValidationResult(valid=True, warnings={len(self.warnings)})"
        return (f"ValidationResult(valid=False, "
                f"errors={self.errors}, warnings={self.warnings})")


# ---------------------------------------------------------------------------
# Build validator
# ---------------------------------------------------------------------------

# Neighbour offsets on the 3-D grid (6-connected)
_NEIGHBOUR_OFFSETS = [
    (1, 0, 0), (-1, 0, 0),
    (0, 1, 0), (0, -1, 0),
    (0, 0, 1), (0, 0, -1),
]

# Dot-product threshold for treating two direction vectors as aligned.
# 0.9 allows ≈25° tolerance while rejecting perpendicular or opposing faces.
DIRECTION_MATCH_THRESHOLD = 0.9

# Direction vectors corresponding to each neighbour offset
_OFFSET_DIRECTIONS = {
    (1, 0, 0): (1, 0, 0),
    (-1, 0, 0): (-1, 0, 0),
    (0, 1, 0): (0, 1, 0),
    (0, -1, 0): (0, -1, 0),
    (0, 0, 1): (0, 0, 1),
    (0, 0, -1): (0, 0, -1),
}


def _dot(a, b):
    """Dot product of two 3-tuples."""
    return a[0] * b[0] + a[1] * b[1] + a[2] * b[2]


def _neg(v):
    """Negate a 3-tuple."""
    return (-v[0], -v[1], -v[2])


class BuildValidator:
    """Validates brick placement according to the grid and hardpoint rules.

    Usage::

        from brick_system import BRICK_TYPES, GRID_SIZES
        validator = BuildValidator(grid_size=2.0, brick_types=BRICK_TYPES)
        validator.place_brick('STRUCTURAL_SPINE', (0, 0, 0))
        result = validator.validate_placement('ENGINE_BLOCK', (0, -4, 0))
        if result:
            validator.place_brick('ENGINE_BLOCK', (0, -4, 0))
    """

    def __init__(self, grid_size=1.0, brick_types=None):
        self.grid_size = grid_size
        self.brick_types = brick_types or {}
        self.placed = {}  # grid cell (gx, gy, gz) → brick_type name
        self._positions = {}  # grid cell → world position

    # -- helpers ------------------------------------------------------------

    def _to_cell(self, pos):
        gs = self.grid_size if self.grid_size else 1.0
        return (
            round(pos[0] / gs),
            round(pos[1] / gs),
            round(pos[2] / gs),
        )

    def _snap(self, pos):
        gs = self.grid_size if self.grid_size else 1.0
        return tuple(round(v / gs) * gs for v in pos)

    def _is_aligned(self, pos):
        snapped = self._snap(pos)
        return all(abs(a - b) < 1e-6 for a, b in zip(snapped, pos))

    # -- placement ----------------------------------------------------------

    def place_brick(self, brick_type_name, pos):
        """Place a brick (bypasses validation — call :meth:`validate_placement`
        first)."""
        cell = self._to_cell(pos)
        self.placed[cell] = brick_type_name
        self._positions[cell] = tuple(pos)

    # -- validation ---------------------------------------------------------

    def validate_placement(self, brick_type_name, pos):
        """Validate whether *brick_type_name* can be placed at *pos*.

        Returns a :class:`ValidationResult`.
        """
        result = ValidationResult()
        cell = self._to_cell(pos)

        # 1. Grid alignment
        if not self._is_aligned(pos):
            result.add_error(
                f"Position {pos} is not grid-aligned "
                f"(grid_size={self.grid_size})"
            )

        # 2. Overlap detection
        if cell in self.placed:
            result.add_error(
                f"Cell {cell} is already occupied by "
                f"{self.placed[cell]}"
            )

        # 3. Brick type known
        brick_def = self.brick_types.get(brick_type_name)
        if brick_def is None:
            result.add_warning(
                f"Unknown brick type '{brick_type_name}'"
            )

        # 4. Hardpoint compatibility with neighbours
        has_any_neighbour = False
        for offset in _NEIGHBOUR_OFFSETS:
            ncell = (cell[0] + offset[0],
                     cell[1] + offset[1],
                     cell[2] + offset[2])
            if ncell not in self.placed:
                continue
            has_any_neighbour = True

            # Check hardpoint compatibility
            neighbour_type_name = self.placed[ncell]
            direction = _OFFSET_DIRECTIONS[offset]
            opposing = _neg(direction)

            compatible = self._check_hardpoint_compatibility(
                brick_def, direction,
                self.brick_types.get(neighbour_type_name), opposing,
            )
            if not compatible:
                result.add_warning(
                    f"No compatible hardpoint pair between "
                    f"{brick_type_name} and {neighbour_type_name} "
                    f"in direction {direction}"
                )

        # 5. Must have at least one neighbour (unless it's the first brick
        #    or a spine segment)
        if (not has_any_neighbour and self.placed
                and brick_type_name != 'STRUCTURAL_SPINE'):
            result.add_error(
                "Brick must be adjacent to at least one existing brick"
            )

        return result

    def _check_hardpoint_compatibility(self, brick_a_def, dir_a,
                                       brick_b_def, dir_b):
        """Return True if brick_a has a hardpoint facing *dir_a* whose role
        is compatible with a hardpoint on brick_b facing *dir_b*."""
        if brick_a_def is None or brick_b_def is None:
            return False

        roles_a = self._get_roles_facing(brick_a_def, dir_a)
        roles_b = self._get_roles_facing(brick_b_def, dir_b)

        for ra in roles_a:
            for rb in roles_b:
                if (ra, rb) in COMPATIBLE_ROLES:
                    return True
        return False

    @staticmethod
    def _get_roles_facing(brick_def, direction):
        """Return hardpoint roles on *brick_def* whose direction matches."""
        roles = []
        for hp in brick_def.get('hardpoints', []):
            hp_dir = tuple(hp.get('direction', (0, 0, 0)))
            if _dot(hp_dir, direction) > DIRECTION_MATCH_THRESHOLD:
                roles.append(hp['role'])
        return roles

    # -- structural connectivity check --------------------------------------

    def check_connectivity(self):
        """Return True if all placed bricks are connected to at least one
        STRUCTURAL_SPINE brick via a path of adjacent bricks."""
        if not self.placed:
            return True

        # Find spine cells
        spine_cells = [cell for cell, bt in self.placed.items()
                       if bt == 'STRUCTURAL_SPINE']
        if not spine_cells:
            return False

        # BFS from spine cells
        visited = set()
        queue = deque(spine_cells)
        while queue:
            current = queue.popleft()
            if current in visited:
                continue
            visited.add(current)
            for offset in _NEIGHBOUR_OFFSETS:
                ncell = (current[0] + offset[0],
                         current[1] + offset[1],
                         current[2] + offset[2])
                if ncell in self.placed and ncell not in visited:
                    queue.append(ncell)

        return len(visited) == len(self.placed)

    # -- power connectivity check -------------------------------------------

    def check_power_connectivity(self):
        """Return list of brick cells that have no path to a power source
        (REACTOR_CORE or POWER_BUS) through power-role hardpoints."""
        power_sources = [cell for cell, bt in self.placed.items()
                         if bt in ('REACTOR_CORE', 'POWER_BUS')]
        if not power_sources:
            return list(self.placed.keys())

        # BFS from power sources
        visited = set()
        queue = deque(power_sources)
        while queue:
            current = queue.popleft()
            if current in visited:
                continue
            visited.add(current)
            for offset in _NEIGHBOUR_OFFSETS:
                ncell = (current[0] + offset[0],
                         current[1] + offset[1],
                         current[2] + offset[2])
                if ncell in self.placed and ncell not in visited:
                    queue.append(ncell)

        # Cells with no power path
        return [cell for cell in self.placed if cell not in visited]

    # -- bulk validation from Ship DNA --------------------------------------

    @classmethod
    def from_ship_dna(cls, dna, brick_types=None):
        """Create a :class:`BuildValidator` pre-populated from Ship DNA.

        Validates every brick placement and returns
        ``(validator, list_of_results)``.
        """
        grid_size = dna.get('grid_size', 1.0)
        validator = cls(grid_size=grid_size, brick_types=brick_types or {})
        results = []
        for brick_data in dna.get('bricks', []):
            bt = brick_data['type']
            pos = tuple(brick_data['pos'])
            result = validator.validate_placement(bt, pos)
            validator.place_brick(bt, pos)
            results.append((bt, pos, result))
        return validator, results


# ---------------------------------------------------------------------------
# Blender registration stubs
# ---------------------------------------------------------------------------


def register():
    """Register this module."""
    pass


def unregister():
    """Unregister this module."""
    pass
