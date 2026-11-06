#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
# Copyright (c) 2004-2026 Manvel Avetisian

"""
What the same small-vector maths costs a Python program in pyrove, NumPy, eigenpy and minieigen.

    pip install numpy eigenpy pyrove
    sudo apt install python3-minieigen      # not on PyPI; on Debian and Ubuntu, for the system's Python
    python3 examples/benchmark.py

A library that cannot be imported stops the run; leave one out by asking, with --libs pyrove,numpy.

What makes the numbers comparable:

- The same maths.  Every statement is run once on the same inputs, in double precision (the only one eigenpy and minieigen have), and
  all libraries must give the same answer before a time is printed.  The matrices do not commute, the quaternions are on opposite
  sides of the sphere: an operation done the wrong way round fails instead of being timed.
- What is checked is what is timed: both run the one statement written below.
- Each library's fastest form that was found, however it reads.  NumPy gets a.dot(b) rather than a @ b, sqrt(a.dot(a)) rather than
  np.linalg.norm(a), a cross product written out over tolist() rather than np.cross (over 20 times faster), and A.T, which is a view
  and no copy.  pyrove gets m.inverse() rather than pyrove.inverse(m), minieigen fromiter() rather than np.array().  Names are in the
  namespace directly, so nobody pays for attribute lookups on a module.
- A process each.  eigenpy and minieigen are both Boost.Python: in one process their converters for the same Eigen types collide
  ("to-Python converter ... already registered; second conversion method ignored") and one library's Quaternion stops working.
- Each library as it is distributed, compiled however its packagers compile it.  That is what a Python user gets, but it means these
  are not comparisons of Eigen's and rove's arithmetic (eigenpy's wheel, for one, reports only SSE and SSE2).

A time is the fastest of --samples runs of about --budget-ms each, over --rounds fresh processes, in a different order each round: what
else the machine is doing can only add to a time, so the fastest is the one to trust, and the largest difference between rounds is
printed with the results (a compile in the background doubled some times here, and left the fastest alone).  It includes timeit's loop
and the lookup of the operands, the same for everyone.  Within 5% is a tie: that much changes between runs, and a tie is no win.

Not compared: single precision (pyrove's vec3 and NumPy's float32 have it, the other two do not).

Run the checks with: pytest -n 4 --timeout=20 examples/test_benchmark.py
"""

import argparse
import json
import math
import platform
import random
import statistics
import subprocess
import sys
import timeit
from dataclasses import dataclass, field
from typing import Callable, Optional

import numpy as np

LIBRARIES = ("pyrove", "numpy", "eigenpy", "minieigen")

INSTALL_HINT = {
    "pyrove": "pip install pyrove (or from this checkout: pip install .)",
    "numpy": "pip install numpy",
    "eigenpy": "pip install eigenpy",
    "minieigen": "sudo apt install python3-minieigen (Debian and Ubuntu; it is not on PyPI, and a venv needs --system-site-packages)",
}

BATCH = 1000

TIE = 1.05  # a ratio of times this close to 1 is noise: quat slerp changes hands between runs


class Disagreement(Exception):
    """Libraries' answers to the same operation differ, or are not answers at all."""


class WorkerFailed(Exception):
    """A library's process could not import it, or an operation raised."""


class IsolationBroken(Exception):
    """A library's process also holds another library under comparison."""


# --- Inputs: plain lists, so no library's types are involved in making them ------------------------------------------------------------

def make_inputs():
    rng = np.random.default_rng(20261105)

    def vec(n=3):
        return rng.uniform(-1, 1, n)

    def unit_quaternion():
        q = rng.normal(size=4)
        return q / np.linalg.norm(q)

    q, r = unit_quaternion(), unit_quaternion()
    if q.dot(r) > 0:  # on opposite sides of the sphere: slerp has to take the shorter arc
        r = -r

    o, d = vec(), 2.5 * vec()
    p = o - 1.3 * d + np.cross(d, vec())  # behind the origin: distances to a line and to a clamped ray differ here

    return {
        "a": vec().tolist(),
        "b": vec().tolist(),
        "k": 2.5,
        "u": vec().tolist(),
        "w4": vec(4).tolist(),
        "A3": vec(9).reshape(3, 3).tolist(),
        "B3": vec(9).reshape(3, 3).tolist(),
        "A4": vec(16).reshape(4, 4).tolist(),
        "B4": vec(16).reshape(4, 4).tolist(),
        "q": q.tolist(),  # x, y, z, w
        "r": r.tolist(),
        "t": 0.3,
        "o": o.tolist(),
        "d": d.tolist(),
        "p": p.tolist(),
        "t0": vec().tolist(),
        "t1": vec().tolist(),
        "t2": vec().tolist(),
        "P": rng.uniform(-1, 1, (BATCH, 3)).tolist(),
        "Q": rng.uniform(-1, 1, (BATCH, 3)).tolist(),
    }


# --- Each library's objects for those inputs, under the same names ---------------------------------------------------------------------
#
# A library is imported only here, in its own process.

def pyrove_namespace(i):
    import pyrove

    V3, M3, M4, Q = pyrove.vec3d, pyrove.mat3d, pyrove.mat4d, pyrove.quatd
    return {
        "Vec3": V3, "Mat4": M4, "Quat": Q,
        "inverse": pyrove.inverse, "slerp": pyrove.slerp, "rotation_matrix": pyrove.rotation_matrix, "rotate": pyrove.rotate,
        "a": V3(*i["a"]), "b": V3(*i["b"]), "k": i["k"], "x": V3(*i["a"]), "u": V3(*i["u"]),
        "w4": pyrove.vec4d(*i["w4"]),
        "A3": M3.from_numpy(np.array(i["A3"])), "B3": M3.from_numpy(np.array(i["B3"])),
        "A4": M4.from_numpy(np.array(i["A4"])), "B4": M4.from_numpy(np.array(i["B4"])),
        "q": Q(*i["q"]), "r": Q(*i["r"]), "t": i["t"],
        "ray": pyrove.ray3d(V3(*i["o"]), V3(*i["d"])), "p": V3(*i["p"]),
        "triangle": pyrove.triangle3d(V3(*i["t0"]), V3(*i["t1"]), V3(*i["t2"])),
        "P": [V3(*v) for v in i["P"]], "Q": [V3(*v) for v in i["Q"]],
        "arr3": np.array(i["a"]), "arr44": np.array(i["A4"]),
    }


def numpy_namespace(i):
    names = {
        "array": np.array, "cross": np.cross, "inv": np.linalg.inv, "det": np.linalg.det, "sqrt": math.sqrt,
        "k": i["k"], "x": np.array(i["a"]),
    }
    for name in ("a", "b", "u", "w4", "A3", "B3", "A4", "B4", "o", "d", "p", "t0", "t1", "t2", "P", "Q"):
        names[name] = np.array(i[name])
    return names


def wxyz(q):
    x, y, z, w = q
    return w, x, y, z


def eigenpy_namespace(i):
    import eigenpy

    return {
        "Quat": eigenpy.Quaternion, "LU": eigenpy.PartialPivLU,
        "q": eigenpy.Quaternion(*wxyz(i["q"])), "r": eigenpy.Quaternion(*wxyz(i["r"])), "t": i["t"],
        "u": np.array(i["u"]), "A3": np.array(i["A3"]), "A4": np.array(i["A4"]),
    }


def minieigen_namespace(i):
    import minieigen

    V3, M3, MX, Q = minieigen.Vector3, minieigen.Matrix3, minieigen.MatrixX, minieigen.Quaternion
    return {
        "Vec3": V3, "MatX": MX, "Quat": Q, "array": np.array, "fromiter": np.fromiter, "sqrt": math.sqrt,
        "a": V3(*i["a"]), "b": V3(*i["b"]), "k": i["k"], "x": V3(*i["a"]), "u": V3(*i["u"]),
        "w4": minieigen.VectorX(i["w4"]),
        "A3": M3(*np.ravel(i["A3"]).tolist()), "B3": M3(*np.ravel(i["B3"]).tolist()),
        "A4": MX(i["A4"]), "B4": MX(i["B4"]),
        "q": Q(*wxyz(i["q"])), "r": Q(*wxyz(i["r"])), "t": i["t"],
        "o": V3(*i["o"]), "d": V3(*i["d"]), "p": V3(*i["p"]),
        "t0": V3(*i["t0"]), "t1": V3(*i["t1"]), "t2": V3(*i["t2"]),
        "P": [V3(*v) for v in i["P"]], "Q": [V3(*v) for v in i["Q"]],
        "arr3": np.array(i["a"]), "arr44": np.array(i["A4"]),
    }


NAMESPACES = {
    "pyrove": pyrove_namespace,
    "numpy": numpy_namespace,
    "eigenpy": eigenpy_namespace,
    "minieigen": minieigen_namespace,
}


def to_array(value):
    """A library's answer as an ndarray of floats; quaternions as x, y, z, w."""
    if isinstance(value, list):
        return np.array([to_array(item) for item in value])
    if hasattr(value, "to_numpy"):  # pyrove
        return np.asarray(value.to_numpy(), dtype=float)
    if hasattr(value, "coeffs"):  # eigenpy's Quaternion
        return np.asarray(value.coeffs(), dtype=float)
    return np.asarray(value, dtype=float)  # NumPy's and Python's own, and minieigen's, which are sequences


# --- The operations --------------------------------------------------------------------------------------------------------------------

class Absent(str):
    """Why a library has no statement for an operation."""


@dataclass(frozen=True)
class Impl:
    stmt: str
    result: Optional[str] = None  # what to read the answer from, for a statement that has no value: the operand it changed
    post: Optional[Callable] = None  # what to do to the answer so that it can be compared; not timed


@dataclass(frozen=True)
class Op:
    section: str
    name: str
    impls: dict = field(hash=False)
    absent: dict = field(hash=False)

    def absent_because(self, lib):
        return self.absent[lib]


def op(section, name, *, pyrove, numpy, eigenpy, minieigen):
    """Every library has to be spoken for: a statement or the reason there is none."""
    impls, absent = {}, {}
    for lib, what in (("pyrove", pyrove), ("numpy", numpy), ("eigenpy", eigenpy), ("minieigen", minieigen)):
        if isinstance(what, Absent):
            absent[lib] = str(what)
        else:
            impls[lib] = what if isinstance(what, Impl) else Impl(what)
    return Op(section, name, impls, absent)


def upper_3x3(array):
    return array[:3, :3]


NUMPY_AGAIN = Absent("eigenpy's vectors and matrices are NumPy arrays: it would be timing NumPy again")
NO_EIGEN_GEOMETRY = Absent("eigenpy has no vector type to compose this from: it would be timing NumPy again")
ALREADY_NDARRAY = Absent("already an ndarray")
NO_QUATERNION = Absent("NumPy has no quaternion type")

# NumPy's fastest cross products: np.cross takes over 10 µs, and this over Python's floats about half of one
NUMPY_CROSS = "array(((v := a.tolist())[1] * (w := b.tolist())[2] - v[2] * w[1], v[2] * w[0] - v[0] * w[2], v[0] * w[1] - v[1] * w[0]))"
NUMPY_TRIANGLE_AREA = ("0.5 * sqrt(((v := (t1 - t0).tolist())[1] * (w := (t2 - t0).tolist())[2] - v[2] * w[1]) ** 2"
                       " + (v[2] * w[0] - v[0] * w[2]) ** 2 + (v[0] * w[1] - v[1] * w[0]) ** 2)")

# The distance from a line by Pythagoras: no vector made for the projection, fewer calls for those who pay for each
LINE_POINT_DISTANCE = "sqrt((w := p - o).dot(w) - w.dot(d) ** 2 / d.dot(d))"

# minieigen's fastest MatrixX to ndarray: array(A4) goes through the sequence protocol and takes about 18 µs, this about 3
MINIEIGEN_MAT4_TO_ARRAY = "array((" + ", ".join("(" + ", ".join(f"A4[{r}, {c}]" for c in range(4)) + ")" for r in range(4)) + "))"

VEC3 = "vec3 (double)"
MAT3 = "mat3 (double)"
MAT4 = "mat4 (double)"
QUAT = "quaternion (double)"
GEOMETRY = "geometry (composed where a library has no such call)"
CONVERT = "to and from NumPy"
BATCHES = "a batch of 1000 (a Python loop, but for NumPy)"

OPS = [
    op(VEC3, "vec3 create", pyrove="Vec3(1.5, 2.5, 3.5)", numpy="array((1.5, 2.5, 3.5))", eigenpy=NUMPY_AGAIN,
       minieigen="Vec3(1.5, 2.5, 3.5)"),
    op(VEC3, "vec3 add", pyrove="a + b", numpy="a + b", eigenpy=NUMPY_AGAIN, minieigen="a + b"),
    op(VEC3, "vec3 scale", pyrove="a * k", numpy="a * k", eigenpy=NUMPY_AGAIN, minieigen="a * k"),
    op(VEC3, "vec3 dot", pyrove="a.dot(b)", numpy="a.dot(b)", eigenpy=NUMPY_AGAIN, minieigen="a.dot(b)"),
    op(VEC3, "vec3 cross", pyrove="a.cross(b)", numpy=NUMPY_CROSS, eigenpy=NUMPY_AGAIN, minieigen="a.cross(b)"),
    op(VEC3, "vec3 length", pyrove="a.length()", numpy="sqrt(a.dot(a))", eigenpy=NUMPY_AGAIN, minieigen="a.norm()"),
    # (pyrove's a / a.length() measures faster than its normalize(a))
    op(VEC3, "vec3 normalized", pyrove="a / a.length()", numpy="a / sqrt(a.dot(a))", eigenpy=NUMPY_AGAIN, minieigen="a.normalized()"),
    op(VEC3, "vec3 add in place", pyrove=Impl("x += b", result="x"), numpy=Impl("x += b", result="x"), eigenpy=NUMPY_AGAIN,
       minieigen=Impl("x += b", result="x")),
    op(VEC3, "vec3 normalize in place", pyrove=Impl("x.normalize()", result="x"), numpy=Impl("x /= sqrt(x.dot(x))", result="x"),
       eigenpy=NUMPY_AGAIN, minieigen=Impl("x.normalize()", result="x")),

    op(MAT3, "mat3 mul", pyrove="A3 * B3", numpy="A3.dot(B3)", eigenpy=NUMPY_AGAIN, minieigen="A3 * B3"),
    op(MAT3, "mat3 * vec3", pyrove="A3 * u", numpy="A3.dot(u)", eigenpy=NUMPY_AGAIN, minieigen="A3 * u"),
    op(MAT3, "mat3 transpose", pyrove="A3.transpose()", numpy="A3.T", eigenpy=NUMPY_AGAIN, minieigen="A3.transpose()"),
    op(MAT3, "mat3 inverse", pyrove="A3.inverse()", numpy="inv(A3)", eigenpy="LU(A3).inverse()", minieigen="A3.inverse()"),
    op(MAT3, "mat3 determinant", pyrove="A3.determinant()", numpy="det(A3)", eigenpy="LU(A3).determinant()", minieigen="A3.determinant()"),

    op(MAT4, "mat4 mul", pyrove="A4 * B4", numpy="A4.dot(B4)", eigenpy=NUMPY_AGAIN, minieigen="A4 * B4"),
    op(MAT4, "mat4 * vec4", pyrove="A4 * w4", numpy="A4.dot(w4)", eigenpy=NUMPY_AGAIN, minieigen="A4 * w4"),
    op(MAT4, "mat4 transpose", pyrove="A4.transpose()", numpy="A4.T", eigenpy=NUMPY_AGAIN, minieigen="A4.transpose()"),
    op(MAT4, "mat4 inverse", pyrove="A4.inverse()", numpy="inv(A4)", eigenpy="LU(A4).inverse()", minieigen="A4.inverse()"),
    op(MAT4, "mat4 determinant", pyrove="A4.determinant()", numpy="det(A4)", eigenpy="LU(A4).determinant()", minieigen="A4.determinant()"),

    op(QUAT, "quat create", pyrove="Quat(0.1, 0.2, 0.3, 0.9)", numpy=NO_QUATERNION, eigenpy="Quat(0.9, 0.1, 0.2, 0.3)",
       minieigen="Quat(0.9, 0.1, 0.2, 0.3)"),
    op(QUAT, "quat compose", pyrove="q * r", numpy=NO_QUATERNION, eigenpy="q * r", minieigen="q * r"),
    op(QUAT, "quat rotate vec3", pyrove="rotate(q, u)", numpy=NO_QUATERNION, eigenpy="q * u", minieigen="q * u"),
    op(QUAT, "quat normalized", pyrove="q.normalized()", numpy=NO_QUATERNION, eigenpy="q.normalized()", minieigen="q.normalized()"),
    op(QUAT, "quat inverse", pyrove="inverse(q)", numpy=NO_QUATERNION, eigenpy="q.inverse()", minieigen="q.inverse()"),
    # (pyrove's call gives a 4x4, which is more to fill and to box than the others' 3x3: it is not flattered)
    op(QUAT, "quat to 3x3 matrix", pyrove=Impl("rotation_matrix(q)", post=upper_3x3), numpy=NO_QUATERNION, eigenpy="q.matrix()",
       minieigen="q.toRotationMatrix()"),
    op(QUAT, "quat slerp", pyrove="slerp(q, r, t)", numpy=NO_QUATERNION, eigenpy="q.slerp(t, r)", minieigen="q.slerp(t, r)"),

    op(GEOMETRY, "triangle area", pyrove="triangle.area()", numpy=NUMPY_TRIANGLE_AREA,
       eigenpy=NO_EIGEN_GEOMETRY, minieigen="0.5 * (t1 - t0).cross(t2 - t0).norm()"),
    op(GEOMETRY, "ray-point distance", pyrove="ray.distance(p)", numpy=LINE_POINT_DISTANCE,
       eigenpy=NO_EIGEN_GEOMETRY, minieigen=LINE_POINT_DISTANCE),

    op(CONVERT, "vec3 to ndarray", pyrove="a.to_numpy()", numpy=ALREADY_NDARRAY, eigenpy=ALREADY_NDARRAY, minieigen="fromiter(a, float, 3)"),
    op(CONVERT, "ndarray to vec3", pyrove="Vec3.from_numpy(arr3)", numpy=ALREADY_NDARRAY, eigenpy=ALREADY_NDARRAY,
       minieigen="Vec3(*arr3.tolist())"),
    op(CONVERT, "mat4 to ndarray", pyrove="A4.to_numpy()", numpy=ALREADY_NDARRAY, eigenpy=ALREADY_NDARRAY,
       minieigen=MINIEIGEN_MAT4_TO_ARRAY),
    op(CONVERT, "ndarray to mat4", pyrove="Mat4.from_numpy(arr44)", numpy=ALREADY_NDARRAY, eigenpy=ALREADY_NDARRAY,
       minieigen="MatX(arr44.tolist())"),

    op(BATCHES, "1000 x vec3 add", pyrove="[v + w for v, w in zip(P, Q)]", numpy="P + Q", eigenpy=NUMPY_AGAIN,
       minieigen="[v + w for v, w in zip(P, Q)]"),
    op(BATCHES, "1000 x mat3 * vec3", pyrove="[A3 * v for v in P]", numpy="P.dot(A3.T)", eigenpy=NUMPY_AGAIN,
       minieigen="[A3 * v for v in P]"),
]


# --- The clock -------------------------------------------------------------------------------------------------------------------------

def measure(stmt, namespace, budget_s, samples):
    """Nanoseconds per run of stmt: the fastest of `samples` runs of about budget_s each.  stmt may assign to the namespace's names."""
    # timeit compiles stmt into a function, where an assignment to x would make x local and unbound
    setup = "global " + ", ".join(namespace) if namespace else "pass"
    timer = timeit.Timer(stmt, setup=setup, globals=namespace)

    number = 1
    elapsed = timer.timeit(number)
    while elapsed < budget_s / 10:
        number *= 2
        elapsed = timer.timeit(number)
    number = max(1, round(number * budget_s / elapsed))

    return min(timer.timeit(number) for _ in range(samples)) / number * 1e9


# --- One library's process -------------------------------------------------------------------------------------------------------------

def run_ops(lib, budget_s, samples):
    """Each operation of lib: its answer, on fresh operands, then its time.  What is checked is what is timed: impl.stmt, both."""
    build = NAMESPACES[lib]
    inputs = make_inputs()
    ops = {}
    for operation in OPS:
        impl = operation.impls.get(lib)
        if impl is None:
            continue

        names = build(inputs)
        if impl.result is None:
            value = eval(impl.stmt, names)
        else:
            exec(impl.stmt, names)
            value = eval(impl.result, names)
        array = to_array(value)
        if impl.post is not None:
            array = impl.post(array)

        ops[operation.name] = {"value": array.tolist(), "ns": measure(impl.stmt, build(inputs), budget_s, samples)}

    library = sys.modules[lib]
    return {
        # (minieigen reports no version)
        "version": getattr(library, "__version__", "version not reported"),
        "ops": ops,
        "foreign": [other for other in LIBRARIES if other not in (lib, "numpy") and other in sys.modules],
    }


def worker_main(lib, budget_s, samples):
    try:
        __import__(lib)
    except ImportError as e:
        print(f"could not import {lib}: {e}\nInstall it with: {INSTALL_HINT[lib]}", file=sys.stderr)
        return 3
    json.dump(run_ops(lib, budget_s, samples), sys.stdout)
    return 0


def run_worker(lib, budget_s, samples):
    """lib's run_ops in a process of its own."""
    if lib not in LIBRARIES:
        raise WorkerFailed(f"no such library: {lib}")
    process = subprocess.run(
        [sys.executable, __file__, "--worker", lib, "--budget-ms", repr(budget_s * 1e3), "--samples", str(samples)],
        capture_output=True, text=True)
    if process.stderr:
        print(process.stderr, file=sys.stderr, end="")
    if process.returncode != 0:
        raise WorkerFailed(f"{lib}: its process exited with {process.returncode} (see above)")
    return json.loads(process.stdout)


# --- Whether the answers are the same --------------------------------------------------------------------------------------------------

def verify_agreement(results):
    """Disagreement, naming every operation at fault, unless all of results' libraries give the same finite answer to each of theirs."""
    problems = []
    for operation in OPS:
        reference_lib, reference = None, None
        for lib in results:
            if lib not in operation.impls:
                continue
            row = results[lib]["ops"].get(operation.name)
            if row is None:
                problems.append(f"{operation.name}: {lib} gave no answer")
                continue
            value = np.array(row["value"], dtype=float)
            if not np.all(np.isfinite(value)):
                problems.append(f"{operation.name}: {lib} answered {value.tolist()}")
                continue
            if reference is None:
                reference_lib, reference = lib, value
            elif value.shape != reference.shape:
                problems.append(f"{operation.name}: {lib}'s answer has shape {value.shape}, {reference_lib}'s {reference.shape}")
            elif not np.allclose(value, reference, rtol=1e-9, atol=1e-12):
                problems.append(f"{operation.name}: {lib} answered {value.tolist()}, {reference_lib} {reference.tolist()}")
    if problems:
        raise Disagreement("not the same maths, so no times:\n  " + "\n  ".join(problems))


def verify_isolation(results):
    problems = [f"{lib}'s process also loaded {', '.join(payload['foreign'])}" for lib, payload in results.items() if payload["foreign"]]
    if problems:
        raise IsolationBroken("; ".join(problems))


def collect(libs, rounds, budget_s, samples):
    """{lib: {version, foreign, ops: {name: {value, ns, spread}}}}, every round verified; ns the fastest round's, each in a new process."""
    shuffler = random.Random(20261105)
    runs = {lib: [] for lib in libs}
    for _ in range(rounds):
        order = list(libs)
        shuffler.shuffle(order)
        this_round = {lib: run_worker(lib, budget_s, samples) for lib in order}
        verify_isolation(this_round)
        verify_agreement(this_round)
        for lib, payload in this_round.items():
            runs[lib].append(payload)

    results = {}
    for lib in libs:
        first = runs[lib][0]
        results[lib] = {"version": first["version"], "foreign": first["foreign"], "ops": {}}
        for name, row in first["ops"].items():
            times = [run["ops"][name]["ns"] for run in runs[lib]]
            results[lib]["ops"][name] = {"value": row["value"], "ns": min(times), "spread": max(times) / min(times)}
    return results


# --- The report ------------------------------------------------------------------------------------------------------------------------

@dataclass(frozen=True)
class Summary:
    shared: int
    pyrove_wins: int
    ratio: float  # the geometric mean of the other library's time over pyrove's: above 1, pyrove is the faster


def checked_time(results, lib, name):
    ns = results[lib]["ops"][name]["ns"]
    if not (math.isfinite(ns) and ns > 0):
        raise ValueError(f"{lib} {name}: a time of {ns}")
    return ns


def summarize(results):
    """{section: {other library: Summary}}, over the operations the library shares with pyrove."""
    by_section = {}
    for operation in OPS:
        if operation.name not in results["pyrove"]["ops"]:
            continue
        ours = checked_time(results, "pyrove", operation.name)
        for lib in results:
            if lib != "pyrove" and operation.name in results[lib]["ops"]:
                ratio = checked_time(results, lib, operation.name) / ours
                by_section.setdefault(operation.section, {}).setdefault(lib, []).append(ratio)
    return {
        section: {
            lib: Summary(len(ratios), sum(r >= TIE for r in ratios), math.exp(statistics.fmean(math.log(r) for r in ratios)))
            for lib, ratios in libs.items()
        }
        for section, libs in by_section.items()
    }


def format_duration(ns):
    if ns < 1e3:
        return f"{ns:.0f} ns"
    if ns < 1e6:
        return f"{ns / 1e3:.1f} µs"
    return f"{ns / 1e6:.1f} ms"


def format_ratio(ratio):
    return f"{ratio:.2f}x" if ratio < 1 else f"{ratio:.1f}x"


def format_comparison(ours, best_other):
    if max(ours, best_other) / min(ours, best_other) < TIE:
        return "tie"
    if ours <= best_other:
        return f"{best_other / ours:.1f}x faster"
    return f"{ours / best_other:.1f}x slower"


def format_table(results):
    """A row for each operation pyrove has a time for, each cell a time or a dash; * the fastest."""
    libs = [lib for lib in LIBRARIES if lib in results]
    head = f"{'':26s}" + "".join(f"{lib:>13s}" for lib in libs) + f"{'pyrove vs best other':>23s}"
    lines = []
    section = None
    for operation in OPS:
        if operation.name not in results["pyrove"]["ops"]:
            continue
        if operation.section != section:
            section = operation.section
            lines += ["", section, head]
        times = {lib: results[lib]["ops"][operation.name]["ns"] for lib in libs if operation.name in results[lib]["ops"]}
        fastest = min(times.values())
        cells = "".join(
            f"{format_duration(times[lib]) + ('*' if times[lib] == fastest else ''):>13s}" if lib in times else f"{'-':>13s}"
            for lib in libs)
        others = [t for lib, t in times.items() if lib != "pyrove"]
        comparison = format_comparison(times["pyrove"], min(others)) if others else "-"
        lines.append(f"{operation.name:26s}{cells}{comparison:>23s}")
    return "\n".join(lines)


def format_summary(results):
    libs = [lib for lib in LIBRARIES if lib in results and lib != "pyrove"]
    lines = [
        "Library's time over pyrove's, above 1x where pyrove is faster: the geometric mean over the operations it shares with pyrove "
        "(pyrove's wins of those, ties not counted)",
        f"{'':54s}" + "".join(f"{lib:>16s}" for lib in libs),
    ]
    for section, row in summarize(results).items():
        cells = "".join(
            f"{f'{format_ratio(row[lib].ratio)} ({row[lib].pyrove_wins}/{row[lib].shared})':>16s}" if lib in row else f"{'-':>16s}"
            for lib in libs)
        lines.append(f"{section.split(' (')[0]:54s}{cells}")
    return "\n".join(lines)


def format_notes(libs):
    notes = ["* fastest; a tie is within 5%.  Answers verified equal across libraries before timing.  Each library in its own process, "
             "as distributed."]
    if "numpy" in libs:
        notes.append("NumPy's transpose is A.T, a view: it copies nothing, which the others' transposes do.")
    if "eigenpy" in libs:
        notes.append("eigenpy: real Eigen calls only (quaternions; inverse and determinant by PartialPivLU, whose matrix is dynamic); "
                     "a dash where it would be timing NumPy again.")
    if "minieigen" in libs:
        notes.append("minieigen: 4x4 by dynamically sized MatrixX, its largest fixed matrix being 3x3.")
    return "\n".join(notes)


def format_header(results):
    versions = ", ".join(f"{lib} {results[lib]['version']}" for lib in LIBRARIES if lib in results)
    return f"Python {platform.python_version()} on {platform.machine()}: {versions}"


# --- The command line ------------------------------------------------------------------------------------------------------------------

def parse_libs(parser, text):
    libs = text.split(",")
    unknown = [lib for lib in libs if lib not in LIBRARIES]
    if unknown:
        parser.error(f"no such library: {', '.join(unknown)} (these are compared: {', '.join(LIBRARIES)})")
    if "pyrove" not in libs:
        parser.error("--libs must include pyrove: the comparison is with it")
    if len(libs) < 2:
        parser.error("--libs needs another library to compare with pyrove")
    return libs


def main():
    parser = argparse.ArgumentParser(description="pyrove, NumPy, eigenpy and minieigen on the same small-vector maths")
    parser.add_argument("--libs", default=",".join(LIBRARIES), help="comma-separated, pyrove among them (default: all)")
    parser.add_argument("--rounds", type=int, default=5, help="new processes per library; the fastest counts (default: %(default)s)")
    parser.add_argument("--samples", type=int, default=5, help="timed runs per operation per round; the fastest counts (default: %(default)s)")
    parser.add_argument("--budget-ms", type=float, default=10.0, help="about how long each timed run is (default: %(default)s)")
    parser.add_argument("--worker", choices=LIBRARIES, help=argparse.SUPPRESS)
    args = parser.parse_args()

    if args.worker:
        return worker_main(args.worker, args.budget_ms / 1e3, args.samples)

    libs = parse_libs(parser, args.libs)
    try:
        results = collect(libs, args.rounds, args.budget_ms / 1e3, args.samples)
    except (WorkerFailed, Disagreement, IsolationBroken) as e:
        print(f"benchmark stopped: {e}", file=sys.stderr)
        return 1

    print(format_header(results))
    print(format_table(results))
    print()
    print(format_summary(results))
    print()
    print(format_notes(libs))
    if args.rounds > 1:
        worst = max((row["spread"], lib, name) for lib in libs for name, row in results[lib]["ops"].items())
        print(f"Largest difference between rounds, slowest over fastest: {worst[0]:.2f}x ({worst[1]}, {worst[2]})")
    return 0


if __name__ == "__main__":
    sys.exit(main())
