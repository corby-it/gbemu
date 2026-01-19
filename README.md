# `gbemu` - GameBoy Emulator

Repository for a multi-platform Gameboy emulator. This project's aim is to write an emulator for the original Gameboy and the Gameboy Color in C++. No-one pretends to be working on the most cycle accurate emulator here, this is just a hobby project and a way to learn how to write a working emulator for a (relatively) simple platform.

Dependencies (as submodules or included in the repo):

- DearImGui (for the GUI)
- ImPlot (for plotting audio signals)
- Tracy (for profiling)
- Miniaudio (to handle audio output)
- Doctest (for unit testing)
- Cereal (for serialization)
- STB (for image file utils)
- `libretro` (for building the `libretro` core)

External dependencies:

- GLFW 3
- Premake (to create VS2022 projects or makefiles)

Implemented features

- Basically fully playable, almost all subsystems are implemented, audio included. Not all Gameboy quirks are implemented though.
- The project can also be compiled as a `libretro` core
- Partial unit testing.
- Partial ROM testing:
    * Passes some Blargg's and Mooneye test roms.
    * Passes cgb-acid2 test.
    * Passes dmg-acid2 test.
    * Passes MagenTest tests.
- Some debug features are present:
    * Register display
    * Loaded cartridge info
    * Memory editor
    * Tile and OAM viewer
    * Background viewer
    * Audio visualization
    * Serial data log
    * Some basic support for step-by-step instruction execution and symbols loading

### TODOs:

Some things still have to be implemented or improved:

- Improve cycle accuracy
- Improve test ROMs results
- Save states support in the `libretro` core
- More unit testing
- Improve code coverage

## Build on Windows

Create a Visual Studio 2022 project for Windows:

```sh
$> premake5 vs2022
```

Open the visual studio solution and build the x64 release configuration of the `gbemu` project. On windows it's necessary to define the `GLFW_PATH` environment variable that points to the root of the GLFW folder.

To build and run tests you have to build the x64 release configuration of the `gbemu-tests` project.

To build the `libretro` core you have to build the x64 release configuration of the `gbemu-core` project.

## Build on Linux

Build and run the main emulator:

```sh
$> cd <repository path>
$> premake5 gmake

# Build and run the main emulator
$> make gbemu config=release -j8
$> ./build/bin/release/gbemu

# Build and run tests
$> make gbemu-test config=release -j8
$> ./build/bin/release/gbemu-tests

# Build libretro core
$> make gbemu-core config=release -j8
# the resulting binary will be in 
# ./build/bin/release/libgbemu-core.so
```

## Screenshots

The main UI on Windows while playing Pokemon Red emulated in the DMG:

![Main UI on Windows](./screenshots/ui-win-dmg.png)

The main UI on Windows while playing Pokemon Silver emulated in the CGB, showing also some background and tiles debug features:

![Main UI on Windows](./screenshots/ui-win-cgb-bg-tiles.png)

Some game screenshot from both DMG and CGB games:

|  |  |
|---|---|
| ![Pokemon Red](./screenshots/pokemon-red.png) | ![Pokemon Silver](./screenshots/pokemon-silver.png) |
| ![The Legend of Zelda - Link's Awakening](./screenshots/links-awakening.png) | ![The Legend of Zelda - Oracle Of Seasons](./screenshots/zelda-oracle-of-seasons.png) |
| ![Super Mario Land 2](./screenshots/super-mario-land-2.png) | ![Super Mario Bros Deluxe](./screenshots/super-mario-bros-deluxe.png) |
