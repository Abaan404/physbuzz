<h2 align="center">PhysBuzz Engine</h2>

<div align="center">
    <p>A non-serious engine I use to learn graphics programming</p>
    <img src="media/demo.gif"></img>
</div>

> [!NOTE]
> This engine is WIP and probably will be for the forseeable future, dont use this pls ok ty.

### Dependencies

This project uses the CMake build system with the following dependencies:

 - [SDL2](https://github.com/libsdl-org/SDL)
 - [Dear ImGui](https://github.com/ocornut/imgui)
 - [GLM](https://github.com/g-truc/glm) (header-only)

The OpenGl branch uses the following dependencies:
 - [glad](https://github.com/Dav1dde/glad)

The Vulkan branch (abandoned for now) uses the following dependencies:
 - [Vulkan](https://www.lunarg.com/vulkan-sdk/)

Most of them should be autodetected and/or included with submodules.

1) Install the packages
```sh
pacman -S sdl2 glm
# or with an equivalent package manager
```

2) Clone the repository
```sh
git clone --recurse-submodules https://github.com/Abaan404/physbuzz
```

3) Build with cmake. (assuming dependencies are satisfied)

```
cd ./build/
cmake ..
```

4) Run the binary
```
./physbuzz
```

Or equivalent with your IDEs cmake build tools.

### Usage
WIP

### Notes
- This is my first ever cpp project so expect to see some horrors in this codebase.
- There are two actively developed branches
  - `opengl` branch is for me to learn the opengl rendering pipeline and eventually port the rendering logic to raw opengl calls.
  - `main` branch is where the actual progress is made (with a feature branch every now and then)
- I'll eventually add testing lol.

### Credits and Acknowledgements
- https://learnopengl.com/ for their actually really good resource on modern opengl.
- [The Cherno's](https://www.youtube.com/@TheCherno) amazing c++ series.
- my sanity for keeping up with me (it didnt but for real this time).
