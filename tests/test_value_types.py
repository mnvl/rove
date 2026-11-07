# SPDX-License-Identifier: MIT
# Copyright (c) 2004-2026 Manvel Avetisian

"""What every value type promises alike: equality by value, hashing with it, pickling at any protocol, copying and a faithful repr."""
import copy
import pickle

import numpy as np
import pytest

import pyrove


def _quat(cls):
    return cls(0.0, 0.0, 0.0, 1.0)


def _slerper(make, quat):
    s = make()
    s.setup(quat(0.0, 0.0, 0.0, 1.0), quat(0.0, 0.5, 0.0, 0.5))
    return s


# One instance of each type; the same call twice makes two equal values
VALUES = {
    "vec2": lambda: pyrove.vec2(1, 2),
    "vec3": lambda: pyrove.vec3(1, 2, 3),
    "vec4": lambda: pyrove.vec4(1, 2, 3, 4),
    "vec3d": lambda: pyrove.vec3d(1, 2, 3),
    "mat3": lambda: pyrove.mat3.from_numpy(np.eye(3, dtype=np.float32) * 2),
    "mat4": lambda: pyrove.translation_matrix(pyrove.vec3(1, 2, 3)),
    "quat": lambda: _quat(pyrove.quat),
    "quatd": lambda: _quat(pyrove.quatd),
    "trs": lambda: pyrove.trs(pyrove.vec3(1, 2, 3), _quat(pyrove.quat), pyrove.vec3(1, 1, 1)),
    "trsd": lambda: pyrove.trsd(pyrove.vec3d(1, 2, 3), _quat(pyrove.quatd), pyrove.vec3d(1, 1, 1)),
    "ray3": lambda: pyrove.ray3(pyrove.vec3(0, 0, 0), pyrove.vec3(1, 0, 0)),
    "segment3": lambda: pyrove.segment3(pyrove.vec3(0, 0, 0), pyrove.vec3(1, 0, 0)),
    "plane": lambda: pyrove.plane(pyrove.vec3(0, 0, 0), pyrove.vec3(0, 1, 0)),
    "triangle3": lambda: pyrove.triangle3(pyrove.vec3(0, 0, 0), pyrove.vec3(1, 0, 0), pyrove.vec3(0, 1, 0)),
    "capsule3": lambda: pyrove.capsule3(pyrove.vec3(0, 0, 0), pyrove.vec3(1, 0, 0), 0.5),
    "sphere3": lambda: pyrove.sphere3(pyrove.vec3(0, 0, 0), 2),
    "aabb3": lambda: pyrove.aabb3(pyrove.vec3(0, 0, 0), pyrove.vec3(1, 1, 1)),
    "obb3": lambda: pyrove.obb3(pyrove.vec3(0, 0, 0), pyrove.vec3(1, 0, 0), pyrove.vec3(0, 1, 0), pyrove.vec3(0, 0, 1)),
    "frustum": lambda: pyrove.frustum(pyrove.perspective_matrix(1.0, 1.0, 1.0, 10.0)),
    "pcg32": lambda: pyrove.pcg32(5),
    "quat_slerper": lambda: _slerper(pyrove.quat_slerper, pyrove.quat),
}

# Each type with something else of the same type to compare with
DIFFERENT = {
    "vec3": lambda: pyrove.vec3(1, 2, 4),
    "mat4": lambda: pyrove.translation_matrix(pyrove.vec3(1, 2, 4)),
    "quat": lambda: pyrove.quat(0.0, 1.0, 0.0, 0.0),
    "trs": lambda: pyrove.trs(pyrove.vec3(9, 2, 3), _quat(pyrove.quat), pyrove.vec3(1, 1, 1)),
    "ray3": lambda: pyrove.ray3(pyrove.vec3(0, 0, 0), pyrove.vec3(0, 1, 0)),
    "segment3": lambda: pyrove.segment3(pyrove.vec3(0, 0, 0), pyrove.vec3(2, 0, 0)),
    "plane": lambda: pyrove.plane(pyrove.vec3(0, 0, 0), pyrove.vec3(1, 0, 0)),
    "capsule3": lambda: pyrove.capsule3(pyrove.vec3(0, 0, 0), pyrove.vec3(1, 0, 0), 0.75),
    "sphere3": lambda: pyrove.sphere3(pyrove.vec3(0, 0, 0), 3),
    "aabb3": lambda: pyrove.aabb3(pyrove.vec3(0, 0, 0), pyrove.vec3(2, 1, 1)),
    "pcg32": lambda: pyrove.pcg32(6),
}


@pytest.mark.parametrize("name", sorted(VALUES))
def test_equal_by_value_and_hashed_with_it(name):
    a, b = VALUES[name](), VALUES[name]()
    assert a == b
    assert not (a != b)
    assert hash(a) == hash(b)
    assert len({a, b}) == 1


@pytest.mark.parametrize("name", sorted(DIFFERENT))
def test_not_equal_when_different(name):
    assert VALUES[name]() != DIFFERENT[name]()


@pytest.mark.parametrize("name", sorted(VALUES))
def test_comparing_with_another_type_is_false_not_an_error(name):
    a = VALUES[name]()
    assert not (a == None)  # noqa: E711
    assert a != None  # noqa: E711
    assert not (a == "text")
    assert a in [None, a]


@pytest.mark.parametrize("name", sorted(VALUES))
@pytest.mark.parametrize("protocol", range(pickle.HIGHEST_PROTOCOL + 1))
def test_pickled_at_any_protocol(name, protocol):
    a = VALUES[name]()
    assert pickle.loads(pickle.dumps(a, protocol)) == a


@pytest.mark.parametrize("name", sorted(VALUES))
def test_copied(name):
    a = VALUES[name]()
    assert copy.copy(a) == a
    assert copy.deepcopy(a) == a
    assert copy.copy(a) is not a


def test_a_generator_after_a_copy_draws_the_same():
    a = pyrove.pcg32(7)
    a.next()
    b = pickle.loads(pickle.dumps(a))
    assert [a.next() for _ in range(5)] == [b.next() for _ in range(5)]


def test_a_slerper_keeps_working_after_a_pickle():
    a = VALUES["quat_slerper"]()
    b = pickle.loads(pickle.dumps(a))
    assert a.interpolate(0.3) == b.interpolate(0.3)


@pytest.mark.parametrize("name", ["vec3", "quat", "mat4", "aabb3", "ray3", "sphere3", "trs", "frustum"])
def test_a_short_state_raises_instead_of_reading_past_it(name):
    a = VALUES[name]()
    cls = type(a)
    with pytest.raises(ValueError):
        cls.__new__(cls).__setstate__(())
    state = a.__getstate__()
    with pytest.raises(ValueError):
        cls.__new__(cls).__setstate__(state[:-1])


def test_an_octree_that_is_not_built_pickles_and_comes_back_usable():
    tree = pyrove.OctreeTriangle3(pyrove.aabb3(pyrove.vec3(-1, -1, -1), pyrove.vec3(1, 1, 1)))
    tree.insert(pyrove.triangle3(pyrove.vec3(0, 0, 0), pyrove.vec3(0.1, 0, 0), pyrove.vec3(0, 0.1, 0)), 3)
    back = pickle.loads(pickle.dumps(tree))
    assert back.query_aabb(pyrove.aabb3(pyrove.vec3(-1, -1, -1), pyrove.vec3(1, 1, 1))) == [3]


# --- repr ---------------------------------------------------------------------

@pytest.mark.parametrize("value", [0.1, 1 / 3, 1e-9, 1e20, -2.5, 16777217.0, 1.17549435e-38, 3.4028235e38, 0.0])
def test_a_repr_reads_back_as_the_same_number(value):
    for vec, width in ((pyrove.vec2d, float), (pyrove.vec2, lambda v: float(np.float32(v)))):
        printed = repr(vec(value, 0.0))
        x = float(printed[printed.index("(") + 1:printed.index(",")])
        assert width(x) == width(value), printed


def test_a_repr_says_what_is_not_a_number():
    assert repr(pyrove.vec2(float("inf"), float("nan"))) == "vec2(inf, nan)"
    assert repr(pyrove.vec2d(float("-inf"), 0.0)) == "vec2d(-inf, 0)"


def test_a_repr_tells_a_double_from_a_float_and_keeps_its_digits():
    assert repr(pyrove.vec3d(0.1, 1 / 3, 1e-9)) == f"vec3d(0.1, {1 / 3!r}, 1e-09)"
    assert repr(pyrove.vec3(0.5, 0.1, 2)) == "vec3(0.5, 0.1, 2)"
    assert repr(pyrove.quatd(0.0, 0.0, 0.0, 1.0)).startswith("quatd(")
    assert repr(pyrove.mat4d()).startswith("mat4d(")
    assert repr(pyrove.mat3d()).startswith("mat3d(")
    assert repr(pyrove.sphere3d(pyrove.vec3d(0, 0, 0), 1)).startswith("sphere3d(")
    assert repr(pyrove.ray3d(pyrove.vec3d(0, 0, 0), pyrove.vec3d(1 / 3, 0, 0))).startswith("ray3d(")
    assert repr(1 / 3) in repr(pyrove.ray3d(pyrove.vec3d(0, 0, 0), pyrove.vec3d(1 / 3, 0, 0)))
    assert repr(pyrove.planed(pyrove.vec3d(0, 0, 0), pyrove.vec3d(0, 1, 0))).startswith("planed(")


@pytest.mark.parametrize("make,name", [
    (lambda: pyrove.pcg32(1), "pcg32"),
    (lambda: pyrove.OctreeTriangle3(pyrove.aabb3(pyrove.vec3(-1, -1, -1), pyrove.vec3(1, 1, 1))), "OctreeTriangle3"),
])
def test_no_type_has_the_default_object_repr(make, name):
    text = repr(make())
    assert text.startswith(name + "(")
    assert " object at 0x" not in text


# --- numpy --------------------------------------------------------------------

def test_a_numpy_scalar_multiplies_as_a_float_does():
    v = pyrove.vec3(1, 2, 3)
    assert isinstance(np.float64(2) * v, pyrove.vec3)
    assert np.float64(2) * v == 2.0 * v
    assert v * np.float32(2) == 2.0 * v


def test_asarray_of_a_vector_or_a_matrix_is_its_numbers():
    assert np.array_equal(np.asarray(pyrove.vec3(1, 2, 3)), [1, 2, 3])
    assert np.asarray(pyrove.vec3(1, 2, 3)).dtype == np.float32
    assert np.asarray(pyrove.vec3d(1, 2, 3)).dtype == np.float64
    m = pyrove.translation_matrix(pyrove.vec3(1, 2, 3))
    assert np.asarray(m).shape == (4, 4)
    assert np.array_equal(np.asarray(m), m.to_numpy())
    assert np.asarray(pyrove.vec3(1, 2, 3), dtype=np.float64).dtype == np.float64


def test_from_numpy_takes_a_read_only_array():
    frozen = np.array([1, 2, 3], dtype=np.float32)
    frozen.setflags(write=False)
    assert pyrove.vec3.from_numpy(frozen) == pyrove.vec3(1, 2, 3)
    eye = np.broadcast_to(np.eye(4, dtype=np.float32), (4, 4))
    assert pyrove.mat4.from_numpy(eye) == pyrove.mat4.from_numpy(np.eye(4, dtype=np.float32))


# --- what is checked ----------------------------------------------------------

@pytest.mark.parametrize("cls", [pyrove.mat4, pyrove.mat4d])
@pytest.mark.parametrize("method", ["column", "projected_column", "projected_row"])
def test_an_index_past_the_matrix_raises(cls, method):
    m = cls()
    m.identity()
    getattr(m, method)(0)
    getattr(m, method)(3)
    for bad in (4, 10 ** 8, -1):
        with pytest.raises((IndexError, TypeError)):
            getattr(m, method)(bad)


def test_an_obb_vertex_past_its_eight_raises():
    box = VALUES["obb3"]()
    box.vertex(7)
    with pytest.raises(IndexError):
        box.vertex(8)
    with pytest.raises(IndexError):
        box.vertex(100)


def test_a_singular_matrix_raises_one_error_both_ways():
    singular = pyrove.mat4()
    singular.zero()
    for call in (singular.inverse, lambda: pyrove.inverse(singular)):
        with pytest.raises(ValueError):
            call()
    # (it used to be a RuntimeError by method: whoever caught that still does)
    with pytest.raises(RuntimeError):
        singular.inverse()


# --- what the README promises -------------------------------------------------

def test_first_contact_of_two_spheres():
    hit = pyrove.first_contact(pyrove.sphere3(pyrove.vec3(0, 0, 0), 1), pyrove.sphere3(pyrove.vec3(5, 0, 0), 1), pyrove.vec3(-5, 0, 0))
    assert hit["time"] == pytest.approx(0.6)
    assert pyrove.first_contact(pyrove.sphere3(pyrove.vec3(0, 0, 0), 1), pyrove.sphere3(pyrove.vec3(5, 0, 0), 1), pyrove.vec3(0, 5, 0)) is None


@pytest.mark.parametrize("vec,scalar", [(pyrove.vec3, float), (pyrove.vec3d, float)])
def test_the_free_functions_of_a_vector(vec, scalar):
    a, b = vec(3, 4, 0), vec(0, 0, 0)
    assert pyrove.length(a) == 5
    assert pyrove.length_sq(a) == 25
    assert pyrove.distance(a, b) == 5
    assert pyrove.distance_sq(a, b) == 25
    assert pyrove.lerp(a, b, 0.5) == vec(1.5, 2, 0)


def test_quaternion_free_functions():
    q = pyrove.axis_angle(pyrove.vec3(0, 0, 2), 3.14159265 / 2)
    assert pyrove.conjugate(q) == pyrove.quat(-q.x, -q.y, -q.z, q.w)
    assert q.z == pytest.approx(q.w)


def test_a_component_can_be_set_by_index():
    v = pyrove.vec3(1, 2, 3)
    v[0] = 5.0
    v[-1] = 7.0
    assert v == pyrove.vec3(5, 2, 7)
    with pytest.raises(IndexError):
        v[3] = 1.0
    with pytest.raises(IndexError):
        pyrove.vec2(1, 2)[2] = 1.0
