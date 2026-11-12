# Windows player visualizer

C++17, Dear ImGui, Win32 and Direct3D 11 rendering prototype in `apex-visualizer/`.
Includes toggles for bounding boxes, health bars, skeletons, line thickness and color.

**Current status: sample data only.** This is a standalone window, not a live Apex
ESP or a transparent in-game overlay. It does not read Apex memory or include
anti-cheat bypasses. No current Apex offsets or SDK were provided.

## Download and run on Windows

Open this repository's **Actions** tab, select the latest successful **Windows
build** run for `main`, and download **ApexVisualizer-Windows-x64** under
**Artifacts** (GitHub sign-in may be required). Extract the downloaded artifact
and the portable ZIP inside it, then double-click `apex_visualizer.exe`.
Windows 10/11 x64 and Direct3D 11 support are required. No development tools are
needed to run the packaged app. The sample-data window opens without a console.

## Windows build

Install Visual Studio 2022 with **Desktop development with C++**, Windows SDK,
CMake 3.24+ and Git. Run in a developer PowerShell:

```powershell
cmake -S apex-visualizer -B build -A x64
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
cpack --config build/CPackConfig.cmake -C Release -B dist
.\build\Release\apex_visualizer.exe
```

Configuration downloads Dear ImGui v1.91.8 from its official Git repository.

## Data and projection contract

`src/projection.hpp` describes the input format. Bones are world-space positions;
`links` defines connections for the sample's 14 semantic joints. These indices
are not Apex bone IDs or offsets. A future data adapter must map actual bone IDs,
transform bone-local coordinates into world coordinates, and provide a consistent
snapshot for each frame.

Projection uses a row-major world-to-clip matrix multiplied by a column vector,
with screen Y pointing down. Transpose column-major inputs before use. Matrix
storage and coordinate conventions must be verified against the source; there
is no universal Apex offset or matrix layout. Points behind the camera, outside
the viewport, or with invalid coordinates are rejected. Segments crossing the
viewport edge are omitted rather than clipped. Boxes currently enclose projected
joints with padding; they are not engine collision bounds.

Live integration requires the exact supported game build, validated entity and
health layouts, bone transforms and index mappings, camera/view-projection layout,
and viewport coordinates. None have been assumed or fabricated here.

## Portable math checks

The projection tests can also run on Linux without ImGui or Windows:

```sh
g++ -std=c++17 -Wall -Wextra -Werror -I apex-visualizer/src apex-visualizer/tests/projection_tests.cpp -o /tmp/projection_tests
/tmp/projection_tests
```

The Windows executable must be built and visually tested on Windows; Linux math
tests do not validate the graphics backend.
