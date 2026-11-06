# SPDX-License-Identifier: MIT
# Copyright (c) 2004-2026 Manvel Avetisian

import numpy as np
import pyrove


def test_simple_path():
    grid = np.ones((20, 20), dtype=np.float32)
    finder = pyrove.a_star(0, 0, 10, 0, grid)
    finder.calculate_path()
    assert not finder.have_failed()

    path = finder.build_path()
    assert len(path) > 0
    assert path[0] == (0, 0)
    assert path[-1] == (10, 0)


def test_path_around_obstacle():
    grid = np.ones((20, 20), dtype=np.float32)
    grid[0:8, 5] = -1  # wall at x=5, y=0..7

    finder = pyrove.a_star(0, 0, 10, 0, grid)
    finder.calculate_path()
    assert not finder.have_failed()

    path = finder.build_path()
    assert path[0] == (0, 0)
    assert path[-1] == (10, 0)
    # path must go around the wall
    assert len(path) > 10


def test_unreachable_goal():
    grid = np.ones((10, 10), dtype=np.float32)
    # wall completely blocking
    grid[:, 5] = -1

    finder = pyrove.a_star(0, 0, 9, 0, grid)
    finder.calculate_path()
    assert finder.have_failed()

    path = finder.build_path()
    assert len(path) == 0


def test_build_cost_map():
    grid = np.ones((10, 10), dtype=np.float32)
    finder = pyrove.a_star(0, 0, 5, 0, grid)
    finder.calculate_path()
    assert not finder.have_failed()

    cost_map = finder.build_cost_map()
    assert cost_map.shape == (10, 10)
    assert cost_map.dtype == np.float32
    assert cost_map[0, 0] == 0.0  # start has cost 0
    assert cost_map[0, 5] > 0.0  # goal has positive cost


def test_start_on_blocked_cell():
    grid = np.ones((10, 10), dtype=np.float32)
    grid[0, 0] = -1  # start is blocked

    finder = pyrove.a_star(0, 0, 5, 5, grid)
    finder.calculate_path()
    assert finder.have_failed()


if __name__ == "__main__":
    test_simple_path()
    test_path_around_obstacle()
    test_unreachable_goal()
    test_build_cost_map()
    test_start_on_blocked_cell()
    print("all tests passed")
