from pathlib import Path

from typing import NamedTuple

from benchmark_format_parser import parse_map_from, parse_scenario_from
from bfs import shortest_path_bfs
from cutter import n_cut
from inference import infer
from mapfaster_input_transform import encode_cuts
from structs import Agent, Grid, GridPath


class Problem(NamedTuple):
    map: Grid
    scenario: list[Agent]


def minimal_paths_for(agents: list[Agent], map: Grid) -> dict[Agent, GridPath]:
    return {
        agent: shortest_path_bfs(map, agent.start, agent.goal)
        for agent in agents
    }


def main():
    map = parse_map_from(Path("D:/dev/pickyrobots/originalmaps/Boston_0_256.map"))
    scenario = parse_scenario_from(Path("D:/dev/pickyrobots/originalscen/scen-even/Boston_0_256-even-1.scen"))

    p = Problem(map, scenario[:50])
    ccpd(p)

    return


def ccpd(p: Problem):
    paths = minimal_paths_for(p.scenario, p.map)

    # batch_cuts = epsilon_cut(2, scenario)
    batch_cuts = n_cut(10, p.scenario)
    # batch_cuts = no_cut(p.scenario)

    tensors = encode_cuts(p.map, paths, batch_cuts)

    print(infer(tensors))

    return


if __name__ == "__main__":
    main()
