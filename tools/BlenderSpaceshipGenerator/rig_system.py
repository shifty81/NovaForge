"""
Player rig / suit interface port + ship connection system.

Defines the relationship between a player's personal rig (suit, tools,
HUD) and the ship systems they interface with.  When a player enters an
InterfacePort (cockpit seat, fleet console, turret seat, etc.) their HUD
changes and they gain authority over specific ship sub-systems.

Architecture
~~~~~~~~~~~~
* **RigStats** – player rig statistics (power, fuel, heat, oxygen …).
* **InterfacePort** – a point on the ship that the player can plug into,
  changing their HUD layer and granting system authority.
* **PlayerRig** – the player's personal equipment + currently linked port.
* **HUDLayer** – enumeration of possible HUD modes.
* **HUDStack** – push/pop overlay stack that determines what the player sees.

Blender operators are provided to visualise interface ports in the viewport.

All computation uses Python stdlib + ``mathutils`` only (no numpy).
"""

import bpy
from mathutils import Vector

# ---------------------------------------------------------------------------
# HUD layers
# ---------------------------------------------------------------------------

HUD_LAYERS = [
    "FPS",
    "COCKPIT",
    "CONSOLE",
    "RIG",
    "FLEET",
    "HACK",
    "BUILD",
    "EDITOR",
    "DEBUG",
]

# Colour hints for viewport visualisation (RGBA)
HUD_LAYER_COLOURS = {
    "FPS":      (0.2, 0.8, 0.2, 0.8),
    "COCKPIT":  (0.1, 0.4, 1.0, 0.8),
    "CONSOLE":  (0.8, 0.6, 0.1, 0.8),
    "RIG":      (0.6, 0.6, 0.6, 0.8),
    "FLEET":    (0.9, 0.2, 0.2, 0.8),
    "HACK":     (0.0, 1.0, 0.5, 0.8),
    "BUILD":    (1.0, 0.8, 0.0, 0.8),
    "EDITOR":   (0.5, 0.5, 1.0, 0.8),
    "DEBUG":    (1.0, 0.0, 1.0, 0.8),
}


class HUDLayer:
    """A single HUD layer with a name and priority."""

    def __init__(self, name="FPS", priority=0):
        self.name = name if name in HUD_LAYERS else "FPS"
        self.priority = priority

    def __repr__(self):
        return f"<HUDLayer {self.name} pri={self.priority}>"


class HUDStack:
    """Push/pop overlay stack for HUD management.

    The topmost layer is the one the player currently sees.  Lower layers
    remain active for overlay compositing (e.g. FPS health bar shows
    through the COCKPIT layer).
    """

    def __init__(self):
        self._stack: list[HUDLayer] = []

    def push(self, layer):
        """Push a layer onto the stack."""
        self._stack.append(layer)

    def pop(self):
        """Pop and return the topmost layer, or None if empty."""
        if self._stack:
            return self._stack.pop()
        return None

    def overlay(self, layer):
        """Insert a layer below the top (for transparent overlays)."""
        if len(self._stack) >= 2:
            self._stack.insert(-1, layer)
        else:
            self._stack.append(layer)

    def get_active(self):
        """Return the topmost (active) layer, or None."""
        return self._stack[-1] if self._stack else None

    @property
    def depth(self):
        return len(self._stack)

    def clear(self):
        self._stack.clear()

    def __repr__(self):
        names = [l.name for l in self._stack]
        return f"<HUDStack [{' > '.join(names)}]>"


# ---------------------------------------------------------------------------
# Rig stats
# ---------------------------------------------------------------------------

class RigStats:
    """Player rig / suit statistics.

    These values are modified by equipped modules, environmental effects,
    and the ship interface port the player is connected to.
    """

    def __init__(self):
        self.power = 100.0
        self.fuel = 100.0
        self.efficiency = 1.0
        self.heat = 0.0
        self.oxygen = 100.0
        self.interface_level = 1
        self.strength = 1.0
        self.speed = 1.0
        self.stability = 1.0

    def clamp(self):
        """Clamp all stats to valid ranges."""
        self.power = max(0.0, min(self.power, 100.0))
        self.fuel = max(0.0, min(self.fuel, 100.0))
        self.efficiency = max(0.0, min(self.efficiency, 5.0))
        self.heat = max(0.0, min(self.heat, 200.0))
        self.oxygen = max(0.0, min(self.oxygen, 100.0))
        self.interface_level = max(1, min(self.interface_level, 5))
        self.strength = max(0.0, min(self.strength, 10.0))
        self.speed = max(0.0, min(self.speed, 10.0))
        self.stability = max(0.0, min(self.stability, 10.0))

    def __repr__(self):
        return (f"<RigStats pwr={self.power:.0f} fuel={self.fuel:.0f} "
                f"O2={self.oxygen:.0f} heat={self.heat:.0f}>")


# ---------------------------------------------------------------------------
# Interface port
# ---------------------------------------------------------------------------

PORT_TYPES = [
    "COCKPIT",
    "FLEET_CONSOLE",
    "DRONE_CONSOLE",
    "TURRET_SEAT",
    "ENGINEERING_PANEL",
    "STRATEGIC_TABLE",
]

# Which HUD layer is entered when a player plugs into this port type
PORT_HUD_MAP = {
    "COCKPIT":          "COCKPIT",
    "FLEET_CONSOLE":    "FLEET",
    "DRONE_CONSOLE":    "CONSOLE",
    "TURRET_SEAT":      "FPS",
    "ENGINEERING_PANEL": "BUILD",
    "STRATEGIC_TABLE":  "FLEET",
}

# Authority changes applied when a player connects to a port
PORT_AUTHORITY = {
    "COCKPIT":          {"navigation": True, "weapons": True, "shields": True},
    "FLEET_CONSOLE":    {"fleet_command": True, "navigation": False},
    "DRONE_CONSOLE":    {"drones": True},
    "TURRET_SEAT":      {"weapons": True},
    "ENGINEERING_PANEL": {"power": True, "repairs": True},
    "STRATEGIC_TABLE":  {"fleet_command": True, "navigation": True},
}


class InterfacePort:
    """A point on the ship that the player can plug their rig into.

    Parameters
    ----------
    port_id : str
        Unique identifier.
    port_type : str
        One of :data:`PORT_TYPES`.
    position : tuple[float, float, float]
        Location in ship-local space.
    """

    def __init__(self, port_id="", port_type="COCKPIT",
                 position=(0, 0, 0)):
        self.port_id = port_id
        self.port_type = port_type if port_type in PORT_TYPES else "COCKPIT"
        self.position = tuple(position)
        self.connected_systems: list[str] = list(
            PORT_AUTHORITY.get(self.port_type, {}).keys()
        )
        self.enter_mode = PORT_HUD_MAP.get(self.port_type, "FPS")
        self.authority_change = dict(
            PORT_AUTHORITY.get(self.port_type, {})
        )
        self.occupied = False
        self.occupant_id = ""

    def enter(self, player_id):
        """Mark this port as occupied by *player_id*.  Returns True on success."""
        if self.occupied:
            return False
        self.occupied = True
        self.occupant_id = player_id
        return True

    def leave(self):
        """Release the port.  Returns the previous occupant ID."""
        pid = self.occupant_id
        self.occupied = False
        self.occupant_id = ""
        return pid

    def __repr__(self):
        status = f"occupied by {self.occupant_id}" if self.occupied else "free"
        return f"<InterfacePort {self.port_id} type={self.port_type} {status}>"


# ---------------------------------------------------------------------------
# PlayerRig
# ---------------------------------------------------------------------------

class PlayerRig:
    """The player's personal equipment and ship interface state.

    Parameters
    ----------
    player_id : str
        Unique player identifier.
    """

    def __init__(self, player_id="player_0"):
        self.player_id = player_id
        self.stats = RigStats()
        self.tools: list[str] = []
        self.survival_modules: list[str] = []
        self.link_port: InterfacePort | None = None
        self.connected_ship = ""
        self.hud = HUDStack()
        # Default HUD starts on FPS
        self.hud.push(HUDLayer("FPS", priority=0))

    def connect_to_port(self, port, ship_id=""):
        """Plug into an :class:`InterfacePort`.  Returns True on success."""
        if port is None:
            return False
        if not port.enter(self.player_id):
            return False
        self.link_port = port
        self.connected_ship = ship_id
        self.hud.push(HUDLayer(port.enter_mode, priority=10))
        return True

    def disconnect(self):
        """Unplug from the current port.  Returns True if was connected."""
        if self.link_port is None:
            return False
        self.link_port.leave()
        self.link_port = None
        self.connected_ship = ""
        self.hud.pop()
        return True

    def tick(self, delta_time):
        """Advance rig simulation (fuel, heat, oxygen consumption)."""
        self.stats.fuel -= 0.1 * delta_time
        self.stats.oxygen -= 0.05 * delta_time
        self.stats.heat = max(0.0, self.stats.heat - 0.2 * delta_time)
        self.stats.clamp()

    def __repr__(self):
        port_info = self.link_port.port_type if self.link_port else "none"
        return f"<PlayerRig {self.player_id} port={port_info} {self.stats}>"


# ---------------------------------------------------------------------------
# Blender operator
# ---------------------------------------------------------------------------

class RIG_OT_visualize_ports(bpy.types.Operator):
    """Visualize interface ports for a demo ship layout"""
    bl_idname = "mesh.rig_visualize_ports"
    bl_label = "Rig: Visualize Interface Ports"
    bl_options = {'REGISTER', 'UNDO'}

    def execute(self, context):
        # Create a demo set of ports
        demo_ports = [
            InterfacePort(port_id="cockpit_main", port_type="COCKPIT",
                          position=(0, 2, 1)),
            InterfacePort(port_id="fleet_console_1", port_type="FLEET_CONSOLE",
                          position=(1, 0, 0)),
            InterfacePort(port_id="drone_console_1", port_type="DRONE_CONSOLE",
                          position=(-1, 0, 0)),
            InterfacePort(port_id="turret_top", port_type="TURRET_SEAT",
                          position=(0, 0, 2)),
            InterfacePort(port_id="engineering_1", port_type="ENGINEERING_PANEL",
                          position=(0, -2, 0)),
            InterfacePort(port_id="strategic_bridge", port_type="STRATEGIC_TABLE",
                          position=(0, 1, 1)),
        ]

        col_name = "InterfacePorts_Preview"
        col = bpy.data.collections.get(col_name)
        if col is None:
            col = bpy.data.collections.new(col_name)
            bpy.context.scene.collection.children.link(col)

        for port in demo_ports:
            empty = bpy.data.objects.new(f"port_{port.port_id}", None)
            empty.empty_display_type = 'PLAIN_AXES'
            empty.empty_display_size = 0.4
            empty.location = Vector(port.position)

            # Colour by the HUD layer this port triggers
            hud_layer = port.enter_mode
            colour = HUD_LAYER_COLOURS.get(hud_layer, (1, 1, 1, 1))
            mat_name = f"port_mat_{hud_layer}"
            mat = bpy.data.materials.get(mat_name)
            if mat is None:
                mat = bpy.data.materials.new(mat_name)
                mat.use_nodes = False
                mat.diffuse_color = colour

            # Store port info as custom properties for inspection
            empty["port_type"] = port.port_type
            empty["enter_mode"] = port.enter_mode
            empty["connected_systems"] = ", ".join(port.connected_systems)

            col.objects.link(empty)

        self.report({'INFO'},
                    f"Visualized {len(demo_ports)} interface ports")
        return {'FINISHED'}


# ---------------------------------------------------------------------------
# Registration
# ---------------------------------------------------------------------------

_classes = (
    RIG_OT_visualize_ports,
)


def register():
    for cls in _classes:
        bpy.utils.register_class(cls)


def unregister():
    for cls in reversed(_classes):
        bpy.utils.unregister_class(cls)
