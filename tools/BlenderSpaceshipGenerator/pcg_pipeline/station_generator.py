"""
NovaForge PCG Pipeline — Station Generator

Generates procedural station metadata with modular sections, docking bays,
and faction-appropriate styling.
"""

import random

# Station templates matching NovaForge factions
STATION_TYPES = {
    "trade":    {"base_modules": ["core", "hangar", "cargo"],     "style": "commercial"},
    "military": {"base_modules": ["core", "weapon", "shield"],    "style": "military"},
    "refinery": {"base_modules": ["core", "engine", "cargo"],     "style": "industrial"},
    "research": {"base_modules": ["core", "sensor", "power"],     "style": "research"},
    "mining":   {"base_modules": ["core", "cargo", "engine"],     "style": "mining"},
}

FACTIONS = ["Solari", "Veyren", "Aurelian", "Keldari"]

MODULE_POOL = ["core", "engine", "weapon", "hangar", "sensor", "power",
               "cargo", "shield"]


def generate_station(seed, planet_id, station_index):
    """Generate station metadata.

    Args:
        seed: Deterministic seed.
        planet_id: Parent planet identifier.
        station_index: Ordinal index among sibling stations.

    Returns:
        dict with station metadata.
    """
    rng = random.Random(seed)

    station_id = f"{planet_id}_station_{station_index:02d}"
    station_type = rng.choice(list(STATION_TYPES.keys()))
    template = STATION_TYPES[station_type]

    # Build module list from template + random extras
    modules = list(template["base_modules"])
    extra_count = rng.randint(0, 3)
    for _ in range(extra_count):
        modules.append(rng.choice(MODULE_POOL))

    faction = rng.choice(FACTIONS)
    docking_bays = rng.randint(1, 4)

    return {
        "station_id": station_id,
        "seed": seed,
        "planet_id": planet_id,
        "type": station_type,
        "style": template["style"],
        "faction": faction,
        "modules": modules,
        "docking_bays": docking_bays,
    }
