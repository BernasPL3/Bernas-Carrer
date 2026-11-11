# Contributing

Keep the native C++ engine and JavaScript engine deterministic and compatible. A change to the simulation must be made in both `native/include/career.hpp` and `web/engine.js`.

Run `python3 tools/test.py` before submitting a change. Build Wii and GameCube separately when changing native code. State clearly whether console behaviour was verified on hardware, in an emulator, or only through compilation.

Keep user-facing text in English. Console text must use characters supported by the built-in bitmap font. Keep existing `BERNASCAREER 1` save files readable unless the save version is deliberately changed and a migration is provided.

Describe the problem, the resulting behaviour and the checks performed in a pull request. Do not commit local saves, credentials, compiler installations, object files or ELF debug outputs.
