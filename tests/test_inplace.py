# SPDX-License-Identifier: MIT
# Copyright (c) 2004-2026 Manvel Avetisian

"""
An in-place operator changes the object and hands back that same object, as Python's own mutable types do.

It used to change the object and hand back a copy, which `x += b` then bound to x: anything else holding the object saw that one step
and none after it, without a word.
"""

import sys

import numpy as np
import pytest

import pyrove

VECTORS = [
    (pyrove.vec2, 2), (pyrove.vec3, 3), (pyrove.vec4, 4),
    (pyrove.vec2d, 2), (pyrove.vec3d, 3), (pyrove.vec4d, 4),
]
MATRICES = [(pyrove.mat3, 3), (pyrove.mat4, 4), (pyrove.mat3d, 3), (pyrove.mat4d, 4)]

VECTOR_OPS = {
    "add": lambda x, y: x.__iadd__(y),
    "sub": lambda x, y: x.__isub__(y),
    "mul": lambda x, y: x.__imul__(y),
    "div": lambda x, y: x.__itruediv__(y),
}


def ids(param):
    return param[0].__name__


def vec(cls, n, start):
    return cls(*[float(start + i) for i in range(n)])


def mat(cls, n, seed):
    dtype = np.float64 if cls.__name__.endswith("d") else np.float32
    return cls.from_numpy(np.random.default_rng(seed).uniform(-1, 1, (n, n)).astype(dtype))


def assert_same_product(got, want):
    """
    Two products of mat() matrices, the same to within rounding but not bit for bit.

    The compiler may fuse a * b + c into one multiply-add (GCC does by default wherever the target has the instruction, as on aarch64),
    and does so differently in `*=` and `*`, which are separate code: their answers differ by up to 2 eps here.  pyrove keeps those
    instructions for their speed; the bits are only promised with dfloat.  Entries of mat() are at most 1, so the error is absolute.
    """
    assert np.abs(got - want).max() <= 8 * np.finfo(want.dtype).eps


# --- It is the same object ---------------------------------------------------------------------------------------------------------

@pytest.mark.parametrize("cls,n", VECTORS, ids=[c.__name__ for c, _ in VECTORS])
def test_vector_in_place_operators_keep_the_object(cls, n):
    other = vec(cls, n, 2)
    x = vec(cls, n, 1)
    before = x
    x += other
    assert x is before
    x -= other
    assert x is before
    x *= other
    assert x is before
    x /= other
    assert x is before
    x *= 2.0
    assert x is before
    x /= 2.0
    assert x is before


@pytest.mark.parametrize("cls,n", MATRICES, ids=[c.__name__ for c, _ in MATRICES])
def test_matrix_in_place_operators_keep_the_object(cls, n):
    other = mat(cls, n, 1)
    m = mat(cls, n, 0)
    before = m
    m *= other
    assert m is before
    m *= 2.0
    assert m is before
    m /= 2.0
    assert m is before


# --- What that is for: whoever else holds it keeps seeing it ------------------------------------------------------------------------

@pytest.mark.parametrize("cls,n", VECTORS, ids=[c.__name__ for c, _ in VECTORS])
def test_an_alias_of_a_vector_sees_every_step(cls, n):
    b = cls(*[1.0] * n)
    x = cls(*[0.0] * n)
    y = x
    for _ in range(3):
        x += b
    assert list(y) == [3.0] * n
    assert list(x) == [3.0] * n


@pytest.mark.parametrize("cls,n", MATRICES, ids=[c.__name__ for c, _ in MATRICES])
def test_an_alias_of_a_matrix_sees_every_step(cls, n):
    m = cls()
    alias = m
    for _ in range(3):
        m *= 2.0
    assert np.array_equal(alias.to_numpy(), 8.0 * np.eye(n))
    assert np.array_equal(m.to_numpy(), 8.0 * np.eye(n))


def test_a_vector_held_in_a_list_sees_every_step():
    held = [pyrove.vec3d(0.0, 0.0, 0.0)]
    x = held[0]
    x += pyrove.vec3d(1.0, 1.0, 1.0)
    x += pyrove.vec3d(1.0, 1.0, 1.0)
    assert list(held[0]) == [2.0, 2.0, 2.0]


def test_a_function_changing_its_argument_is_seen_by_the_caller():
    def advance(position, step):
        position += step
        position += step

    p = pyrove.vec3(0.0, 0.0, 0.0)
    advance(p, pyrove.vec3(1.0, 2.0, 3.0))
    assert list(p) == [2.0, 4.0, 6.0]


@pytest.mark.parametrize("make", [
    lambda: pyrove.ray3d(pyrove.vec3d(1.0, 2.0, 3.0), pyrove.vec3d(0.0, 0.0, 1.0)),
], ids=["ray3d"])
def test_in_place_on_a_member_writes_through_to_its_owner(make):
    ray = make()
    ray.r0 += pyrove.vec3d(1.0, 1.0, 1.0)
    ray.r0 += pyrove.vec3d(1.0, 1.0, 1.0)
    assert list(ray.r0) == [3.0, 4.0, 5.0]


def test_in_place_on_a_triangles_vertex_changes_the_triangle():
    t = pyrove.triangle3d(pyrove.vec3d(0.0, 0.0, 0.0), pyrove.vec3d(1.0, 0.0, 0.0), pyrove.vec3d(0.0, 1.0, 0.0))
    t.B *= 2.0
    t.B *= 2.0
    assert t.area() == pytest.approx(2.0)


# --- It still does the arithmetic it did, and to nothing else ----------------------------------------------------------------------

@pytest.mark.parametrize("name", VECTOR_OPS)
@pytest.mark.parametrize("cls,n", VECTORS, ids=[c.__name__ for c, _ in VECTORS])
def test_vector_in_place_agrees_with_the_operator_and_leaves_the_operand_alone(cls, n, name):
    x, y = vec(cls, n, 1), vec(cls, n, 5)
    y_before = list(y)
    out_of_place = {"add": x + y, "sub": x - y, "mul": x * y, "div": x / y}[name]
    VECTOR_OPS[name](x, y)
    assert list(x) == list(out_of_place)
    assert list(y) == y_before


@pytest.mark.parametrize("cls,n", VECTORS, ids=[c.__name__ for c, _ in VECTORS])
def test_vector_in_place_by_a_scalar(cls, n):
    x = vec(cls, n, 1)
    want = list(x * 4.0)
    x *= 4.0
    assert list(x) == want
    want = list(x / 2.0)
    x /= 2.0
    assert list(x) == want


@pytest.mark.parametrize("cls,n", MATRICES, ids=[c.__name__ for c, _ in MATRICES])
def test_matrix_in_place_composes_in_the_order_of_the_operator(cls, n):
    a, b = mat(cls, n, 0), mat(cls, n, 1)
    want = (a * b).to_numpy()
    assert not np.allclose(want, (b * a).to_numpy())  # or this could not tell the orders apart
    b_before = b.to_numpy()
    a *= b
    assert_same_product(a.to_numpy(), want)
    assert np.array_equal(b.to_numpy(), b_before)


# --- Being the same object, an operand can now be the object itself -------------------------------------------------------------------

@pytest.mark.parametrize("cls,n", VECTORS, ids=[c.__name__ for c, _ in VECTORS])
def test_a_vector_operated_on_by_itself(cls, n):
    x = vec(cls, n, 1)
    want = list(x + x)
    x += x
    assert list(x) == want

    x = vec(cls, n, 1)
    want = list(x * x)
    x *= x
    assert list(x) == want

    x = vec(cls, n, 1)
    x -= x
    assert list(x) == [0.0] * n

    x = vec(cls, n, 1)
    x /= x
    assert list(x) == [1.0] * n


@pytest.mark.parametrize("cls,n", MATRICES, ids=[c.__name__ for c, _ in MATRICES])
def test_a_matrix_squared_in_place(cls, n):
    m = mat(cls, n, 0)
    want = (m * m).to_numpy()
    m *= m
    assert_same_product(m.to_numpy(), want)


# --- When it fails, it fails as before --------------------------------------------------------------------------------------------------

def test_a_wrong_operand_raises_and_changes_nothing():
    x = pyrove.vec3d(1.0, 2.0, 3.0)
    before = x
    with pytest.raises(TypeError):
        x += "not a vector"
    assert x is before
    assert list(x) == [1.0, 2.0, 3.0]


def test_mixed_precision_is_still_refused():
    x = pyrove.vec3d(1.0, 2.0, 3.0)
    with pytest.raises(TypeError):
        x += pyrove.vec3(1.0, 1.0, 1.0)


# --- It does not leak or lose the object -------------------------------------------------------------------------------------------

def test_the_reference_count_stays_put():
    b = pyrove.vec3d(1.0, 1.0, 1.0)
    x = pyrove.vec3d(0.0, 0.0, 0.0)
    before = sys.getrefcount(x)
    for _ in range(1000):
        x += b
    assert sys.getrefcount(x) == before
    assert list(x) == [1000.0, 1000.0, 1000.0]


def test_an_object_outlives_the_name_it_was_changed_under():
    x = pyrove.mat4d()
    y = x
    x *= 3.0
    del x
    assert y.to_numpy()[0, 0] == 3.0
