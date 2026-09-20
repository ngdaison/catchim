# MIGRATION MAP - BẢN ĐỒ DI CHUYỂN MODULE

Tài liệu này vạch rõ lộ trình chuyển đổi từ mã nguồn TypeScript sang mã nguồn C++20, đồng thời chỉ ra vị trí các file đích trong `/app`.

---

## 1. Bản đồ Chuyển đổi File & Module

### 1.1. Core Types, IDs & Time
| Web Module | C++ Target | Tiến độ |
|---|---|---|
| `web/src/wasm/media-time.ts` | `app/src/core/time/TimelineTime.h / .cpp` | Sẵn sàng |
| `web/src/native/opencut-wasm-compat.ts` | `app/src/core/time/Timecode.h / .cpp` | Sẵn sàng |
| `web/src/timeline/types.ts` (IDs) | `app/src/core/ids/Ids.h` | Sẵn sàng |
| `web/src/utils/` | `app/src/core/utils/` | Sẵn sàng |
| Logging & Error Types | `app/src/core/logging/Logger.h`, `app/src/core/errors/Errors.h` | Sẵn sàng |

### 1.2. Project Model & Storage
| Web Module | C++ Target | Tiến độ |
|---|---|---|
| `web/src/project/types.ts` | `app/src/editor/project/Project.h`, `ProjectSettings.h` | Sẵn sàng |
| `web/src/services/storage/migrations/index.ts` | `app/src/editor/project/ProjectSerializer.h / .cpp` | Sẵn sàng |
| `web/src/services/storage/service.ts` | `app/src/storage/ProjectStorage.h / .cpp` | Sẵn sàng |
| `web/src/core/managers/save-manager.ts` | `app/src/storage/AutosaveManager.h / .cpp` | Sẵn sàng |

### 1.3. Timeline Engine & Commands
| Web Module | C++ Target | Tiến độ |
|---|---|---|
| `web/src/timeline/types.ts` (Track) | `app/src/editor/timeline/Track.h / .cpp` | Sẵn sàng |
| `web/src/timeline/types.ts` (Clip) | `app/src/editor/timeline/Clip.h / .cpp` | Sẵn sàng |
| `web/src/timeline/scenes.ts` | `app/src/editor/timeline/Timeline.h / .cpp` | Sẵn sàng |
| `web/src/timeline/snapping/` | `app/src/editor/timeline/SnapEngine.h / .cpp` | Sẵn sàng |
| `web/src/core/managers/commands.ts` | `app/src/editor/history/CommandHistory.h / .cpp` | Sẵn sàng |
| `web/src/commands/timeline/` | `app/src/editor/history/commands/TimelineCommands.h / .cpp` | Sẵn sàng |

### 1.4. Media & Audio Backend
| Web Module | C++ Target | Tiến độ |
|---|---|---|
| `web/src/media/types.ts` | `app/src/media/MediaAsset.h / .cpp` | Sẵn sàng |
| `web/src/media/metadata.ts` | `app/src/media/probe/MediaProbe.h / .cpp` | Sẵn sàng |
| `web/src/media/mediabunny.ts` | `app/src/media/decoder/VideoDecoder.h`, `FFmpegVideoDecoder.h / .cpp` | Sẵn sàng |
| `web/src/services/video-cache/` | `app/src/media/thumbnail/ThumbnailGenerator.h / .cpp` | Sẵn sàng |
| `web/src/services/waveform-cache/` | `app/src/media/waveform/WaveformGenerator.h / .cpp` | Sẵn sàng |
| `web/src/core/managers/audio-manager.ts` | `app/src/audio/AudioEngine.h`, `AudioMixer.h / .cpp` | Sẵn sàng |

### 1.5. Playback & Rendering
| Web Module | C++ Target | Tiến độ |
|---|---|---|
| `web/src/core/managers/playback-manager.ts` | `app/src/editor/playback/PlaybackController.h / .cpp` | Sẵn sàng |
| `web/src/services/renderer/scene-builder.ts` | `app/src/render/SceneBuilder.h / .cpp` | Sẵn sàng |
| `web/src/services/renderer/canvas-renderer.ts` | `app/src/render/RenderEngine.h`, `Compositor.h / .cpp` | Sẵn sàng |
| `web/src/effects/definitions/` | `app/src/render/effects/` | Sẵn sàng |
| `web/src/masks/builtin/` | `app/src/render/masks/` | Sẵn sàng |

### 1.6. UI & Qt Widgets
| Web Module | C++ Target | Tiến độ |
|---|---|---|
| `web/src/app/globals.css` | `app/src/ui/theme/Theme.h`, `Colors.h`, `Metrics.h` | Sẵn sàng |
| `web/src/app/editor/[project_id]/page.tsx` | `app/src/ui/MainWindow.h / .cpp` | Sẵn sàng |
| `web/src/components/editor/editor-header.tsx` | `app/src/ui/shell/EditorHeader.h / .cpp` | Sẵn sàng |
| `web/src/components/editor/panels/assets/` | `app/src/ui/panels/assets/AssetsPanel.h / .cpp` | Sẵn sàng |
| `web/src/preview/components/` | `app/src/ui/preview/PreviewWidget.h`, `PreviewPanel.h / .cpp` | Sẵn sàng |
| `web/src/components/editor/panels/properties/` | `app/src/ui/panels/properties/PropertiesPanel.h / .cpp` | Sẵn sàng |
| `web/src/timeline/components/` | `app/src/ui/timeline/TimelinePanel.h`, `TimelineTracksWidget.h / .cpp` | Sẵn sàng |
| `web/src/components/editor/export-button.tsx` | `app/src/ui/dialogs/ExportDialog.h / .cpp` | Sẵn sàng |
