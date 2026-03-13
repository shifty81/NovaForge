#!/usr/bin/env python3
"""
AtlasForge Generator — Blender addon packager.

Creates two ready-to-install ZIP files:

  AtlasForgeGenerator.zip           — Blender 4.2+ Extension (blender_manifest.toml
                                       at the ZIP root, as required by the Extensions
                                       platform).
  AtlasForgeGenerator_legacy.zip    — Blender 2.80 – 4.1 legacy add-on (single
                                       directory "BlenderSpaceshipGenerator/" at the
                                       ZIP root).

Run from *any* directory:
    python tools/BlenderSpaceshipGenerator/package.py

The ZIPs are written to the repository root (or the current working directory if run
from elsewhere).
"""

import os
import sys
import zipfile
import shutil

# ---------------------------------------------------------------------------
# Paths
# ---------------------------------------------------------------------------

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
REPO_ROOT = os.path.abspath(os.path.join(SCRIPT_DIR, "..", ".."))

# Output directory — default to repo root so the ZIPs are easy to find.
# Override with the first CLI argument if provided.
if len(sys.argv) > 1:
    OUTPUT_DIR = os.path.abspath(sys.argv[1])
else:
    OUTPUT_DIR = REPO_ROOT

# Files / directories to include (relative to SCRIPT_DIR).
# NOTE: __pycache__ directories and *.pyc files are always excluded.
INCLUDE_FILES = [
    "__init__.py",
    "blender_manifest.toml",
    "animation_system.py",
    "asteroid_generator.py",
    "atlas_exporter.py",
    "brick_system.py",
    "build_validator.py",
    "collision_generator.py",
    "damage_system.py",
    "density_field.py",
    "fleet_logistics.py",
    "interior_generator.py",
    "lod_generator.py",
    "module_system.py",
    "novaforge_importer.py",
    "pcg_panel.py",
    "power_system.py",
    "render_setup.py",
    "rig_system.py",
    "ship_generator.py",
    "ship_parts.py",
    "slot_grid.py",
    "station_generator.py",
    "texture_generator.py",
    "traversal_system.py",
]

INCLUDE_DIRS = [
    "pcg_pipeline",
]


def _should_skip(name: str) -> bool:
    """Return True for files/directories that should not be included."""
    return (
        name == "__pycache__"
        or name.endswith(".pyc")
        or name.endswith(".pyo")
        or name.startswith(".")
    )


def _add_dir_to_zip(zf: zipfile.ZipFile, src_dir: str, zip_prefix: str) -> None:
    """Recursively add *src_dir* into *zf* under *zip_prefix*."""
    for root, dirs, files in os.walk(src_dir):
        # Skip unwanted directories in-place so os.walk doesn't descend into them.
        dirs[:] = [d for d in dirs if not _should_skip(d)]
        for filename in files:
            if _should_skip(filename):
                continue
            abs_path = os.path.join(root, filename)
            rel_path = os.path.relpath(abs_path, src_dir)
            zf.write(abs_path, os.path.join(zip_prefix, rel_path))


def build_extension_zip(output_path: str) -> None:
    """
    Blender 4.2+ Extension ZIP.

    ``blender_manifest.toml`` and all addon files sit directly at the root of
    the archive (no containing folder).  This is what Blender's Extensions
    platform requires.
    """
    with zipfile.ZipFile(output_path, "w", compression=zipfile.ZIP_DEFLATED) as zf:
        # Top-level files
        for filename in INCLUDE_FILES:
            src = os.path.join(SCRIPT_DIR, filename)
            if not os.path.isfile(src):
                print(f"  WARNING: {filename} not found — skipped")
                continue
            zf.write(src, filename)

        # Subdirectories
        for dirname in INCLUDE_DIRS:
            src_dir = os.path.join(SCRIPT_DIR, dirname)
            if not os.path.isdir(src_dir):
                print(f"  WARNING: {dirname}/ not found — skipped")
                continue
            _add_dir_to_zip(zf, src_dir, dirname)

    size_kb = os.path.getsize(output_path) / 1024
    print(f"  Created: {output_path}  ({size_kb:.1f} KB)")


def build_legacy_zip(output_path: str) -> None:
    """
    Blender 2.80 – 4.1 legacy add-on ZIP.

    All addon files sit inside a single top-level directory
    ``BlenderSpaceshipGenerator/``.  Blender discovers the addon by finding
    ``__init__.py`` inside that directory.
    """
    folder = "BlenderSpaceshipGenerator"

    with zipfile.ZipFile(output_path, "w", compression=zipfile.ZIP_DEFLATED) as zf:
        # Top-level files
        for filename in INCLUDE_FILES:
            src = os.path.join(SCRIPT_DIR, filename)
            if not os.path.isfile(src):
                print(f"  WARNING: {filename} not found — skipped")
                continue
            zf.write(src, os.path.join(folder, filename))

        # Subdirectories
        for dirname in INCLUDE_DIRS:
            src_dir = os.path.join(SCRIPT_DIR, dirname)
            if not os.path.isdir(src_dir):
                print(f"  WARNING: {dirname}/ not found — skipped")
                continue
            _add_dir_to_zip(zf, src_dir, os.path.join(folder, dirname))

    size_kb = os.path.getsize(output_path) / 1024
    print(f"  Created: {output_path}  ({size_kb:.1f} KB)")


def verify_zip(zip_path: str, required_top_level: list) -> bool:
    """Verify that *required_top_level* entries exist in the ZIP."""
    with zipfile.ZipFile(zip_path, "r") as zf:
        names = zf.namelist()
    ok = True
    for entry in required_top_level:
        if not any(n == entry or n.startswith(entry + "/") for n in names):
            print(f"  ERROR: '{entry}' missing from {zip_path}")
            ok = False
    return ok


def main() -> int:
    os.makedirs(OUTPUT_DIR, exist_ok=True)

    ext_zip = os.path.join(OUTPUT_DIR, "AtlasForgeGenerator.zip")
    legacy_zip = os.path.join(OUTPUT_DIR, "AtlasForgeGenerator_legacy.zip")

    print("Building AtlasForge Generator addon ZIPs...")
    print()

    # Blender 4.2+ Extension
    print("  [Blender 4.2+ Extension]")
    build_extension_zip(ext_zip)
    ext_ok = verify_zip(ext_zip, ["__init__.py", "blender_manifest.toml"])

    print()

    # Blender 2.80 – 4.1 legacy
    print("  [Blender 2.80 – 4.1 Legacy]")
    build_legacy_zip(legacy_zip)
    legacy_ok = verify_zip(legacy_zip, [
        "BlenderSpaceshipGenerator/__init__.py",
        "BlenderSpaceshipGenerator/blender_manifest.toml",
    ])

    print()
    if ext_ok and legacy_ok:
        print("All ZIPs built successfully.")
        print()
        print("Installation:")
        print(f"  Blender 4.2+  → Edit > Preferences > Extensions > Install from Disk")
        print(f"                  Select: {ext_zip}")
        print(f"  Blender < 4.2 → Edit > Preferences > Add-ons > Install")
        print(f"                  Select: {legacy_zip}")
        return 0
    else:
        print("One or more ZIPs failed verification — check warnings above.")
        return 1


if __name__ == "__main__":
    sys.exit(main())
