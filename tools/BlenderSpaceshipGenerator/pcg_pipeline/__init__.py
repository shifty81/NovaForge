"""
NovaForge PCG Pipeline

Seed-based procedural content generation pipeline for the NovaForge universe.
Generates galaxies, star systems, planets, stations, ships, and characters
using deterministic seeds for reproducible content.

This module integrates with the BlenderSpaceshipGenerator addon and can be
used standalone (without Blender) for metadata generation, or with Blender
in headless mode for full mesh/texture/LOD asset export.

Usage (standalone metadata generation):
    from pcg_pipeline import batch_generate
    batch_generate.generate_universe(seed=123456, num_systems=5,
                                     output_dir='build')

Usage (Blender headless asset export):
    blender --background --python -c "
        from pcg_pipeline import batch_generate
        batch_generate.generate_universe(seed=123456, num_systems=5,
                                         output_dir='build', export_meshes=True)
    "

Usage (CLI):
    python -m pcg_pipeline --seed 123456 --systems 5 --output-dir build
"""

__version__ = "1.0.0"
__pipeline_name__ = "NovaForge Generator"

from . import galaxy_generator
from . import system_generator
from . import planet_generator
from . import station_generator
from . import ship_generator
from . import character_generator
from . import batch_generate

__all__ = [
    "galaxy_generator",
    "system_generator",
    "planet_generator",
    "station_generator",
    "ship_generator",
    "character_generator",
    "batch_generate",
]
