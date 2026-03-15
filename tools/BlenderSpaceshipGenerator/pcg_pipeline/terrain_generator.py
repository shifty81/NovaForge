"""
NovaForge PCG Pipeline — Terrain Generator

Generates deterministic terrain metadata for a planet, including a
heightmap grid, biome-appropriate feature placement, and resource
deposit locations.  The output is a lightweight JSON-serialisable dict
that game systems or Blender operators can consume to build the actual
mesh / texture.

Terrain parameters are derived from the parent planet's biome,
temperature, and atmosphere so that hot volcanic worlds differ visibly
from temperate forests or frozen tundra.
"""

import math
import random

# ── Biome-specific terrain presets ────────────────────────────────────

BIOME_PRESETS = {
    "volcanic": {
        "base_height": 0.4,
        "roughness": 0.85,
        "peak_probability": 0.12,
        "valley_depth": 0.3,
        "features": ["lava_flow", "caldera", "obsidian_field", "vent"],
        "resource_types": ["pyroxeres", "omber", "dark_ochre"],
    },
    "desert": {
        "base_height": 0.25,
        "roughness": 0.35,
        "peak_probability": 0.04,
        "valley_depth": 0.15,
        "features": ["dune_field", "mesa", "canyon", "dry_riverbed"],
        "resource_types": ["scordite", "veldspar", "golden_cytoserocin"],
    },
    "temperate": {
        "base_height": 0.3,
        "roughness": 0.45,
        "peak_probability": 0.06,
        "valley_depth": 0.2,
        "features": ["hill", "river", "lake", "forest_grove", "meadow"],
        "resource_types": ["veldspar", "plagioclase", "condensed_scordite"],
    },
    "forest": {
        "base_height": 0.35,
        "roughness": 0.5,
        "peak_probability": 0.07,
        "valley_depth": 0.25,
        "features": ["dense_canopy", "ravine", "waterfall", "clearing"],
        "resource_types": ["kernite", "jaspet", "hemorphite"],
    },
    "tundra": {
        "base_height": 0.2,
        "roughness": 0.3,
        "peak_probability": 0.03,
        "valley_depth": 0.1,
        "features": ["ice_sheet", "frozen_lake", "snow_drift", "permafrost_crack"],
        "resource_types": ["crokite", "bistot", "glacial_mass"],
    },
    "ice": {
        "base_height": 0.15,
        "roughness": 0.25,
        "peak_probability": 0.02,
        "valley_depth": 0.08,
        "features": ["glacier", "ice_spire", "crevasse", "frozen_geyser"],
        "resource_types": ["mercoxit", "arkonor", "clear_icicle"],
    },
}


# ── Heightmap generation ──────────────────────────────────────────────

def _noise_2d(rng, x, y, frequency=1.0):
    """Simple value-noise approximation using hashed random offsets.

    Not a true Perlin/Simplex implementation, but deterministic and
    sufficient for metadata-level terrain sketches.
    """
    # Hash the grid coordinates into a repeatable seed
    ix = int(math.floor(x * frequency))
    iy = int(math.floor(y * frequency))
    cell_seed = hash((ix, iy, rng.getrandbits(0))) % (2**31)
    cell_rng = random.Random(cell_seed ^ hash((ix, iy)))
    return cell_rng.random()


def _generate_heightmap(rng, grid_size, preset):
    """Generate a square heightmap grid.

    Returns a list-of-lists (row-major) with float values in [0, 1].
    """
    base = preset["base_height"]
    roughness = preset["roughness"]
    peak_prob = preset["peak_probability"]
    valley_depth = preset["valley_depth"]

    heightmap = []
    for row in range(grid_size):
        row_data = []
        for col in range(grid_size):
            # Base terrain with seeded noise
            nx = col / max(grid_size - 1, 1)
            ny = row / max(grid_size - 1, 1)

            h = base
            # Multi-octave noise (2 octaves)
            h += rng.uniform(-roughness, roughness) * 0.5
            h += rng.uniform(-roughness, roughness) * 0.25

            # Random peaks
            if rng.random() < peak_prob:
                h += rng.uniform(0.1, 0.4)

            # Random valleys
            if rng.random() < peak_prob * 0.5:
                h -= rng.uniform(0.05, valley_depth)

            # Clamp to [0, 1]
            h = max(0.0, min(1.0, h))
            row_data.append(round(h, 4))
        heightmap.append(row_data)

    return heightmap


# ── Feature placement ─────────────────────────────────────────────────

def _place_features(rng, grid_size, preset, num_features):
    """Place biome-appropriate terrain features on the grid.

    Returns a list of feature dicts with grid position and type.
    """
    available = preset["features"]
    features = []
    for _ in range(num_features):
        feature_type = rng.choice(available)
        gx = rng.randint(0, grid_size - 1)
        gy = rng.randint(0, grid_size - 1)
        radius = round(rng.uniform(1.0, max(grid_size * 0.15, 2.0)), 2)
        intensity = round(rng.uniform(0.3, 1.0), 2)
        features.append({
            "type": feature_type,
            "grid_x": gx,
            "grid_y": gy,
            "radius": radius,
            "intensity": intensity,
        })
    return features


# ── Resource deposits ─────────────────────────────────────────────────

def _place_resources(rng, grid_size, preset, num_deposits):
    """Place resource deposit markers on the terrain grid.

    Returns a list of resource dicts.
    """
    available = preset["resource_types"]
    deposits = []
    for _ in range(num_deposits):
        res_type = rng.choice(available)
        gx = rng.randint(0, grid_size - 1)
        gy = rng.randint(0, grid_size - 1)
        quantity = rng.randint(100, 10000)
        depth = round(rng.uniform(0.0, 1.0), 2)
        deposits.append({
            "type": res_type,
            "grid_x": gx,
            "grid_y": gy,
            "quantity": quantity,
            "depth": depth,
        })
    return deposits


# ── Public API ────────────────────────────────────────────────────────

def generate_terrain(seed, planet_id, biome="temperate", grid_size=32,
                     temperature_k=280.0):
    """Generate terrain metadata for a planet surface region.

    Args:
        seed: Deterministic seed for this terrain patch.
        planet_id: Parent planet identifier.
        biome: Planet biome string (must match a key in BIOME_PRESETS).
        grid_size: Edge length of the square heightmap grid (4–128).
        temperature_k: Equilibrium temperature; influences feature density.

    Returns:
        dict with ``heightmap``, ``features``, ``resources``, and
        summary statistics.
    """
    rng = random.Random(seed)

    # Clamp grid size to a sensible range
    grid_size = max(4, min(128, grid_size))

    # Fall back to temperate if biome is unknown
    preset = BIOME_PRESETS.get(biome, BIOME_PRESETS["temperate"])

    # Generate heightmap
    heightmap = _generate_heightmap(rng, grid_size, preset)

    # Feature count scales with grid area and temperature extremes
    area = grid_size * grid_size
    temp_factor = 1.0 + abs(temperature_k - 280.0) / 500.0
    num_features = max(2, int(area * 0.01 * temp_factor))
    features = _place_features(rng, grid_size, preset, num_features)

    # Resource deposits — fewer on extreme worlds
    num_deposits = max(1, int(area * 0.005))
    resources = _place_resources(rng, grid_size, preset, num_deposits)

    # Compute summary statistics
    flat = [h for row in heightmap for h in row]
    min_h = min(flat)
    max_h = max(flat)
    avg_h = sum(flat) / len(flat)

    return {
        "planet_id": planet_id,
        "seed": seed,
        "biome": biome,
        "grid_size": grid_size,
        "temperature_k": temperature_k,
        "heightmap": heightmap,
        "features": features,
        "resources": resources,
        "stats": {
            "min_height": round(min_h, 4),
            "max_height": round(max_h, 4),
            "avg_height": round(avg_h, 4),
            "num_features": len(features),
            "num_resources": len(resources),
        },
    }
