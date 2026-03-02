"""
NovaForge PCG Pipeline — Batch Generate

Single-command universe generation that orchestrates all sub-generators
and writes output to a structured build directory.

Usage (CLI):
    python -m pcg_pipeline.batch_generate [--seed SEED] [--systems N]
                                          [--output-dir DIR]

Usage (library):
    from pcg_pipeline.batch_generate import generate_universe
    result = generate_universe(seed=123456, num_systems=5,
                               output_dir='build')
"""

import argparse
import json
import os
import sys

from . import galaxy_generator
from . import ship_generator as ship_gen


def generate_universe(seed=123456, num_systems=5, output_dir="build",
                      export_meshes=False, blender_bin="blender"):
    """Generate a full NovaForge universe.

    Args:
        seed: Universe master seed.
        num_systems: Number of star systems.
        output_dir: Root output directory for all generated data.
        export_meshes: If True, invoke Blender headless for mesh export
                       (requires Blender installed).
        blender_bin: Path to Blender binary.

    Returns:
        dict with the full galaxy data.
    """
    galaxy = galaxy_generator.generate_galaxy(seed, num_systems)

    # Ensure output directories exist
    dirs = ["systems", "planets", "stations", "ships", "characters"]
    for d in dirs:
        os.makedirs(os.path.join(output_dir, d), exist_ok=True)

    # Write galaxy manifest
    galaxy_path = os.path.join(output_dir, "galaxy.json")
    with open(galaxy_path, "w") as fh:
        json.dump(galaxy, fh, indent=2)
        fh.write("\n")

    # Write per-system, per-asset JSON files
    for system in galaxy["systems"]:
        sid = system["system_id"]

        # System metadata
        sys_path = os.path.join(output_dir, "systems", f"{sid}.json")
        with open(sys_path, "w") as fh:
            json.dump(system, fh, indent=2)
            fh.write("\n")

        # Planet metadata
        for planet in system["planets"]:
            p_path = os.path.join(output_dir, "planets",
                                  f"{planet['planet_id']}.json")
            with open(p_path, "w") as fh:
                json.dump(planet, fh, indent=2)
                fh.write("\n")

        # Station metadata
        for station in system["stations"]:
            st_path = os.path.join(output_dir, "stations",
                                   f"{station['station_id']}.json")
            with open(st_path, "w") as fh:
                json.dump(station, fh, indent=2)
                fh.write("\n")

        # Ship metadata + optional Blender export
        for ship in system["ships"]:
            ship_path = ship_gen.save_ship_metadata(
                ship, os.path.join(output_dir, "ships"))

            if export_meshes:
                ship_gen.export_ship_blender(
                    ship_path,
                    os.path.join(output_dir, "ships"),
                    blender_bin=blender_bin,
                )

        # Character metadata
        for char in system["characters"]:
            c_path = os.path.join(output_dir, "characters",
                                  f"{char['char_id']}.json")
            with open(c_path, "w") as fh:
                json.dump(char, fh, indent=2)
                fh.write("\n")

    return galaxy


def main(argv=None):
    """CLI entry-point for batch generation."""
    parser = argparse.ArgumentParser(
        description="NovaForge PCG Pipeline — Batch Universe Generator")
    parser.add_argument("--seed", type=int, default=123456,
                        help="Universe master seed (default: 123456)")
    parser.add_argument("--systems", type=int, default=5,
                        help="Number of star systems (default: 5)")
    parser.add_argument("--output-dir", default="build",
                        help="Output directory (default: build)")
    parser.add_argument("--export-meshes", action="store_true",
                        help="Export meshes via Blender headless mode")
    parser.add_argument("--blender-bin", default="blender",
                        help="Path to Blender binary")
    args = parser.parse_args(argv)

    print(f"NovaForge Generator — generating universe (seed={args.seed}, "
          f"systems={args.systems})")

    galaxy = generate_universe(
        seed=args.seed,
        num_systems=args.systems,
        output_dir=args.output_dir,
        export_meshes=args.export_meshes,
        blender_bin=args.blender_bin,
    )

    total_planets = sum(len(s["planets"]) for s in galaxy["systems"])
    total_stations = sum(len(s["stations"]) for s in galaxy["systems"])
    total_ships = sum(len(s["ships"]) for s in galaxy["systems"])
    total_chars = sum(len(s["characters"]) for s in galaxy["systems"])

    print(f"✓ Generated {len(galaxy['systems'])} systems, "
          f"{total_planets} planets, {total_stations} stations, "
          f"{total_ships} ships, {total_chars} characters")
    print(f"✓ Output written to {os.path.abspath(args.output_dir)}/")
    return 0


if __name__ == "__main__":
    sys.exit(main())
