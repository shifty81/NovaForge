"""
Power flow simulation for EVE-style capacitor mechanics.

Implements the PowerFlowSystem from ENGINE_INTEGRATION.md §11:

- Reactors generate power; engines, shields, and weapons consume it.
- A ship-wide capacitor stores excess generation.
- When the capacitor is empty, non-essential systems (shields, weapons)
  are disabled.
- Power flows only through bricks connected to the spine.

This module is a pure-Python reference implementation that works with
Ship DNA brick lists and :class:`damage_system.ShipDamageState`.
It does **not** depend on ``bpy`` so it can be tested outside Blender
and ported to C++ for the Atlas engine.
"""

# ---------------------------------------------------------------------------
# Power generation / consumption per brick type (MW)
# ---------------------------------------------------------------------------

POWER_GENERATION = {
    'REACTOR_CORE': 100.0,
    'POWER_BUS': 0.0,
    'STRUCTURAL_SPINE': 0.0,
}

POWER_CONSUMPTION = {
    'ENGINE_BLOCK': 20.0,
    'THRUSTER': 5.0,
    'SHIELD_EMITTER': 15.0,
    'HARDPOINT_MOUNT': 10.0,
    'SENSOR_MAST': 3.0,
    'ANTENNA_DISH': 1.0,
    'DOCKING_CLAMP': 2.0,
}

# Default capacitor size per ship class (MJ)
CAPACITOR_SIZE = {
    'SHUTTLE': 300.0,
    'FIGHTER': 400.0,
    'CORVETTE': 800.0,
    'FRIGATE': 1200.0,
    'DESTROYER': 2000.0,
    'CRUISER': 3500.0,
    'BATTLECRUISER': 4500.0,
    'BATTLESHIP': 6000.0,
    'CARRIER': 8000.0,
    'DREADNOUGHT': 10000.0,
    'CAPITAL': 15000.0,
    'TITAN': 20000.0,
    'INDUSTRIAL': 1000.0,
    'MINING_BARGE': 600.0,
    'EXHUMER': 900.0,
    'EXPLORER': 500.0,
    'HAULER': 1100.0,
    'EXOTIC': 600.0,
}

# Non-essential system types that are disabled when capacitor is empty
NON_ESSENTIAL_TYPES = frozenset([
    'SHIELD_EMITTER',
    'HARDPOINT_MOUNT',
    'SENSOR_MAST',
    'ANTENNA_DISH',
])

# Essential system types (always stay active if connected)
ESSENTIAL_TYPES = frozenset([
    'ENGINE_BLOCK',
    'THRUSTER',
    'DOCKING_CLAMP',
])


# ---------------------------------------------------------------------------
# Power component (per-brick)
# ---------------------------------------------------------------------------


class PowerComponent:
    """Tracks power generation and consumption for a single brick."""

    __slots__ = ('brick_type', 'generation', 'consumption', 'active',
                 'connected')

    def __init__(self, brick_type, connected=True):
        self.brick_type = brick_type
        self.generation = POWER_GENERATION.get(brick_type, 0.0)
        self.consumption = POWER_CONSUMPTION.get(brick_type, 0.0)
        self.active = True
        self.connected = connected

    @property
    def net_power(self):
        """Net power contribution (positive = generation, negative = draw)."""
        if not self.connected:
            return 0.0
        gen = self.generation if self.active else 0.0
        con = self.consumption if self.active else 0.0
        return gen - con

    def __repr__(self):
        return (f"PowerComponent(type={self.brick_type}, "
                f"gen={self.generation}, con={self.consumption}, "
                f"active={self.active})")


# ---------------------------------------------------------------------------
# Ship power state
# ---------------------------------------------------------------------------


class ShipPowerState:
    """Manages power flow for an entire ship.

    Initialise from a Ship DNA dict::

        power = ShipPowerState.from_ship_dna(dna)

    Then call :meth:`tick` each frame with a ``dt`` value::

        events = power.tick(dt=1/60)
    """

    def __init__(self, ship_class='CRUISER'):
        self.ship_class = ship_class
        self.capacitor_max = CAPACITOR_SIZE.get(ship_class, 1000.0)
        self.capacitor = self.capacitor_max
        self.components = {}      # brick_id -> PowerComponent
        self.disabled_ids = set()  # brick IDs whose systems are disabled

    # -- construction -------------------------------------------------------

    @classmethod
    def from_ship_dna(cls, dna):
        """Build a :class:`ShipPowerState` from a Ship DNA dict."""
        ship_class = dna.get('class', 'CRUISER')
        state = cls(ship_class=ship_class)
        for idx, brick_data in enumerate(dna.get('bricks', [])):
            brick_type = brick_data['type']
            # Only register bricks that generate or consume power
            if brick_type in POWER_GENERATION or brick_type in POWER_CONSUMPTION:
                state.components[idx] = PowerComponent(brick_type)
        return state

    @classmethod
    def from_damage_state(cls, damage_state, ship_class='CRUISER'):
        """Build from a :class:`damage_system.ShipDamageState`."""
        state = cls(ship_class=ship_class)
        for bid, brick in damage_state.bricks.items():
            bt = brick.brick_type
            if bt in POWER_GENERATION or bt in POWER_CONSUMPTION:
                comp = PowerComponent(bt, connected=brick.connected_to_spine)
                state.components[bid] = comp
        return state

    # -- queries ------------------------------------------------------------

    @property
    def total_generation(self):
        """Total active power generation (MW)."""
        return sum(c.generation for c in self.components.values()
                   if c.active and c.connected)

    @property
    def total_consumption(self):
        """Total active power consumption (MW)."""
        return sum(c.consumption for c in self.components.values()
                   if c.active and c.connected)

    @property
    def net_power(self):
        """Net power balance (positive = surplus)."""
        return self.total_generation - self.total_consumption

    @property
    def capacitor_fraction(self):
        """Capacitor charge as a fraction (0.0 – 1.0)."""
        return self.capacitor / max(self.capacitor_max, 1.0)

    @property
    def power_stable(self):
        """True if generation meets or exceeds consumption."""
        return self.net_power >= 0

    # -- tick ---------------------------------------------------------------

    def tick(self, dt=1.0):
        """Advance the power simulation by *dt* seconds.

        Returns a list of event tuples describing state changes.
        """
        events = []

        # Calculate net power
        net = self.net_power
        self.capacitor += net * dt
        self.capacitor = max(0.0, min(self.capacitor, self.capacitor_max))

        # Disable non-essentials when capacitor is empty
        if self.capacitor <= 0:
            for bid, comp in self.components.items():
                if (comp.brick_type in NON_ESSENTIAL_TYPES
                        and comp.active and comp.connected):
                    comp.active = False
                    self.disabled_ids.add(bid)
                    events.append(('system_disabled', bid, comp.brick_type))

        # Re-enable non-essentials when capacitor is above 20 %
        elif self.capacitor_fraction > 0.2:
            for bid in list(self.disabled_ids):
                comp = self.components.get(bid)
                if comp is not None and not comp.active:
                    comp.active = True
                    events.append(('system_enabled', bid, comp.brick_type))
            self.disabled_ids.clear()

        return events

    # -- mutation -----------------------------------------------------------

    def remove_brick(self, brick_id):
        """Remove a brick from the power simulation (e.g. after destruction)."""
        self.components.pop(brick_id, None)
        self.disabled_ids.discard(brick_id)

    def set_connected(self, brick_id, connected):
        """Update the connectivity flag for a brick."""
        comp = self.components.get(brick_id)
        if comp is not None:
            comp.connected = connected

    def sync_with_damage_state(self, damage_state):
        """Synchronise power connectivity from a
        :class:`damage_system.ShipDamageState`."""
        # Remove destroyed bricks
        dead_ids = [bid for bid in self.components
                    if bid not in damage_state.bricks]
        for bid in dead_ids:
            self.remove_brick(bid)

        # Update connectivity
        for bid, comp in self.components.items():
            brick = damage_state.bricks.get(bid)
            if brick is not None:
                comp.connected = brick.connected_to_spine

    def get_power_summary(self):
        """Return a summary dict of the current power state."""
        return {
            'total_generation': self.total_generation,
            'total_consumption': self.total_consumption,
            'net_power': self.net_power,
            'capacitor': round(self.capacitor, 2),
            'capacitor_max': self.capacitor_max,
            'capacitor_fraction': round(self.capacitor_fraction, 4),
            'power_stable': self.power_stable,
            'disabled_systems': len(self.disabled_ids),
        }


# ---------------------------------------------------------------------------
# Blender registration stubs
# ---------------------------------------------------------------------------


def register():
    """Register this module."""
    pass


def unregister():
    """Unregister this module."""
    pass
