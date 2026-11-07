# SPDX-License-Identifier: MIT
# Copyright (c) 2004-2026 Manvel Avetisian

"""The README's first Python block is what PyPI shows first: it runs from top to bottom, and what it computes is a number."""
import math
import re
import runpy
import sys
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
README = (ROOT / "README.md").read_text(encoding="utf-8")
BLOCKS = re.findall(r"```python\n(.*?)```", README, flags=re.S)


def test_the_readme_has_python_blocks():
    assert BLOCKS


def test_the_first_readme_block_runs():
    exec(compile(BLOCKS[0], "README.md", "exec"), {"__name__": "readme"})


def test_the_readme_plane_example_hits_the_plane():
    namespace = {}
    exec(compile(BLOCKS[0], "README.md", "exec"), namespace)
    assert math.isfinite(namespace["t"])


def test_the_example_script_runs_and_prints_no_nan(capsys, monkeypatch):
    monkeypatch.setattr(sys, "argv", ["example.py"])
    runpy.run_path(str(ROOT / "examples" / "example.py"), run_name="__main__")
    assert "nan" not in capsys.readouterr().out.lower()


def test_the_description_on_pypi_has_no_relative_links():
    # pypi.org resolves nothing against the repository
    relative = [link for link in re.findall(r"\]\(([^)]+)\)", README) if not re.match(r"https?://|#", link)]
    assert relative == []
