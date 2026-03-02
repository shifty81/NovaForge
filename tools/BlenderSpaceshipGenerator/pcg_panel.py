"""
Blender UI integration for the NovaForge PCG Pipeline.

Provides operators and a sidebar panel that let artists generate an entire
universe (galaxies → systems → planets → stations → ships → characters)
directly inside Blender.  The metadata is written to a configurable output
directory and, when Blender is available, ships and stations can optionally
be materialised as meshes in the current scene.
"""

import json
import os

import bpy
from bpy.props import (
    BoolProperty,
    EnumProperty,
    IntProperty,
    StringProperty,
)

# PCG pipeline modules (imported at runtime so the addon loads even when the
# pipeline package is not on sys.path — which may happen when installed as a
# normal Blender addon).
_pcg = None


def _ensure_pcg():
    """Lazy-import the pcg_pipeline package."""
    global _pcg
    if _pcg is None:
        from . import pcg_pipeline as _pcg_mod
        _pcg = _pcg_mod
    return _pcg


# ---------------------------------------------------------------------------
# Properties
# ---------------------------------------------------------------------------


class PCGPipelineProperties(bpy.types.PropertyGroup):
    """Properties for the PCG Pipeline panel."""

    universe_seed: IntProperty(
        name="Universe Seed",
        description="Master seed for deterministic universe generation",
        default=123456,
        min=0,
    )

    num_systems: IntProperty(
        name="Star Systems",
        description="Number of star systems to generate",
        default=5,
        min=1,
        max=100,
    )

    output_dir: StringProperty(
        name="Output Directory",
        description="Root directory for generated JSON files",
        subtype='DIR_PATH',
        default="//pcg_build",
    )

    materialize_ships: BoolProperty(
        name="Materialize Ships",
        description="Create ship meshes in the scene for the first system",
        default=False,
    )

    system_seed: IntProperty(
        name="System Seed",
        description="Seed for single-system generation",
        default=42,
        min=0,
    )


# ---------------------------------------------------------------------------
# Operators
# ---------------------------------------------------------------------------


class PCG_OT_generate_universe(bpy.types.Operator):
    """Generate a full NovaForge universe and write JSON metadata"""
    bl_idname = "pcg.generate_universe"
    bl_label = "Generate Universe"
    bl_options = {'REGISTER', 'UNDO'}

    def execute(self, context):
        pcg = _ensure_pcg()
        props = context.scene.pcg_props

        output_dir = bpy.path.abspath(props.output_dir)
        if not output_dir:
            self.report({'ERROR'}, "Set an output directory first")
            return {'CANCELLED'}

        galaxy = pcg.batch_generate.generate_universe(
            seed=props.universe_seed,
            num_systems=props.num_systems,
            output_dir=output_dir,
            export_meshes=False,
        )

        total_planets = sum(len(s["planets"]) for s in galaxy["systems"])
        total_stations = sum(len(s["stations"]) for s in galaxy["systems"])
        total_ships = sum(len(s["ships"]) for s in galaxy["systems"])
        total_chars = sum(len(s["characters"]) for s in galaxy["systems"])

        self.report(
            {'INFO'},
            f"Generated {len(galaxy['systems'])} systems, "
            f"{total_planets} planets, {total_stations} stations, "
            f"{total_ships} ships, {total_chars} characters → {output_dir}",
        )
        return {'FINISHED'}


class PCG_OT_generate_system(bpy.types.Operator):
    """Generate a single star system and write JSON metadata"""
    bl_idname = "pcg.generate_system"
    bl_label = "Generate System"
    bl_options = {'REGISTER', 'UNDO'}

    def execute(self, context):
        pcg = _ensure_pcg()
        props = context.scene.pcg_props

        output_dir = bpy.path.abspath(props.output_dir)
        if not output_dir:
            self.report({'ERROR'}, "Set an output directory first")
            return {'CANCELLED'}

        system = pcg.system_generator.generate_system(
            props.system_seed, "system_single",
        )

        os.makedirs(output_dir, exist_ok=True)
        sys_path = os.path.join(output_dir, "system_single.json")
        with open(sys_path, "w") as fh:
            json.dump(system, fh, indent=2)
            fh.write("\n")

        self.report(
            {'INFO'},
            f"Generated system with {len(system['planets'])} planets, "
            f"{len(system['stations'])} stations, "
            f"{len(system['ships'])} ships → {sys_path}",
        )
        return {'FINISHED'}


class PCG_OT_materialize_ships(bpy.types.Operator):
    """Load ship metadata from the output directory and build meshes"""
    bl_idname = "pcg.materialize_ships"
    bl_label = "Materialize Ships"
    bl_options = {'REGISTER', 'UNDO'}

    def execute(self, context):
        from . import ship_generator
        from . import atlas_exporter

        props = context.scene.pcg_props
        output_dir = bpy.path.abspath(props.output_dir)
        ships_dir = os.path.join(output_dir, "ships")

        if not os.path.isdir(ships_dir):
            self.report({'ERROR'}, f"No ships directory at {ships_dir}")
            return {'CANCELLED'}

        count = 0
        for filename in sorted(os.listdir(ships_dir)):
            if not filename.endswith(".json"):
                continue
            filepath = os.path.join(ships_dir, filename)
            try:
                with open(filepath) as fh:
                    ship_data = json.load(fh)
            except (json.JSONDecodeError, IOError):
                continue

            config = atlas_exporter.parse_ship_config(ship_data)
            ship_generator.generate_spaceship(
                ship_class=config['ship_class'],
                seed=config['seed'],
                generate_interior=config.get('generate_interior', True),
                module_slots=config.get('module_slots', 2),
                hull_complexity=config.get('hull_complexity', 1.0),
                symmetry=config.get('symmetry', True),
                style=config.get('style', 'MIXED'),
            )
            count += 1

        self.report({'INFO'}, f"Materialized {count} ship(s) in the scene")
        return {'FINISHED'}


# ---------------------------------------------------------------------------
# Panel
# ---------------------------------------------------------------------------


class PCG_PT_pipeline_panel(bpy.types.Panel):
    """Sidebar panel for the NovaForge PCG Pipeline"""
    bl_label = "PCG Pipeline"
    bl_idname = "PCG_PT_pipeline_panel"
    bl_space_type = 'VIEW_3D'
    bl_region_type = 'UI'
    bl_category = 'Spaceship'
    bl_options = {'DEFAULT_CLOSED'}

    def draw(self, context):
        layout = self.layout
        props = context.scene.pcg_props

        layout.label(text="Universe Generation:", icon='WORLD')
        layout.prop(props, "universe_seed")
        layout.prop(props, "num_systems")
        layout.prop(props, "output_dir")
        layout.operator("pcg.generate_universe", icon='WORLD_DATA')

        layout.separator()
        layout.label(text="Single System:", icon='LIGHT_SUN')
        layout.prop(props, "system_seed")
        layout.operator("pcg.generate_system", icon='LIGHT_SUN')

        layout.separator()
        layout.label(text="Materialize:", icon='MESH_CUBE')
        layout.operator("pcg.materialize_ships", icon='IMPORT')


# ---------------------------------------------------------------------------
# Registration
# ---------------------------------------------------------------------------

classes = (
    PCGPipelineProperties,
    PCG_OT_generate_universe,
    PCG_OT_generate_system,
    PCG_OT_materialize_ships,
    PCG_PT_pipeline_panel,
)


def register():
    for cls in classes:
        bpy.utils.register_class(cls)
    bpy.types.Scene.pcg_props = bpy.props.PointerProperty(
        type=PCGPipelineProperties,
    )


def unregister():
    del bpy.types.Scene.pcg_props
    for cls in reversed(classes):
        bpy.utils.unregister_class(cls)
