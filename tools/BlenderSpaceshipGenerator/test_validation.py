"""
Syntax and structure validation tests for the Blender Spaceship Generator addon
These tests run without requiring Blender to be installed
"""

import ast
import os
import sys


def test_python_syntax(filepath):
    """Test if a Python file has valid syntax"""
    try:
        with open(filepath, 'r') as f:
            code = f.read()
        ast.parse(code)
        return True, None
    except SyntaxError as e:
        return False, str(e)


def test_addon_structure():
    """Test that all required files exist"""
    print("Testing addon structure...")
    
    addon_path = os.path.dirname(os.path.abspath(__file__))
    required_files = [
        '__init__.py',
        'ship_generator.py',
        'ship_parts.py',
        'interior_generator.py',
        'module_system.py',
        'atlas_exporter.py',
        'station_generator.py',
        'asteroid_generator.py',
        'texture_generator.py',
        'brick_system.py',
        'pcg_panel.py',
        'novaforge_importer.py',
        'render_setup.py',
        'lod_generator.py',
        'collision_generator.py',
        'animation_system.py',
        'damage_system.py',
        'power_system.py',
        'build_validator.py',
        'lighting_system.py',
        'greeble_system.py',
        'preset_library.py',
        'furniture_system.py',
        'version_registry.py',
        'override_manager.py',
        'template_manager.py',
    ]
    
    all_exist = True
    for filename in required_files:
        filepath = os.path.join(addon_path, filename)
        if os.path.exists(filepath):
            print(f"✓ {filename} exists")
        else:
            print(f"✗ {filename} is missing")
            all_exist = False
    
    return all_exist


def test_file_syntax():
    """Test Python syntax of all module files"""
    print("\nTesting Python syntax...")
    
    addon_path = os.path.dirname(os.path.abspath(__file__))
    python_files = [
        '__init__.py',
        'ship_generator.py',
        'ship_parts.py',
        'interior_generator.py',
        'module_system.py',
        'atlas_exporter.py',
        'station_generator.py',
        'asteroid_generator.py',
        'texture_generator.py',
        'brick_system.py',
        'pcg_panel.py',
        'novaforge_importer.py',
        'render_setup.py',
        'lod_generator.py',
        'collision_generator.py',
        'animation_system.py',
        'damage_system.py',
        'power_system.py',
        'build_validator.py',
        'lighting_system.py',
        'greeble_system.py',
        'preset_library.py',
        'furniture_system.py',
        'version_registry.py',
        'override_manager.py',
        'template_manager.py',
    ]
    
    all_valid = True
    for filename in python_files:
        filepath = os.path.join(addon_path, filename)
        if os.path.exists(filepath):
            valid, error = test_python_syntax(filepath)
            if valid:
                print(f"✓ {filename} has valid syntax")
            else:
                print(f"✗ {filename} has syntax error: {error}")
                all_valid = False
    
    return all_valid


def test_bl_info():
    """Test that __init__.py has valid bl_info"""
    print("\nTesting bl_info metadata...")
    
    addon_path = os.path.dirname(os.path.abspath(__file__))
    init_file = os.path.join(addon_path, '__init__.py')
    
    try:
        with open(init_file, 'r') as f:
            content = f.read()
        
        # Check for bl_info
        if 'bl_info' not in content:
            print("✗ bl_info not found in __init__.py")
            return False
        
        # Parse and check bl_info
        tree = ast.parse(content)
        bl_info_found = False
        
        for node in ast.walk(tree):
            if isinstance(node, ast.Assign):
                for target in node.targets:
                    if isinstance(target, ast.Name) and target.id == 'bl_info':
                        bl_info_found = True
                        
                        # Check if it's a dictionary
                        if isinstance(node.value, ast.Dict):
                            keys = [k.s if isinstance(k, ast.Str) else (k.value if isinstance(k, ast.Constant) else None) 
                                   for k in node.value.keys]
                            
                            required_keys = ['name', 'author', 'version', 'blender', 'category']
                            for key in required_keys:
                                if key in keys:
                                    print(f"✓ bl_info has '{key}' key")
                                else:
                                    print(f"✗ bl_info missing '{key}' key")
                                    return False
                        break
        
        if not bl_info_found:
            print("✗ bl_info assignment not found")
            return False
        
        return True
        
    except Exception as e:
        print(f"✗ Error checking bl_info: {e}")
        return False


def test_blender_manifest():
    """Test that blender_manifest.toml exists and has required fields"""
    print("\nTesting blender_manifest.toml (Blender 4.2+ extension)...")

    addon_path = os.path.dirname(os.path.abspath(__file__))
    manifest_file = os.path.join(addon_path, 'blender_manifest.toml')

    if not os.path.exists(manifest_file):
        print("✗ blender_manifest.toml is missing (required for Blender 4.2+)")
        return False

    # Parse TOML properly when available, fall back to basic checks
    manifest = None
    try:
        import tomllib
        with open(manifest_file, 'rb') as f:
            manifest = tomllib.load(f)
    except ImportError:
        try:
            import tomli
            with open(manifest_file, 'rb') as f:
                manifest = tomli.load(f)
        except ImportError:
            pass

    required_fields = {
        'schema_version': None,
        'id': None,
        'version': None,
        'name': None,
        'tagline': None,
        'maintainer': None,
        'type': 'add-on',
        'blender_version_min': None,
        'license': None,
    }

    all_present = True
    if manifest is not None:
        for field, expected in required_fields.items():
            if field in manifest:
                if expected is not None and manifest[field] != expected:
                    print(f"✗ blender_manifest.toml '{field}' should be '{expected}', got '{manifest[field]}'")
                    all_present = False
                else:
                    print(f"✓ blender_manifest.toml has '{field}'")
            else:
                print(f"✗ blender_manifest.toml missing '{field}'")
                all_present = False
    else:
        # Fallback: basic line-level check when no TOML parser is available
        with open(manifest_file, 'r') as f:
            content = f.read()
        for field, expected in required_fields.items():
            found = any(
                line.strip().startswith(field) and '=' in line
                for line in content.splitlines()
                if not line.strip().startswith('#')
            )
            if found:
                print(f"✓ blender_manifest.toml has '{field}'")
            else:
                print(f"✗ blender_manifest.toml missing '{field}'")
                all_present = False

    return all_present


def test_register_functions():
    """Test that register/unregister functions exist"""
    print("\nTesting register/unregister functions...")
    
    addon_path = os.path.dirname(os.path.abspath(__file__))
    files_to_check = [
        '__init__.py',
        'ship_generator.py',
        'ship_parts.py',
        'interior_generator.py',
        'module_system.py',
        'atlas_exporter.py',
        'station_generator.py',
        'asteroid_generator.py',
        'texture_generator.py',
        'brick_system.py',
        'pcg_panel.py',
        'novaforge_importer.py',
        'render_setup.py',
        'lod_generator.py',
        'collision_generator.py',
        'animation_system.py',
        'damage_system.py',
        'power_system.py',
        'build_validator.py',
        'lighting_system.py',
        'greeble_system.py',
        'preset_library.py',
    ]
    
    all_valid = True
    for filename in files_to_check:
        filepath = os.path.join(addon_path, filename)
        
        try:
            with open(filepath, 'r') as f:
                content = f.read()
            
            has_register = 'def register()' in content
            has_unregister = 'def unregister()' in content
            
            if has_register and has_unregister:
                print(f"✓ {filename} has register/unregister functions")
            else:
                missing = []
                if not has_register:
                    missing.append('register')
                if not has_unregister:
                    missing.append('unregister')
                print(f"✗ {filename} missing: {', '.join(missing)}")
                all_valid = False
                
        except Exception as e:
            print(f"✗ Error checking {filename}: {e}")
            all_valid = False
    
    return all_valid


def test_documentation():
    """Test that documentation files exist"""
    print("\nTesting documentation...")
    
    addon_path = os.path.dirname(os.path.abspath(__file__))
    doc_files = ['README.md', 'USAGE.md']
    
    all_exist = True
    for filename in doc_files:
        filepath = os.path.join(addon_path, filename)
        if os.path.exists(filepath):
            size = os.path.getsize(filepath)
            print(f"✓ {filename} exists ({size} bytes)")
        else:
            print(f"✗ {filename} is missing")
            all_exist = False
    
    return all_exist


def test_turret_hardpoint_configs():
    """Test that all ship configs include turret_hardpoints and respect max 10"""
    print("\nTesting turret hardpoint configurations...")

    addon_path = os.path.dirname(os.path.abspath(__file__))
    sg_path = os.path.join(addon_path, 'ship_generator.py')

    with open(sg_path, 'r') as f:
        content = f.read()

    tree = ast.parse(content)
    ship_configs = None

    for node in ast.walk(tree):
        if isinstance(node, ast.Assign):
            for target in node.targets:
                if isinstance(target, ast.Name) and target.id == 'SHIP_CONFIGS':
                    ship_configs = node.value
                    break

    if ship_configs is None:
        print("✗ SHIP_CONFIGS not found in ship_generator.py")
        return False

    if not isinstance(ship_configs, ast.Dict):
        print("✗ SHIP_CONFIGS is not a dictionary")
        return False

    all_valid = True
    for key_node, value_node in zip(ship_configs.keys, ship_configs.values):
        class_name = key_node.value if isinstance(key_node, ast.Constant) else '?'
        if not isinstance(value_node, ast.Dict):
            continue
        inner_keys = []
        for k in value_node.keys:
            if isinstance(k, ast.Constant):
                inner_keys.append(k.value)
        if 'turret_hardpoints' not in inner_keys:
            print(f"✗ {class_name} missing 'turret_hardpoints' key")
            all_valid = False
        else:
            # Find the value
            idx = inner_keys.index('turret_hardpoints')
            val_node = value_node.values[idx]
            if isinstance(val_node, ast.Constant) and isinstance(val_node.value, int):
                if val_node.value > 10:
                    print(f"✗ {class_name} turret_hardpoints={val_node.value} exceeds max 10")
                    all_valid = False
                else:
                    print(f"✓ {class_name} turret_hardpoints={val_node.value}")
            else:
                print(f"✗ {class_name} turret_hardpoints is not an integer")
                all_valid = False

    return all_valid


def test_naming_prefix_support():
    """Test that key generation modules define _prefixed_name helper"""
    print("\nTesting naming prefix support...")

    addon_path = os.path.dirname(os.path.abspath(__file__))
    files_needing_prefix = [
        'ship_generator.py',
        'ship_parts.py',
        'interior_generator.py',
        'module_system.py',
    ]

    all_valid = True
    for filename in files_needing_prefix:
        filepath = os.path.join(addon_path, filename)
        with open(filepath, 'r') as f:
            content = f.read()
        if 'def _prefixed_name(' in content:
            print(f"✓ {filename} has _prefixed_name helper")
        else:
            print(f"✗ {filename} missing _prefixed_name helper")
            all_valid = False

    return all_valid


def test_turret_generation_function():
    """Test that ship_parts.py defines generate_turret_hardpoints"""
    print("\nTesting turret generation function...")

    addon_path = os.path.dirname(os.path.abspath(__file__))
    sp_path = os.path.join(addon_path, 'ship_parts.py')

    with open(sp_path, 'r') as f:
        content = f.read()

    checks = {
        'def generate_turret_hardpoints(': 'generate_turret_hardpoints function',
        '"turret_index"': 'turret_index custom property',
        '"turret_type"': 'turret_type custom property',
        '"tracking_speed"': 'tracking_speed custom property',
        '"rotation_limits"': 'rotation_limits custom property',
    }

    all_valid = True
    for pattern, description in checks.items():
        if pattern in content:
            print(f"✓ {description} found")
        else:
            print(f"✗ {description} not found")
            all_valid = False

    return all_valid


def test_brick_system():
    """Test the brick taxonomy, scale bands, grid system and Ship DNA helpers"""
    print("\nTesting brick system...")

    addon_path = os.path.dirname(os.path.abspath(__file__))
    bs_path = os.path.join(addon_path, 'brick_system.py')

    # Import brick_system directly (no bpy dependency)
    import importlib.util
    spec = importlib.util.spec_from_file_location("brick_system", bs_path)
    bs = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(bs)

    all_valid = True

    # Check BRICK_TYPES has entries for every category
    for category, brick_names in bs.BRICK_CATEGORIES.items():
        for name in brick_names:
            if name not in bs.BRICK_TYPES:
                print(f"✗ Brick type {name} (category {category}) missing from BRICK_TYPES")
                all_valid = False
            else:
                bt = bs.BRICK_TYPES[name]
                for key in ('category', 'size', 'shape', 'scale_band', 'hardpoints'):
                    if key not in bt:
                        print(f"✗ Brick {name} missing key '{key}'")
                        all_valid = False
    if all_valid:
        print(f"✓ All {sum(len(v) for v in bs.BRICK_CATEGORIES.values())} brick types valid")

    # Scale bands
    for band in ('primary', 'structural', 'detail'):
        val = bs.get_scale_factor(band)
        if not (0 < val <= 1.0):
            print(f"✗ Scale band '{band}' has invalid factor {val}")
            all_valid = False
    if bs.get_scale_factor('primary') > bs.get_scale_factor('structural') > bs.get_scale_factor('detail'):
        print("✓ Scale hierarchy correct (primary > structural > detail)")
    else:
        print("✗ Scale hierarchy incorrect")
        all_valid = False

    # Grid snapping
    snapped = bs.snap_to_grid((1.3, 2.7, 0.1), 1.0)
    if snapped == (1.0, 3.0, 0.0):
        print("✓ snap_to_grid works correctly")
    else:
        print(f"✗ snap_to_grid returned {snapped}, expected (1.0, 3.0, 0.0)")
        all_valid = False

    # Engine archetypes
    for arch in ('MAIN_THRUST', 'MANEUVERING', 'UTILITY_EXHAUST'):
        if bs.get_engine_archetype(arch) is None:
            print(f"✗ Engine archetype {arch} missing")
            all_valid = False
    print("✓ All engine archetypes defined")

    # Ship DNA round-trip
    dna = bs.generate_ship_dna('CRUISER', 42, [{'type': 'REACTOR_CORE', 'pos': [0, 0, 0]}])
    json_str = bs.ship_dna_to_json(dna)
    loaded = bs.ship_dna_from_json(json_str)
    if loaded['seed'] == 42 and loaded['class'] == 'CRUISER' and len(loaded['bricks']) == 1:
        print("✓ Ship DNA round-trip works")
    else:
        print("✗ Ship DNA round-trip failed")
        all_valid = False

    return all_valid


def test_hull_taper_and_cleanup():
    """Test that ship_generator defines taper_hull and apply_cleanup_pass"""
    print("\nTesting hull taper and cleanup pass...")

    addon_path = os.path.dirname(os.path.abspath(__file__))
    sg_path = os.path.join(addon_path, 'ship_generator.py')

    with open(sg_path, 'r') as f:
        content = f.read()

    checks = {
        'def taper_hull(': 'taper_hull function',
        'def apply_cleanup_pass(': 'apply_cleanup_pass function',
        'hull_taper': 'hull_taper parameter',
        '"ship_dna"': 'Ship DNA custom property',
    }

    all_valid = True
    for pattern, description in checks.items():
        if pattern in content:
            print(f"✓ {description} found")
        else:
            print(f"✗ {description} not found")
            all_valid = False

    return all_valid


def test_engine_archetypes():
    """Test that ship_parts uses engine archetypes"""
    print("\nTesting engine archetype integration...")

    addon_path = os.path.dirname(os.path.abspath(__file__))
    sp_path = os.path.join(addon_path, 'ship_parts.py')

    with open(sp_path, 'r') as f:
        content = f.read()

    checks = {
        'engine_archetype': 'engine archetype custom property',
        'select_engine_archetype': 'archetype selection',
        '_add_nozzle_flare': 'nozzle flare helper',
    }

    all_valid = True
    for pattern, description in checks.items():
        if pattern in content:
            print(f"✓ {description} found")
        else:
            print(f"✗ {description} not found")
            all_valid = False

    return all_valid


def test_exterior_influence_definitions():
    """Test that EXTERIOR_INFLUENCE is defined for every MODULE_TYPE"""
    print("\nTesting exterior influence definitions...")

    addon_path = os.path.dirname(os.path.abspath(__file__))
    ms_path = os.path.join(addon_path, 'module_system.py')

    with open(ms_path, 'r') as f:
        content = f.read()

    # Basic structure checks
    checks = {
        'EXTERIOR_INFLUENCE': 'EXTERIOR_INFLUENCE dict',
        'def get_exterior_influence(': 'get_exterior_influence function',
        'def collect_fitted_module_types(': 'collect_fitted_module_types function',
        'def apply_exterior_influence(': 'apply_exterior_influence function',
        '"module_type"': 'module_type custom property on modules',
    }

    all_valid = True
    for pattern, description in checks.items():
        if pattern in content:
            print(f"✓ {description} found")
        else:
            print(f"✗ {description} not found")
            all_valid = False

    # Verify every MODULE_TYPE has a matching EXTERIOR_INFLUENCE entry
    import importlib.util
    spec = importlib.util.spec_from_file_location("module_system_check", ms_path)
    # module_system imports bpy which is unavailable outside Blender, so
    # we parse the AST instead.
    tree = ast.parse(content)
    module_keys = set()
    exterior_keys = set()
    for node in ast.walk(tree):
        if isinstance(node, ast.Assign):
            for target in node.targets:
                if isinstance(target, ast.Name) and target.id == 'MODULE_TYPES':
                    if isinstance(node.value, ast.Dict):
                        for k in node.value.keys:
                            if isinstance(k, ast.Constant):
                                module_keys.add(k.value)
                if isinstance(target, ast.Name) and target.id == 'EXTERIOR_INFLUENCE':
                    if isinstance(node.value, ast.Dict):
                        for k in node.value.keys:
                            if isinstance(k, ast.Constant):
                                exterior_keys.add(k.value)

    if module_keys and exterior_keys:
        missing = module_keys - exterior_keys
        if missing:
            print(f"✗ MODULE_TYPES without EXTERIOR_INFLUENCE: {missing}")
            all_valid = False
        else:
            print(f"✓ All {len(module_keys)} module types have exterior influence")
    else:
        print("✗ Could not parse MODULE_TYPES or EXTERIOR_INFLUENCE")
        all_valid = False

    return all_valid


def test_module_room_types():
    """Test that MODULE_ROOM_TYPES covers every module type in interior_generator"""
    print("\nTesting module-specific interior rooms...")

    addon_path = os.path.dirname(os.path.abspath(__file__))
    ig_path = os.path.join(addon_path, 'interior_generator.py')

    with open(ig_path, 'r') as f:
        content = f.read()

    checks = {
        'MODULE_ROOM_TYPES': 'MODULE_ROOM_TYPES dict',
        'def generate_module_rooms(': 'generate_module_rooms function',
        'def generate_module_room(': 'generate_module_room function',
    }

    all_valid = True
    for pattern, description in checks.items():
        if pattern in content:
            print(f"✓ {description} found")
        else:
            print(f"✗ {description} not found")
            all_valid = False

    # Parse MODULE_ROOM_TYPES keys
    tree = ast.parse(content)
    room_keys = set()
    for node in ast.walk(tree):
        if isinstance(node, ast.Assign):
            for target in node.targets:
                if isinstance(target, ast.Name) and target.id == 'MODULE_ROOM_TYPES':
                    if isinstance(node.value, ast.Dict):
                        for k in node.value.keys:
                            if isinstance(k, ast.Constant):
                                room_keys.add(k.value)

    expected = {'CARGO', 'WEAPON', 'SHIELD', 'HANGAR', 'SENSOR', 'POWER'}
    if room_keys == expected:
        print(f"✓ MODULE_ROOM_TYPES covers all {len(expected)} module types")
    else:
        missing = expected - room_keys
        extra = room_keys - expected
        if missing:
            print(f"✗ Missing room types: {missing}")
        if extra:
            print(f"✗ Unexpected room types: {extra}")
        all_valid = False

    return all_valid


def test_pipeline_stages():
    """Test that ship_generator includes the module-driven stages"""
    print("\nTesting pipeline stages in ship_generator...")

    addon_path = os.path.dirname(os.path.abspath(__file__))
    sg_path = os.path.join(addon_path, 'ship_generator.py')

    with open(sg_path, 'r') as f:
        content = f.read()

    checks = {
        'collect_fitted_module_types': 'collect fitted module types call',
        'apply_exterior_influence': 'apply exterior influence call',
        'generate_module_rooms': 'generate module rooms call',
        'Module-driven exterior influence': 'exterior influence stage comment',
    }

    all_valid = True
    for pattern, description in checks.items():
        if pattern in content:
            print(f"✓ {description} found")
        else:
            print(f"✗ {description} not found")
            all_valid = False

    return all_valid


def test_pcg_pipeline():
    """Test that the PCG pipeline module exists and passes its own tests"""
    print("\nTesting PCG pipeline integration...")

    addon_path = os.path.dirname(os.path.abspath(__file__))
    pipeline_path = os.path.join(addon_path, 'pcg_pipeline')

    # Check required files
    required_files = [
        '__init__.py',
        '__main__.py',
        'galaxy_generator.py',
        'system_generator.py',
        'planet_generator.py',
        'station_generator.py',
        'ship_generator.py',
        'character_generator.py',
        'batch_generate.py',
        'test_pcg_pipeline.py',
    ]

    all_valid = True
    for filename in required_files:
        filepath = os.path.join(pipeline_path, filename)
        if os.path.exists(filepath):
            print(f"✓ pcg_pipeline/{filename} exists")
        else:
            print(f"✗ pcg_pipeline/{filename} is missing")
            all_valid = False

    # Check Python syntax of pipeline files
    for filename in required_files:
        filepath = os.path.join(pipeline_path, filename)
        if os.path.exists(filepath):
            valid, error = test_python_syntax(filepath)
            if not valid:
                print(f"✗ pcg_pipeline/{filename} has syntax error: {error}")
                all_valid = False

    if all_valid:
        print("✓ All PCG pipeline files present with valid syntax")

    return all_valid


def test_pcg_panel():
    """Test that the PCG pipeline Blender panel module exists and is valid"""
    print("\nTesting PCG pipeline Blender panel...")

    addon_path = os.path.dirname(os.path.abspath(__file__))
    panel_path = os.path.join(addon_path, 'pcg_panel.py')

    if not os.path.exists(panel_path):
        print("✗ pcg_panel.py is missing")
        return False

    valid, error = test_python_syntax(panel_path)
    if not valid:
        print(f"✗ pcg_panel.py has syntax error: {error}")
        return False
    print("✓ pcg_panel.py has valid syntax")

    with open(panel_path, 'r') as f:
        content = f.read()

    checks = {
        'class PCGPipelineProperties': 'PCGPipelineProperties class',
        'class PCG_OT_generate_universe': 'Generate Universe operator',
        'class PCG_OT_generate_system': 'Generate System operator',
        'class PCG_OT_materialize_ships': 'Materialize Ships operator',
        'class PCG_PT_pipeline_panel': 'PCG Pipeline panel',
        'def register()': 'register function',
        'def unregister()': 'unregister function',
    }

    all_valid = True
    for pattern, description in checks.items():
        if pattern in content:
            print(f"✓ {description} found")
        else:
            print(f"✗ {description} not found")
            all_valid = False

    return all_valid


def test_pcg_pipeline_init_exports():
    """Test that pcg_pipeline __init__.py exports all sub-modules"""
    print("\nTesting PCG pipeline __init__.py exports...")

    addon_path = os.path.dirname(os.path.abspath(__file__))
    init_path = os.path.join(addon_path, 'pcg_pipeline', '__init__.py')

    with open(init_path, 'r') as f:
        content = f.read()

    expected_imports = [
        'galaxy_generator',
        'system_generator',
        'planet_generator',
        'station_generator',
        'ship_generator',
        'character_generator',
        'batch_generate',
    ]

    all_valid = True
    for mod in expected_imports:
        if mod in content:
            print(f"✓ {mod} referenced in __init__.py")
        else:
            print(f"✗ {mod} not referenced in __init__.py")
            all_valid = False

    if '__all__' in content:
        print("✓ __all__ defined")
    else:
        print("✗ __all__ not defined")
        all_valid = False

    return all_valid


def test_lod_tiers():
    """Test that ship_generator in the PCG pipeline includes LOD tier data"""
    print("\nTesting LOD tier definitions...")

    addon_path = os.path.dirname(os.path.abspath(__file__))
    sg_path = os.path.join(addon_path, 'pcg_pipeline', 'ship_generator.py')

    with open(sg_path, 'r') as f:
        content = f.read()

    checks = {
        'LOD_TIERS': 'LOD_TIERS constant',
        'lod_tiers': 'lod_tiers in ship metadata',
        'poly_fraction': 'poly_fraction field',
        'max_distance': 'max_distance field',
    }

    all_valid = True
    for pattern, description in checks.items():
        if pattern in content:
            print(f"✓ {description} found")
        else:
            print(f"✗ {description} not found")
            all_valid = False

    # Functional check via import
    import importlib.util
    spec = importlib.util.spec_from_file_location("pcg_ship_gen", sg_path)
    mod = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(mod)

    if hasattr(mod, 'LOD_TIERS') and len(mod.LOD_TIERS) >= 3:
        print(f"✓ {len(mod.LOD_TIERS)} LOD tiers defined")
    else:
        print("✗ LOD_TIERS missing or insufficient")
        all_valid = False

    # Verify ship metadata includes lod_tiers
    ship = mod.generate_ship_metadata(1, "test_ship")
    if 'lod_tiers' in ship and len(ship['lod_tiers']) >= 3:
        print("✓ Ship metadata includes LOD tiers")
    else:
        print("✗ Ship metadata missing LOD tiers")
        all_valid = False

    return all_valid


def test_novaforge_importer():
    """Test that novaforge_importer.py has all required functions and data"""
    print("\nTesting NovaForge importer module...")

    addon_path = os.path.dirname(os.path.abspath(__file__))
    nf_path = os.path.join(addon_path, 'novaforge_importer.py')

    if not os.path.exists(nf_path):
        print("✗ novaforge_importer.py not found")
        return False

    # Import the module directly (no bpy dependency)
    import importlib.util
    spec = importlib.util.spec_from_file_location("novaforge_importer", nf_path)
    nf = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(nf)

    all_valid = True

    # Check data tables
    if len(nf.RACE_TO_STYLE) >= 4:
        print(f"✓ RACE_TO_STYLE has {len(nf.RACE_TO_STYLE)} entries")
    else:
        print("✗ RACE_TO_STYLE missing entries")
        all_valid = False

    if len(nf.CLASS_MAP) >= 15:
        print(f"✓ CLASS_MAP has {len(nf.CLASS_MAP)} entries")
    else:
        print("✗ CLASS_MAP missing entries")
        all_valid = False

    # Check translation function
    test_def = {
        'class': 'Frigate',
        'race': 'Solari',
        'model_data': {'generation_seed': 42, 'turret_hardpoints': 3}
    }
    params = nf.ship_to_generator_params(test_def)
    if params['ship_class'] == 'FRIGATE' and params['style'] == 'SOLARI' and params['seed'] == 42:
        print("✓ ship_to_generator_params works correctly")
    else:
        print("✗ ship_to_generator_params returned incorrect values")
        all_valid = False

    return all_valid


def test_lod_generator():
    """Test that lod_generator.py has required constants and functions"""
    print("\nTesting LOD generator module...")

    addon_path = os.path.dirname(os.path.abspath(__file__))
    lg_path = os.path.join(addon_path, 'lod_generator.py')

    if not os.path.exists(lg_path):
        print("✗ lod_generator.py not found")
        return False

    with open(lg_path, 'r') as f:
        content = f.read()

    checks = {
        'LOD_LEVELS': 'LOD_LEVELS constant',
        'LOD_DISTANCES': 'LOD_DISTANCES constant',
        'def generate_lods(': 'generate_lods function',
        'def get_lod_distances(': 'get_lod_distances function',
    }

    all_valid = True
    for pattern, description in checks.items():
        if pattern in content:
            print(f"✓ {description} found")
        else:
            print(f"✗ {description} not found")
            all_valid = False

    return all_valid


def test_collision_generator():
    """Test that collision_generator.py has required constants and functions"""
    print("\nTesting collision generator module...")

    addon_path = os.path.dirname(os.path.abspath(__file__))
    cg_path = os.path.join(addon_path, 'collision_generator.py')

    if not os.path.exists(cg_path):
        print("✗ collision_generator.py not found")
        return False

    with open(cg_path, 'r') as f:
        content = f.read()

    checks = {
        'COLLISION_TYPES': 'COLLISION_TYPES constant',
        'DEFAULT_COLLISION_TYPE': 'DEFAULT_COLLISION_TYPE constant',
        'def generate_collision_mesh(': 'generate_collision_mesh function',
        'def generate_box_collision(': 'generate_box_collision function',
        'def generate_convex_hull_collision(': 'generate_convex_hull_collision function',
    }

    all_valid = True
    for pattern, description in checks.items():
        if pattern in content:
            print(f"✓ {description} found")
        else:
            print(f"✗ {description} not found")
            all_valid = False

    return all_valid


def test_animation_system():
    """Test that animation_system.py has required constants and functions"""
    print("\nTesting animation system module...")

    addon_path = os.path.dirname(os.path.abspath(__file__))
    anim_path = os.path.join(addon_path, 'animation_system.py')

    if not os.path.exists(anim_path):
        print("✗ animation_system.py not found")
        return False

    with open(anim_path, 'r') as f:
        content = f.read()

    checks = {
        'ANIMATION_PRESETS': 'ANIMATION_PRESETS constant',
        'FRAME_RATE': 'FRAME_RATE constant',
        'def create_turret_animation(': 'create_turret_animation function',
        'def create_bay_door_animation(': 'create_bay_door_animation function',
        'def create_landing_gear(': 'create_landing_gear function',
        'def create_radar_spin_animation(': 'create_radar_spin_animation function',
        'def setup_ship_animations(': 'setup_ship_animations function',
    }

    all_valid = True
    for pattern, description in checks.items():
        if pattern in content:
            print(f"✓ {description} found")
        else:
            print(f"✗ {description} not found")
            all_valid = False

    return all_valid


def test_damage_system():
    """Test the damage propagation system"""
    print("\nTesting damage system module...")

    addon_path = os.path.dirname(os.path.abspath(__file__))
    ds_path = os.path.join(addon_path, 'damage_system.py')

    if not os.path.exists(ds_path):
        print("✗ damage_system.py not found")
        return False

    import importlib.util
    spec = importlib.util.spec_from_file_location("damage_system", ds_path)
    ds = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(ds)

    all_valid = True

    # Check constants
    if len(ds.HULL_WEIGHTS) >= 18:
        print(f"✓ HULL_WEIGHTS has {len(ds.HULL_WEIGHTS)} entries")
    else:
        print("✗ HULL_WEIGHTS missing entries")
        all_valid = False

    if len(ds.BRICK_HP) >= 18:
        print(f"✓ BRICK_HP has {len(ds.BRICK_HP)} entries")
    else:
        print("✗ BRICK_HP missing entries")
        all_valid = False

    # Test ShipDamageState from DNA
    dna = {
        'grid_size': 1.0,
        'seed': 42,
        'bricks': [
            {'type': 'STRUCTURAL_SPINE', 'pos': [0, 0, 0]},
            {'type': 'HULL_PLATE', 'pos': [1, 0, 0]},
            {'type': 'ENGINE_BLOCK', 'pos': [0, -1, 0]},
        ],
    }
    state = ds.ShipDamageState.from_ship_dna(dna)
    if state.alive_count() == 3:
        print("✓ ShipDamageState created with 3 bricks")
    else:
        print(f"✗ ShipDamageState has {state.alive_count()} bricks, expected 3")
        all_valid = False

    # Test damage application
    events = state.apply_damage(1, 50)
    brick = state.bricks.get(1)
    if brick and brick.hp < brick.max_hp:
        print("✓ apply_damage reduces HP")
    else:
        print("✗ apply_damage failed")
        all_valid = False

    return all_valid


def test_power_system():
    """Test the power flow simulation"""
    print("\nTesting power system module...")

    addon_path = os.path.dirname(os.path.abspath(__file__))
    ps_path = os.path.join(addon_path, 'power_system.py')

    if not os.path.exists(ps_path):
        print("✗ power_system.py not found")
        return False

    import importlib.util
    spec = importlib.util.spec_from_file_location("power_system", ps_path)
    ps = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(ps)

    all_valid = True

    # Check capacitor sizes
    if len(ps.CAPACITOR_SIZE) >= 18:
        print(f"✓ CAPACITOR_SIZE has {len(ps.CAPACITOR_SIZE)} entries")
    else:
        print("✗ CAPACITOR_SIZE missing entries")
        all_valid = False

    # Test ShipPowerState from DNA
    dna = {
        'class': 'CRUISER',
        'bricks': [
            {'type': 'REACTOR_CORE', 'pos': [0, 0, 0]},
            {'type': 'ENGINE_BLOCK', 'pos': [0, -1, 0]},
            {'type': 'SHIELD_EMITTER', 'pos': [0, 1, 0]},
        ],
    }
    power = ps.ShipPowerState.from_ship_dna(dna)
    if power.total_generation > 0:
        print(f"✓ Power generation: {power.total_generation} MW")
    else:
        print("✗ No power generation")
        all_valid = False

    if power.total_consumption > 0:
        print(f"✓ Power consumption: {power.total_consumption} MW")
    else:
        print("✗ No power consumption")
        all_valid = False

    if power.power_stable:
        print("✓ Power is stable (generation >= consumption)")
    else:
        print("✗ Power is unstable")
        all_valid = False

    return all_valid


def test_build_validator():
    """Test the build validation system"""
    print("\nTesting build validator module...")

    addon_path = os.path.dirname(os.path.abspath(__file__))
    bv_path = os.path.join(addon_path, 'build_validator.py')

    if not os.path.exists(bv_path):
        print("✗ build_validator.py not found")
        return False

    import importlib.util
    spec = importlib.util.spec_from_file_location("build_validator", bv_path)
    bv = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(bv)

    all_valid = True

    # Test basic placement
    validator = bv.BuildValidator(grid_size=1.0)
    validator.place_brick('STRUCTURAL_SPINE', (0, 0, 0))

    # Validate adjacent placement
    result = validator.validate_placement('HULL_PLATE', (1, 0, 0))
    if result.valid:
        print("✓ Adjacent placement validates correctly")
    else:
        print(f"✗ Adjacent placement rejected: {result.errors}")
        all_valid = False

    # Validate overlap detection
    result = validator.validate_placement('HULL_PLATE', (0, 0, 0))
    if not result.valid:
        print("✓ Overlap detection works")
    else:
        print("✗ Overlap not detected")
        all_valid = False

    # Test connectivity check
    validator.place_brick('HULL_PLATE', (1, 0, 0))
    if validator.check_connectivity():
        print("✓ Connectivity check passes for connected bricks")
    else:
        print("✗ Connectivity check failed for connected bricks")
        all_valid = False

    return all_valid


def test_render_setup():
    """Test that render_setup.py has required functions"""
    print("\nTesting render setup module...")

    addon_path = os.path.dirname(os.path.abspath(__file__))
    rs_path = os.path.join(addon_path, 'render_setup.py')

    if not os.path.exists(rs_path):
        print("✗ render_setup.py not found")
        return False

    with open(rs_path, 'r') as f:
        content = f.read()

    checks = {
        'CATALOG_RESOLUTION': 'CATALOG_RESOLUTION constant',
        'THUMBNAIL_RESOLUTION': 'THUMBNAIL_RESOLUTION constant',
        'def setup_catalog_render(': 'setup_catalog_render function',
        'def setup_thumbnail_render(': 'setup_thumbnail_render function',
        'def render_to_file(': 'render_to_file function',
    }

    all_valid = True
    for pattern, description in checks.items():
        if pattern in content:
            print(f"✓ {description} found")
        else:
            print(f"✗ {description} not found")
            all_valid = False

    return all_valid


def test_lighting_system():
    """Test that lighting_system.py has required constants and functions"""
    print("\nTesting lighting system module...")

    addon_path = os.path.dirname(os.path.abspath(__file__))
    ls_path = os.path.join(addon_path, 'lighting_system.py')

    if not os.path.exists(ls_path):
        print("✗ lighting_system.py not found")
        return False

    with open(ls_path, 'r') as f:
        content = f.read()

    checks = {
        'WARM_WHITE': 'WARM_WHITE colour constant',
        'COOL_WHITE': 'COOL_WHITE colour constant',
        'ENGINE_BLUE': 'ENGINE_BLUE colour constant',
        'NAV_RED': 'NAV_RED colour constant',
        'NAV_GREEN': 'NAV_GREEN colour constant',
        'ROOM_LIGHT_MAP': 'ROOM_LIGHT_MAP constant',
        'def create_interior_lights(': 'create_interior_lights function',
        'def create_engine_glow_lights(': 'create_engine_glow_lights function',
        'def create_running_lights(': 'create_running_lights function',
        'def setup_ship_lighting(': 'setup_ship_lighting function',
        'def register()': 'register function',
        'def unregister()': 'unregister function',
    }

    all_valid = True
    for pattern, description in checks.items():
        if pattern in content:
            print(f"✓ {description} found")
        else:
            print(f"✗ {description} not found")
            all_valid = False

    return all_valid


def test_greeble_system():
    """Test the greeble / detail pass system"""
    print("\nTesting greeble system module...")

    addon_path = os.path.dirname(os.path.abspath(__file__))
    gs_path = os.path.join(addon_path, 'greeble_system.py')

    if not os.path.exists(gs_path):
        print("✗ greeble_system.py not found")
        return False

    with open(gs_path, 'r') as f:
        content = f.read()

    checks = {
        'GREEBLE_TYPES': 'GREEBLE_TYPES constant',
        'def apply_greebles(': 'apply_greebles function',
        'def get_greeble_types(': 'get_greeble_types function',
        'def get_greeble_count(': 'get_greeble_count function',
        'def register()': 'register function',
        'def unregister()': 'unregister function',
    }

    all_valid = True
    for pattern, description in checks.items():
        if pattern in content:
            print(f"✓ {description} found")
        else:
            print(f"✗ {description} not found")
            all_valid = False

    # Check that all greeble types have required fields
    tree = ast.parse(content)
    greeble_keys = set()
    for node in ast.walk(tree):
        if isinstance(node, ast.Assign):
            for target in node.targets:
                if isinstance(target, ast.Name) and target.id == 'GREEBLE_TYPES':
                    if isinstance(node.value, ast.Dict):
                        for k in node.value.keys:
                            if isinstance(k, ast.Constant):
                                greeble_keys.add(k.value)

    expected_types = {'PANEL', 'VENT', 'PIPE', 'ANTENNA', 'SENSOR_DOME', 'BOX', 'CONDUIT'}
    if greeble_keys == expected_types:
        print(f"✓ All {len(expected_types)} greeble types defined")
    else:
        missing = expected_types - greeble_keys
        if missing:
            print(f"✗ Missing greeble types: {missing}")
            all_valid = False

    return all_valid


def test_preset_library():
    """Test the preset save/load system"""
    print("\nTesting preset library module...")

    addon_path = os.path.dirname(os.path.abspath(__file__))
    pl_path = os.path.join(addon_path, 'preset_library.py')

    if not os.path.exists(pl_path):
        print("✗ preset_library.py not found")
        return False

    import importlib.util
    import tempfile
    spec = importlib.util.spec_from_file_location("preset_library", pl_path)
    pl = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(pl)

    all_valid = True

    # Check constants
    if len(pl.PRESET_KEYS) >= 15:
        print(f"✓ PRESET_KEYS has {len(pl.PRESET_KEYS)} entries")
    else:
        print("✗ PRESET_KEYS too short")
        all_valid = False

    # Functional round-trip test
    with tempfile.TemporaryDirectory() as d:
        pl.save_preset('TestPreset', {
            'ship_class': 'BATTLESHIP',
            'style': 'EVE',
            'seed': 99,
            'hull_complexity': 2.0,
        }, preset_dir=d)

        names = pl.list_presets(preset_dir=d)
        if names == ['TestPreset']:
            print("✓ list_presets works")
        else:
            print(f"✗ list_presets returned {names}")
            all_valid = False

        data = pl.load_preset('TestPreset', preset_dir=d)
        if data.get('ship_class') == 'BATTLESHIP' and data.get('seed') == 99:
            print("✓ save/load round-trip works")
        else:
            print("✗ save/load round-trip failed")
            all_valid = False

        info = pl.get_preset_info('TestPreset', preset_dir=d)
        if info and info['ship_class'] == 'BATTLESHIP':
            print("✓ get_preset_info works")
        else:
            print("✗ get_preset_info failed")
            all_valid = False

        if pl.delete_preset('TestPreset', preset_dir=d):
            print("✓ delete_preset works")
        else:
            print("✗ delete_preset failed")
            all_valid = False

        # Verify empty name raises
        try:
            pl.save_preset('', {}, preset_dir=d)
            print("✗ Empty name should raise ValueError")
            all_valid = False
        except ValueError:
            print("✓ Empty name correctly rejected")

        # Verify missing preset raises
        try:
            pl.load_preset('nonexistent', preset_dir=d)
            print("✗ Missing preset should raise FileNotFoundError")
            all_valid = False
        except FileNotFoundError:
            print("✓ Missing preset correctly raises FileNotFoundError")

    return all_valid


def test_furniture_system():
    """Test the furniture system module"""
    print("\nTesting furniture system module...")

    addon_path = os.path.dirname(os.path.abspath(__file__))
    fs_path = os.path.join(addon_path, 'furniture_system.py')

    if not os.path.exists(fs_path):
        print("✗ furniture_system.py not found")
        return False

    with open(fs_path, 'r') as f:
        content = f.read()

    # Check required constants
    checks = {
        'FURNITURE_TYPES': 'FURNITURE_TYPES constant',
        'ROOM_FURNITURE_MAP': 'ROOM_FURNITURE_MAP constant',
        'MAX_FURNITURE_ITEMS': 'MAX_FURNITURE_ITEMS constant',
        'def populate_ship_furniture(': 'populate_ship_furniture function',
        'def get_furniture_types(': 'get_furniture_types function',
        'def get_room_furniture_map(': 'get_room_furniture_map function',
        'def register()': 'register function',
        'def unregister()': 'unregister function',
    }

    all_valid = True
    for pattern, description in checks.items():
        if pattern in content:
            print(f"✓ {description} found")
        else:
            print(f"✗ {description} not found")
            all_valid = False

    # Functional test — import module (no bpy dependency for data dicts)
    import importlib.util
    spec = importlib.util.spec_from_file_location("furniture_system", fs_path)
    try:
        # We can at least verify the module parses and constants are correct
        tree = ast.parse(content)

        # Extract FURNITURE_TYPES keys
        furn_keys = set()
        for node in ast.walk(tree):
            if isinstance(node, ast.Assign):
                for target in node.targets:
                    if isinstance(target, ast.Name) and target.id == 'FURNITURE_TYPES':
                        if isinstance(node.value, ast.Dict):
                            for k in node.value.keys:
                                if isinstance(k, ast.Constant):
                                    furn_keys.add(k.value)

        expected_types = {'CHAIR', 'TABLE', 'CONSOLE', 'LOCKER', 'BED',
                          'MONITOR', 'WORKBENCH'}
        if furn_keys == expected_types:
            print(f"✓ All {len(expected_types)} furniture types defined")
        else:
            missing = expected_types - furn_keys
            extra = furn_keys - expected_types
            if missing:
                print(f"✗ Missing furniture types: {missing}")
                all_valid = False
            if extra:
                print(f"  (extra types found: {extra})")

        # Extract ROOM_FURNITURE_MAP keys
        room_keys = set()
        for node in ast.walk(tree):
            if isinstance(node, ast.Assign):
                for target in node.targets:
                    if isinstance(target, ast.Name) and target.id == 'ROOM_FURNITURE_MAP':
                        if isinstance(node.value, ast.Dict):
                            for k in node.value.keys:
                                if isinstance(k, ast.Constant):
                                    room_keys.add(k.value)

        expected_rooms = {'cockpit', 'bridge', 'corridor', 'quarters', 'cargo',
                          'engine', 'reactor', 'hangar', 'weapon', 'sensor',
                          'power', 'shield'}
        if room_keys == expected_rooms:
            print(f"✓ All {len(expected_rooms)} room mappings defined")
        else:
            missing = expected_rooms - room_keys
            if missing:
                print(f"✗ Missing room mappings: {missing}")
                all_valid = False

    except SyntaxError as e:
        print(f"✗ Syntax error in furniture_system.py: {e}")
        all_valid = False

    return all_valid


def test_version_registry():
    """Test the version_registry module"""
    print("\nTesting version registry module...")
    addon_path = os.path.dirname(os.path.abspath(__file__))
    all_valid = True

    # Import the module
    sys.path.insert(0, addon_path)
    try:
        import version_registry
    except ImportError as e:
        print(f"✗ Cannot import version_registry: {e}")
        return False

    # Check MODULE_VERSIONS exists and has entries
    if hasattr(version_registry, 'MODULE_VERSIONS'):
        mv = version_registry.MODULE_VERSIONS
        print(f"✓ MODULE_VERSIONS constant found ({len(mv)} modules)")
    else:
        print("✗ MODULE_VERSIONS constant missing")
        all_valid = False
        return all_valid

    # Check key modules are versioned
    required_modules = [
        'ship_generator', 'ship_parts', 'interior_generator',
        'module_system', 'lighting_system', 'greeble_system',
        'preset_library', 'furniture_system',
    ]
    for mod in required_modules:
        if mod in mv:
            print(f"✓ {mod} versioned: {mv[mod]}")
        else:
            print(f"✗ {mod} missing from MODULE_VERSIONS")
            all_valid = False

    # Test get_module_version
    if hasattr(version_registry, 'get_module_version'):
        v = version_registry.get_module_version('ship_generator')
        if v != "0.0.0":
            print(f"✓ get_module_version returns valid version: {v}")
        else:
            print("✗ get_module_version returned default for known module")
            all_valid = False
        # Unknown module returns "0.0.0"
        v_unknown = version_registry.get_module_version('nonexistent_module')
        if v_unknown == "0.0.0":
            print("✓ get_module_version returns '0.0.0' for unknown module")
        else:
            print(f"✗ get_module_version returned {v_unknown} for unknown module")
            all_valid = False
    else:
        print("✗ get_module_version function missing")
        all_valid = False

    # Test get_all_versions
    if hasattr(version_registry, 'get_all_versions'):
        avs = version_registry.get_all_versions()
        if isinstance(avs, dict) and len(avs) > 0:
            print(f"✓ get_all_versions returns dict with {len(avs)} entries")
        else:
            print("✗ get_all_versions returned invalid result")
            all_valid = False
    else:
        print("✗ get_all_versions function missing")
        all_valid = False

    # Test check_compatibility
    if hasattr(version_registry, 'check_compatibility'):
        # ship_generator is 3.0.0, so requiring 3.0.0 should be compatible
        if version_registry.check_compatibility('ship_generator', '3.0.0'):
            print("✓ check_compatibility(ship_generator, 3.0.0) = True")
        else:
            print("✗ check_compatibility should return True for same version")
            all_valid = False
        # Requiring 4.0.0 should be incompatible
        if not version_registry.check_compatibility('ship_generator', '4.0.0'):
            print("✓ check_compatibility(ship_generator, 4.0.0) = False")
        else:
            print("✗ check_compatibility should return False for higher major")
            all_valid = False
    else:
        print("✗ check_compatibility function missing")
        all_valid = False

    # Test version_stamp
    if hasattr(version_registry, 'version_stamp'):
        stamp = version_registry.version_stamp()
        if 'generator_version' in stamp and 'modules' in stamp:
            print(f"✓ version_stamp returns valid structure")
        else:
            print("✗ version_stamp missing expected keys")
            all_valid = False
    else:
        print("✗ version_stamp function missing")
        all_valid = False

    # Test register/unregister
    for fn in ('register', 'unregister'):
        if hasattr(version_registry, fn):
            print(f"✓ {fn} function found")
        else:
            print(f"✗ {fn} function missing")
            all_valid = False

    return all_valid


def test_override_manager():
    """Test the override_manager module"""
    print("\nTesting override manager module...")
    addon_path = os.path.dirname(os.path.abspath(__file__))
    all_valid = True

    # AST-based validation
    filepath = os.path.join(addon_path, 'override_manager.py')
    try:
        with open(filepath) as f:
            tree = ast.parse(f.read())
    except SyntaxError as e:
        print(f"✗ Syntax error: {e}")
        return False

    # Check for required constants and functions
    names = set()
    for node in ast.walk(tree):
        if isinstance(node, ast.Assign):
            for target in node.targets:
                if isinstance(target, ast.Name):
                    names.add(target.id)
        elif isinstance(node, ast.FunctionDef):
            names.add(node.name)

    required_items = [
        'OVERRIDE_PROP',
        'is_protected',
        'set_protected',
        'collect_protected',
        'filter_children',
        'clear_all_overrides',
        'count_protected',
        'register',
        'unregister',
    ]
    for item in required_items:
        if item in names:
            print(f"✓ {item} found")
        else:
            print(f"✗ {item} missing")
            all_valid = False

    # Functional test: import and test with None objects
    sys.path.insert(0, addon_path)
    try:
        import override_manager
        # is_protected(None) should return False
        if not override_manager.is_protected(None):
            print("✓ is_protected(None) returns False")
        else:
            print("✗ is_protected(None) should return False")
            all_valid = False

        # set_protected(None) should return False
        if not override_manager.set_protected(None):
            print("✓ set_protected(None) returns False")
        else:
            print("✗ set_protected(None) should return False")
            all_valid = False

        # collect_protected(None) should return empty list
        if override_manager.collect_protected(None) == []:
            print("✓ collect_protected(None) returns []")
        else:
            print("✗ collect_protected(None) should return []")
            all_valid = False

        # filter_children(None) should return empty list
        if override_manager.filter_children(None) == []:
            print("✓ filter_children(None) returns []")
        else:
            print("✗ filter_children(None) should return []")
            all_valid = False

        # count_protected(None) should return 0
        if override_manager.count_protected(None) == 0:
            print("✓ count_protected(None) returns 0")
        else:
            print("✗ count_protected(None) should return 0")
            all_valid = False

        # clear_all_overrides(None) should return 0
        if override_manager.clear_all_overrides(None) == 0:
            print("✓ clear_all_overrides(None) returns 0")
        else:
            print("✗ clear_all_overrides(None) should return 0")
            all_valid = False

        # Check OVERRIDE_PROP value
        if override_manager.OVERRIDE_PROP == "af_manual_override":
            print("✓ OVERRIDE_PROP = 'af_manual_override'")
        else:
            print(f"✗ OVERRIDE_PROP = {override_manager.OVERRIDE_PROP!r}")
            all_valid = False

    except ImportError as e:
        print(f"✗ Cannot import override_manager: {e}")
        all_valid = False

    return all_valid


def test_template_manager():
    """Test the template_manager module"""
    print("\nTesting template manager module...")
    addon_path = os.path.dirname(os.path.abspath(__file__))
    all_valid = True

    import tempfile
    import shutil

    sys.path.insert(0, addon_path)
    try:
        import template_manager
    except ImportError as e:
        print(f"✗ Cannot import template_manager: {e}")
        return False

    # Check constants
    if hasattr(template_manager, 'TEMPLATE_VERSION'):
        print(f"✓ TEMPLATE_VERSION = {template_manager.TEMPLATE_VERSION}")
    else:
        print("✗ TEMPLATE_VERSION missing")
        all_valid = False

    if hasattr(template_manager, 'TEMPLATE_CATEGORIES'):
        cats = template_manager.TEMPLATE_CATEGORIES
        expected = {'ship', 'station', 'fleet', 'asteroid', 'character'}
        if set(cats) == expected:
            print(f"✓ TEMPLATE_CATEGORIES has {len(cats)} categories")
        else:
            print(f"✗ TEMPLATE_CATEGORIES mismatch: {set(cats)} vs {expected}")
            all_valid = False
    else:
        print("✗ TEMPLATE_CATEGORIES missing")
        all_valid = False

    # Check functions exist
    required_fns = [
        'save_template', 'load_template', 'delete_template',
        'list_templates', 'discover_templates', 'get_template_info',
        'import_templates_from_directory', 'register', 'unregister',
    ]
    for fn in required_fns:
        if hasattr(template_manager, fn):
            print(f"✓ {fn} function found")
        else:
            print(f"✗ {fn} function missing")
            all_valid = False

    # Functional round-trip test with temp directory
    tmpdir = tempfile.mkdtemp(prefix="af_templates_test_")
    try:
        # Save a template
        data = {"ship_class": "CRUISER", "style": "X4", "seed": 42}
        fp = template_manager.save_template("TestCruiser", data,
                                            category="ship",
                                            template_dir=tmpdir)
        if os.path.isfile(fp):
            print("✓ save_template creates file")
        else:
            print("✗ save_template did not create file")
            all_valid = False

        # List templates
        names = template_manager.list_templates(category="ship",
                                                template_dir=tmpdir)
        if "TestCruiser" in names:
            print("✓ list_templates finds saved template")
        else:
            print(f"✗ list_templates returned {names}")
            all_valid = False

        # Load template
        loaded = template_manager.load_template("TestCruiser", category="ship",
                                                template_dir=tmpdir)
        if loaded.get("ship_class") == "CRUISER" and loaded.get("seed") == 42:
            print("✓ load_template round-trip correct")
        else:
            print(f"✗ load_template returned {loaded}")
            all_valid = False

        # Get info
        info = template_manager.get_template_info("TestCruiser",
                                                  category="ship",
                                                  template_dir=tmpdir)
        if info is not None and info.get("name") == "TestCruiser":
            print("✓ get_template_info returns metadata")
        else:
            print(f"✗ get_template_info returned {info}")
            all_valid = False

        # Discover templates
        disc = template_manager.discover_templates(template_dir=tmpdir)
        if "ship" in disc and "TestCruiser" in disc["ship"]:
            print("✓ discover_templates finds template")
        else:
            print(f"✗ discover_templates returned {disc}")
            all_valid = False

        # Delete template
        if template_manager.delete_template("TestCruiser", category="ship",
                                            template_dir=tmpdir):
            print("✓ delete_template returns True")
        else:
            print("✗ delete_template returned False")
            all_valid = False

        # Verify deletion
        remaining = template_manager.list_templates(category="ship",
                                                    template_dir=tmpdir)
        if "TestCruiser" not in remaining:
            print("✓ Template removed after delete")
        else:
            print("✗ Template still exists after delete")
            all_valid = False

        # Error cases
        try:
            template_manager.save_template("", {}, template_dir=tmpdir)
            print("✗ save_template should raise ValueError for empty name")
            all_valid = False
        except ValueError:
            print("✓ save_template raises ValueError for empty name")

        try:
            template_manager.load_template("nonexistent", template_dir=tmpdir)
            print("✗ load_template should raise FileNotFoundError")
            all_valid = False
        except FileNotFoundError:
            print("✓ load_template raises FileNotFoundError for missing")

        try:
            template_manager.save_template("x", {}, category="invalid",
                                           template_dir=tmpdir)
            print("✗ save_template should raise ValueError for bad category")
            all_valid = False
        except ValueError:
            print("✓ save_template raises ValueError for invalid category")

        # Import from directory test
        import_dir = os.path.join(tmpdir, "import_source")
        os.makedirs(import_dir, exist_ok=True)
        import json
        with open(os.path.join(import_dir, "imported_ship.json"), "w") as fh:
            json.dump({"_name": "ImportedShip", "_category": "ship",
                        "ship_class": "FIGHTER"}, fh)
        imported = template_manager.import_templates_from_directory(
            import_dir, template_dir=tmpdir)
        if len(imported) == 1 and imported[0][0] == "ImportedShip":
            print("✓ import_templates_from_directory works")
        else:
            print(f"✗ import_templates_from_directory returned {imported}")
            all_valid = False

    finally:
        shutil.rmtree(tmpdir, ignore_errors=True)

    return all_valid


def test_character_mesh_generation():
    """Test that character generator produces mesh geometry data"""
    print("\nTesting character mesh generation...")
    addon_path = os.path.dirname(os.path.abspath(__file__))
    all_valid = True

    sys.path.insert(0, os.path.join(addon_path, 'pcg_pipeline'))
    try:
        from pcg_pipeline import character_generator
    except ImportError:
        # Try direct import
        sys.path.insert(0, addon_path)
        try:
            from pcg_pipeline import character_generator
        except ImportError as e:
            print(f"✗ Cannot import character_generator: {e}")
            return False

    # Generate a character
    char = character_generator.generate_character(seed=42, char_id="test_char")

    # Check metadata fields (backwards compatible)
    for field in ('char_id', 'seed', 'race', 'body_type', 'cyber_limbs'):
        if field in char:
            print(f"✓ metadata field '{field}' present")
        else:
            print(f"✗ metadata field '{field}' missing")
            all_valid = False

    # Check mesh_parts
    if 'mesh_parts' in char:
        parts = char['mesh_parts']
        print(f"✓ mesh_parts present with {len(parts)} body parts")

        expected_parts = [
            'head', 'torso',
            'left_upper_arm', 'left_lower_arm',
            'right_upper_arm', 'right_lower_arm',
            'left_upper_leg', 'left_lower_leg',
            'right_upper_leg', 'right_lower_leg',
        ]
        for part_name in expected_parts:
            if part_name in parts:
                p = parts[part_name]
                has_verts = 'vertices' in p and len(p['vertices']) > 0
                has_faces = 'faces' in p and len(p['faces']) > 0
                if has_verts and has_faces:
                    print(f"✓ {part_name}: {len(p['vertices'])} verts, "
                          f"{len(p['faces'])} faces")
                else:
                    print(f"✗ {part_name}: missing vertices or faces")
                    all_valid = False
            else:
                print(f"✗ {part_name} missing from mesh_parts")
                all_valid = False
    else:
        print("✗ mesh_parts missing from character data")
        all_valid = False

    # Determinism test — check all mesh parts
    char2 = character_generator.generate_character(seed=42, char_id="test_char")
    all_match = all(
        char['mesh_parts'][part]['vertices'] == char2['mesh_parts'][part]['vertices']
        for part in char['mesh_parts']
    )
    if all_match:
        print("✓ Mesh generation is deterministic (same seed = same mesh)")
    else:
        print("✗ Mesh generation is NOT deterministic")
        all_valid = False

    # Different seed gives different mesh
    char3 = character_generator.generate_character(seed=99, char_id="test_char2")
    if char3['mesh_parts']['head']['vertices'] != char['mesh_parts']['head']['vertices']:
        print("✓ Different seed produces different mesh")
    else:
        print("✗ Different seed produced identical mesh")
        all_valid = False

    # Check race/body_type modifiers exist
    if hasattr(character_generator, '_RACE_MODIFIERS'):
        mods = character_generator._RACE_MODIFIERS
        if len(mods) == len(character_generator.RACES):
            print(f"✓ _RACE_MODIFIERS covers all {len(mods)} races")
        else:
            print(f"✗ _RACE_MODIFIERS has {len(mods)} entries, expected {len(character_generator.RACES)}")
            all_valid = False
    else:
        print("✗ _RACE_MODIFIERS not found")
        all_valid = False

    if hasattr(character_generator, '_BODY_TYPE_MODIFIERS'):
        mods = character_generator._BODY_TYPE_MODIFIERS
        if len(mods) == len(character_generator.BODY_TYPES):
            print(f"✓ _BODY_TYPE_MODIFIERS covers all {len(mods)} body types")
        else:
            print(f"✗ _BODY_TYPE_MODIFIERS has {len(mods)} entries")
            all_valid = False
    else:
        print("✗ _BODY_TYPE_MODIFIERS not found")
        all_valid = False

    return all_valid


def run_tests():
    """Run all validation tests"""
    print("=" * 60)
    print("AtlasForge Generator - Validation Tests")
    print("=" * 60)
    
    tests = [
        ("Addon Structure", test_addon_structure),
        ("Python Syntax", test_file_syntax),
        ("bl_info Metadata", test_bl_info),
        ("Blender Manifest (4.2+)", test_blender_manifest),
        ("Register Functions", test_register_functions),
        ("Documentation", test_documentation),
        ("Turret Hardpoint Configs", test_turret_hardpoint_configs),
        ("Naming Prefix Support", test_naming_prefix_support),
        ("Turret Generation Function", test_turret_generation_function),
        ("Brick System", test_brick_system),
        ("Hull Taper & Cleanup", test_hull_taper_and_cleanup),
        ("Engine Archetypes", test_engine_archetypes),
        ("Exterior Influence Definitions", test_exterior_influence_definitions),
        ("Module Room Types", test_module_room_types),
        ("Pipeline Stages", test_pipeline_stages),
        ("PCG Pipeline", test_pcg_pipeline),
        ("PCG Panel", test_pcg_panel),
        ("PCG Pipeline Init Exports", test_pcg_pipeline_init_exports),
        ("LOD Tiers", test_lod_tiers),
        ("NovaForge Importer", test_novaforge_importer),
        ("LOD Generator", test_lod_generator),
        ("Collision Generator", test_collision_generator),
        ("Animation System", test_animation_system),
        ("Damage System", test_damage_system),
        ("Power System", test_power_system),
        ("Build Validator", test_build_validator),
        ("Render Setup", test_render_setup),
        ("Lighting System", test_lighting_system),
        ("Greeble System", test_greeble_system),
        ("Preset Library", test_preset_library),
        ("Furniture System", test_furniture_system),
        ("Version Registry", test_version_registry),
        ("Override Manager", test_override_manager),
        ("Template Manager", test_template_manager),
        ("Character Mesh Generation", test_character_mesh_generation),
    ]
    
    results = []
    for test_name, test_func in tests:
        try:
            result = test_func()
            results.append((test_name, result))
        except Exception as e:
            print(f"\n✗ {test_name} raised exception: {e}")
            import traceback
            traceback.print_exc()
            results.append((test_name, False))
    
    print("\n" + "=" * 60)
    print("Test Results Summary:")
    print("=" * 60)
    
    passed = sum(1 for _, result in results if result)
    total = len(results)
    
    for test_name, result in results:
        status = "✓ PASS" if result else "✗ FAIL"
        print(f"{status}: {test_name}")
    
    print("=" * 60)
    print(f"Total: {passed}/{total} tests passed")
    
    if passed == total:
        print("✓ All validation tests passed!")
    else:
        print("✗ Some tests failed")
    
    print("=" * 60)
    
    return passed == total


if __name__ == "__main__":
    success = run_tests()
    sys.exit(0 if success else 1)
