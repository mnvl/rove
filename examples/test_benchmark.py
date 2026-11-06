# SPDX-License-Identifier: MIT
# Copyright (c) 2004-2026 Manvel Avetisian

"""
Contract tests for benchmark.py: what it times must be the same maths in every library, and anything else must stop it.

Not collected with tests/ on purpose: these need every library compared (eigenpy, minieigen), which pyrove's own CI does not install.
A library that is missing fails here rather than skipping.

    pytest -n 4 --timeout=20 examples/test_benchmark.py
"""

import copy
import math
import os
import subprocess
import sys
import textwrap

import numpy as np
import pytest

import benchmark

pytestmark = pytest.mark.timeout(20)

SCRIPT = os.path.join(os.path.dirname(os.path.abspath(__file__)), "benchmark.py")
INP = benchmark.make_inputs()


# --- The answers, worked out without any of the libraries' linear algebra ---------------------------------------------------------

def py_matmul(a, b):
    return [[sum(a[i][k] * b[k][j] for k in range(len(b))) for j in range(len(b[0]))] for i in range(len(a))]


def py_matvec(a, v):
    return [sum(a[i][k] * v[k] for k in range(len(v))) for i in range(len(a))]


def py_transpose(a):
    return [list(row) for row in zip(*a)]


def py_det(a):
    if len(a) == 1:
        return a[0][0]
    return sum((-1) ** j * a[0][j] * py_det([row[:j] + row[j + 1:] for row in a[1:]]) for j in range(len(a)))


def py_inverse(a):
    """Gauss-Jordan with partial pivoting."""
    n = len(a)
    m = [list(row) + [float(i == j) for j in range(n)] for i, row in enumerate(a)]
    for c in range(n):
        pivot = max(range(c, n), key=lambda i: abs(m[i][c]))
        m[c], m[pivot] = m[pivot], m[c]
        m[c] = [x / m[c][c] for x in m[c]]
        for i in range(n):
            if i != c:
                m[i] = [x - m[i][c] * y for x, y in zip(m[i], m[c])]
    return [row[n:] for row in m]


def py_cross(a, b):
    return [a[1] * b[2] - a[2] * b[1], a[2] * b[0] - a[0] * b[2], a[0] * b[1] - a[1] * b[0]]


def py_dot(a, b):
    return sum(x * y for x, y in zip(a, b))


def py_hamilton(p, q):
    """Hamilton product of quaternions as (x, y, z, w), by its multiplication table i^2 = j^2 = k^2 = ijk = -1."""
    basis = {  # (index of p's component, index of q's component) -> (sign, index of the result's component); order x y z w
        (3, 3): (1, 3), (3, 0): (1, 0), (3, 1): (1, 1), (3, 2): (1, 2),
        (0, 3): (1, 0), (1, 3): (1, 1), (2, 3): (1, 2),
        (0, 0): (-1, 3), (1, 1): (-1, 3), (2, 2): (-1, 3),
        (0, 1): (1, 2), (1, 0): (-1, 2),
        (1, 2): (1, 0), (2, 1): (-1, 0),
        (2, 0): (1, 1), (0, 2): (-1, 1),
    }
    out = [0.0, 0.0, 0.0, 0.0]
    for (i, j), (sign, k) in basis.items():
        out[k] += sign * p[i] * q[j]
    return out


def py_conjugate(q):
    return [-q[0], -q[1], -q[2], q[3]]


def py_rotate(q, v):
    """The sandwich q v q*, for a unit q."""
    return py_hamilton(py_hamilton(q, [v[0], v[1], v[2], 0.0]), py_conjugate(q))[:3]


def py_rotation_matrix(q):
    """Each column is what q does to a basis vector."""
    return py_transpose([py_rotate(q, e) for e in ([1.0, 0.0, 0.0], [0.0, 1.0, 0.0], [0.0, 0.0, 1.0])])


def py_slerp(p, q, t):
    if py_dot(p, q) < 0:
        q = [-x for x in q]
    # p (p* q)^t: a turn of t times the angle of the relative rotation, about its axis
    rel = py_hamilton(py_conjugate(p), q)
    half = math.atan2(math.sqrt(py_dot(rel[:3], rel[:3])), rel[3])
    s = math.sin(t * half) / math.sin(half)
    return py_hamilton(p, [rel[0] * s, rel[1] * s, rel[2] * s, math.cos(t * half)])


def py_heron(p0, p1, p2):
    lengths = [math.dist(p0, p1), math.dist(p1, p2), math.dist(p2, p0)]
    s = sum(lengths) / 2
    return math.sqrt(s * (s - lengths[0]) * (s - lengths[1]) * (s - lengths[2]))


def py_line_point_distance(o, d, p):
    """|(p - o) x d| / |d|: not the projection the libraries are composed from."""
    w = [x - y for x, y in zip(p, o)]
    c = py_cross(w, d)
    return math.sqrt(py_dot(c, c) / py_dot(d, d))


def expected_values():
    i = INP
    a, b, k = i["a"], i["b"], i["k"]
    length = math.sqrt(py_dot(a, a))
    unit_a = [x / length for x in a]
    add = [x + y for x, y in zip(a, b)]
    return {
        "vec3 create": [1.5, 2.5, 3.5],
        "vec3 add": add,
        "vec3 scale": [x * k for x in a],
        "vec3 dot": py_dot(a, b),
        "vec3 cross": py_cross(a, b),
        "vec3 length": length,
        "vec3 normalized": unit_a,
        "vec3 add in place": add,
        "vec3 normalize in place": unit_a,
        "mat3 mul": py_matmul(i["A3"], i["B3"]),
        "mat3 * vec3": py_matvec(i["A3"], i["u"]),
        "mat3 transpose": py_transpose(i["A3"]),
        "mat3 inverse": py_inverse(i["A3"]),
        "mat3 determinant": py_det(i["A3"]),
        "mat4 mul": py_matmul(i["A4"], i["B4"]),
        "mat4 * vec4": py_matvec(i["A4"], i["w4"]),
        "mat4 transpose": py_transpose(i["A4"]),
        "mat4 inverse": py_inverse(i["A4"]),
        "mat4 determinant": py_det(i["A4"]),
        "quat create": [0.1, 0.2, 0.3, 0.9],
        "quat compose": py_hamilton(i["q"], i["r"]),
        "quat rotate vec3": py_rotate(i["q"], i["u"]),
        "quat normalized": i["q"],
        "quat inverse": py_conjugate(i["q"]),
        "quat to 3x3 matrix": py_rotation_matrix(i["q"]),
        "quat slerp": py_slerp(i["q"], i["r"], i["t"]),
        "triangle area": py_heron(i["t0"], i["t1"], i["t2"]),
        "ray-point distance": py_line_point_distance(i["o"], i["d"], i["p"]),
        "vec3 to ndarray": a,
        "ndarray to vec3": a,
        "mat4 to ndarray": i["A4"],
        "ndarray to mat4": i["A4"],
        "1000 x vec3 add": [[x + y for x, y in zip(p, q)] for p, q in zip(i["P"], i["Q"])],
        "1000 x mat3 * vec3": [py_matvec(i["A3"], p) for p in i["P"]],
    }


EXPECTED = expected_values()


@pytest.fixture(scope="module")
def workers():
    return {lib: benchmark.run_worker(lib, budget_s=0.0, samples=1) for lib in benchmark.LIBRARIES}


# --- The maths: every library's statement against the worked-out answer ---------------------------------------------------------------

def test_every_operation_has_a_worked_out_answer():
    assert {op.name for op in benchmark.OPS} == set(EXPECTED)


@pytest.mark.parametrize("op", benchmark.OPS, ids=lambda op: op.name)
def test_every_library_computes_the_worked_out_answer(workers, op):
    want = np.array(EXPECTED[op.name], dtype=float)
    for lib in op.impls:
        got = np.array(workers[lib]["ops"][op.name]["value"], dtype=float)
        assert got.shape == want.shape, f"{lib} {op.name}: shape {got.shape}, want {want.shape}"
        assert np.allclose(got, want, rtol=1e-9, atol=1e-12), f"{lib} {op.name}: {got} != {want}"


def test_the_workers_report_exactly_the_operations_they_implement(workers):
    for lib in benchmark.LIBRARIES:
        assert set(workers[lib]["ops"]) == {op.name for op in benchmark.OPS if lib in op.impls}


def test_times_are_positive_numbers(workers):
    for lib, payload in workers.items():
        for name, row in payload["ops"].items():
            assert row["ns"] > 0, f"{lib} {name}"


def test_a_comparison_has_someone_to_compare_with():
    for op in benchmark.OPS:
        assert "pyrove" in op.impls, op.name
        assert len(op.impls) >= 2, op.name


def test_each_library_absent_from_an_operation_has_a_stated_reason():
    for op in benchmark.OPS:
        for lib in set(benchmark.LIBRARIES) - set(op.impls):
            assert op.absent_because(lib).strip(), f"{lib} {op.name}"


def test_the_gaps_are_the_real_ones():
    by_name = {op.name: op for op in benchmark.OPS}
    # NumPy has no quaternion; eigenpy's vectors and matrix products are NumPy again, which would be timing NumPy twice
    assert "numpy" not in by_name["quat compose"].impls
    assert "eigenpy" not in by_name["vec3 add"].impls
    assert "eigenpy" not in by_name["mat4 mul"].impls
    # these do reach Eigen through eigenpy
    assert "eigenpy" in by_name["quat compose"].impls
    assert "eigenpy" in by_name["mat4 inverse"].impls


# --- The inputs: they must be able to tell a wrong answer from a right one -------------------------------------------------------------

def test_inputs_are_the_same_every_time():
    assert benchmark.make_inputs() == benchmark.make_inputs()


@pytest.mark.parametrize("name", ["3", "4"])
def test_matrices_do_not_commute(name):
    a, b = INP["A" + name], INP["B" + name]
    assert not np.allclose(py_matmul(a, b), py_matmul(b, a), rtol=1e-3)


@pytest.mark.parametrize("name", ["A3", "B3", "A4", "B4"])
def test_matrices_are_neither_symmetric_nor_affine_nor_badly_conditioned(name):
    a = np.array(INP[name])
    assert not np.allclose(a, a.T, rtol=1e-3)
    assert not np.allclose(a[-1, :-1], 0.0)
    assert np.linalg.cond(a) < 100


def test_quaternions_are_unit_and_on_opposite_sides():
    q, r = INP["q"], INP["r"]
    assert math.isclose(py_dot(q, q), 1.0, rel_tol=1e-12)
    assert math.isclose(py_dot(r, r), 1.0, rel_tol=1e-12)
    # slerp's shorter arc only does anything if the dot product is negative
    assert py_dot(q, r) < -0.05


def test_quaternions_do_not_commute():
    assert not np.allclose(py_hamilton(INP["q"], INP["r"]), py_hamilton(INP["r"], INP["q"]), rtol=1e-3)


def test_ray_direction_is_not_unit_and_the_point_is_off_the_line():
    assert abs(py_dot(INP["d"], INP["d"]) - 1.0) > 0.1
    assert py_line_point_distance(INP["o"], INP["d"], INP["p"]) > 0.1


def test_the_point_is_behind_the_ray_origin():
    # rove's ray::distance is to the whole line: a point behind the origin tells a line from a clamped ray
    w = [x - y for x, y in zip(INP["p"], INP["o"])]
    assert py_dot(w, INP["d"]) < 0


def test_triangle_is_not_degenerate():
    assert py_heron(INP["t0"], INP["t1"], INP["t2"]) > 0.05


def test_batches_hold_a_thousand_different_vectors():
    assert len(INP["P"]) == len(INP["Q"]) == 1000
    assert len({tuple(p) for p in INP["P"]}) == 1000


# --- verify_agreement: the check that stands between a wrong statement and a table ------------------------------------------------------

@pytest.fixture
def agreeing(workers):
    return copy.deepcopy({lib: workers[lib] for lib in ("pyrove", "numpy")})


def test_agreeing_libraries_pass(agreeing):
    benchmark.verify_agreement(agreeing)


def test_noise_at_the_last_bits_passes(agreeing):
    row = agreeing["numpy"]["ops"]["mat4 inverse"]
    row["value"] = (np.array(row["value"]) * (1 + 1e-13)).tolist()
    benchmark.verify_agreement(agreeing)


def test_swapped_operands_are_caught(agreeing):
    agreeing["numpy"]["ops"]["mat4 mul"]["value"] = py_matmul(INP["B4"], INP["A4"])
    with pytest.raises(benchmark.Disagreement, match="mat4 mul"):
        benchmark.verify_agreement(agreeing)


def test_a_small_real_difference_is_caught(agreeing):
    row = agreeing["numpy"]["ops"]["vec3 length"]
    row["value"] = row["value"] * (1 + 1e-6)
    with pytest.raises(benchmark.Disagreement, match="vec3 length"):
        benchmark.verify_agreement(agreeing)


def test_a_different_shape_is_caught(agreeing):
    # a mat4 rotation against a 3x3 one, say
    row = agreeing["numpy"]["ops"]["mat3 mul"]
    row["value"] = np.eye(4).tolist()
    with pytest.raises(benchmark.Disagreement, match="mat3 mul"):
        benchmark.verify_agreement(agreeing)


def test_nan_everywhere_is_not_agreement(agreeing):
    for payload in agreeing.values():
        payload["ops"]["vec3 add"]["value"] = [float("nan")] * 3
    with pytest.raises(benchmark.Disagreement, match="vec3 add"):
        benchmark.verify_agreement(agreeing)


def test_a_single_library_answering_nan_is_caught_even_with_nobody_to_compare_with(workers):
    payload = copy.deepcopy(workers["pyrove"])
    payload["ops"]["vec3 add"]["value"] = [float("inf")] * 3
    with pytest.raises(benchmark.Disagreement, match="vec3 add"):
        benchmark.verify_agreement({"pyrove": payload})


def test_an_operation_gone_missing_is_an_error_not_a_gap(agreeing):
    del agreeing["numpy"]["ops"]["vec3 cross"]
    with pytest.raises(benchmark.Disagreement, match="vec3 cross"):
        benchmark.verify_agreement(agreeing)


def test_every_disagreement_is_reported_not_just_the_first(agreeing):
    agreeing["numpy"]["ops"]["vec3 add"]["value"] = [0.0, 0.0, 0.0]
    agreeing["numpy"]["ops"]["vec3 dot"]["value"] = 12345.0
    with pytest.raises(benchmark.Disagreement) as caught:
        benchmark.verify_agreement(agreeing)
    assert "vec3 add" in str(caught.value) and "vec3 dot" in str(caught.value)


# --- The clock --------------------------------------------------------------------------------------------------------------------------

def test_measure_tells_slow_from_fast():
    fast = benchmark.measure("1", {}, budget_s=0.002, samples=3)
    slow = benchmark.measure("sum(range(2000))", {}, budget_s=0.002, samples=3)
    assert slow > 10 * fast > 0


def test_measure_reads_about_the_right_time_per_call():
    # time.sleep(0.001) cannot take less than a millisecond, and a per-call figure mistaken for a total would be far off
    ns = benchmark.measure("sleep(0.001)", {"sleep": __import__("time").sleep}, budget_s=0.002, samples=1)
    assert 1e6 <= ns < 5e6


def test_measure_runs_the_statement_in_the_namespace_given():
    ns = {"x": 0}
    benchmark.measure("x += 1", ns, budget_s=0.0, samples=2)
    assert ns["x"] >= 2


def test_measure_lets_a_broken_statement_raise():
    with pytest.raises(ZeroDivisionError):
        benchmark.measure("1 / 0", {}, budget_s=0.0, samples=1)


def test_in_place_operations_are_checked_on_one_step(workers):
    # the timed loop runs += thousands of times; the value checked must be of one step on fresh data, or nothing would match
    assert np.allclose(workers["pyrove"]["ops"]["vec3 add in place"]["value"], EXPECTED["vec3 add in place"])


def test_collect_takes_each_operations_fastest_round_as_noise_only_ever_adds(workers, monkeypatch):
    # what another process's load does: pyrove's middle round and numpy's first are slowed, by different amounts
    times = {"pyrove": iter([100.0, 900.0, 80.0]), "numpy": iter([500.0, 300.0, 310.0])}

    def fake_worker(lib, budget_s, samples):
        payload = copy.deepcopy(workers[lib])
        ns = next(times[lib])
        for row in payload["ops"].values():
            row["ns"] = ns
        return payload

    monkeypatch.setattr(benchmark, "run_worker", fake_worker)
    results = benchmark.collect(["pyrove", "numpy"], rounds=3, budget_s=0.0, samples=1)
    assert results["pyrove"]["ops"]["vec3 add"]["ns"] == 80.0
    assert results["numpy"]["ops"]["vec3 add"]["ns"] == 300.0
    # and what the noise was is not hidden
    assert results["pyrove"]["ops"]["vec3 add"]["spread"] == pytest.approx(900.0 / 80.0)


def test_collect_checks_every_round_not_just_the_first(workers, monkeypatch):
    rounds = iter([0, 0, 1, 1])  # two libraries a round: the second round's numpy goes wrong

    def fake_worker(lib, budget_s, samples):
        payload = copy.deepcopy(workers[lib])
        if next(rounds) == 1 and lib == "numpy":
            payload["ops"]["vec3 add"]["value"] = [0.0, 0.0, 0.0]
        return payload

    monkeypatch.setattr(benchmark, "run_worker", fake_worker)
    with pytest.raises(benchmark.Disagreement, match="vec3 add"):
        benchmark.collect(["pyrove", "numpy"], rounds=2, budget_s=0.0, samples=1)


# --- The report ---------------------------------------------------------------------------------------------------------------------

def synthetic(rows):
    """{op name: {lib: ns}} to the shape collect() gives."""
    libs = {lib for times in rows.values() for lib in times}
    return {lib: {"version": "1", "ops": {n: {"ns": t[lib]} for n, t in rows.items() if lib in t}} for lib in libs}


def test_summary_is_a_geometric_mean_of_the_shared_operations():
    a, b = benchmark.OPS[0].name, benchmark.OPS[1].name
    assert benchmark.OPS[0].section == benchmark.OPS[1].section
    # pyrove 2x and 8x faster: geometric mean 4, where an arithmetic one would flatter it with 5
    results = synthetic({a: {"pyrove": 10.0, "numpy": 20.0}, b: {"pyrove": 10.0, "numpy": 80.0}})
    row = benchmark.summarize(results)[benchmark.OPS[0].section]["numpy"]
    assert row.shared == 2 and row.pyrove_wins == 2
    assert row.ratio == pytest.approx(4.0)


def test_summary_counts_losses_and_stays_below_one_when_pyrove_is_slower():
    a, b = benchmark.OPS[0].name, benchmark.OPS[1].name
    results = synthetic({a: {"pyrove": 40.0, "numpy": 10.0}, b: {"pyrove": 40.0, "numpy": 40.0}})
    row = benchmark.summarize(results)[benchmark.OPS[0].section]["numpy"]
    assert row.pyrove_wins == 0
    assert row.ratio == pytest.approx(0.5)


def test_summary_leaves_out_operations_a_library_does_not_have():
    a, b = benchmark.OPS[0].name, benchmark.OPS[1].name
    results = synthetic({a: {"pyrove": 10.0, "numpy": 20.0}, b: {"pyrove": 10.0}})
    row = benchmark.summarize(results)[benchmark.OPS[0].section]["numpy"]
    assert row.shared == 1
    assert row.ratio == pytest.approx(2.0)


@pytest.mark.parametrize("bad", [0.0, -1.0, float("nan")])
def test_summary_refuses_a_time_that_is_no_time(bad):
    a = benchmark.OPS[0].name
    with pytest.raises(ValueError):
        benchmark.summarize(synthetic({a: {"pyrove": 10.0, "numpy": bad}}))


def test_table_marks_the_fastest_and_shows_no_invented_numbers():
    q = next(op for op in benchmark.OPS if op.name == "quat compose")
    # NumPy is among the libraries compared, with a time for something else and none for this
    results = synthetic({q.name: {"pyrove": 70.0, "eigenpy": 190.0, "minieigen": 220.0}, "vec3 add": {"pyrove": 60.0, "numpy": 300.0}})
    text = benchmark.format_table(results)
    row = next(line for line in text.splitlines() if q.name in line)
    assert "70 ns*" in row
    assert "190 ns*" not in row
    # NumPy has no quaternion: a dash, never a 0
    assert " - " in row or row.rstrip().endswith("-")
    assert " 0 ns" not in row


def test_table_says_how_pyrove_fares_against_the_best_other_library():
    a = benchmark.OPS[0].name
    assert "2.0x faster" in benchmark.format_table(synthetic({a: {"pyrove": 10.0, "numpy": 20.0, "minieigen": 90.0}}))
    assert "4.0x slower" in benchmark.format_table(synthetic({a: {"pyrove": 40.0, "numpy": 10.0, "minieigen": 90.0}}))


def test_a_difference_within_the_noise_is_a_tie_in_the_table_whoever_is_ahead():
    a = benchmark.OPS[0].name
    assert "tie" in benchmark.format_table(synthetic({a: {"pyrove": 100.0, "numpy": 103.0}}))
    assert "tie" in benchmark.format_table(synthetic({a: {"pyrove": 103.0, "numpy": 100.0}}))
    assert "tie" not in benchmark.format_table(synthetic({a: {"pyrove": 100.0, "numpy": 110.0}}))


def test_a_tie_is_no_win():
    a, b = benchmark.OPS[0].name, benchmark.OPS[1].name
    results = synthetic({a: {"pyrove": 100.0, "numpy": 103.0}, b: {"pyrove": 100.0, "numpy": 200.0}})
    assert benchmark.summarize(results)[benchmark.OPS[0].section]["numpy"].pyrove_wins == 1


def test_summary_does_not_round_a_heavy_loss_to_nothing():
    a = benchmark.OPS[0].name
    text = benchmark.format_summary(synthetic({a: {"pyrove": 100.0, "numpy": 2.0}}))
    assert "0.02x" in text
    assert "0.0x" not in text


def test_table_has_a_unit_that_fits():
    a = benchmark.OPS[0].name
    text = benchmark.format_table(synthetic({a: {"pyrove": 1_500.0, "numpy": 2_500_000.0}}))
    assert "1.5 µs" in text and "2.5 ms" in text


# --- The command line, end to end ---------------------------------------------------------------------------------------------------

def cli(*args, extra_path=None):
    env = dict(os.environ)
    if extra_path:
        env["PYTHONPATH"] = str(extra_path) + os.pathsep + env.get("PYTHONPATH", "")
    return subprocess.run([sys.executable, SCRIPT, "--rounds", "1", "--samples", "1", "--budget-ms", "0", *args],
                          capture_output=True, text=True, env=env, timeout=18)


def test_all_four_libraries_run_together_without_trampling_each_other():
    out = cli()
    assert out.returncode == 0, out.stderr
    # in one process, eigenpy and minieigen's Boost.Python converters collide with exactly this warning, and one Quaternion breaks
    assert "already registered" not in out.stderr
    assert "Traceback" not in out.stderr
    for lib in benchmark.LIBRARIES:
        assert lib in out.stdout
    for op in benchmark.OPS:
        assert op.name in out.stdout


def test_workers_are_isolated_from_each_other(workers):
    for lib, payload in workers.items():
        assert payload["foreign"] == [], f"{lib} worker loaded {payload['foreign']}"


def test_a_library_that_is_missing_stops_the_benchmark(tmp_path):
    (tmp_path / "eigenpy.py").write_text("raise ImportError('not here')\n")
    out = cli(extra_path=tmp_path)
    assert out.returncode != 0
    assert "eigenpy" in out.stderr and "install" in out.stderr.lower()
    assert "vec3 add" not in out.stdout


def test_leaving_a_library_out_takes_asking(tmp_path):
    (tmp_path / "eigenpy.py").write_text("raise ImportError('not here')\n")
    out = cli("--libs", "pyrove,numpy", extra_path=tmp_path)
    assert out.returncode == 0, out.stderr
    assert "eigenpy" not in out.stdout and "minieigen" not in out.stdout
    assert "vec3 add" in out.stdout


def test_an_unknown_library_is_an_error():
    out = cli("--libs", "pyrove,glm")
    assert out.returncode != 0
    assert "glm" in out.stderr


def test_a_comparison_needs_pyrove_in_it():
    out = cli("--libs", "numpy,minieigen")
    assert out.returncode != 0
    assert "pyrove" in out.stderr


def test_a_library_answering_wrongly_stops_the_benchmark_and_names_the_operation(tmp_path):
    # An eigenpy that composes quaternions in the wrong order, and otherwise gets as far as being called
    (tmp_path / "eigenpy.py").write_text(textwrap.dedent("""
        import numpy as np
        __version__ = "stub"

        def hamilton(p, q):
            return (p[3]*q[0] + p[0]*q[3] + p[1]*q[2] - p[2]*q[1],
                    p[3]*q[1] - p[0]*q[2] + p[1]*q[3] + p[2]*q[0],
                    p[3]*q[2] + p[0]*q[1] - p[1]*q[0] + p[2]*q[3],
                    p[3]*q[3] - p[0]*q[0] - p[1]*q[1] - p[2]*q[2])

        class Quaternion:
            def __init__(self, w, x, y, z): self.c = (x, y, z, w)
            def coeffs(self): return np.array(self.c)
            def __mul__(self, o):
                if isinstance(o, Quaternion):
                    x, y, z, w = hamilton(o.c, self.c)  # the wrong way round
                    return Quaternion(w, x, y, z)
                return o
            def normalized(self): return self
            def inverse(self): return self
            def matrix(self): return np.eye(3)
            def slerp(self, t, o): return self

        class PartialPivLU:
            def __init__(self, a): self.a = a
            def inverse(self): return np.linalg.inv(self.a)
            def determinant(self): return np.linalg.det(self.a)
    """))
    out = cli("--libs", "pyrove,eigenpy", extra_path=tmp_path)
    assert out.returncode != 0
    assert "quat compose" in out.stderr
    assert "quat compose" not in out.stdout


def test_a_worker_that_loads_another_compared_library_stops_the_benchmark(tmp_path):
    (tmp_path / "eigenpy.py").write_text("import pyrove\nraise ImportError('stop here')\n")
    # (loading it is enough; the stub's import error is what ends this one)
    assert cli("--libs", "pyrove,eigenpy", extra_path=tmp_path).returncode != 0

    (tmp_path / "eigenpy.py").write_text(textwrap.dedent("""
        import pyrove
        import numpy as np
        __version__ = "stub"
        class Quaternion:
            def __init__(self, w, x, y, z): self.q = pyrove.quatd(x, y, z, w)
            def coeffs(self): return self.q.to_numpy()
            def _wrap(self, q):
                out = Quaternion(1.0, 0.0, 0.0, 0.0)
                out.q = q
                return out
            def __mul__(self, o):
                if isinstance(o, Quaternion):
                    return self._wrap(self.q * o.q)
                return np.array(list(self.q * pyrove.vec3d(*o)))
            def normalized(self): return self._wrap(self.q.normalized())
            def inverse(self): return self._wrap(pyrove.inverse(self.q))
            def matrix(self): return pyrove.rotation_matrix(self.q).to_numpy()[:3, :3]
            def slerp(self, t, o): return self._wrap(pyrove.slerp(self.q, o.q, t))
        class PartialPivLU:
            def __init__(self, a): self.a = a
            def inverse(self): return np.linalg.inv(self.a)
            def determinant(self): return np.linalg.det(self.a)
    """))
    out = cli("--libs", "pyrove,eigenpy", extra_path=tmp_path)
    assert out.returncode != 0
    assert "pyrove" in out.stderr and "eigenpy" in out.stderr
