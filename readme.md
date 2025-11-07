Clarinoid
=========

This repository hosts the hardware and software that power the Clarinoid digital wind instrument. It combines the cross platform runtime, Teensy-based firmware, and a Windows simulator/test harness so hardware and sound design can move in lockstep.

Quick orientation
-----------------
- `src/` is the canonical source tree.
  - `library/clarinoid_core` is the shared static library that runs on both Teensy and desktop. Public headers live under `include/clarinoid/core/...`, grouping APIs by domain (basic utilities, midi, synth, UI, etc.). The extra `core` segment keeps the public include surface inside a common `clarinoid/` prefix without clashing with sibling libraries.
  - `library/clarinoid_sim` adds desktop-only helpers (logging, Win32/DirectX glue, simulator plumbing) with headers exposed as `clarinoid/sim/...`.
  - `apps/ClarinoidSimulator` is the Win32 GUI simulator built with DirectX and ImGui.
  - `apps/clarinoid-core.tests` holds the gtest suite. The same sources feed two executables so we continually verify the codebase builds cleanly in both C++17 and C++20.
  - `apps/TESTDEVICE` and `apps/TESTDEVICE-PERF` are PlatformIO firmware entry points for the main prototype and the stripped-down performance bench build. Both pull in `clarinoid_core` from `../../library`.
- `projects/` contains the legacy PlatformIO project roots kept around while everything migrates to the `src/apps` layout. They mirror the firmware sources; prefer the copies under `src/apps` going forward.
- `build/vs-x86` is generated output from the CMake preset (`Visual Studio 2022 Win32`). It is safe to delete and regenerate.
- `notes/`, `misc/`, and `CAD/` gather design notes, scratch experiments, and mechanical/electrical files. `notes/development.md` is still the catch-all for ad hoc dev observations.

Building
--------

Desktop simulator and tests
~~~~~~~~~~~~~~~~~~~~~~~~~~~
1. Run `OpenDesktopInVisualStudio.cmd` (calls `cmake --preset vs-x86` and launches the generated solution).
2. Build `ClarinoidSimulator`, `ClarinoidTestsCpp17`, or `ClarinoidTestsCpp20` inside Visual Studio.

Command-line equivalents:

```
cmake --preset vs-x86
cmake --build --preset vs-x86-Debug
ctest --preset tests-x86-Debug
```

Firmware on Teensy
~~~~~~~~~~~~~~~~~~
1. Open the desired app folder under `src/apps/<name>` in VS Code with the PlatformIO extension.
2. The provided `platformio.ini` files already point `lib_extra_dirs` at `../../library`, exposing `clarinoid_core` (and `clarinoid_sim` if desired).
3. Build or upload with the usual PlatformIO tasks (`pio run -t upload`). `TESTDEVICE` targets the Teensy Micromod configuration; `TESTDEVICE-PERF` defaults to Teensy 4.0 with higher optimisation and no exceptions.

FAQs
----

- Why Visual Studio for desktop builds instead of VS Code? The Windows simulator leans on DirectX, ImGui, and needs robust native debugging. VS currently provides the smoothest setup, whereas PlatformIO in VS Code handles the embedded toolchain well.
- What is the extra `clarinoid/core` directory under `include`? All public headers are exposed as `clarinoid/<module>/...`. `clarinoid_core` installs under `clarinoid/core`, while `clarinoid_sim` installs under `clarinoid/sim`. The extra level avoids collisions as more Clarinoid libraries appear and keeps includes tidy (`#include <clarinoid/core/basic/Array.hpp>`).
- Why do we ship both C++17 and C++20 test executables? Teensyduino pins us to C++17, but desktop tooling can already take advantage of C++20. Building the same unit tests twice guarantees the headers stay C++17-clean for firmware while catching opportunities to use C++20-only features on desktop without regressions.
- Where do the miscellaneous experiments and mechanical files live? Hardware, CAD, and long-form docs sit under `CAD/` and `notes/`; quick experiments and one-off benchmarks stay in `misc/`.

Housekeeping ideas
------------------
- Retire or replace the duplicated `projects/*` PlatformIO trees once every workflow points at the `src/apps/*` variants (stubs that include the canonical `platformio.ini` would preserve backwards compatibility).
- Centralise shared PlatformIO settings (board type, optimisation flags, common macros) in an include file to keep the firmware apps from drifting apart.
- Sweep the `misc/` directory and either graduate useful tools into `src/apps`/`src/library` or archive them elsewhere to reduce noise.

