"""
Template manager for the Blender Spaceship Generator addon.

Discovers, imports, and exports ship / station / fleet JSON templates
from a central directory.  Templates carry version metadata so they
can be validated against the current generator modules.

The default template directory is ``<addon_path>/templates/``.

Usage::

    template_manager.save_template(name, data, category="ship")
    data = template_manager.load_template(name, category="ship")
    names = template_manager.list_templates(category="ship")
    template_manager.discover_templates()  # scan all categories
"""

import json
import os


# ---------------------------------------------------------------------------
# Constants
# ---------------------------------------------------------------------------

# Default directory — sibling of this file.
_DEFAULT_TEMPLATE_DIR = os.path.join(
    os.path.dirname(os.path.abspath(__file__)), "templates"
)

TEMPLATE_VERSION = 1

# Recognised template categories and their sub-directory names.
TEMPLATE_CATEGORIES = ["ship", "station", "fleet", "asteroid", "character"]


# ---------------------------------------------------------------------------
# Internal helpers
# ---------------------------------------------------------------------------

def _ensure_dir(path):
    """Create *path* directory tree if it does not exist."""
    os.makedirs(path, exist_ok=True)
    return path


def _category_dir(category, template_dir=None):
    """Return the directory path for a given *category*.

    Raises ``ValueError`` for unrecognised categories.
    """
    if category not in TEMPLATE_CATEGORIES:
        raise ValueError(
            f"Unknown template category {category!r}. "
            f"Choose from {TEMPLATE_CATEGORIES}"
        )
    base = template_dir or _DEFAULT_TEMPLATE_DIR
    return _ensure_dir(os.path.join(base, category))


def _sanitize_name(name):
    """Return a filesystem-safe version of *name*."""
    return "".join(c if (c.isalnum() or c in "-_") else "_" for c in name)


def _template_path(name, category, template_dir=None):
    """Full file path for a template."""
    d = _category_dir(category, template_dir)
    safe = _sanitize_name(name)
    return os.path.join(d, f"{safe}.json")


# ---------------------------------------------------------------------------
# Public API
# ---------------------------------------------------------------------------

def save_template(name, data, category="ship", template_dir=None):
    """Save a template to disk.

    Args:
        name: Human-readable template name.
        data: Dict of template data.
        category: One of :data:`TEMPLATE_CATEGORIES`.
        template_dir: Optional override for the root template directory.

    Returns:
        The file path the template was written to.

    Raises:
        ValueError: If *name* is empty or *category* is invalid.
    """
    if not name or not name.strip():
        raise ValueError("Template name must not be empty")

    payload = {
        "_version": TEMPLATE_VERSION,
        "_name": name,
        "_category": category,
    }
    payload.update(data)

    filepath = _template_path(name, category, template_dir)
    with open(filepath, "w") as fh:
        json.dump(payload, fh, indent=2)

    return filepath


def load_template(name, category="ship", template_dir=None):
    """Load a template from disk.

    Args:
        name: Template name (filename stem).
        category: One of :data:`TEMPLATE_CATEGORIES`.
        template_dir: Optional override for the root template directory.

    Returns:
        Dict of template data (internal metadata keys stripped).

    Raises:
        FileNotFoundError: If the template file does not exist.
    """
    filepath = _template_path(name, category, template_dir)
    if not os.path.isfile(filepath):
        raise FileNotFoundError(f"Template not found: {filepath}")

    with open(filepath, "r") as fh:
        data = json.load(fh)

    return {k: v for k, v in data.items() if not k.startswith("_")}


def delete_template(name, category="ship", template_dir=None):
    """Delete a template file.

    Returns:
        ``True`` if deleted, ``False`` if the file did not exist.
    """
    filepath = _template_path(name, category, template_dir)
    if os.path.isfile(filepath):
        os.remove(filepath)
        return True
    return False


def list_templates(category="ship", template_dir=None):
    """Return a sorted list of template names in *category*.

    Returns:
        List of name strings (without ``.json`` extension).
    """
    d = _category_dir(category, template_dir)
    names = []
    for fname in sorted(os.listdir(d)):
        if fname.endswith(".json"):
            names.append(fname[:-5])
    return names


def discover_templates(template_dir=None):
    """Scan all categories and return a summary dict.

    Returns:
        ``{category: [name, ...], ...}`` for every category that has
        at least one template.
    """
    result = {}
    for cat in TEMPLATE_CATEGORIES:
        try:
            names = list_templates(cat, template_dir)
            if names:
                result[cat] = names
        except Exception:
            pass
    return result


def get_template_info(name, category="ship", template_dir=None):
    """Return metadata for a template without loading the full payload.

    Returns:
        Dict with ``version``, ``name``, ``category`` keys, or ``None``
        if the template does not exist.
    """
    filepath = _template_path(name, category, template_dir)
    if not os.path.isfile(filepath):
        return None

    with open(filepath, "r") as fh:
        data = json.load(fh)

    return {
        "version": data.get("_version", 0),
        "name": data.get("_name", name),
        "category": data.get("_category", category),
    }


def import_templates_from_directory(source_dir, template_dir=None):
    """Bulk-import ``.json`` files from *source_dir* into the template store.

    Each JSON file must contain a ``_category`` key (defaulting to
    ``"ship"``).  The file's stem is used as the template name.

    Args:
        source_dir: Directory to scan for ``.json`` files.
        template_dir: Optional override for the root template directory.

    Returns:
        List of ``(name, category)`` tuples that were imported.
    """
    if not os.path.isdir(source_dir):
        return []

    imported = []
    for fname in sorted(os.listdir(source_dir)):
        if not fname.endswith(".json"):
            continue

        src_path = os.path.join(source_dir, fname)
        try:
            with open(src_path, "r") as fh:
                data = json.load(fh)
        except (json.JSONDecodeError, OSError):
            continue

        name = data.get("_name", fname[:-5])
        category = data.get("_category", "ship")
        if category not in TEMPLATE_CATEGORIES:
            category = "ship"

        save_template(name, data, category=category, template_dir=template_dir)
        imported.append((name, category))

    return imported


# ---------------------------------------------------------------------------
# Registration
# ---------------------------------------------------------------------------

def register():
    """Register this module."""
    pass


def unregister():
    """Unregister this module."""
    pass
