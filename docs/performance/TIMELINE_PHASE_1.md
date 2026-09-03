# Timeline Performance Phase 1

## Phase

Phase 0 audit plus the first Phase 1 timeline hot-path optimization.

## Repository Map

- UI layer: `apps/web/src/components`, `apps/web/src/timeline/components`, `apps/desktop`.
- State layer: `apps/web/src/editor`, `apps/web/src/core/managers`, `apps/web/src/timeline/timeline-store.ts`.
- Timeline layer: `apps/web/src/timeline`, `apps/web/src/commands/timeline`.
- Preview/render layer: `apps/web/src/preview`, `apps/web/src/services/renderer`.
- Media layer: `apps/web/src/media`, `mediabunny`, browser media APIs.
- Audio layer: `apps/web/src/core/managers/audio-manager.ts`, `apps/web/src/media/audio*.ts`, `apps/web/src/retime`.
- Persistence/export layer: `apps/web/src/services/storage`, `apps/web/src/export`.
- Rust/WASM layer: `rust/crates/*`, `rust/wasm`, `apps/web/src/wasm`.
- GPU layer: Rust `gpu`, `compositor`, WGSL shaders, plus web renderer services.

## What Changed

- Added sorted snap-point construction in `apps/web/src/timeline/snapping/build.ts`.
- Updated `resolveTimelineSnap` to use a bounded binary-search scan for sorted snap points, with the previous linear scan retained as a fallback for unsorted callers.
- Added `resolveSortedTimelineSnap` for callers that already own sorted snap points, avoiding an O(n) sortedness guard on every drag-frame query.
- Cached move-group snap points for the active drag session in `apps/web/src/timeline/controllers/element-interaction-controller.ts`.
- Reused the active drag's `memberTimeOffsets` map instead of allocating it every time the drag view is read.
- Avoided repeated `getDisplayTracks` cloning inside group move validation.
- Replaced per-track `slice().reduce()` row offset calculation with one precomputed prefix-offset pass.
- Added a sorted-track overlap fast path in placement checks. It uses binary search plus prefix max-end metadata and falls back to the existing linear behavior when elements are not sorted.
- Added snapping and overlap correctness tests plus 10k-object benchmark fixtures.
- Added an initial C++ timeline core in `native/opencut_core` with sorted clip indexing, prefix max-end metadata, overlap checks, visible range queries, insert, delete, move, trim, split, CMake, C ABI entry points, and standalone assert-based tests.
- Built the C++ core to an Emscripten single-file WASM module at `apps/web/public/wasm/opencut_core.js`.
- Added a web runtime bridge in `apps/web/src/native/opencut-core.ts`.
- Warm-load the C++/WASM module from `EditorProvider`.
- Timeline placement overlap checks now call the C++/WASM path when the module is loaded, with the existing TypeScript path retained as fallback.

## Why

Timeline drag snapping rebuilt every candidate snap point on each mousemove. On large projects this scales with all clips and animation keyframes, even when the timeline itself is unchanged during a drag. The new flow builds the snap set once at drag start, sorts it once, and resolves each subsequent mousemove by scanning only snap points inside the visible threshold window.

This keeps the current TypeScript/Rust runtime path intact while introducing a small C++ timeline core candidate. Existing Rust time/WASM utilities are already used for media-time correctness, so the C++ module is isolated until benchmark and parity tests show that a native boundary is worth it for a specific timeline operation.

The C++ module is now an optional active web runtime dependency for placement overlap checks. If the WASM module fails to load or `NEXT_PUBLIC_OPENCUT_NATIVE_TIMELINE=0` is set, the existing TypeScript implementation is used.

## Benchmark

Added:

```text
apps/web/src/timeline/snapping/__tests__/resolve.bench.ts
apps/web/src/timeline/placement/__tests__/overlap.bench.ts
native/opencut_core/tests/timeline_tests.cpp
apps/web/public/wasm/opencut_core.js
```

Fixtures:

```text
10,000 sorted snap points
1,000 snap queries
10,000 sorted track elements
1,000 placement overlap queries
```

The local machine currently does not have `bun` available in PATH, so the Bun benchmark could not be executed in this run. The fixture is ready to run with:

```bash
bun test apps/web/src/timeline/snapping/__tests__/resolve.bench.ts
bun test apps/web/src/timeline/placement/__tests__/overlap.bench.ts
```

## Verification

- `cargo test -p time`: passed, 10 tests.
- Targeted ESLint from `apps/web`: passed for changed files.
- C++ configure/build/test: passed through Visual Studio bundled CMake/Ninja and MSVC developer environment.
- C++ Emscripten WASM build: passed after installing repo-local `.tools/emsdk`.
- Node smoke test against `apps/web/public/wasm/opencut_core.js`: passed (`insert=0`, `free=1`, `overlap=0`).
- `npx tsc -p apps/web/tsconfig.json --noEmit`: blocked by current npm environment and pre-existing project issues, primarily missing `bun:test` type declarations under npm plus existing arity errors in storage/stickers modules.
- `bun test`: blocked because `bun` is not available in PATH.

## Known Limitations

- Timeline elements are still rendered as DOM/React elements. Large-project viewport virtualization or a canvas/WebGPU visualization layer remains a future phase.
- Group move overlap checks still use the existing placement behavior for correctness. A sorted interval index is the next candidate after adding broader behavior tests.
- No UI styling, colors, spacing, layout dimensions, or user-visible interaction behavior were intentionally changed in this phase.
