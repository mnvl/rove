# SPDX-License-Identifier: MIT
# Copyright (c) 2004-2026 Manvel Avetisian

"""
dot, cross, normalize and normalize_or_zero take double vectors as they take float ones, and stay in double.

They were bound for float alone, so pyrove.cross(vec3d, vec3d) was a TypeError while a.cross(b) worked.
"""

import math

import pytest

import pyrove

# Differs from 1 by less than a float can hold: a value that went through float comes back as 1 exactly
JUST_OVER_1 = 1.0 + 1e-12

DOUBLE_VECTORS = [(pyrove.vec2d, 2), (pyrove.vec3d, 3), (pyrove.vec4d, 4)]
FLOAT_VECTORS = [(pyrove.vec2, 2), (pyrove.vec3, 3), (pyrove.vec4, 4)]
ids = [c.__name__ for c, _ in DOUBLE_VECTORS]


def test_the_test_value_cannot_survive_float():
    assert pyrove.vec3(JUST_OVER_1, 0.0, 0.0).x == 1.0
    assert pyrove.vec3d(JUST_OVER_1, 0.0, 0.0).x == JUST_OVER_1


# --- dot ---------------------------------------------------------------------------------------------------------------------------

@pytest.mark.parametrize("cls,n", DOUBLE_VECTORS, ids=ids)
def test_dot_of_doubles(cls, n):
    a = cls(*[float(i + 1) for i in range(n)])
    b = cls(*[float(i + 10) for i in range(n)])
    assert pyrove.dot(a, b) == sum((i + 1.0) * (i + 10.0) for i in range(n))


@pytest.mark.parametrize("cls,n", DOUBLE_VECTORS, ids=ids)
def test_dot_stays_in_double(cls, n):
    a = cls(*[JUST_OVER_1] + [0.0] * (n - 1))
    b = cls(*[1.0] + [0.0] * (n - 1))
    assert pyrove.dot(a, b) == JUST_OVER_1


@pytest.mark.parametrize("cls,n", DOUBLE_VECTORS, ids=ids)
def test_dot_function_is_the_method(cls, n):
    a = cls(*[0.1 * (i + 1) for i in range(n)])
    b = cls(*[0.7 / (i + 1) for i in range(n)])
    assert pyrove.dot(a, b) == a.dot(b)


def test_dot_of_double_quaternions_stays_in_double():
    assert pyrove.dot(pyrove.quatd(JUST_OVER_1, 0.0, 0.0, 0.0), pyrove.quatd(1.0, 0.0, 0.0, 0.0)) == JUST_OVER_1
    assert pyrove.dot(pyrove.quatd(1.0, 2.0, 3.0, 4.0), pyrove.quatd(5.0, 6.0, 7.0, 8.0)) == 70.0


def test_dot_of_float_quaternions_still_works():
    assert pyrove.dot(pyrove.quat(1.0, 2.0, 3.0, 4.0), pyrove.quat(5.0, 6.0, 7.0, 8.0)) == 70.0


# --- cross -------------------------------------------------------------------------------------------------------------------------

def test_cross_of_doubles_is_a_double_vector_and_right_handed():
    c = pyrove.cross(pyrove.vec3d(1.0, 0.0, 0.0), pyrove.vec3d(0.0, 1.0, 0.0))
    assert isinstance(c, pyrove.vec3d)
    assert list(c) == [0.0, 0.0, 1.0]


def test_cross_does_not_commute():
    a, b = pyrove.vec3d(1.0, 2.0, 3.0), pyrove.vec3d(-4.0, 5.0, 0.5)
    assert list(pyrove.cross(a, b)) == [-x for x in pyrove.cross(b, a)]
    assert list(pyrove.cross(a, b)) == list(a.cross(b))


def test_cross_stays_in_double():
    c = pyrove.cross(pyrove.vec3d(JUST_OVER_1, 0.0, 0.0), pyrove.vec3d(0.0, 1.0, 0.0))
    assert c.z == JUST_OVER_1


# --- normalize and normalize_or_zero ---------------------------------------------------------------------------------------------------

@pytest.mark.parametrize("cls,n", DOUBLE_VECTORS, ids=ids)
@pytest.mark.parametrize("function", [pyrove.normalize, pyrove.normalize_or_zero], ids=["normalize", "normalize_or_zero"])
def test_normalize_gives_the_same_type_and_a_unit_length(function, cls, n):
    v = cls(*[float(i + 1) for i in range(n)])
    length = math.sqrt(sum((i + 1.0) ** 2 for i in range(n)))
    u = function(v)
    assert isinstance(u, cls)
    assert list(u) == pytest.approx([(i + 1.0) / length for i in range(n)], rel=1e-15)


@pytest.mark.parametrize("cls,n", DOUBLE_VECTORS, ids=ids)
def test_normalize_gives_a_new_vector_and_leaves_its_argument(cls, n):
    v = cls(*[float(i + 1) for i in range(n)])
    before = list(v)
    u = pyrove.normalize(v)
    assert u is not v
    assert list(v) == before


@pytest.mark.parametrize("cls,n", DOUBLE_VECTORS, ids=ids)
def test_normalize_agrees_with_the_method(cls, n):
    v = cls(*[0.3 * (i + 1) for i in range(n)])
    u = pyrove.normalize(v)
    v.normalize()
    assert list(u) == list(v)


def test_normalize_stays_in_double():
    # 3-4-5 scaled by a number a float would round: the double answer is exactly 0.6 and 0.8 to the last few bits, the float one is not
    u = pyrove.normalize(pyrove.vec2d(3.0, 4.0))
    assert u.x == pytest.approx(0.6, rel=1e-15)
    assert u.y == pytest.approx(0.8, rel=1e-15)
    assert abs(u.x - 0.6) < abs(float(pyrove.normalize(pyrove.vec2(3.0, 4.0)).x) - 0.6)


@pytest.mark.parametrize("cls,n", DOUBLE_VECTORS, ids=ids)
def test_normalize_of_no_length_raises(cls, n):
    with pytest.raises(ValueError):
        pyrove.normalize(cls(*[0.0] * n))


@pytest.mark.parametrize("cls,n", DOUBLE_VECTORS, ids=ids)
def test_normalize_of_what_is_not_a_number_raises(cls, n):
    with pytest.raises(ValueError):
        pyrove.normalize(cls(*[float("nan")] + [1.0] * (n - 1)))


@pytest.mark.parametrize("cls,n", DOUBLE_VECTORS, ids=ids)
def test_normalize_or_zero_hands_back_no_length(cls, n):
    assert list(pyrove.normalize_or_zero(cls(*[0.0] * n))) == [0.0] * n


# --- The float ones are as they were, and the two do not mix -----------------------------------------------------------------------

@pytest.mark.parametrize("cls,n", FLOAT_VECTORS, ids=[c.__name__ for c, _ in FLOAT_VECTORS])
def test_float_vectors_stay_float(cls, n):
    v = cls(*[float(i + 1) for i in range(n)])
    assert isinstance(pyrove.normalize(v), cls)
    assert isinstance(pyrove.normalize_or_zero(v), cls)


def test_float_cross_stays_float():
    assert isinstance(pyrove.cross(pyrove.vec3(1.0, 0.0, 0.0), pyrove.vec3(0.0, 1.0, 0.0)), pyrove.vec3)


@pytest.mark.parametrize("function", [pyrove.dot, pyrove.cross], ids=["dot", "cross"])
def test_mixed_precision_is_refused(function):
    with pytest.raises(TypeError):
        function(pyrove.vec3(1.0, 0.0, 0.0), pyrove.vec3d(0.0, 1.0, 0.0))
    with pytest.raises(TypeError):
        function(pyrove.vec3d(1.0, 0.0, 0.0), pyrove.vec3(0.0, 1.0, 0.0))


def test_mixed_precision_quaternions_are_refused():
    with pytest.raises(TypeError):
        pyrove.dot(pyrove.quat(0.0, 0.0, 0.0, 1.0), pyrove.quatd(0.0, 0.0, 0.0, 1.0))
