"""
AtlasForge Generator — Blender Addon
Procedural content generation (PCG) tool for spaceships, stations, and asteroids.
Engine-agnostic asset generation pipeline designed for integration into multiple projects.
"""

bl_info = {
    "name": "AtlasForge Generator",
    "author": "AtlasForge",
    "version": (3, 0, 0),
    "blender": (2, 80, 0),
    "location": "View3D > Sidebar > AtlasForge",
    "description": "Engine-based PCG asset pipeline for procedural spaceships, stations, and asteroids",
    "category": "Add Mesh",
}

import importlib
import traceback

import bpy
from bpy.props import (
    EnumProperty,
    BoolProperty,
    IntProperty,
    FloatProperty,
    StringProperty,
)

# ---------------------------------------------------------------------------
# Deferred submodule loading
# ---------------------------------------------------------------------------
# Submodule imports are kept out of module-level scope so that Blender can
# always read ``bl_info`` without triggering heavyweight or broken imports.
# The dict is populated in ``register()`` and cleared in ``unregister()``.

_submodule_names = [
    "ship_generator",
    "ship_parts",
    "interior_generator",
    "module_system",
    "atlas_exporter",
    "station_generator",
    "asteroid_generator",
    "texture_generator",
    "brick_system",
    "pcg_panel",
    "novaforge_importer",
    "render_setup",
    "lod_generator",
    "collision_generator",
    "animation_system",
    "damage_system",
    "power_system",
    "build_validator",
    "density_field",
    "slot_grid",
    "traversal_system",
    "fleet_logistics",
    "rig_system",
]

_submodules: dict = {}
_failed_submodules: list = []


def _load_submodules():
    """Import (or reload) every submodule listed in ``_submodule_names``.

    Populates ``_submodules`` and ``_failed_submodules``.  Each submodule
    that fails is set to ``None`` so operators can detect missing modules.
    """
    _failed_submodules.clear()
    for name in _submodule_names:
        try:
            mod = importlib.import_module(f".{name}", package=__name__)
            _submodules[name] = mod
        except Exception as exc:
            _submodules[name] = None
            _failed_submodules.append((name, exc))
            print(f"[AtlasForge] WARNING: failed to import {name}: {exc}")
            traceback.print_exc()


def _reload_submodules():
    """Reload already-imported submodules (for addon hot-reload in Blender)."""
    _failed_submodules.clear()
    for name in _submodule_names:
        existing = _submodules.get(name)
        if existing is not None:
            try:
                _submodules[name] = importlib.reload(existing)
            except Exception as exc:
                _submodules[name] = None
                _failed_submodules.append((name, exc))
                print(f"[AtlasForge] WARNING: failed to reload {name}: {exc}")
                traceback.print_exc()
        else:
            # Was not loaded before — try a fresh import
            try:
                mod = importlib.import_module(f".{name}", package=__name__)
                _submodules[name] = mod
            except Exception as exc:
                _submodules[name] = None
                _failed_submodules.append((name, exc))
                print(f"[AtlasForge] WARNING: failed to import {name}: {exc}")
                traceback.print_exc()


def _get_mod(name):
    """Return the loaded submodule *name*, or ``None`` if unavailable."""
    return _submodules.get(name)


class SpaceshipGeneratorProperties(bpy.types.PropertyGroup):
    """Properties for the spaceship generator"""
    
    ship_class: EnumProperty(
        name="Ship Class",
        description="Type of ship to generate",
        items=[
            ('SHUTTLE', "Shuttle", "Small transport vessel"),
            ('FIGHTER', "Fighter", "Single-seat combat ship"),
            ('CORVETTE', "Corvette", "Small multi-crew combat ship"),
            ('FRIGATE', "Frigate", "Medium combat/utility ship"),
            ('DESTROYER', "Destroyer", "Heavy combat ship"),
            ('CRUISER', "Cruiser", "Large multi-role ship"),
            ('BATTLECRUISER', "Battlecruiser", "Heavy attack cruiser"),
            ('BATTLESHIP', "Battleship", "Heavy capital ship"),
            ('CARRIER', "Carrier", "Fleet carrier ship"),
            ('DREADNOUGHT', "Dreadnought", "Siege capital ship"),
            ('CAPITAL', "Capital", "Largest class capital ship"),
            ('TITAN', "Titan", "Supercapital flagship"),
            ('INDUSTRIAL', "Industrial", "Cargo hauler"),
            ('MINING_BARGE', "Mining Barge", "Mining vessel"),
            ('EXHUMER', "Exhumer", "Advanced mining vessel"),
            ('EXPLORER', "Explorer", "NMS-style long range exploration ship"),
            ('HAULER', "Hauler", "NMS-style heavy cargo transport"),
            ('EXOTIC', "Exotic", "NMS-style rare experimental ship"),
        ],
        default='FIGHTER'
    )
    
    generate_interior: BoolProperty(
        name="Generate Interior",
        description="Generate ship interior with rooms and corridors",
        default=True
    )
    
    module_slots: IntProperty(
        name="Module Slots",
        description="Number of additional module slots",
        default=2,
        min=0,
        max=10
    )
    
    seed: IntProperty(
        name="Random Seed",
        description="Seed for procedural generation",
        default=1,
        min=1
    )
    
    hull_complexity: FloatProperty(
        name="Hull Complexity",
        description="Complexity of hull geometry",
        default=1.0,
        min=0.1,
        max=3.0
    )
    
    symmetry: BoolProperty(
        name="Symmetry",
        description="Generate symmetric ship design",
        default=True
    )
    
    style: EnumProperty(
        name="Style",
        description="Ship design style",
        items=[
            ('MIXED', "Mixed", "Mixed style from all inspirations"),
            ('X4', "X4", "X4 Foundations style"),
            ('ELITE', "Elite Dangerous", "Elite Dangerous style"),
            ('EVE', "Eve Online", "Eve Online style"),
            ('SOLARI', "Solari", "EVEOFFLINE Solari faction - golden, elegant"),
            ('VEYREN', "Veyren", "EVEOFFLINE Veyren faction - angular, utilitarian"),
            ('AURELIAN', "Aurelian", "EVEOFFLINE Aurelian faction - sleek, organic"),
            ('KELDARI', "Keldari", "EVEOFFLINE Keldari faction - rugged, industrial"),
            ('NMS', "No Man's Sky", "No Man's Sky style - colorful, varied, organic"),
        ],
        default='MIXED'
    )

    novaforge_json_path: StringProperty(
        name="Ship JSON",
        description="Path to an EVEOFFLINE ship JSON file to import",
        subtype='FILE_PATH',
        default=""
    )

    novaforge_export_path: StringProperty(
        name="Export Path",
        description="Directory to export OBJ files for the EVEOFFLINE asset pipeline",
        subtype='DIR_PATH',
        default=""
    )

    station_type: EnumProperty(
        name="Station Type",
        description="Type of station to generate",
        items=[
            ('INDUSTRIAL', "Industrial", "Manufacturing and production"),
            ('MILITARY', "Military", "Naval and defense installation"),
            ('COMMERCIAL', "Commercial", "Trade and commerce hub"),
            ('RESEARCH', "Research", "Scientific research facility"),
            ('MINING', "Mining", "Ore refinement and mining support"),
            ('ASTRAHUS', "Astrahus", "Medium Upwell citadel"),
            ('FORTIZAR', "Fortizar", "Large Upwell citadel"),
            ('KEEPSTAR', "Keepstar", "Extra-large Upwell citadel"),
        ],
        default='INDUSTRIAL'
    )

    station_faction: EnumProperty(
        name="Station Faction",
        description="Faction style for the station",
        items=[
            ('SOLARI', "Solari", "Golden cathedral style"),
            ('VEYREN', "Veyren", "Industrial block style"),
            ('AURELIAN', "Aurelian", "Organic dome style"),
            ('KELDARI', "Keldari", "Rusted patchwork style"),
        ],
        default='SOLARI'
    )

    belt_layout: EnumProperty(
        name="Belt Layout",
        description="Asteroid belt shape",
        items=[
            ('SEMICIRCLE', "Semicircle", "Standard semicircular belt"),
            ('SPHERE', "Sphere", "Spherical distribution"),
            ('CLUSTER', "Cluster", "Dense anomaly cluster"),
            ('RING', "Ring", "Sparse outer ring"),
        ],
        default='SEMICIRCLE'
    )

    belt_ore_type: EnumProperty(
        name="Primary Ore",
        description="Primary ore type for the belt",
        items=[
            ('DUSTITE', "Dustite", "Brown-orange, common ore"),
            ('FERRITE', "Ferrite", "Gray metallic ore"),
            ('IGNAITE', "Ignaite", "Red-brown volcanic ore"),
            ('CRYSTITE', "Crystite", "Green crystalline ore"),
            ('SHADITE', "Shadite", "Golden-brown ore"),
            ('CORITE', "Corite", "Blue-cyan icy ore"),
            ('LUMINE', "Lumine", "Dark red dense ore"),
            ('SANGITE', "Sangite", "Bright red metallic ore"),
            ('GLACITE', "Glacite", "Golden valuable ore"),
            ('DENSITE', "Densite", "Light gray banded ore"),
            ('VOIDITE', "Voidite", "Dark nullsec ore"),
            ('SPODUMAIN', "Spodumain", "Silvery reflective ore"),
            ('PYRANITE', "Pyranite", "Purple rare ore"),
            ('STELLITE', "Stellite", "Green luminescent ore"),
            ('COSMITE', "Cosmite", "Orange-gold most valuable ore"),
            ('NEXORITE', "Nexorite", "Cyan crystalline radioactive ore"),
        ],
        default='DUSTITE'
    )

    belt_count: IntProperty(
        name="Asteroid Count",
        description="Number of asteroids in the belt",
        default=30,
        min=5,
        max=200
    )

    generate_textures: BoolProperty(
        name="Generate Textures",
        description="Apply procedural PBR textures to the generated ship",
        default=True
    )

    weathering: FloatProperty(
        name="Weathering",
        description="Amount of surface weathering, dirt, and wear",
        default=0.0,
        min=0.0,
        max=1.0
    )

    naming_prefix: StringProperty(
        name="Naming Prefix",
        description="Project naming prefix applied to all generated elements (e.g. 'EVEOFFLINE')",
        default=""
    )

    turret_hardpoints: IntProperty(
        name="Turret Hardpoints",
        description="Number of turret hardpoints (visual turret fittings with base, ring and barrel)",
        default=0,
        min=0,
        max=10
    )

    hull_taper: FloatProperty(
        name="Hull Taper",
        description="Silhouette taper factor (lower = more tapered, 1.0 = no taper)",
        default=0.85,
        min=0.5,
        max=1.0
    )

    ship_dna_export_path: StringProperty(
        name="Ship DNA Path",
        description="File path to export Ship DNA JSON for reproducible ships",
        subtype='FILE_PATH',
        default=""
    )

    launcher_hardpoints: IntProperty(
        name="Launcher Hardpoints",
        description="Number of missile/torpedo launcher hardpoints",
        default=0,
        min=0,
        max=8
    )

    drone_bays: IntProperty(
        name="Drone Bays",
        description="Number of drone bay recesses",
        default=0,
        min=0,
        max=5
    )

    generate_lods: BoolProperty(
        name="Generate LODs",
        description="Create LOD1-LOD3 decimated meshes for game engine use",
        default=False
    )

    generate_collision: BoolProperty(
        name="Generate Collision",
        description="Create simplified collision meshes for physics",
        default=False
    )

    collision_type: EnumProperty(
        name="Collision Type",
        description="Type of collision mesh to generate",
        items=[
            ('AUTO', "Auto", "Choose collision type based on ship class"),
            ('BOX', "Box", "Simple bounding box"),
            ('CONVEX_HULL', "Convex Hull", "Single convex hull"),
            ('MULTI_CONVEX', "Multi Convex", "Decomposed convex parts"),
        ],
        default='AUTO'
    )

    generate_animations: BoolProperty(
        name="Generate Animations",
        description="Set up animation actions for turrets, bays, and sensors",
        default=False
    )

    novaforge_data_dir: StringProperty(
        name="Data Directory",
        description="Path to project data/ships/ directory for batch import",
        subtype='DIR_PATH',
        default=""
    )

    batch_output_path: StringProperty(
        name="Batch Output",
        description="Directory for batch-generated OBJ + JSON output",
        subtype='DIR_PATH',
        default=""
    )


class SPACESHIP_OT_generate(bpy.types.Operator):
    """Generate a procedural spaceship"""
    bl_idname = "mesh.generate_spaceship"
    bl_label = "Generate Spaceship"
    bl_options = {'REGISTER', 'UNDO'}
    
    def execute(self, context):
        sg = _get_mod("ship_generator")
        if sg is None:
            self.report({'ERROR'}, "ship_generator not loaded")
            return {'CANCELLED'}

        props = context.scene.spaceship_props
        
        # Generate the spaceship
        hull = sg.generate_spaceship(
            ship_class=props.ship_class,
            seed=props.seed,
            generate_interior=props.generate_interior,
            module_slots=props.module_slots,
            hull_complexity=props.hull_complexity,
            symmetry=props.symmetry,
            style=props.style,
            naming_prefix=props.naming_prefix,
            turret_hardpoints=props.turret_hardpoints,
            hull_taper=props.hull_taper,
        )

        # Apply procedural textures if requested
        if props.generate_textures:
            tg = _get_mod("texture_generator")
            if tg is None:
                self.report({'WARNING'}, "texture_generator not loaded, skipping textures")
            else:
                tg.apply_textures_to_ship(
                    hull,
                    style=props.style,
                    seed=props.seed,
                    weathering=props.weathering,
                )

        # Generate LODs if requested
        if props.generate_lods:
            lg = _get_mod("lod_generator")
            if lg is None:
                self.report({'WARNING'}, "lod_generator not loaded, skipping LODs")
            else:
                lg.generate_lods(
                    hull,
                    ship_class=props.ship_class,
                    naming_prefix=props.naming_prefix,
                )

        # Generate collision mesh if requested
        if props.generate_collision:
            cg = _get_mod("collision_generator")
            if cg is None:
                self.report({'WARNING'}, "collision_generator not loaded, skipping collision")
            else:
                col_type = None if props.collision_type == 'AUTO' else props.collision_type
                cg.generate_collision_mesh(
                    hull,
                    collision_type=col_type,
                    ship_class=props.ship_class,
                    naming_prefix=props.naming_prefix,
                )

        # Set up animations if requested
        if props.generate_animations:
            anim = _get_mod("animation_system")
            if anim is None:
                self.report({'WARNING'}, "animation_system not loaded, skipping animations")
            else:
                scale = sg.SHIP_CONFIGS.get(props.ship_class, {}).get('scale', 1.0)
                anim.setup_ship_animations(
                    hull,
                    scale=scale,
                    naming_prefix=props.naming_prefix,
                )

        self.report({'INFO'}, f"Generated {props.ship_class} class spaceship")
        return {'FINISHED'}


class SPACESHIP_OT_import_novaforge(bpy.types.Operator):
    """Import ships from EVEOFFLINE JSON data and generate them"""
    bl_idname = "mesh.import_novaforge_ships"
    bl_label = "Import from EVEOFFLINE JSON"
    bl_options = {'REGISTER', 'UNDO'}

    def execute(self, context):
        ae = _get_mod("atlas_exporter")
        sg = _get_mod("ship_generator")
        if ae is None:
            self.report({'ERROR'}, "atlas_exporter not loaded")
            return {'CANCELLED'}
        if sg is None:
            self.report({'ERROR'}, "ship_generator not loaded")
            return {'CANCELLED'}

        props = context.scene.spaceship_props
        json_path = bpy.path.abspath(props.novaforge_json_path)

        if not json_path or not json_path.endswith('.json'):
            self.report({'ERROR'}, "Select a valid EVEOFFLINE ship JSON file")
            return {'CANCELLED'}

        import os
        if not os.path.isfile(json_path):
            self.report({'ERROR'}, f"File not found: {json_path}")
            return {'CANCELLED'}

        try:
            ships = ae.load_ship_data(json_path)
        except Exception as e:
            self.report({'ERROR'}, f"Failed to load JSON: {e}")
            return {'CANCELLED'}

        count = 0
        for ship_id, ship_data in ships.items():
            config = ae.parse_ship_config(ship_data)
            sg.generate_spaceship(
                ship_class=config['ship_class'],
                seed=config['seed'],
                generate_interior=config['generate_interior'],
                module_slots=config['module_slots'],
                hull_complexity=config['hull_complexity'],
                symmetry=config['symmetry'],
                style=config['style'],
            )
            count += 1

        self.report({'INFO'}, f"Generated {count} ships from EVEOFFLINE data")
        return {'FINISHED'}


class SPACESHIP_OT_export_obj(bpy.types.Operator):
    """Export the selected ship as OBJ for the EVEOFFLINE/Atlas engine"""
    bl_idname = "mesh.export_novaforge_obj"
    bl_label = "Export OBJ for Atlas"
    bl_options = {'REGISTER'}

    def execute(self, context):
        ae = _get_mod("atlas_exporter")
        if ae is None:
            self.report({'ERROR'}, "atlas_exporter not loaded")
            return {'CANCELLED'}

        props = context.scene.spaceship_props
        export_dir = bpy.path.abspath(props.novaforge_export_path)

        if not export_dir:
            self.report({'ERROR'}, "Set an export directory first")
            return {'CANCELLED'}

        import os
        os.makedirs(export_dir, exist_ok=True)

        obj = context.active_object
        if obj is None:
            self.report({'ERROR'}, "Select a ship object to export")
            return {'CANCELLED'}

        filename = obj.name.replace(' ', '_') + '.obj'
        filepath = os.path.join(export_dir, filename)

        # Select all children too
        bpy.ops.object.select_all(action='DESELECT')
        obj.select_set(True)
        for child in obj.children_recursive:
            child.select_set(True)

        ae.export_obj(filepath)

        self.report({'INFO'}, f"Exported to {filepath}")
        return {'FINISHED'}


class SPACESHIP_OT_generate_station(bpy.types.Operator):
    """Generate a procedural space station"""
    bl_idname = "mesh.generate_station"
    bl_label = "Generate Station"
    bl_options = {'REGISTER', 'UNDO'}

    def execute(self, context):
        stg = _get_mod("station_generator")
        if stg is None:
            self.report({'ERROR'}, "station_generator not loaded")
            return {'CANCELLED'}

        props = context.scene.spaceship_props
        stg.generate_station(
            station_type=props.station_type,
            faction=props.station_faction,
            seed=props.seed,
        )
        self.report({'INFO'}, f"Generated {props.station_type} station ({props.station_faction})")
        return {'FINISHED'}


class SPACESHIP_OT_generate_asteroid_belt(bpy.types.Operator):
    """Generate a procedural asteroid belt"""
    bl_idname = "mesh.generate_asteroid_belt"
    bl_label = "Generate Asteroid Belt"
    bl_options = {'REGISTER', 'UNDO'}

    def execute(self, context):
        ag = _get_mod("asteroid_generator")
        if ag is None:
            self.report({'ERROR'}, "asteroid_generator not loaded")
            return {'CANCELLED'}

        props = context.scene.spaceship_props
        ag.generate_asteroid_belt(
            layout=props.belt_layout,
            ore_types=[props.belt_ore_type],
            count=props.belt_count,
            seed=props.seed,
        )
        self.report({'INFO'}, f"Generated {props.belt_layout} belt with {props.belt_count} asteroids")
        return {'FINISHED'}


class SPACESHIP_OT_export_ship_dna(bpy.types.Operator):
    """Export Ship DNA JSON for the selected ship"""
    bl_idname = "mesh.export_ship_dna"
    bl_label = "Export Ship DNA"
    bl_options = {'REGISTER'}

    def execute(self, context):
        import os

        props = context.scene.spaceship_props
        export_path = bpy.path.abspath(props.ship_dna_export_path)

        if not export_path or not export_path.endswith('.json'):
            self.report({'ERROR'}, "Set a valid .json export path first")
            return {'CANCELLED'}

        obj = context.active_object
        if obj is None or "ship_dna" not in obj:
            self.report({'ERROR'}, "Select a generated ship (hull) with Ship DNA")
            return {'CANCELLED'}

        os.makedirs(os.path.dirname(export_path) or '.', exist_ok=True)
        with open(export_path, 'w') as f:
            f.write(obj["ship_dna"])

        self.report({'INFO'}, f"Ship DNA exported to {export_path}")
        return {'FINISHED'}


class SPACESHIP_OT_import_novaforge_dir(bpy.types.Operator):
    """Import a single ship from the NovaForge data directory"""
    bl_idname = "mesh.import_novaforge_ship"
    bl_label = "Import from Project Data"
    bl_options = {'REGISTER', 'UNDO'}

    def execute(self, context):
        ni = _get_mod("novaforge_importer")
        sg = _get_mod("ship_generator")
        if ni is None:
            self.report({'ERROR'}, "novaforge_importer not loaded")
            return {'CANCELLED'}
        if sg is None:
            self.report({'ERROR'}, "ship_generator not loaded")
            return {'CANCELLED'}

        props = context.scene.spaceship_props
        data_dir = bpy.path.abspath(props.novaforge_data_dir)

        if not data_dir:
            self.report({'ERROR'}, "Set the project data/ships/ directory first")
            return {'CANCELLED'}

        import os
        if not os.path.isdir(data_dir):
            self.report({'ERROR'}, f"Directory not found: {data_dir}")
            return {'CANCELLED'}

        ships = ni.load_ships_from_directory(data_dir)
        if not ships:
            self.report({'WARNING'}, "No ship definitions found in directory")
            return {'CANCELLED'}

        # Generate the first ship found
        ship_id, ship_def = next(iter(ships.items()))
        params = ni.ship_to_generator_params(ship_def)
        sg.generate_spaceship(**{
            k: v for k, v in params.items()
            if k in ('ship_class', 'seed', 'style', 'turret_hardpoints',
                     'hull_taper', 'generate_interior', 'module_slots',
                     'naming_prefix')
        })

        self.report({'INFO'}, f"Generated ship '{ship_id}' from project data")
        return {'FINISHED'}


class SPACESHIP_OT_batch_novaforge(bpy.types.Operator):
    """Batch generate all ships from the NovaForge data directory"""
    bl_idname = "mesh.batch_novaforge_ships"
    bl_label = "Batch Generate All Ships"
    bl_options = {'REGISTER', 'UNDO'}

    def execute(self, context):
        ni = _get_mod("novaforge_importer")
        sg = _get_mod("ship_generator")
        if ni is None:
            self.report({'ERROR'}, "novaforge_importer not loaded")
            return {'CANCELLED'}
        if sg is None:
            self.report({'ERROR'}, "ship_generator not loaded")
            return {'CANCELLED'}

        props = context.scene.spaceship_props
        data_dir = bpy.path.abspath(props.novaforge_data_dir)

        if not data_dir:
            self.report({'ERROR'}, "Set the project data/ships/ directory first")
            return {'CANCELLED'}

        import os
        if not os.path.isdir(data_dir):
            self.report({'ERROR'}, f"Directory not found: {data_dir}")
            return {'CANCELLED'}

        ships = ni.load_ships_from_directory(data_dir)
        all_params = ni.get_all_generator_params(ships)

        count = 0
        for ship_id, params in all_params:
            gen_kwargs = {
                k: v for k, v in params.items()
                if k in ('ship_class', 'seed', 'style', 'turret_hardpoints',
                         'hull_taper', 'generate_interior', 'module_slots',
                         'naming_prefix')
            }
            sg.generate_spaceship(**gen_kwargs)
            count += 1

        self.report({'INFO'}, f"Batch generated {count} ships from project data")
        return {'FINISHED'}


class SPACESHIP_OT_catalog_render(bpy.types.Operator):
    """Set up catalog camera and lighting for the selected ship"""
    bl_idname = "mesh.catalog_render"
    bl_label = "Setup Catalog Render"
    bl_options = {'REGISTER', 'UNDO'}

    def execute(self, context):
        rs = _get_mod("render_setup")
        if rs is None:
            self.report({'ERROR'}, "render_setup not loaded")
            return {'CANCELLED'}

        obj = context.active_object
        if obj is None:
            self.report({'ERROR'}, "Select a ship object first")
            return {'CANCELLED'}

        rs.setup_catalog_render(obj)
        self.report({'INFO'}, "Catalog render configured")
        return {'FINISHED'}


class SPACESHIP_OT_batch_generate(bpy.types.Operator):
    """Batch generate ships and export to output directory"""
    bl_idname = "mesh.batch_generate_all"
    bl_label = "Batch Generate & Export"
    bl_options = {'REGISTER'}

    def execute(self, context):
        import os

        ni = _get_mod("novaforge_importer")
        sg = _get_mod("ship_generator")
        ae = _get_mod("atlas_exporter")
        if ni is None:
            self.report({'ERROR'}, "novaforge_importer not loaded")
            return {'CANCELLED'}
        if sg is None:
            self.report({'ERROR'}, "ship_generator not loaded")
            return {'CANCELLED'}
        if ae is None:
            self.report({'ERROR'}, "atlas_exporter not loaded")
            return {'CANCELLED'}

        props = context.scene.spaceship_props
        output_dir = bpy.path.abspath(props.batch_output_path)

        if not output_dir:
            self.report({'ERROR'}, "Set an output directory first")
            return {'CANCELLED'}

        os.makedirs(output_dir, exist_ok=True)

        data_dir = bpy.path.abspath(props.novaforge_data_dir)
        if not data_dir or not os.path.isdir(data_dir):
            self.report({'ERROR'}, "Set a valid project data directory")
            return {'CANCELLED'}

        ships = ni.load_ships_from_directory(data_dir)
        all_params = ni.get_all_generator_params(ships)

        count = 0
        for ship_id, params in all_params:
            gen_kwargs = {
                k: v for k, v in params.items()
                if k in ('ship_class', 'seed', 'style', 'turret_hardpoints',
                         'hull_taper', 'generate_interior', 'module_slots',
                         'naming_prefix')
            }
            hull = sg.generate_spaceship(**gen_kwargs)

            # Export OBJ
            filepath = os.path.join(output_dir, f"{ship_id}.obj")
            bpy.ops.object.select_all(action='DESELECT')
            hull.select_set(True)
            for child in hull.children_recursive:
                child.select_set(True)
            ae.export_obj(filepath)
            count += 1

        self.report({'INFO'}, f"Batch exported {count} ships to {output_dir}")
        return {'FINISHED'}


class SPACESHIP_OT_novaforge_pipeline(bpy.types.Operator):
    """Full pipeline: generate from project JSON, apply textures, export OBJ"""
    bl_idname = "mesh.novaforge_pipeline_export"
    bl_label = "Full Pipeline Export"
    bl_options = {'REGISTER'}

    def execute(self, context):
        import os

        ni = _get_mod("novaforge_importer")
        sg = _get_mod("ship_generator")
        ae = _get_mod("atlas_exporter")
        if ni is None:
            self.report({'ERROR'}, "novaforge_importer not loaded")
            return {'CANCELLED'}
        if sg is None:
            self.report({'ERROR'}, "ship_generator not loaded")
            return {'CANCELLED'}
        if ae is None:
            self.report({'ERROR'}, "atlas_exporter not loaded")
            return {'CANCELLED'}

        props = context.scene.spaceship_props
        data_dir = bpy.path.abspath(props.novaforge_data_dir)
        output_dir = bpy.path.abspath(props.batch_output_path or props.novaforge_export_path)

        if not data_dir or not os.path.isdir(data_dir):
            self.report({'ERROR'}, "Set a valid project data directory")
            return {'CANCELLED'}

        if not output_dir:
            self.report({'ERROR'}, "Set an output directory")
            return {'CANCELLED'}

        os.makedirs(output_dir, exist_ok=True)

        ships = ni.load_ships_from_directory(data_dir)
        all_params = ni.get_all_generator_params(ships)

        tg = _get_mod("texture_generator")
        lg = _get_mod("lod_generator")

        count = 0
        for ship_id, params in all_params:
            gen_kwargs = {
                k: v for k, v in params.items()
                if k in ('ship_class', 'seed', 'style', 'turret_hardpoints',
                         'hull_taper', 'generate_interior', 'module_slots',
                         'naming_prefix')
            }
            hull = sg.generate_spaceship(**gen_kwargs)

            # Apply textures
            if props.generate_textures and tg is not None:
                tg.apply_textures_to_ship(
                    hull,
                    style=params.get('style', 'SOLARI'),
                    seed=params.get('seed', 1),
                    weathering=props.weathering,
                )

            # Generate LODs
            if props.generate_lods and lg is not None:
                lg.generate_lods(
                    hull,
                    ship_class=params.get('ship_class', 'FRIGATE'),
                    naming_prefix=params.get('naming_prefix', ''),
                )

            # Export OBJ
            filepath = os.path.join(output_dir, f"{ship_id}.obj")
            bpy.ops.object.select_all(action='DESELECT')
            hull.select_set(True)
            for child in hull.children_recursive:
                child.select_set(True)
            ae.export_obj(filepath)
            count += 1

        self.report({'INFO'}, f"Pipeline exported {count} ships to {output_dir}")
        return {'FINISHED'}


class SPACESHIP_PT_main_panel(bpy.types.Panel):
    """Main panel for AtlasForge generator"""
    bl_label = "AtlasForge Generator"
    bl_idname = "SPACESHIP_PT_main_panel"
    bl_space_type = 'VIEW_3D'
    bl_region_type = 'UI'
    bl_category = 'AtlasForge'
    
    def draw(self, context):
        layout = self.layout
        props = context.scene.spaceship_props
        
        layout.label(text="Ship Configuration:")
        layout.prop(props, "ship_class")
        layout.prop(props, "style")
        layout.prop(props, "seed")
        
        layout.separator()
        layout.label(text="Generation Options:")
        layout.prop(props, "generate_interior")
        layout.prop(props, "module_slots")
        layout.prop(props, "hull_complexity")
        layout.prop(props, "symmetry")

        layout.separator()
        layout.label(text="Naming & Hardpoints:")
        layout.prop(props, "naming_prefix")
        layout.prop(props, "turret_hardpoints")
        layout.prop(props, "launcher_hardpoints")
        layout.prop(props, "drone_bays")

        layout.separator()
        layout.label(text="Hull Shaping:")
        layout.prop(props, "hull_taper")

        layout.separator()
        layout.label(text="Texture Options:")
        layout.prop(props, "generate_textures")
        if props.generate_textures:
            layout.prop(props, "weathering")
        
        layout.separator()
        layout.operator("mesh.generate_spaceship", icon='MESH_CUBE')

        layout.separator()
        layout.label(text="Batch Generation:")
        layout.prop(props, "batch_output_path")
        layout.operator("mesh.batch_generate_all", icon='FILE_REFRESH')

        layout.separator()
        layout.label(text="Project Integration:")
        layout.prop(props, "novaforge_data_dir")
        layout.operator("mesh.import_novaforge_ship", icon='IMPORT')
        layout.operator("mesh.batch_novaforge_ships", icon='FILE_REFRESH')
        layout.operator("mesh.novaforge_pipeline_export", icon='EXPORT')
        layout.operator("mesh.catalog_render", icon='RENDER_STILL')

        layout.separator()
        layout.label(text="AtlasForge Export:")
        layout.prop(props, "novaforge_json_path")
        layout.operator("mesh.import_novaforge_ships", icon='IMPORT')
        layout.prop(props, "novaforge_export_path")
        layout.operator("mesh.export_novaforge_obj", icon='EXPORT')

        layout.separator()
        layout.label(text="Station Generation:")
        layout.prop(props, "station_type")
        layout.prop(props, "station_faction")
        layout.operator("mesh.generate_station", icon='WORLD')

        layout.separator()
        layout.label(text="Asteroid Belt Generation:")
        layout.prop(props, "belt_layout")
        layout.prop(props, "belt_ore_type")
        layout.prop(props, "belt_count")
        layout.operator("mesh.generate_asteroid_belt", icon='OUTLINER_OB_POINTCLOUD')

        layout.separator()
        layout.label(text="Ship DNA Export:")
        layout.prop(props, "ship_dna_export_path")
        layout.operator("mesh.export_ship_dna", icon='FILE_TEXT')

        layout.separator()
        layout.label(text="Game Engine Helpers:")
        layout.prop(props, "generate_lods")
        layout.prop(props, "generate_collision")
        if props.generate_collision:
            layout.prop(props, "collision_type")
        layout.prop(props, "generate_animations")


# Registration
classes = (
    SpaceshipGeneratorProperties,
    SPACESHIP_OT_generate,
    SPACESHIP_OT_import_novaforge,
    SPACESHIP_OT_export_obj,
    SPACESHIP_OT_generate_station,
    SPACESHIP_OT_generate_asteroid_belt,
    SPACESHIP_OT_export_ship_dna,
    SPACESHIP_OT_import_novaforge_dir,
    SPACESHIP_OT_batch_novaforge,
    SPACESHIP_OT_catalog_render,
    SPACESHIP_OT_batch_generate,
    SPACESHIP_OT_novaforge_pipeline,
    SPACESHIP_PT_main_panel,
)


def register():
    # Load submodules on first registration; reload on subsequent calls
    if _submodules:
        _reload_submodules()
    else:
        _load_submodules()

    # Register each class individually so that a single failure does not
    # prevent the remaining classes (including the main panel) from loading.
    registered_count = 0
    for cls in classes:
        try:
            bpy.utils.register_class(cls)
            registered_count += 1
        except Exception as exc:  # noqa: BLE001
            print(f"[AtlasForge] WARNING: register_class({cls.__name__}) failed: {exc}")
            traceback.print_exc()

    try:
        bpy.types.Scene.spaceship_props = bpy.props.PointerProperty(
            type=SpaceshipGeneratorProperties
        )
    except Exception as exc:  # noqa: BLE001
        print(f"[AtlasForge] WARNING: could not attach spaceship_props to Scene: {exc}")
        traceback.print_exc()

    # Register submodules — skip any that failed to import
    for name in _submodule_names:
        mod = _submodules.get(name)
        if mod is not None and hasattr(mod, "register"):
            try:
                mod.register()
            except Exception as exc:  # noqa: BLE001
                print(f"[AtlasForge] WARNING: {name}.register() failed: {exc}")

    # Inform the user that the addon loaded successfully
    print(f"[AtlasForge] Addon registered ({registered_count}/{len(classes)} classes)")
    if _failed_submodules:
        for name, exc in _failed_submodules:
            print(f"[AtlasForge]   ⚠ {name}: {exc}")
    else:
        print("[AtlasForge] All submodules loaded OK")


def unregister():
    # Unregister submodules in reverse order — skip any that failed to import
    for name in reversed(_submodule_names):
        mod = _submodules.get(name)
        if mod is not None and hasattr(mod, "unregister"):
            try:
                mod.unregister()
            except Exception as exc:  # noqa: BLE001
                print(f"[AtlasForge] WARNING: {name}.unregister() failed: {exc}")
                traceback.print_exc()

    _submodules.clear()

    if hasattr(bpy.types.Scene, "spaceship_props"):
        try:
            del bpy.types.Scene.spaceship_props
        except Exception as exc:  # noqa: BLE001
            print(f"[AtlasForge] WARNING: could not remove spaceship_props: {exc}")

    for cls in reversed(classes):
        try:
            bpy.utils.unregister_class(cls)
        except Exception as exc:  # noqa: BLE001
            print(f"[AtlasForge] WARNING: unregister_class({cls.__name__}) failed: {exc}")

    print("[AtlasForge] Addon unregistered")


if __name__ == "__main__":
    register()
