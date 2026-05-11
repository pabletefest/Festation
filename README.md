## Festation

An experimental Sony Playstation 1 (PSX) emulator written in C++ and using OpenGL for the HW renderer. This projects is currently in development and aims to be cross-platform (currently Windows and Linux supported).

### Progress
<img width="1023" height="541" alt="imagen" src="https://github.com/user-attachments/assets/613dbd4b-26ad-4426-8370-a5f9be6475ff" />

## Building
In order to build successfully, a compiler compatible with C++23 standard is needed.

The project can be built using CMake as follows:
```sh
git clone https://github.com/pabletefest/Festation.git
cd Festation
mkdir build
cd build
cmake ..
```

This will build the project inside the build folder using installed compiler on your system.

## External dependencies
Currently using the following third-party libraries:
  - [GLFW 3.4](https://github.com/glfw/glfw)
  - [Glad](https://github.com/Dav1dde/glad)
  - [glm](https://github.com/g-truc/glm)
  - [ImGui](https://github.com/ocornut/imgui)

## Copyright

Festation is Copyright © 2024 - 2024 pabletefest. It is licensed under the terms of the GNU General Public License (GPL) 3.0 or any later version. See LICENSE for details.

Sony Playstation is a registered trademark of Sony Group Corporation.

