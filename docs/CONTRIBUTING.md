# Contributing to Nova Forge

Thank you for your interest in contributing to Nova Forge! This document provides guidelines for contributing to the project.

## 📋 Table of Contents

- [Code of Conduct](#code-of-conduct)
- [How Can I Contribute?](#how-can-i-contribute)
- [Getting Started](#getting-started)
- [Development Process](#development-process)
- [Style Guidelines](#style-guidelines)
- [Commit Messages](#commit-messages)
- [Pull Request Process](#pull-request-process)

---

## Code of Conduct

This project aims to be welcoming and inclusive. Please be respectful and constructive in all interactions.

---

## How Can I Contribute?

### 🐛 Reporting Bugs

Before creating bug reports, please check the existing issues to avoid duplicates. When creating a bug report, include:

- **Clear title and description**
- **Steps to reproduce** the issue
- **Expected behavior** vs actual behavior
- **Environment details** (OS, C++ compiler version, etc.)
- **Screenshots** if applicable

### 💡 Suggesting Features

Feature suggestions are welcome! Please:

- Check the [roadmap](ROADMAP.md) first
- Search existing issues for similar suggestions
- Provide clear use cases and benefits
- Consider implementation complexity

### 📝 Improving Documentation

Documentation improvements are always appreciated:

- Fix typos or clarify confusing sections
- Add examples or tutorials
- Update outdated information
- Improve organization

### 🎨 Adding Content

The easiest way to contribute is by adding game content:

#### Ships
Add new ship definitions in `data/ships/`:
```json
{
  "ship_id": {
    "name": "Ship Name",
    "class": "Frigate",
    "hull_hp": 500,
    "armor_hp": 400,
    "shield_hp": 600,
    ...
  }
}
```

#### Modules
Add new modules in `data/modules/`:
```json
{
  "module_id": {
    "name": "Module Name",
    "type": "weapon",
    "slot": "high",
    ...
  }
}
```

#### Missions
Add new missions in `data/missions/`:
```json
{
  "mission_id": {
    "name": "Mission Name",
    "level": 1,
    "type": "combat",
    ...
  }
}
```

### 🔧 Code Contributions

See [Priority Areas](#priority-areas) for where help is most needed.

---

## Getting Started

### 1. Fork and Clone

```bash
# Fork the repository on GitHub, then:
git clone https://github.com/YOUR_USERNAME/NovaForge.git
cd NovaForge
```

### 2. Set Up Development Environment

```bash
# Check build dependencies
make check-deps

# Build everything (recommended for first time)
./scripts/build_all.sh

# Run tests to verify setup
make test
```

See [docs/BUILDING.md](BUILDING.md) for full build instructions and
platform-specific details.

### 3. Create a Branch

```bash
git checkout -b feature/your-feature-name
# or
git checkout -b fix/bug-description
```

---

## Development Process

### 1. Make Your Changes

- Follow the [style guidelines](#style-guidelines)
- Add tests for new functionality
- Update documentation as needed
- Keep changes focused and minimal

### 2. Test Your Changes

```bash
# Run all tests (server + engine)
make test

# Run server tests only
make test-server

# Run engine tests only
make test-engine
```

### 3. Update Documentation

- Update relevant docs in `docs/` folder
- Add docstrings to new functions/classes
- Update README.md if needed

---

## Style Guidelines

All C++ code must follow the project's coding standards.  See
**[docs/CODING_GUIDELINES.md](CODING_GUIDELINES.md)** for the full reference.

Key points:

- **Naming**: Match the dominant style of the module you are editing
  (see per-module table in the guidelines).
- **File size**: Keep `.cpp` files under 600 lines; split into domain files
  when they grow beyond 400.
- **Includes**: Order as own header → project → third-party → standard library.
- **No magic numbers**: Use named `constexpr` constants.
- **Logging**: Use the module's Logger — never raw `std::cout`/`std::cerr` in
  `cpp_server/`.
- **Error handling**: No bare `catch (...)`; always log caught exceptions.

### JSON Data Files

Use consistent formatting:

```json
{
  "entity_id": {
    "name": "Entity Name",
    "property": value,
    "nested": {
      "key": "value"
    }
  }
}
```

- Use 2-space indentation
- Use double quotes
- Keep IDs lowercase with underscores
- Group related properties together

### Documentation

- Use Markdown for all documentation
- Include table of contents for long documents
- Use clear, descriptive headings
- Add code examples where helpful
- Keep line length reasonable (80-100 chars)

---

## Commit Messages

Write clear, descriptive commit messages:

### Format

```
Short summary (50 chars or less)

More detailed explanation if needed. Wrap at 72 characters.
Explain what changed and why, not how.

- Bullet points are okay
- Use present tense: "Add feature" not "Added feature"
```

### Examples

Good:
```
Add manufacturing system with blueprint research

Implements ME/TE research, manufacturing queue, and blueprint
copying mechanics. Includes tests and documentation.
```

Bad:
```
Fixed stuff
```

---

## Pull Request Process

### 1. Before Submitting

- [ ] Tests pass: `make test`
- [ ] Code follows [coding guidelines](CODING_GUIDELINES.md)
- [ ] Documentation updated
- [ ] Commit messages are clear
- [ ] Branch is up to date with main

### 2. Submit PR

1. Push your branch to GitHub
2. Create Pull Request from your branch to `main`
3. Fill out the PR template:
   - **Title**: Clear, descriptive summary
   - **Description**: What changes and why
   - **Testing**: How you tested the changes
   - **Related Issues**: Link any related issues

### 3. Review Process

- Maintainers will review your PR
- Address any feedback or requested changes
- Keep discussion focused and constructive
- Be patient - reviews take time

### 4. After Merge

- Delete your feature branch
- Pull latest changes from main
- Celebrate! 🎉

---

## Priority Areas

Help is especially welcome in these areas:

### 🔥 High Priority

1. **Vertical Slice Integration**
   - Wire up server systems to client rendering and UI
   - End-to-end gameplay in one star system

2. **Client Polish & Optimization**
   - Performance profiling (20 Hz server, 60 FPS client with 500+ entities)
   - Network smoothness at 100ms latency
   - Spatial partitioning for rendering

3. **Testing & Bug Reports**
   - Test existing features
   - Report bugs with reproduction steps
   - Add test coverage for untested areas

### 📊 Medium Priority

1. **UI/UX Improvements**
   - Atlas UI polish and theming
   - Better visual feedback
   - HUD improvements

2. **Additional Ships & Content**
   - More ship definitions and variations
   - More modules and variations
   - New missions and storylines

3. **Content Balance Pass**
   - Missions, rewards, difficulty scaling
   - Tuning for solo and co-op play

### 📝 Low Priority

1. **Community Tools**
   - Mission editor improvements
   - Ship designer polish
   - Mod manager UX

2. **Additional Content**
   - More NPC varieties
   - More mission types
   - Exploration sites

3. **Documentation**
   - API reference improvements
   - Tutorial expansion
   - System design documentation

---

## Questions?

- **Documentation**: Check the docs in this folder
- **Issues**: Search or create an issue on GitHub
- **Discussions**: Use GitHub Discussions for questions
- **Roadmap**: See [ROADMAP.md](ROADMAP.md)

---

## Thank You!

Your contributions make Nova Forge better for everyone. Thank you for taking the time to contribute! 🚀

---

*This contributing guide is based on best practices from open source projects.*
