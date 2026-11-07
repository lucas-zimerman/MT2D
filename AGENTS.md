# Notes for contributors and coding agents

## Language rule

**New code must be written in C (C99 or older, no C++ features), except tests.**

* Tests in `tests/` stay C++ (`*.cpp`).
* New `.c` / `.h` files must compile as C and as C++: wrap declarations in `extern "C"` guards
  (`#ifdef __cplusplus`), use `struct`/`typedef` the C way, no overloads, no default arguments,
  no `std::`, no exceptions, no `bool` without `<stdbool.h>`-style fallback, no `#pragma region`
  (DJGPP builds with `-Werror=unknown-pragmas`).
* Existing C++ engine files stay as they are. Only touch them to make their headers usable from C.
* Keep the DJGPP/Watcom limits in mind: 8.3 file names for DOS-visible files, no heap-heavy code.

## Pull requests

Every pull request is referenced as `([#id](prurl))`, for example
`([#26](https://github.com/lucas-zimerman/MT2D/pull/26))`, wherever a PR is mentioned.

**Each item of `CHANGELOG.md` must end with that reference** to the PR that made the change (an
issue can be linked before it, e.g. `([#3](.../issues/3))`). Open the PR first, then add its number
to the entries.

## Other rules

See [`CONTRIBUTING.md`](CONTRIBUTING.md): update `CHANGELOG.md`, keep include letter case, run the
tests (`cmake -S tests -B build/tests && cmake --build build/tests && ctest --test-dir build/tests`).
