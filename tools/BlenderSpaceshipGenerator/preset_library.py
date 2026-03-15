"""
Preset library for the Blender Spaceship Generator addon.

Save and load ship generation presets as JSON files.  Each preset stores
the full set of generator parameters so that a user can recreate an
exact ship configuration later.

Presets are stored in a configurable directory (defaults to
``<addon_path>/presets/``).

Usage from the save/load operators::

    preset_library.save_preset(name, props_dict)
    props_dict = preset_library.load_preset(name)
    names = preset_library.list_presets()
"""

import json
import os


# ---------------------------------------------------------------------------
# Constants
# ---------------------------------------------------------------------------

# Default directory for presets — relative to this file's location.
_DEFAULT_PRESET_DIR = os.path.join(os.path.dirname(os.path.abspath(__file__)),
                                   'presets')

PRESET_VERSION = 1

# Keys that are persisted in a preset (20 parameters).
PRESET_KEYS = [
    'ship_class',
    'style',
    'seed',
    'generate_interior',
    'module_slots',
    'hull_complexity',
    'symmetry',
    'generate_textures',
    'weathering',
    'naming_prefix',
    'turret_hardpoints',
    'launcher_hardpoints',
    'drone_bays',
    'hull_taper',
    'generate_lods',
    'generate_collision',
    'collision_type',
    'generate_animations',
    'generate_lighting',
    'greeble_density',
]


# ---------------------------------------------------------------------------
# Internal helpers
# ---------------------------------------------------------------------------

def _ensure_preset_dir(preset_dir=None):
    """Create the preset directory if it does not exist and return its path."""
    d = preset_dir or _DEFAULT_PRESET_DIR
    os.makedirs(d, exist_ok=True)
    return d


def _preset_path(name, preset_dir=None):
    """Return the full file path for a preset *name*."""
    d = _ensure_preset_dir(preset_dir)
    # Sanitise name: allow only alnum, dash, underscore
    safe = ''.join(c if (c.isalnum() or c in '-_') else '_' for c in name)
    return os.path.join(d, f"{safe}.json")


# ---------------------------------------------------------------------------
# Public API
# ---------------------------------------------------------------------------

def save_preset(name, props, preset_dir=None):
    """Save a generation preset to disk.

    Args:
        name: Human-readable preset name (used as filename stem).
        props: Dict (or Blender PropertyGroup-like object) with generator
               parameters.  Only keys listed in :data:`PRESET_KEYS` are
               stored.
        preset_dir: Optional override for the preset directory.

    Returns:
        The file path the preset was saved to.

    Raises:
        ValueError: If *name* is empty.
    """
    if not name or not name.strip():
        raise ValueError("Preset name must not be empty")

    data = {"_version": PRESET_VERSION, "_name": name}

    for key in PRESET_KEYS:
        # Support both dict-style and attribute-style access
        if isinstance(props, dict):
            if key in props:
                data[key] = props[key]
        else:
            if hasattr(props, key):
                val = getattr(props, key)
                # Blender enums come as strings, bools/ints/floats are fine
                data[key] = val

    filepath = _preset_path(name, preset_dir)
    with open(filepath, 'w') as f:
        json.dump(data, f, indent=2)

    return filepath


def load_preset(name, preset_dir=None):
    """Load a generation preset from disk.

    Args:
        name: Preset name (filename stem without ``.json``).
        preset_dir: Optional override for the preset directory.

    Returns:
        Dict of generator parameter key/value pairs.

    Raises:
        FileNotFoundError: If the preset file does not exist.
    """
    filepath = _preset_path(name, preset_dir)
    if not os.path.isfile(filepath):
        raise FileNotFoundError(f"Preset not found: {filepath}")

    with open(filepath, 'r') as f:
        data = json.load(f)

    # Strip internal metadata before returning
    return {k: v for k, v in data.items() if not k.startswith('_')}


def delete_preset(name, preset_dir=None):
    """Delete a preset file from disk.

    Args:
        name: Preset name.
        preset_dir: Optional override for the preset directory.

    Returns:
        ``True`` if deleted, ``False`` if the file did not exist.
    """
    filepath = _preset_path(name, preset_dir)
    if os.path.isfile(filepath):
        os.remove(filepath)
        return True
    return False


def list_presets(preset_dir=None):
    """Return a sorted list of available preset names.

    Args:
        preset_dir: Optional override for the preset directory.

    Returns:
        List of preset name strings (without ``.json`` extension).
    """
    d = _ensure_preset_dir(preset_dir)
    names = []
    for fname in sorted(os.listdir(d)):
        if fname.endswith('.json'):
            names.append(fname[:-5])
    return names


def get_preset_info(name, preset_dir=None):
    """Return metadata for a preset without loading all parameters.

    Args:
        name: Preset name.
        preset_dir: Optional override for the preset directory.

    Returns:
        Dict with ``_version``, ``_name``, ``ship_class``, ``style``.
        Returns ``None`` if the preset does not exist.
    """
    filepath = _preset_path(name, preset_dir)
    if not os.path.isfile(filepath):
        return None

    with open(filepath, 'r') as f:
        data = json.load(f)

    return {
        'version': data.get('_version', 0),
        'name': data.get('_name', name),
        'ship_class': data.get('ship_class', ''),
        'style': data.get('style', ''),
    }


# ---------------------------------------------------------------------------
# Registration
# ---------------------------------------------------------------------------

def register():
    """Register this module."""
    pass


def unregister():
    """Unregister this module."""
    pass
