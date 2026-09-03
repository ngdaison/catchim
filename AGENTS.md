# Agents.md

## Architecture

All business and computational logic is housed in `native/opencut_core` written in 100% C++ (C++20). The web frontend under `apps/web/` is a Next.js / React UI shell that communicates with the C++ engine compiled via WebAssembly (Emscripten).

### `native/opencut_core/`

The single source of truth for all non-UI code (100% C++20). Everything platform-agnostic belongs here:
- **`opencut::time`**: High-precision media time (120,000 ticks/sec), rational framerates, SMPTE timecode parser and formatter.
- **`opencut::timeline`**: Multi-track data structure, clip CRUD, interval indexing, collision/overlap checking, magnet snapping, group move & ripple editing.
- **`opencut::compositor`**: Layer transforms (matrix math), 17 blend modes, frame clear and layer composition.
- **`opencut::masks`**: Analytical Signed Distance Fields (SDF) for shapes, soft feathering, inversion.
- **`opencut::effects`**: Color adjustments, filters (vignette, blur kernels, invert).
- **`opencut::audio`**: Keyframe volume envelopes, audio fade curves, fast waveform decimation.
- **`opencut::c_api`**: Clean C ABI and Emscripten WebAssembly exports.

### `apps/web/`

The Next.js web application is the UI shell — it owns presentation, UI components, interaction, and styling, while calling into the C++ engine via WebAssembly bindings (`apps/web/src/native/`).

## Web

### React

- Read components before using them. They may already apply classes, which affects what you need to pass and how to override them.
