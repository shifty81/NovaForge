"""
NovaForge PCG Pipeline — Ship Generator

Generates ship metadata and provides a helper for headless Blender export
via the BlenderSpaceshipGenerator addon.  Ship classes and factions align
with existing NovaForge game data in ``data/ships/``.

LOD (Level of Detail) tiers are embedded in every ship's metadata so that
the Atlas engine (or any runtime) can request reduced-poly assets:

  - **LOD0** — full detail, used in close-up / hangar view
  - **LOD1** — half poly-count, used at mid-range
  - **LOD2** — quarter poly-count, used at long range
"""

import json
import os
import random
import subprocess

# Ship classes matching NovaForge / BlenderSpaceshipGenerator configs
SHIP_CLASSES = [
    "Shuttle", "Fighter", "Corvette", "Frigate", "Destroyer",
    "Cruiser", "Battlecruiser", "Battleship", "Carrier", "Capital",
    "Dreadnought", "Titan", "Industrial", "Mining Barge", "Exhumer",
    "Explorer", "Hauler",
]

FACTIONS = ["Solari", "Veyren", "Aurelian", "Keldari"]

MODULE_POOL = ["engine", "weapon", "core", "shield", "sensor", "cargo",
               "hangar", "power"]

# LOD tier definitions — each tier describes the fraction of the full
# poly-count that should be retained and the maximum view distance (in
# game-world metres) at which that tier is used.
LOD_TIERS = [
    {"tier": 0, "poly_fraction": 1.0,  "max_distance": 500},
    {"tier": 1, "poly_fraction": 0.5,  "max_distance": 2000},
    {"tier": 2, "poly_fraction": 0.25, "max_distance": 8000},
]


def generate_ship_metadata(seed, ship_id):
    """Generate ship metadata JSON.

    Args:
        seed: Deterministic seed for this ship.
        ship_id: Unique identifier string.

    Returns:
        dict with ship metadata including LOD tiers.
    """
    rng = random.Random(seed)

    ship_class = rng.choice(SHIP_CLASSES)
    faction = rng.choice(FACTIONS)
    modules = rng.sample(MODULE_POOL, k=rng.randint(2, min(5, len(MODULE_POOL))))
    hardpoints = rng.randint(0, 10)

    return {
        "ship_id": ship_id,
        "seed": seed,
        "class": ship_class,
        "faction": faction,
        "modules": modules,
        "hardpoints": hardpoints,
        "lod_tiers": list(LOD_TIERS),
    }


def export_ship_blender(ship_json_path, output_dir, blender_bin="blender"):
    """Export a ship mesh using Blender in headless mode.

    Requires Blender to be installed and the BlenderSpaceshipGenerator
    addon to be available.

    Args:
        ship_json_path: Path to the ship metadata JSON file.
        output_dir: Directory to write exported OBJ files.
        blender_bin: Path to Blender binary (default ``blender``).

    Returns:
        int return code from subprocess.
    """
    addon_dir = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    exporter = os.path.join(addon_dir, "atlas_exporter.py")

    os.makedirs(output_dir, exist_ok=True)
    result = subprocess.run(
        [
            blender_bin, "--background", "--python", exporter, "--",
            "--ship_json", ship_json_path,
            "--output_dir", output_dir,
        ],
        capture_output=True,
        text=True,
    )
    return result.returncode


def save_ship_metadata(ship_data, output_dir):
    """Persist ship metadata to a JSON file in *output_dir*.

    Args:
        ship_data: dict returned by :func:`generate_ship_metadata`.
        output_dir: Target directory.

    Returns:
        Path to the written JSON file.
    """
    os.makedirs(output_dir, exist_ok=True)
    path = os.path.join(output_dir, f"{ship_data['ship_id']}.json")
    with open(path, "w") as fh:
        json.dump(ship_data, fh, indent=2)
        fh.write("\n")
    return path
