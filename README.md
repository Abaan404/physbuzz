<h2 align="center">PhysBuzz Engine</h2>

<div align="center">
    <p>A non-serious engine I use to learn graphics programming</p>
    <img src="media/demo.gif"></img>
</div>

> [!NOTE]
> This engine is WIP and probably will be for the forseeable future, dont use this pls ok ty.

### Dependencies

This project uses the CMake build system with the following dependencies:

 - [GLM](https://github.com/g-truc/glm)
 - [glad](https://github.com/Dav1dde/glad)
 - [SDL2](https://github.com/libsdl-org/SDL)
 - [Dear ImGui](https://github.com/ocornut/imgui)

The Vulkan branch (abandoned for now) uses the following dependencies:
 - [Vulkan](https://www.lunarg.com/vulkan-sdk/)

All dependencies should be satisfied with git submodules.

1) Clone the repository
```sh
git clone --recurse-submodules https://github.com/Abaan404/physbuzz
```

2) Build with cmake.
```sh
cmake -DCMAKE_BUILD_TYPE=Release -S . -B ./build
cmake --build ./build
```

3) Run the binary
```sh
./build/physbuzz
```

Or equivalent with your IDEs cmake build tools.

### Usage
WIP

### Notes
- This is my first ever cpp project so expect to see some horrors in this codebase (i'm trying).
- I'll eventually add testing lol.

### Credits and Acknowledgements
- https://learnopengl.com/ for their actually really good resource on modern opengl.
- [The Cherno's](https://www.youtube.com/@TheCherno) amazing c++ and opengl series.
- my sanity for keeping up with me (it didnt but for real this time).
