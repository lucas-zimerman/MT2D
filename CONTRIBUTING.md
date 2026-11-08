# Contributing to MT2D

Thanks for your interest in MT2D! This guide explains how to build the engine, how to send changes, and how to write changelog entries.

## Building

MT2D is a library of C/C++ sources (`MT2D/`) that you compile together with your own project. There is no single build system; pick the one for your target platform.

### Choosing features

Engine features are toggled with defines in `MT2D/MT2D_Terminal_Define.h`, for example:

- `SDL_USE`: draw with SDL2 instead of the system terminal (disabled by default).
- `SDL_USE_AUDIO`: enable SDL audio.
- `SDL_stbimage`: use `SDL_stbimage` instead of SDL_image (for platforms such as UWP).
- `MT2D_SDL_GPU`: use SDL_GPU instead of the SDL render calls.

Read the comments in that file before changing anything.

### Platforms

- **Windows / Linux / macOS**: add all `.c` files (and the remaining `ObjectCore/*.cpp` C++ files) under `MT2D/` (including the `Audio`, `Container`, `Cursor`, ... subfolders) to your compiler or IDE project. When `SDL_USE` is enabled, also link SDL2, SDL2_image and SDL2_mixer.
- **Android**: `Android.mk` at the repo root builds the engine as a module. It expects `SDL`, `SDL2_image` and `SDL2_mixer` next to the project. See `TestCase/Use Android Studio.txt` for a full walkthrough.
- **MS-DOS**: built with Open Watcom or DJGPP (`examples/cmake/djgpp.cmake`, also used by the CI). File names must follow the 8.3 format (e.g. `M2Render.c`, `M2Init.c`). See `Tools/msdos/readme.txt` for the helper scripts used when debugging. The experimental VGA mode (320x200, 256 colors, BMP image sprites, still C++ in `MT2D/_MSDOS/`) is opt-in with `MSDOS_USE_GRAPHICS`, see `examples/README.md`.
- **Emscripten**: supported through SDL (no audio yet).

### Testing your change

Run the automated tests (see [`tests/README.md`](tests/README.md)) with `cmake -S tests -B build/tests && cmake --build build/tests && ctest --test-dir build/tests --output-on-failure`. They cover the portable engine code only. Build and run the programs in [`examples/`](examples/README.md) (e.g. `cmake -S examples/menu -B examples/menu/build && cmake --build examples/menu/build`) or one of the samples in `TestCase/` (e.g. `TestCase/ContainerTest`) against your modified engine. If you touched platform specific code, say in the PR which platforms you tested; if you could not test a platform, say that too.

## Contributing

1. Fork the repository and create a branch from `master`. Use a descriptive name such as `feat/...` or `fix/...`.
2. Make your change, keeping the existing code style:
   - Use spaces, not tabs.
   - Keep includes' letter case exactly matching the file names (this matters on Linux).
   - Do not break the other platforms: keep OS specific code in its own OS folder and keep MS-DOS file names in the 8.3 format.
   - Prefer `MT2D_FILE` over `FILE` for file access.
3. Update `CHANGELOG.md` (see below).
4. Open a pull request against `master`. Describe what changed, why, and how you tested it. Keep PRs focused; unrelated changes belong in separate PRs.

## Changelog

Every user-visible change needs an entry in [`CHANGELOG.md`](CHANGELOG.md). The file follows this format:

```markdown
# Changelog

## Unreleased

### Breaking

- Description of a change that requires users to update their code.

### Features

- Add something new ([#12](https://github.com/lucas-zimerman/MT2D/pull/12))

### Fixes

- Fix something that was broken ([#12](https://github.com/lucas-zimerman/MT2D/pull/12))

## 2.0.1 - 2019-03-24

### Features
...
```

Rules:

- New entries go under `## Unreleased`. When a version is released, that heading becomes `## <version> - <YYYY-MM-DD>` and a new empty `Unreleased` section is added on top.
- Use only the sections `### Breaking`, `### Features` and `### Fixes`, in that order, and omit sections that are empty.
- One bullet per change, starting with a verb in the imperative (`Add`, `Fix`, `Remove`, `Rename`, ...), written as a short sentence with no trailing period required but consistent within the section.
- End each bullet with a link to the pull request: `([#N](https://github.com/lucas-zimerman/MT2D/pull/N))`. Since the PR number is only known after opening it, push the changelog entry in a follow-up commit.
- For `Breaking` changes, explain what users must do to migrate and include a short code snippet when helpful.
- Wrap code identifiers, functions and file names in backticks.

## License

By contributing you agree that your contributions are licensed under the project's [LICENSE](LICENSE).
