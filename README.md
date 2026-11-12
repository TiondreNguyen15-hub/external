# Windows player visualizer

C++17, Dear ImGui, Win32 and Direct3D 11 rendering prototype in `apex-visualizer/`.
Includes toggles for bounding boxes, health bars, skeletons, line thickness and color.

**Current status: sample data only.** This is a standalone window, not a live Apex
ESP or a transparent in-game overlay. It does not read Apex memory or include
anti-cheat bypasses. No current Apex offsets or SDK were provided.

## Download and run on Windows

Download [ApexVisualizer-Windows-x64.zip](downloads/ApexVisualizer-Windows-x64.zip)
from the `downloads` folder in the Code tab. On the ZIP's GitHub file page,
click **Download raw file**. Extract the ZIP, then double-click
`apex_visualizer.exe`. A SHA-256 checksum is in `downloads/SHA256SUMS.txt`.

This checked-in executable was cross-compiled for Windows x64 with llvm-mingw
20261104 and Dear ImGui v1.91.8. Its PE format and DLL imports were inspected;
it has not been run on Windows. It displays sample players only.
Windows 10/11 x64 and Direct3D 11 support are required.

The **Actions** tab also provides a ZIP artifact after each successful
**Windows build** run. Those artifacts are built with MSVC from that run's source.

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
