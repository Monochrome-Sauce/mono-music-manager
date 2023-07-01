# Mono Music Manager

This is the backend library for a simple music manager.
The project uses `libmpv` to gain its playback capabilities.

## Dependencies

- gcc/clang (build)
- meson (build)
- pkg-config (build)
- libmpv >= 1.109 (library)
- sigc++ >= 2.10 (library)
- spdlog >= 1.9 (library)
- sqlite >= 3.37 (library)
- youtube-dl (optional, library)

## Compiling the project

Go to the base of the project's code and execute the following commands:

1. `meson setup build`
2. `cd build`
3. `meson compile`
