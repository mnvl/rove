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
    # (made by a wheel's build; a checkout's CMake build has none, and its module is not in the package)
    module = Path(pyrove._bindings.__file__)
    if module.parent.name == "pyrove":
        stub = (module.parent / "pyrove_bind.pyi").read_text(encoding="utf-8")
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


# --- the variable and an installed package --------------------------------------------------------------------

def _an_installed_package(tmp_path):
    import shutil
    import pyrove
    package = tmp_path / "installed" / "pyrove"
    package.mkdir(parents=True)
    shutil.copy(Path(pyrove.__file__), package / "__init__.py")
    shutil.copy(Path(pyrove._bindings.__file__), package)
    return package


def _which_binding(tmp_path, binding_dir):
    import os
    import subprocess
    import sys
    env = {**os.environ, "ROVE_PYTHON_BINDINGS_DIR": str(binding_dir), "PYTHONPATH": str(tmp_path / "installed")}
    code = "import pyrove; print(pyrove._bindings.__file__)"
    return subprocess.run([sys.executable, "-W", "always", "-c", code], capture_output=True, text=True, env=env, cwd=tmp_path)


def test_a_variable_that_names_nothing_leaves_an_installed_package_its_own_module(tmp_path):
    package = _an_installed_package(tmp_path)
    (tmp_path / "empty").mkdir()
    for nothing in (tmp_path / "empty", tmp_path / "missing"):
        result = _which_binding(tmp_path, nothing)
        assert result.returncode == 0, result.stderr
        assert Path(result.stdout.strip()).parent == package
        assert "ROVE_PYTHON_BINDINGS_DIR" in result.stderr


def test_a_variable_that_names_a_module_picks_it(tmp_path):
    import shutil
    import pyrove
    _an_installed_package(tmp_path)
    elsewhere = tmp_path / "elsewhere"
    elsewhere.mkdir()
    shutil.copy(Path(pyrove._bindings.__file__), elsewhere)
    result = _which_binding(tmp_path, elsewhere)
    assert result.returncode == 0, result.stderr
    assert Path(result.stdout.strip()).parent == elsewhere


def test_every_place_that_says_the_version_says_the_same():
    import json
    import re
    import pyrove
    root = Path(__file__).resolve().parents[1]
    in_cmake = re.search(r"project\(rove VERSION ([0-9.]+)", (root / "CMakeLists.txt").read_text(encoding="utf-8")).group(1)
    in_port = json.loads((root / "ports" / "rove" / "vcpkg.json").read_text(encoding="utf-8"))["version"]
    in_changelog = re.search(r"^## \[([0-9.]+)\]", (root / "CHANGELOG.md").read_text(encoding="utf-8"), flags=re.M).group(1)
    assert {in_cmake, in_port, in_changelog, pyrove.__version__} == {in_cmake}


# --- help() ---------------------------------------------------------------------------------------------------

def test_help_of_the_package_lists_every_function_and_overload():
    import pydoc
    import pyrove
    text = pydoc.render_doc(pyrove, renderer=pydoc.plaintext)
    assert "dot(a: vec2, b: vec2) -> float" in text
    assert "dot(a: vec3d, b: vec3d) -> float" in text
    assert "normalize(v: vec3) -> vec3" in text
    # every function the package has, with no internal names or placeholders of nanobind's
    for name in pyrove.__all__:
        if hasattr(getattr(pyrove, name), "__nb_signature__"):
            assert f"{name}(" in text, name
    assert "pyrove_bind." not in pyrove.__doc__
    assert "\\" not in pyrove.__doc__


def test_the_package_keeps_its_own_description():
    import pyrove
    assert pyrove.__doc__.startswith("rove's Python bindings")
