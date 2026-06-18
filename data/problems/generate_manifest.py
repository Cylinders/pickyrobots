#!/usr/bin/env python3
import json
from pathlib import Path


ROOT = Path(__file__).resolve().parent
MAPS_DIR = ROOT / "maps"
SCENARIOS_DIR = ROOT / "scenarios"
OUTPUT = ROOT / "manifest.jsonl"


def scenario_base_name(path: Path) -> str:
    name = path.name

    if name.endswith(".map.scen"):
        return name[:-len(".map.scen")]

    if name.endswith(".scen"):
        return name[:-len(".scen")]

    return path.stem


def matches_map(scen_base: str, map_stem: str) -> bool:
    return (
            scen_base == map_stem
            or scen_base.startswith(map_stem + "_")
            or scen_base.startswith(map_stem + "-")
            or scen_base.startswith(map_stem + ".")
    )


def main() -> None:
    maps = sorted(MAPS_DIR.glob("*.map"))
    scenarios = sorted(SCENARIOS_DIR.glob("*.scen"))

    map_by_stem = {p.stem: p for p in maps}
    map_stems = sorted(map_by_stem, key=len, reverse=True)

    scenarios_by_map: dict[str, list[Path]] = {stem: [] for stem in map_by_stem}

    for scen in scenarios:
        scen_stem = scenario_base_name(scen)

        match = next(
            (stem for stem in map_stems if matches_map(scen_stem, stem)),
            None,
        )

        if match is None:
            print(f"warning: no map found for {scen.name}")
            continue

        scenarios_by_map[match].append(scen)

    with OUTPUT.open("w", encoding="utf-8") as out:
        for map_stem in sorted(scenarios_by_map):
            scens = scenarios_by_map[map_stem]
            if not scens:
                continue

            row = {
                "id": map_stem,
                "map": str(map_by_stem[map_stem].relative_to(ROOT)),
                "scenarios": [
                    str(s.relative_to(ROOT))
                    for s in sorted(scens)
                ],
            }

            out.write(json.dumps(row, separators=(",", ":")) + "\n")


if __name__ == "__main__":
    main()