<p align="center">
	<img src="./readme-images/img01.png" width="48%" title="Extract mode">
	<img src="./readme-images/img02.png" width="48%" title="Create mode">
	<img src="./readme-images/img03.png" width="48%" title="Open pre/prx dialog">
	<img src="./readme-images/img04.png" width="48%" title="Add files dialog">
</p>

# NSPRE-GUI

Create and extract Neversoft pre/prx files.

Built using [nspre](https://github.com/BryanRykowski/nspre) and [Dear ImGui](https://github.com/ocornut/imgui).

ⓒ 2025 Bryan Rykowski - [MIT License](./LICENSE)

## Building

### Prerequisites
- CMake
- SDL2 development files

### Instructions
Clone the repository.
```
git clone https://github.com/BryanRykowski/nspre-gui
```
Enter the directory and clone the submodules.
```
cd nspre-gui/
git submodule update --init --recursive
```
Create a build directory and generate the build files.
```
mkdir build
cmake -S . -B build/
```
Build the project.
```
cmake --build build/
```
Binary will be at `build/nspre-gui`