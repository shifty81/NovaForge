"""
Version registry for the Blender Spaceship Generator addon.

Tracks semantic versions for every generator module so that template
files, presets, and exported assets carry provenance metadata.  When a
module's output format changes, its version is bumped and consuming
code can decide whether to re-generate.

Usage::

    version_registry.get_module_version("ship_generator")   # "3.0.0"
    version_registry.get_all_versions()                      # dict
    version_registry.check_compatibility("ship_generator", "2.0.0")  # True/False
"""


# ---------------------------------------------------------------------------
# Constants
# ---------------------------------------------------------------------------

# Semantic versions for every generator / system module.
# Bump *major* when the output format changes in a breaking way,
# *minor* for backwards-compatible additions, *patch* for bug-fixes.

MODULE_VERSIONS = {
    "ship_generator":       "3.0.0",
    "ship_parts":           "3.0.0",
    "interior_generator":   "3.0.0",
    "module_system":        "3.0.0",
    "atlas_exporter":       "2.0.0",
    "station_generator":    "2.0.0",
    "asteroid_generator":   "2.0.0",
    "texture_generator":    "2.0.0",
    "brick_system":         "1.0.0",
    "novaforge_importer":   "2.0.0",
    "render_setup":         "1.0.0",
    "lod_generator":        "1.0.0",
    "collision_generator":  "1.0.0",
    "animation_system":     "1.0.0",
    "damage_system":        "1.0.0",
    "power_system":         "1.0.0",
    "build_validator":      "1.0.0",
    "density_field":        "1.0.0",
    "slot_grid":            "1.0.0",
    "traversal_system":     "1.0.0",
    "fleet_logistics":      "1.0.0",
    "rig_system":           "1.0.0",
    "lighting_system":      "1.0.0",
    "greeble_system":       "1.0.0",
    "preset_library":       "1.0.0",
    "furniture_system":     "1.0.0",
    "pcg_pipeline":         "1.1.0",
}


# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------

def _parse_semver(version_str):
    """Parse a ``"major.minor.patch"`` string into an ``(int, int, int)`` tuple.

    Raises ``ValueError`` on malformed input.
    """
    parts = version_str.strip().split(".")
    if len(parts) != 3:
        raise ValueError(f"Invalid semver string: {version_str!r}")
    return tuple(int(p) for p in parts)


# ---------------------------------------------------------------------------
# Public API
# ---------------------------------------------------------------------------

def get_module_version(module_name):
    """Return the semantic version string for *module_name*.

    Args:
        module_name: Module name (e.g. ``"ship_generator"``).

    Returns:
        Version string such as ``"3.0.0"``, or ``"0.0.0"`` if the module
        is not registered.
    """
    return MODULE_VERSIONS.get(module_name, "0.0.0")


def get_all_versions():
    """Return a copy of the full ``{module_name: version}`` dict."""
    return dict(MODULE_VERSIONS)


def check_compatibility(module_name, required_version):
    """Check whether the current module version satisfies *required_version*.

    The check passes when the current *major* version equals the required
    *major* version **and** the current *minor* version is greater-than or
    equal-to the required *minor* version (standard semver compatibility).

    Args:
        module_name: Module name.
        required_version: Minimum version string (e.g. ``"2.1.0"``).

    Returns:
        ``True`` if compatible, ``False`` otherwise.
    """
    current = get_module_version(module_name)
    try:
        cur = _parse_semver(current)
        req = _parse_semver(required_version)
    except ValueError:
        return False

    # Same major AND current minor >= required minor
    return cur[0] == req[0] and cur[1] >= req[1]


def version_stamp():
    """Return a compact dict suitable for embedding in exported JSON files.

    Example::

        {
            "generator_version": "3.0.0",
            "modules": {"ship_generator": "3.0.0", ...}
        }
    """
    return {
        "generator_version": get_module_version("ship_generator"),
        "modules": get_all_versions(),
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
