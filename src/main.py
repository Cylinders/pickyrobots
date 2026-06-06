from pathlib import Path

from torch import Tensor
from typing import NamedTuple

from benchmark_format_parser import parse_map_from, parse_scenario_from
from bfs import shortest_path_bfs
from cutter import n_cut, up_to_n_cuts, no_cut
from inference import infer
from mapfaster_input_transform import encode_cuts
from structs import Agent, Grid, GridPath
import torch.nn.functional as F

class Problem(NamedTuple):
    map: Grid
    scenario: list[Agent]


def minimal_paths_for(agents: list[Agent], map: Grid) -> dict[Agent, GridPath]:
    return {
        agent: shortest_path_bfs(map, agent.start, agent.goal)
        for agent in agents
    }


def main():
    map = parse_map_from(Path("..\maps\Boston_0_256.map"))
    scenario = parse_scenario_from(Path("..\scenes\scen-even\Boston_0_256-even-20.scen"))

    p = Problem(map, scenario[:50])
    ccpd(p)

    return


type Cut = list[list[Agent]]
type CutOutput = list[list[float]]

def associate_cut_outputs(
    batch_cuts: list[Cut],
    output: Tensor,
) -> list[tuple[Cut, CutOutput]]:
    flat_outputs: list[list[float]] = output.detach().cpu().tolist()

    result: list[tuple[Cut, CutOutput]] = []
    i = 0

    for cut in batch_cuts:
        n_groups = len(cut)
        cut_output = flat_outputs[i:i + n_groups]
        result.append((cut, cut_output))
        i += n_groups

    assert i == len(flat_outputs)
    return result

def choose_best_cut_and_algos(
    cut_outputs: list[tuple[Cut, CutOutput]],
) -> tuple[Cut, list[tuple[list[Agent], int, float]], float]:
    best_cut: Cut | None = None
    best_group_choices: list[tuple[list[Agent], int, float]] | None = None
    best_cut_score = float("-inf")

    for cut, output in cut_outputs:
        assert len(cut) == len(output)

        group_choices: list[tuple[list[Agent], int, float]] = []

        for group, nodes in zip(cut, output):
            algo_idx, algo_score = max(enumerate(nodes), key=lambda x: x[1])
            group_choices.append((group, algo_idx, algo_score))

        cut_score = sum(score for _, _, score in group_choices) / len(group_choices)

        if cut_score > best_cut_score:
            best_cut = cut
            best_group_choices = group_choices
            best_cut_score = cut_score

    assert best_cut is not None
    assert best_group_choices is not None

    return best_cut, best_group_choices, best_cut_score

def ccpd(p: Problem):
    paths = minimal_paths_for(p.scenario, p.map)

    # batch_cuts = epsilon_cut(2, scenario)
    # batch_cuts:  list[list[list[Agent]]] = up_to_n_cuts(20, p.scenario)
    batch_cuts = no_cut(p.scenario)

    tensor: Tensor = encode_cuts(p.map, paths, batch_cuts)

    output = infer(tensor)
    print(F.softmax(output))

    cut_to_output = associate_cut_outputs(batch_cuts, output)

    # algo index, score
    print([(x[1], x[2]) for x in choose_best_cut_and_algos(cut_to_output)[1]])

    return


if __name__ == "__main__":
    main()
