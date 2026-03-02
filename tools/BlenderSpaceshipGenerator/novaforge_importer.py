"""
NovaForge ship data importer.

Reads ship definitions from NovaForge ``data/ships/*.json`` files and
translates them into parameters understood by the Blender generator.

Each NovaForge ship JSON entry contains a ``model_data`` block::

    "model_data": {
        "turret_hardpoints": 4,
        "launcher_hardpoints": 3,
        "drone_bays": 2,
        "engine_count": 3,
        "generation_seed": 77268
    }

The importer maps the NovaForge *race* field (Solari, Veyren, Aurelian,
Keldari) to the Blender generator *style* parameter and derives generator
settings from the ship class name and model_data.
"""

import json
import os
import glob as _glob


# ---------------------------------------------------------------------------
# NovaForge race -> Blender style mapping
# ---------------------------------------------------------------------------

RACE_TO_STYLE = {
    'Solari':   'SOLARI',
    'Veyren':   'VEYREN',
    'Aurelian': 'AURELIAN',
    'Keldari':  'KELDARI',
}

# ---------------------------------------------------------------------------
# NovaForge class name -> generator ship_class mapping
# ---------------------------------------------------------------------------

CLASS_MAP = {
    'Shuttle':        'SHUTTLE',
    'Fighter':        'FIGHTER',
    'Corvette':       'CORVETTE',
    'Frigate':        'FRIGATE',
    'Destroyer':      'DESTROYER',
    'Cruiser':        'CRUISER',
    'Battlecruiser':  'BATTLECRUISER',
    'Battleship':     'BATTLESHIP',
    'Carrier':        'CARRIER',
    'Dreadnought':    'DREADNOUGHT',
    'Capital':        'CAPITAL',
    'Titan':          'TITAN',
    'Industrial':     'INDUSTRIAL',
    'Mining Barge':   'MINING_BARGE',
    'Exhumer':        'EXHUMER',
    'Explorer':       'EXPLORER',
    'Hauler':         'HAULER',
    'Exotic':         'EXOTIC',
}

# ---------------------------------------------------------------------------
# NovaForge scale table (game-units) -- 1 game unit ~ 50 m
# Used to derive Blender scale from ship class.
# ---------------------------------------------------------------------------

NOVAFORGE_SCALES = {
    'Shuttle':        {'length': 1.0, 'width': 0.5, 'height': 0.4},
    'Fighter':        {'length': 1.5, 'width': 0.6, 'height': 0.4},
    'Corvette':       {'length': 2.5, 'width': 0.7, 'height': 0.5},
    'Frigate':        {'length': 3.5, 'width': 0.9, 'height': 0.7},
    'Destroyer':      {'length': 5.0, 'width': 0.7, 'height': 0.6},
    'Cruiser':        {'length': 6.0, 'width': 1.8, 'height': 1.5},
    'Battlecruiser':  {'length': 8.5, 'width': 2.5, 'height': 2.0},
    'Battleship':     {'length': 12.0, 'width': 3.5, 'height': 3.0},
    'Carrier':        {'length': 15.0, 'width': 6.0, 'height': 4.0},
    'Dreadnought':    {'length': 12.0, 'width': 4.5, 'height': 5.0},
    'Capital':        {'length': 18.0, 'width': 5.0, 'height': 4.5},
    'Titan':          {'length': 25.0, 'width': 8.0, 'height': 7.0},
    'Mining Barge':   {'length': 6.0, 'width': 3.0, 'height': 2.0},
    'Industrial':     {'length': 6.0, 'width': 2.5, 'height': 2.0},
    'Exhumer':        {'length': 5.0, 'width': 2.5, 'height': 1.8},
    'Explorer':       {'length': 2.0, 'width': 0.6, 'height': 0.4},
    'Hauler':         {'length': 5.5, 'width': 2.0, 'height': 1.8},
    'Exotic':         {'length': 2.5, 'width': 0.8, 'height': 0.5},
}


# ---------------------------------------------------------------------------
# Loader helpers
# ---------------------------------------------------------------------------


def load_ships_from_file(filepath):
    """Load ship definitions from a single NovaForge JSON file.

    Returns:
        dict mapping ship id -> ship definition dict.

    Raises:
        FileNotFoundError, json.JSONDecodeError on bad input.
    """
    with open(filepath, 'r', encoding='utf-8') as fh:
        return json.load(fh)


def load_ships_from_directory(directory):
    """Load all ``*.json`` ship files from *directory*.

    Returns:
        dict mapping ship id -> ship definition dict (merged from all files).
    """
    ships = {}
    pattern = os.path.join(directory, '*.json')
    for path in sorted(_glob.glob(pattern)):
        try:
            data = load_ships_from_file(path)
            ships.update(data)
        except (json.JSONDecodeError, OSError):
            pass  # skip bad files
    return ships


# ---------------------------------------------------------------------------
# Translation
# ---------------------------------------------------------------------------


def ship_to_generator_params(ship_def):
    """Translate a single NovaForge ship definition into keyword arguments
    suitable for :func:`ship_generator.generate_spaceship`.

    Args:
        ship_def: dict with at minimum ``class``, ``race``, and ``model_data``.

    Returns:
        dict of keyword arguments.
    """
    nf_class = ship_def.get('class', 'Frigate')
    race = ship_def.get('race', 'Keldari')
    model = ship_def.get('model_data', {})

    ship_class = CLASS_MAP.get(nf_class, 'FRIGATE')
    style = RACE_TO_STYLE.get(race, 'SOLARI')

    seed = model.get('generation_seed', 1)
    turrets = model.get('turret_hardpoints', 0)
    launchers = model.get('launcher_hardpoints', 0)
    drone_bays = model.get('drone_bays', 0)
    engine_count = model.get('engine_count', 2)

    nf_scale = NOVAFORGE_SCALES.get(nf_class, NOVAFORGE_SCALES['Frigate'])

    return {
        'ship_class': ship_class,
        'seed': seed,
        'style': style,
        'turret_hardpoints': turrets,
        'launcher_hardpoints': launchers,
        'drone_bays': drone_bays,
        'engine_count_override': engine_count,
        'hull_taper': 0.85,
        'generate_interior': True,
        'module_slots': 2,
        'naming_prefix': ship_def.get('id', ''),
        'novaforge_scale': nf_scale,
    }


def get_all_generator_params(ships_dict):
    """Convert an entire ship dictionary (id -> def) to a list of
    ``(ship_id, generator_params)`` tuples."""
    results = []
    for ship_id, ship_def in ships_dict.items():
        params = ship_to_generator_params(ship_def)
        results.append((ship_id, params))
    return results


# ---------------------------------------------------------------------------
# Blender registration stubs
# ---------------------------------------------------------------------------


def register():
    """Register this module."""
    pass


def unregister():
    """Unregister this module."""
    pass
