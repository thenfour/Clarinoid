
Clarinoid.

extra dev stuff / personal notes: check out notes/dev notes.md

Description of the project structure, which projects are available, dev tasks...

* CMake project in general, generating visual studio projects for desktop apps, and allows vs code platformio for hardware development env.
* Desktop apps are in visual studio
  * why? because vs code is not working well for desktop projects. see dev notes.
* unit tests
* changes from CLARINOID2
  * new dir structure
    * makes projects clearer
    * gives proper home for includes vs src vs 3rdparty (imgui, gtest, et al) etc.
    * required to make unit tests, cmake, platformio, etc all happy
  * multiple cpp units allowed; just a cleaner more C++like structure
* dir structure
  * `\src\library\clarinoid_core\include\clarinoid\core\basic\Array.hpp`
    * `src`: as opposed to documents, kicad, etc.
    * `library`: libraries, as opposed to exe projects
    * `clarinoid_core`: shared core project; most library code here. as opposed to platform or app-specific libraries. This is a static library that appears in visual studio, it's a built project.
    * `include`: include files for the core project. this is a shared include location (for `#include <clarinoid/core/...>`)
    * `clarinoid`: a pretty synonym for the core project; making `#include <clarinoid/...` look nice.
    * `core`: ?? not sure why this dir level is necessary.
    * `basic`: not a real library, but we've always organized like this.
    * `Array.hpp`
* Why both C++17 and C++20 tests? I don't remember. Teensyduino is `C++17` only.

