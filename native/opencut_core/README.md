# OpenCut Native Core

This module is the first C++ performance-core candidate for timeline hot paths.

Current scope:

- Integer tick based timeline ranges.
- Track-local sorted clip index.
- Overlap checks for drag, trim, and insertion.
- Visible range queries for future timeline virtualization.
- Move and trim commands with structured errors.
- Insert, delete, split, move, and trim commands.
- A small C ABI in `include/opencut/c_api.h` for future WASM/native bindings.

This code is intentionally not wired into the web app yet. The current web app already uses Rust/WASM for media time and has TypeScript timeline behavior that must be preserved. The next integration step should add a thin WASM/native bridge and compare benchmark data before replacing the TypeScript path.

Build when CMake and a C++20 compiler are available:

```bash
cmake -S native/opencut_core -B native/opencut_core/build -DCMAKE_BUILD_TYPE=Release
cmake --build native/opencut_core/build --config Release
ctest --test-dir native/opencut_core/build --output-on-failure
```

On this Windows checkout, CMake and Ninja are bundled with Visual Studio. If they are not in PATH, run:

```bat
call "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat" -arch=x64
"C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" -S native/opencut_core -B native/opencut_core/build -G Ninja -DCMAKE_BUILD_TYPE=Debug
"C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" --build native/opencut_core/build --config Debug
"C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\ctest.exe" --test-dir native/opencut_core/build --output-on-failure
```

Build the web runtime module with Emscripten:

```bash
bun run build:cpp:wasm
```

On the local Windows setup with repo-local `.tools/emsdk` and Visual Studio bundled CMake/Ninja:

```bash
npm run build:cpp:wasm:win
```

That emits `opencut_core.js` into `apps/web/public/wasm/`. The module is built with Emscripten `SINGLE_FILE=1`, so the WASM payload is embedded in the JS loader. The web app preloads that module from `EditorProvider` and uses it for timeline placement overlap checks when available. Set `NEXT_PUBLIC_OPENCUT_NATIVE_TIMELINE=0` to force the TypeScript fallback.
