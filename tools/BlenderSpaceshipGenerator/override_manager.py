"""
Manual override protection for the Blender Spaceship Generator addon.

Objects (or sub-trees) whose Blender custom property ``af_manual_override``
is set to ``True`` are skipped during regeneration.  This lets artists
hand-edit a generated object and keep those edits safe when the rest of
the ship is regenerated.

Usage from the generate operator::

    override_manager.is_protected(obj)          # True / False
    override_manager.set_protected(obj, True)   # mark object
    override_manager.collect_protected(root)    # list all protected children
    override_manager.filter_children(root)      # children NOT protected
"""


# ---------------------------------------------------------------------------
# Constants
# ---------------------------------------------------------------------------

# Custom-property key stored on protected Blender objects.
OVERRIDE_PROP = "af_manual_override"


# ---------------------------------------------------------------------------
# Public API
# ---------------------------------------------------------------------------

def is_protected(obj):
    """Return ``True`` if *obj* has the manual-override flag set.

    A ``None`` object is never protected (returns ``False``).
    """
    if obj is None:
        return False
    return bool(obj.get(OVERRIDE_PROP, False))


def set_protected(obj, protected=True):
    """Set or clear the manual-override flag on *obj*.

    Args:
        obj: Blender object.
        protected: ``True`` to protect, ``False`` to clear.

    Returns:
        ``True`` on success, ``False`` if *obj* is ``None``.
    """
    if obj is None:
        return False
    obj[OVERRIDE_PROP] = 1 if protected else 0
    return True


def collect_protected(root_obj):
    """Return a list of all protected objects in the sub-tree of *root_obj*.

    The root itself is included if it is protected.  Traversal is
    depth-first over ``children_recursive``.

    Args:
        root_obj: Top-level Blender object to scan.

    Returns:
        List of protected Blender objects.
    """
    if root_obj is None:
        return []

    result = []
    if is_protected(root_obj):
        result.append(root_obj)

    for child in root_obj.children_recursive:
        if is_protected(child):
            result.append(child)

    return result


def filter_children(root_obj):
    """Return children of *root_obj* that are **not** protected.

    Only direct children are considered — descendants of a protected child
    are implicitly protected (excluded) too.

    Args:
        root_obj: Top-level Blender object.

    Returns:
        List of unprotected direct children.
    """
    if root_obj is None:
        return []

    return [child for child in root_obj.children if not is_protected(child)]


def clear_all_overrides(root_obj):
    """Remove the override flag from *root_obj* and every descendant.

    Args:
        root_obj: Top-level Blender object.

    Returns:
        Number of flags cleared.
    """
    if root_obj is None:
        return 0

    count = 0
    for obj in [root_obj] + list(root_obj.children_recursive):
        if OVERRIDE_PROP in obj:
            del obj[OVERRIDE_PROP]
            count += 1
    return count


def count_protected(root_obj):
    """Return the number of protected objects beneath *root_obj*.

    Args:
        root_obj: Top-level Blender object.

    Returns:
        Integer count (0 when *root_obj* is ``None``).
    """
    return len(collect_protected(root_obj))


# ---------------------------------------------------------------------------
# Registration
# ---------------------------------------------------------------------------

def register():
    """Register this module."""
    pass


def unregister():
    """Unregister this module."""
    pass
