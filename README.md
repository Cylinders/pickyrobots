# MAPF DATASETS AND STANDARDIZED SOLUTION FORMATS. 

This is the code supplement to the work published in: 

**Datasets and a Standardized Solution Format for Multi Agent Pathfinding Algorithm Selection**

By: 

Ruthvik Venkatesan. _Github:_ [https://github.com/cylinders](url)

Marco Sitar. _Github:_ [https://github.com/cylinders](url)

Third Author. (TBD) 

How to use: 

**Algorithms can be rebenchmarked in directory entitled "mapf-benchmark": **

Copy and paste these commands from the root directory to build the benchmarker: 

```
cd mapf-benchmark
mkdir build
cmake -S . -B build-release -DCMAKE_BUILD_TYPE=Release
cmake --build build-release -j
```

Then, navigate to mapf-benchmark/data/problems/maps and copy and paste your maps. 

Navigate to mapf-benchmark/data/problems/scenario and copy and paste your scenarios. Each scenaroi file should have the NAME of the map file (example.map), not the whole path. Path is assumed to be within ./maps

If you want to add subdirectories within /maps, make sure you indicate that in your scenarios. 

You can use scenewrite.py to generate the custom maps and agent sets that we describe in our paper. 

Before you run the project, make sure to run: 

```python3 generate_manifest.py```

Then: 

navigate to mapf-benchmark/build-release and run ./mapf-dataset to benchmark problems and algorithm selectors on your chosen device. **We highly recommend using WSL if running on a windows machine, in order to easily get all dependencies working.**


**Required Dependencies: **
Boost

