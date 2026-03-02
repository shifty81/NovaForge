"""
NovaForge PCG Pipeline — Character Generator

Generates procedural character metadata matching the NovaForge race and
body type system defined in ``character_mesh_system.h``.
"""

import random

RACES = ["TerranDescendant", "SynthBorn", "PureAlien", "HybridEvolutionary"]

BODY_TYPES = ["Organic", "Augmented", "Cybernetic", "FullSynth", "MechFrame"]

CYBER_LIMB_SLOTS = [
    "left_arm", "right_arm", "left_leg", "right_leg",
    "torso_core", "spine",
]


def generate_character(seed, char_id):
    """Generate character metadata.

    Args:
        seed: Deterministic seed.
        char_id: Unique identifier string.

    Returns:
        dict with character metadata.
    """
    rng = random.Random(seed)

    race = rng.choice(RACES)
    body_type = rng.choice(BODY_TYPES)

    # Cyber limbs — only for appropriate body types
    cyber_limbs = []
    if body_type in ("Augmented", "Cybernetic", "FullSynth"):
        num_limbs = rng.randint(1, len(CYBER_LIMB_SLOTS))
        cyber_limbs = rng.sample(CYBER_LIMB_SLOTS, k=num_limbs)

    return {
        "char_id": char_id,
        "seed": seed,
        "race": race,
        "body_type": body_type,
        "cyber_limbs": cyber_limbs,
    }
