# SPDX-License-Identifier: MIT
# Copyright (c) 2004-2026 Manvel Avetisian

"""pyrove's loader: which built binding it takes for this interpreter."""
from pathlib import Path


# --- which interpreter the binding was built for ------------------------------

def _expected_tag():
    import sys
    import sysconfig
    free_threaded = "t" if sysconfig.get_config_var("Py_GIL_DISABLED") else ""
    return f"cpython-{sys.version_info.major}{sys.version_info.minor}{free_threaded}"


def test_the_binding_is_built_for_this_interpreters_abi():
    """A free-threaded interpreter shares its cache_tag with the GIL build
    of the same version; only the extension's tag tells them apart."""
    import pyrove
    assert pyrove._running_python_tag() == _expected_tag()
    assert pyrove._guess_python_tags([Path(pyrove._bindings.__file__)]) == [_expected_tag()]


def test_a_free_threaded_build_is_told_apart_from_a_gil_build():
    import pyrove
    built = [Path("pyrove_bind.cpython-314t-darwin.so"), Path("pyrove_bind.cpython-314-x86_64-linux-gnu.so")]
    assert pyrove._guess_python_tags(built) == ["cpython-314", "cpython-314t"]


def test_windows_extension_names_carry_the_same_tags():
    import pyrove
    built = [Path("pyrove_bind.cp313-win_amd64.pyd"), Path("pyrove_bind.cp314t-win_amd64.pyd")]
    assert pyrove._guess_python_tags(built) == ["cpython-313", "cpython-314t"]


# --- what the package lists ---------------------------------------------------

def test_dir_lists_the_api_and_not_the_loaders_imports():
    import pyrove
    listed = dir(pyrove)
    assert "vec3" in listed and "dot" in listed and "abs" in listed and "__version__" in listed
    for internal in ("ModuleType", "Path", "annotations", "importlib", "os", "re", "sys", "pyrove_bind"):
        assert internal not in listed


def test_the_package_ships_its_types():
    import pyrove
    package = Path(pyrove.__file__).parent
    assert (package / "py.typed").exists()
    assert (package / "__init__.pyi").exists()
    stub = (package / "pyrove_bind.pyi").read_text(encoding="utf-8")
    # (regenerate with nanobind's stubgen.py when the bindings change: every public name of the module has to be in it)
    missing = [name for name in pyrove.__all__ if f"class {name}" not in stub and f"def {name}(" not in stub and f"{name}:" not in stub]
    assert missing == []


def test_every_public_class_says_what_it_is():
    import inspect
    import pyrove
    missing = [name for name in pyrove.__all__ if inspect.isclass(getattr(pyrove, name)) and not (getattr(pyrove, name).__doc__ or "").strip()]
    assert missing == []


def test_the_version_is_the_projects():
    import re
    import pyrove
    root = Path(__file__).resolve().parents[1]
    text = (root / "pyproject.toml").read_text(encoding="utf-8")
    assert pyrove.__version__ == re.search(r'^version = "(.*)"$', text, flags=re.M).group(1)
