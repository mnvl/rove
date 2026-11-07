# SPDX-License-Identifier: MIT
# Copyright (c) 2004-2026 Manvel Avetisian

"""rove's Python bindings: vectors, matrices, quaternions, shapes, collisions, spatial indices and more.

An installed pyrove (pip, or pip install -e) has its compiled module in the package, unless $ROVE_PYTHON_BINDINGS_DIR names a directory that
has one.  A source checkout finds the module CMake built: there, else build/lib of the checkout or of the project it is a subdirectory of.
"""
from __future__ import annotations

import importlib as _importlib
import importlib.machinery as _machinery
import os as _os
import re as _re
import sys as _sys
import warnings as _warnings
from pathlib import Path as _Path
from types import ModuleType as _ModuleType

__version__ = "3.0.1"


def _binding_dirs() -> list[_Path]:
    explicit = _os.environ.get("ROVE_PYTHON_BINDINGS_DIR")
    if explicit:
        return [_Path(explicit).expanduser()]

    rove_root = _Path(__file__).resolve().parents[1]   # the checkout
    parent_root = rove_root.parent                    # a project that adds rove as a subdirectory
    return [
        parent_root / "build" / "lib",                # built by that project
        rove_root / "build" / "lib",                  # standalone rove build
        rove_root / "build",
        rove_root / "cmake-build-debug" / "lib",
        rove_root / "cmake-build-release" / "lib",
    ]


def _module_files(bindings_dir: _Path, module_name: str) -> list[_Path]:
    patterns = (
        f"{module_name}*.so",
        f"{module_name}*.pyd",
        f"{module_name}*.dylib",
        f"{module_name}*.dll",
    )
    result: list[_Path] = []
    for pattern in patterns:
        result.extend(sorted(bindings_dir.glob(pattern)))
    return result


def _running_python_tag() -> str:
    """The tag an extension needs to load here.  Not cache_tag: a
    free-threaded interpreter shares that with the GIL build of its version,
    and only the extension suffix says cpython-314t rather than cpython-314."""
    return _abi_tag(_machinery.EXTENSION_SUFFIXES[0])


# An extension's ABI in its file name: cpython-313 / cpython-314t (Linux, macOS) or cp313 / cp314t (Windows: .cp313-win_amd64.pyd)
_ABI = _re.compile(r"(?:cpython-|(?<![a-z])cp)(\d+t?)")


def _abi_tag(name: str) -> str | None:
    match = _ABI.search(name)
    return f"cpython-{match.group(1)}" if match else None


def _guess_python_tags(paths: list[_Path]) -> list[str]:
    tags = set()
    for path in paths:
        tag = _abi_tag(path.name)
        if tag:
            tags.add(tag)
    return sorted(tags)


def _import_required_binding(module_name: str) -> _ModuleType:
    # Installed: the module is part of the package, beside this file (a wheel) or where an editable install's import hook has it.  The
    # variable picks another where it names a directory that has one; one that names nothing is no reason to fail with the right module here.
    explicit = _os.environ.get("ROVE_PYTHON_BINDINGS_DIR")
    if not (explicit and _module_files(_Path(explicit).expanduser(), module_name)):
        try:
            module = _importlib.import_module(f"{__name__}.{module_name}")
        except ModuleNotFoundError:
            pass
        else:
            if explicit:
                _warnings.warn(f"ROVE_PYTHON_BINDINGS_DIR={explicit} has no {module_name}: using the one this pyrove is installed with",
                               RuntimeWarning, stacklevel=2)
            # (the classes' __module__ is pyrove_bind, so a pickle of one names it)
            _sys.modules.setdefault(module_name, module)
            return module

    # A source checkout: the module CMake built (e.g. build/lib/pyrove_bind*.so).
    for candidate in _binding_dirs():
        if candidate.exists() and str(candidate) not in _sys.path:
            _sys.path.insert(0, str(candidate))

    try:
        module = _importlib.import_module(module_name)
        # Keep compatibility with code expecting pyrove.pyrove_bind.
        _sys.modules.setdefault(f"{__name__}.{module_name}", module)
        return module
    except (ModuleNotFoundError, ImportError) as exc:
        extension_suffixes = ", ".join(_machinery.EXTENSION_SUFFIXES)
        built_files: list[_Path] = []
        for candidate in _binding_dirs():
            if candidate.exists():
                built_files.extend(_module_files(candidate, module_name))

        running_tag = _running_python_tag()
        built_tags = _guess_python_tags(built_files)
        mismatch_note = ""
        if built_tags and running_tag not in built_tags:
            mismatch_note = (
                "\nDetected ABI mismatch:\n"
                f"- active interpreter tag: {running_tag}\n"
                f"- built module tags: {', '.join(built_tags)}\n"
            )

        raise ImportError(
            f"Could not import required binding module '{module_name}'.\n"
            "Install pyrove with pip, or build the module first:\n"
            "  cmake -S . -B build -DROVE_PYTHON=ON && cmake --build build\n"
            f"extension_suffixes={extension_suffixes}\n"
            f"module_candidates={[path.name for path in built_files]}"
            f"{mismatch_note}"
        ) from exc


_bindings: _ModuleType = _import_required_binding("pyrove_bind")
pyrove_bind = _bindings

for _name in dir(_bindings):
    if _name.startswith("_"):
        continue
    globals()[_name] = getattr(_bindings, _name)

# abs, min and max are rove's on vectors: pyrove.min(a, b).  A star import would put them over Python's own, so it leaves them out.
_BUILTINS_KEPT = {"abs", "min", "max"}
__all__ = [name for name in dir(_bindings) if not name.startswith("_") and name not in _BUILTINS_KEPT]

# (`from __future__ import annotations` above leaves this name in the module)
del annotations


def __dir__() -> list[str]:
    """The API, without the loader's own names."""
    return sorted(set(__all__) | _BUILTINS_KEPT | {"__version__"})
