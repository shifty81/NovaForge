"""
Validation tests for the NovaForge PCG Pipeline.

These tests run without Blender and verify the deterministic generation
logic, data structures, and cross-module integration.

Usage:
    python tools/BlenderSpaceshipGenerator/pcg_pipeline/test_pcg_pipeline.py
"""

import json
import os
import sys
import tempfile

# Ensure the parent package is importable when running as a script
_here = os.path.dirname(os.path.abspath(__file__))
_addon_root = os.path.dirname(_here)
if _addon_root not in sys.path:
    sys.path.insert(0, _addon_root)

from pcg_pipeline import galaxy_generator
from pcg_pipeline import system_generator
from pcg_pipeline import planet_generator
from pcg_pipeline import terrain_generator
from pcg_pipeline import station_generator
from pcg_pipeline import ship_generator
from pcg_pipeline import character_generator
from pcg_pipeline import batch_generate


# ── helpers ───────────────────────────────────────────────────────────

def _assert(condition, msg):
    if not condition:
        raise AssertionError(msg)


# ── individual tests ──────────────────────────────────────────────────

def test_galaxy_determinism():
    """Same seed must produce identical galaxies."""
    print("Testing galaxy determinism...")
    g1 = galaxy_generator.generate_galaxy(42, num_systems=3)
    g2 = galaxy_generator.generate_galaxy(42, num_systems=3)
    _assert(json.dumps(g1, sort_keys=True) == json.dumps(g2, sort_keys=True),
            "Galaxy output differs for the same seed")
    _assert(len(g1["systems"]) == 3, "Expected 3 systems")
    print("✓ Galaxy determinism verified")
    return True


def test_system_structure():
    """System must contain required top-level keys."""
    print("\nTesting system structure...")
    system = system_generator.generate_system(12345, "test_sys")
    required = {"system_id", "seed", "stars", "planets", "terrains",
                "stations", "ships", "characters"}
    missing = required - set(system.keys())
    _assert(not missing, f"System missing keys: {missing}")
    _assert(len(system["stars"]) >= 1, "System must have at least one star")
    _assert(len(system["planets"]) >= 1, "System must have at least one planet")
    print(f"✓ System has {len(system['planets'])} planets, "
          f"{len(system['stations'])} stations, {len(system['ships'])} ships")
    return True


def test_star_types():
    """All generated stars must use valid spectral classes."""
    print("\nTesting star type validity...")
    valid_types = set(system_generator.STAR_TYPES.keys())
    for seed in range(100):
        system = system_generator.generate_system(seed, f"sys_{seed}")
        for star in system["stars"]:
            _assert(star["type"] in valid_types,
                    f"Invalid star type: {star['type']}")
    print(f"✓ All star types valid across 100 seeds")
    return True


def test_planet_biomes():
    """Planets must have valid biomes and atmosphere data."""
    print("\nTesting planet generation...")
    valid_biomes = {"volcanic", "desert", "temperate", "forest", "tundra", "ice"}
    for seed in range(50):
        p = planet_generator.generate_planet(seed, f"planet_{seed}",
                                             star_type="G", orbit_index=2,
                                             total_planets=6)
        _assert(p["biome"] in valid_biomes, f"Invalid biome: {p['biome']}")
        _assert(p["radius_earth"] > 0, "Radius must be positive")
        _assert(p["orbit_au"] > 0, "Orbit must be positive")
        _assert(isinstance(p["atmosphere"], dict), "Atmosphere must be a dict")
    print("✓ Planet biomes and parameters valid across 50 seeds")
    return True


def test_planet_foliage_and_liquids():
    """Temperate/forest terrestrial planets should have foliage and liquids."""
    print("\nTesting planet foliage and liquids...")
    found_foliage = False
    found_liquid = False
    for seed in range(200):
        p = planet_generator.generate_planet(seed, f"planet_{seed}",
                                             star_type="G", orbit_index=1,
                                             total_planets=5)
        if p["foliage"]:
            found_foliage = True
            for entry in p["foliage"]:
                _assert("type" in entry and "density" in entry,
                        "Foliage entry must have type and density")
        if p["liquids"]:
            found_liquid = True
            _assert("sea_level" in p["liquids"],
                    "Liquids must have sea_level")
    _assert(found_foliage, "No foliage generated across 200 seeds")
    _assert(found_liquid, "No liquids generated across 200 seeds")
    print("✓ Foliage and liquid generation verified")
    return True


def test_station_generation():
    """Station must have required fields and valid type."""
    print("\nTesting station generation...")
    valid_types = set(station_generator.STATION_TYPES.keys())
    for seed in range(50):
        s = station_generator.generate_station(seed, "planet_0", 0)
        _assert(s["type"] in valid_types, f"Invalid station type: {s['type']}")
        _assert(len(s["modules"]) >= 3, "Station must have at least base modules")
        _assert(s["faction"] in station_generator.FACTIONS,
                f"Invalid faction: {s['faction']}")
    print("✓ Station generation valid across 50 seeds")
    return True


def test_ship_generation():
    """Ship metadata must have required fields."""
    print("\nTesting ship generation...")
    for seed in range(50):
        s = ship_generator.generate_ship_metadata(seed, f"ship_{seed}")
        _assert(s["class"] in ship_generator.SHIP_CLASSES,
                f"Invalid class: {s['class']}")
        _assert(s["faction"] in ship_generator.FACTIONS,
                f"Invalid faction: {s['faction']}")
        _assert(0 <= s["hardpoints"] <= 10, "Hardpoints out of range")
        _assert(len(s["modules"]) >= 2, "Ship must have at least 2 modules")
    print("✓ Ship generation valid across 50 seeds")
    return True


def test_character_generation():
    """Character metadata must have valid race/body/limbs."""
    print("\nTesting character generation...")
    for seed in range(50):
        c = character_generator.generate_character(seed, f"char_{seed}")
        _assert(c["race"] in character_generator.RACES,
                f"Invalid race: {c['race']}")
        _assert(c["body_type"] in character_generator.BODY_TYPES,
                f"Invalid body type: {c['body_type']}")
        for limb in c["cyber_limbs"]:
            _assert(limb in character_generator.CYBER_LIMB_SLOTS,
                    f"Invalid limb slot: {limb}")
    print("✓ Character generation valid across 50 seeds")
    return True


def test_terrain_determinism():
    """Same seed must produce identical terrain."""
    print("\nTesting terrain determinism...")
    t1 = terrain_generator.generate_terrain(42, "planet_test", biome="temperate",
                                            grid_size=16)
    t2 = terrain_generator.generate_terrain(42, "planet_test", biome="temperate",
                                            grid_size=16)
    _assert(json.dumps(t1, sort_keys=True) == json.dumps(t2, sort_keys=True),
            "Terrain output differs for the same seed")
    _assert(len(t1["heightmap"]) == 16, "Expected 16-row heightmap")
    _assert(len(t1["heightmap"][0]) == 16, "Expected 16-column heightmap")
    print("✓ Terrain determinism verified")
    return True


def test_terrain_biome_features():
    """Each biome must produce features from its preset list."""
    print("\nTesting terrain biome features...")
    for biome, preset in terrain_generator.BIOME_PRESETS.items():
        t = terrain_generator.generate_terrain(99, f"planet_{biome}",
                                               biome=biome, grid_size=16,
                                               temperature_k=300.0)
        _assert(t["biome"] == biome, f"Biome mismatch: {t['biome']}")
        _assert(t["grid_size"] == 16, "Grid size mismatch")
        _assert(len(t["features"]) >= 1, f"No features for biome {biome}")
        for feat in t["features"]:
            _assert(feat["type"] in preset["features"],
                    f"Feature '{feat['type']}' not valid for {biome}")
        _assert(len(t["resources"]) >= 1, f"No resources for biome {biome}")
        for res in t["resources"]:
            _assert(res["type"] in preset["resource_types"],
                    f"Resource '{res['type']}' not valid for {biome}")
    print(f"✓ All {len(terrain_generator.BIOME_PRESETS)} biomes produce "
          f"valid features and resources")
    return True


def test_terrain_heightmap_range():
    """Heightmap values must be clamped to [0, 1]."""
    print("\nTesting terrain heightmap range...")
    for seed in range(100):
        t = terrain_generator.generate_terrain(seed, f"planet_{seed}",
                                               biome="volcanic", grid_size=8)
        for row in t["heightmap"]:
            for h in row:
                _assert(0.0 <= h <= 1.0,
                        f"Height {h} out of [0,1] range (seed={seed})")
    print("✓ Heightmap values in [0,1] across 100 seeds")
    return True


def test_terrain_grid_clamp():
    """Grid size must be clamped to [4, 128]."""
    print("\nTesting terrain grid size clamping...")
    t_small = terrain_generator.generate_terrain(1, "p", grid_size=1)
    _assert(t_small["grid_size"] == 4, "Grid not clamped to minimum 4")
    _assert(len(t_small["heightmap"]) == 4, "Heightmap rows mismatch")

    t_large = terrain_generator.generate_terrain(1, "p", grid_size=999)
    _assert(t_large["grid_size"] == 128, "Grid not clamped to maximum 128")
    _assert(len(t_large["heightmap"]) == 128, "Heightmap rows mismatch")
    print("✓ Grid size clamping works correctly")
    return True


def test_terrain_stats():
    """Terrain stats must be present and consistent with heightmap."""
    print("\nTesting terrain stats...")
    t = terrain_generator.generate_terrain(42, "p", biome="forest",
                                           grid_size=16)
    stats = t["stats"]
    _assert("min_height" in stats, "Missing min_height")
    _assert("max_height" in stats, "Missing max_height")
    _assert("avg_height" in stats, "Missing avg_height")
    _assert("num_features" in stats, "Missing num_features")
    _assert("num_resources" in stats, "Missing num_resources")
    _assert(stats["min_height"] <= stats["avg_height"] <= stats["max_height"],
            "Stats ordering violated: min <= avg <= max")
    _assert(stats["num_features"] == len(t["features"]),
            "Feature count mismatch")
    _assert(stats["num_resources"] == len(t["resources"]),
            "Resource count mismatch")
    print("✓ Terrain stats are consistent")
    return True


def test_batch_generate():
    """Batch generate must produce all expected output files."""
    print("\nTesting batch generation...")
    with tempfile.TemporaryDirectory() as tmpdir:
        galaxy = batch_generate.generate_universe(
            seed=42, num_systems=2, output_dir=tmpdir)

        # Galaxy manifest
        _assert(os.path.isfile(os.path.join(tmpdir, "galaxy.json")),
                "galaxy.json not created")

        # Per-system files
        for system in galaxy["systems"]:
            sid = system["system_id"]
            _assert(os.path.isfile(
                os.path.join(tmpdir, "systems", f"{sid}.json")),
                f"System file for {sid} not created")

            for planet in system["planets"]:
                _assert(os.path.isfile(
                    os.path.join(tmpdir, "planets",
                                 f"{planet['planet_id']}.json")),
                    f"Planet file for {planet['planet_id']} not created")

            for terrain in system.get("terrains", []):
                _assert(os.path.isfile(
                    os.path.join(tmpdir, "terrains",
                                 f"{terrain['planet_id']}_terrain.json")),
                    f"Terrain file for {terrain['planet_id']} not created")

            for ship in system["ships"]:
                _assert(os.path.isfile(
                    os.path.join(tmpdir, "ships",
                                 f"{ship['ship_id']}.json")),
                    f"Ship file for {ship['ship_id']} not created")

        # Validate JSON round-trip
        with open(os.path.join(tmpdir, "galaxy.json")) as fh:
            loaded = json.load(fh)
        _assert(loaded["universe_seed"] == 42, "Seed mismatch in output")
        _assert(len(loaded["systems"]) == 2, "System count mismatch")

    print("✓ Batch generation produced all expected files")
    return True


def test_batch_determinism():
    """Two batch runs with the same seed must produce identical output."""
    print("\nTesting batch determinism...")
    with tempfile.TemporaryDirectory() as d1, \
         tempfile.TemporaryDirectory() as d2:
        g1 = batch_generate.generate_universe(seed=99, num_systems=3,
                                              output_dir=d1)
        g2 = batch_generate.generate_universe(seed=99, num_systems=3,
                                              output_dir=d2)
        _assert(json.dumps(g1, sort_keys=True) == json.dumps(g2, sort_keys=True),
                "Batch outputs differ for same seed")
    print("✓ Batch determinism verified")
    return True


def test_ship_lod_tiers():
    """Ship metadata must include LOD tier data."""
    print("\nTesting ship LOD tiers...")
    s = ship_generator.generate_ship_metadata(42, "lod_test")
    _assert("lod_tiers" in s, "Ship metadata missing lod_tiers")
    _assert(len(s["lod_tiers"]) >= 3, "Need at least 3 LOD tiers")
    for tier in s["lod_tiers"]:
        _assert("tier" in tier, "LOD tier missing 'tier' field")
        _assert("poly_fraction" in tier, "LOD tier missing 'poly_fraction'")
        _assert("max_distance" in tier, "LOD tier missing 'max_distance'")
        _assert(0 < tier["poly_fraction"] <= 1.0,
                f"poly_fraction out of range: {tier['poly_fraction']}")
    print(f"✓ Ship has {len(s['lod_tiers'])} LOD tiers with valid fields")
    return True


def test_pipeline_package_imports():
    """pcg_pipeline package should export all sub-modules."""
    print("\nTesting pipeline package imports...")
    import pcg_pipeline
    expected = [
        "galaxy_generator", "system_generator", "planet_generator",
        "terrain_generator", "station_generator", "ship_generator",
        "character_generator", "batch_generate",
    ]
    for name in expected:
        _assert(hasattr(pcg_pipeline, name),
                f"pcg_pipeline missing attribute: {name}")
    _assert(hasattr(pcg_pipeline, "__version__"),
            "pcg_pipeline missing __version__")
    print(f"✓ All {len(expected)} sub-modules accessible via pcg_pipeline")
    return True


# ── runner ────────────────────────────────────────────────────────────

def run_tests():
    """Execute all PCG pipeline tests."""
    print("=" * 60)
    print("NovaForge PCG Pipeline — Validation Tests")
    print("=" * 60)

    tests = [
        ("Galaxy Determinism", test_galaxy_determinism),
        ("System Structure", test_system_structure),
        ("Star Types", test_star_types),
        ("Planet Biomes", test_planet_biomes),
        ("Planet Foliage & Liquids", test_planet_foliage_and_liquids),
        ("Terrain Determinism", test_terrain_determinism),
        ("Terrain Biome Features", test_terrain_biome_features),
        ("Terrain Heightmap Range", test_terrain_heightmap_range),
        ("Terrain Grid Clamp", test_terrain_grid_clamp),
        ("Terrain Stats", test_terrain_stats),
        ("Station Generation", test_station_generation),
        ("Ship Generation", test_ship_generation),
        ("Character Generation", test_character_generation),
        ("Batch Generate", test_batch_generate),
        ("Batch Determinism", test_batch_determinism),
        ("Ship LOD Tiers", test_ship_lod_tiers),
        ("Pipeline Package Imports", test_pipeline_package_imports),
    ]

    results = []
    for name, func in tests:
        try:
            result = func()
            results.append((name, result))
        except Exception as exc:
            print(f"\n✗ {name} raised exception: {exc}")
            import traceback
            traceback.print_exc()
            results.append((name, False))

    print("\n" + "=" * 60)
    print("Test Results Summary:")
    print("=" * 60)

    passed = sum(1 for _, r in results if r)
    total = len(results)

    for name, result in results:
        status = "✓ PASS" if result else "✗ FAIL"
        print(f"{status}: {name}")

    print("=" * 60)
    print(f"Total: {passed}/{total} tests passed")

    if passed == total:
        print("✓ All PCG pipeline tests passed!")
    else:
        print("✗ Some tests failed")

    print("=" * 60)
    return passed == total


if __name__ == "__main__":
    success = run_tests()
    sys.exit(0 if success else 1)
