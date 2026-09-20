# FEATURE MAP - ÁNH XẠ TÍNH NĂNG WEB VÀ C++ APP

Tài liệu này theo dõi toàn diện trạng thái chuyển đổi của từng tính năng từ `web` sang `app`.  
Quy ước trạng thái:
- **Detected**: Đã phát hiện và phân tích trong mã nguồn web.
- **Mapped**: Đã định nghĩa lớp và cấu trúc C++ tương ứng.
- **Implemented**: Đã viết mã nguồn C++.
- **Verified**: Đã biên dịch, chạy thử và kiểm chứng đối chiếu với web qua bộ test tự động.

---

## 1. Project Management
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| New Project (Tạo mới) | `web/src/core/managers/project-manager.ts` | `catchim::editor::EditorEngine::newProject` | Verified |
| Load Project (Mở dự án) | `web/src/services/storage/service.ts` | `catchim::storage::ProjectStorage::load` | Verified |
| Save Project (Lưu dự án) | `web/src/core/managers/save-manager.ts` | `catchim::storage::ProjectStorage::saveAtomic` | Verified |
| Autosave (Tự động lưu) | `web/src/core/managers/save-manager.ts` | `catchim::storage::AutosaveManager` | Verified |
| Rename Project (Đổi tên) | `web/src/components/editor/editor-header.tsx` | `catchim::editor::Project::setName` | Verified |
| Delete Project (Xóa dự án) | `web/src/project/components/delete-project-dialog.tsx` | `catchim::storage::ProjectStorage::deleteProject` | Verified |
| Project Settings (FPS, Size, Bg) | `web/src/project/types.ts` | `catchim::editor::ProjectSettings` | Verified |
| Project Serialization (v31 schema) | `web/src/services/storage/migrations/index.ts` | `catchim::editor::ProjectSerializer` | Verified |
| Multi-scene support (Tạo, xóa, switch, duplicate) | `web/src/timeline/scenes.ts` | `catchim::editor::Project::createScene`, `duplicateScene` | Verified |

---

## 2. Timeline & Clips
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| Tracks (Video, Audio, Text, Graphic, Effect) | `web/src/timeline/types.ts` | `catchim::editor::Track`, `TrackType` | Verified |
| Bookmarks / Markers | `web/src/timeline/bookmarks/` | `catchim::editor::Bookmark`, `Timeline::toggleBookmark` | Verified |
| Add Clip (Thêm clip) | `web/src/timeline/creation.ts` | `catchim::editor::AddClipCommand` | Verified |
| Move Clip (Di chuyển clip) | `web/src/timeline/controllers/element-interaction-controller.ts` | `catchim::editor::MoveClipCommand` | Verified |
| Trim Clip Start / End | `web/src/timeline/controllers/resize-controller.ts` | `catchim::editor::TrimClipCommand` | Verified |
| Split Clip | `web/src/actions/definitions.ts` (`split`) | `catchim::editor::SplitClipCommand` | Verified |
| Split Left / Split Right | `web/src/actions/definitions.ts` (`split-left`, `split-right`) | `catchim::editor::SplitLeftCommand`, `SplitRightCommand` | Verified |
| Delete Selected | `web/src/actions/definitions.ts` (`delete-selected`) | `catchim::editor::DeleteClipCommand` | Verified |
| Duplicate Clip | `web/src/actions/definitions.ts` (`duplicate-selected`) | `catchim::editor::DuplicateClipCommand` | Verified |
| Auto Snapping (Playhead, Boundary, Bookmarks) | `web/src/timeline/snapping/` | `catchim::editor::SnapEngine` | Verified |
| Ripple Editing / Ripple Delete | `web/src/timeline/update-pipeline.ts` | `catchim::editor::RippleDeleteCommand`, `Timeline::applyRipple` | Verified |
| Audio Separation (Extract/Recover) | `web/src/timeline/audio-separation/` | `catchim::editor::ExtractSourceAudioCommand`, `RecoverSourceAudioCommand` | Verified |
| Mute / Unmute Track & Clip | `web/src/timeline/audio-state.ts` | `catchim::editor::Track::setMuted`, `Clip::setMuted` | Verified |
| Hide / Show Track & Clip | `web/src/timeline/types.ts` | `catchim::editor::Track::setHidden`, `Clip::setHidden` | Verified |
| Graph Editor (Bezier easing) | `web/src/timeline/components/graph-editor/` | `catchim::core::BezierSolver` | Verified |
| Keyframe Animation Channels | `web/src/animation/keyframes.ts` | `catchim::editor::AnimationChannel`, `Keyframe` | Verified |

---

## 3. Actions, Keybindings & Clipboard
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| Action Registry (Definitions & Handlers) | `web/src/actions/definitions.ts`, `registry.ts` | `catchim::editor::ActionRegistry` | Verified |
| Keybinding Dispatcher | `web/src/actions/keybinding.ts` | `catchim::editor::ActionRegistry::invokeShortcut` | Verified |
| History (Undo / Redo) | `web/src/actions/definitions.ts` | `catchim::editor::CommandHistory` | Verified |
| Clipboard Copy & Paste | `web/src/clipboard/`, `web/src/core/managers/clipboard-manager.ts` | `catchim::editor::ClipboardManager`, `PasteClipsCommand` | Verified |

---

## 4. Playback & Transport
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| Play / Pause Toggle | `web/src/core/managers/playback-manager.ts` | `catchim::editor::PlaybackController::toggle` | Verified |
| Precise Seeking | `web/src/timeline/controllers/seek-controller.ts` | `catchim::editor::PlaybackController::seek` | Verified |
| Step Frame (Forward / Backward) | `web/src/actions/definitions.ts` | `catchim::editor::PlaybackController::stepFrame` | Verified |
| Jump 5 Seconds | `web/src/actions/definitions.ts` | `catchim::editor::PlaybackController::jumpSeconds` | Verified |
| Master Clock & Sync | `web/src/core/managers/playback-manager.ts` | `catchim::editor::PlaybackClock` (`steady_clock`) | Verified |
| Timecode Formatting & Parsing | `web/src/native/opencut-wasm-compat.ts` | `catchim::core::Timecode` | Verified |

---

## 5. Media & Audio Engine
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| Media Asset Probe (FFprobe/FFmpeg) | `web/src/media/metadata.ts` | `catchim::media::MediaProbe` | Verified |
| Waveform Generation & Cache | `web/src/services/waveform-cache/` | `catchim::media::WaveformGenerator` | Verified |
| Thumbnail Strip Generation & Cache | `web/src/services/video-cache/` | `catchim::media::ThumbnailGenerator` | Verified |
| Multi-track Audio Mixer | `web/src/core/managers/audio-manager.ts` | `catchim::audio::AudioMixer` | Verified |
| Audio Engine Runtime | `web/src/native/opencut-wasm-compat.ts` | `catchim::audio::AudioEngine` | Verified |

---

## 6. Compositing, Text & Visual Effects
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| Layer Compositing (z-order) | `web/src/services/renderer/canvas-renderer.ts` | `catchim::render::Compositor` | Verified |
| Transform (Pos, Scale, Rot, Opacity) | `web/src/canvas/` | `catchim::render::Transform` | Verified |
| Color Grading / Adjustments | `web/src/native/opencut-wasm-compat.ts` | `catchim::render::Compositor::applyColorGrading` | Verified |
| Vignette Effect | `web/src/native/opencut-wasm-compat.ts` | `catchim::render::Compositor::applyVignette` | Verified |
| Gaussian Blur Effect | `web/src/effects/definitions/blur.ts` | `catchim::render::Compositor::applyGaussianBlur` | Verified |
| Chroma Key (Green Screen) | `web/src/native/opencut-wasm-compat.ts` | `catchim::render::Compositor::applyChromaKey` | Verified |
| Grayscale Filter | `web/src/native/opencut-wasm-compat.ts` | `catchim::render::Compositor::applyGrayscale` | Verified |
| Text Element Rasterization | `web/src/services/renderer/nodes/text-node.ts` | `catchim::render::TextRasterizer` | Verified |

---

## 7. Export Pipeline
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| Offline Frame Rendering | `web/src/services/renderer/scene-exporter.ts` | `catchim::exporting::SceneExporter` | Verified |
| Export Settings (Bitrate, Presets, FPS, Quality) | `web/src/export/defaults.ts` | `catchim::exporting::ExportSettings` | Verified |
| Progress Reporting & Cancellation | `web/src/components/editor/export-button.tsx` | `catchim::exporting::SceneExporter::setProgressCallback` | Verified |

---

## 8. Selection System
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| Single Selection | `web/src/selection/state.ts` | `catchim::editor::SelectionManager::select` | Verified |
| Multi / Toggle Selection (Ctrl+Click) | `web/src/selection/state.ts` | `catchim::editor::SelectionManager::toggle` | Verified |
| Range Selection (Shift+Click) | `web/src/selection/state.ts` | `catchim::editor::SelectionManager::selectRange` | Verified |
| Box / Marquee Selection | `web/src/selection/state.ts` | `catchim::editor::SelectionManager::applyBoxSelection` | Verified |
| Selection Pruning & Cleanup | `web/src/selection/state.ts` | `catchim::editor::SelectionManager::prune` | Verified |

---

## 9. Retime & Speed Control
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| Rate Clamping (0.01x - 5.0x) | `web/src/retime/rate.ts` | `catchim::editor::RetimeEngine::clampRate` | Verified |
| Time Mapping (Clip <-> Source) | `web/src/retime/resolve.ts` | `catchim::editor::RetimeEngine::getSourceTimeAtClipTime` | Verified |
| Duration Span Scaling | `web/src/retime/resolve.ts` | `catchim::editor::RetimeEngine::getTimelineDurationForSourceSpan` | Verified |
| Change Speed Command (Undo/Redo) | `web/src/actions/` | `catchim::editor::ChangeClipSpeedCommand` | Verified |

---

## 10. Subtitles & Captions (SRT)
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| SRT Parsing (SubRip format) | `web/src/subtitles/srt.ts` | `catchim::subtitles::SrtParser::parse` | Verified |
| Subtitle Timestamp Conversion | `web/src/subtitles/srt.ts` | `catchim::subtitles::SrtParser::parseTimestamp` | Verified |
| SRT Serialization / Export | `web/src/subtitles/` | `catchim::subtitles::SrtSerializer::serialize` | Verified |
| Import Subtitles as Text Track | `web/src/subtitles/insert.ts` | `catchim::editor::ImportSubtitlesCommand` | Verified |

---

## 11. Vector Shapes & Background Renderer
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| Solid Background Fill | `web/src/background/` | `catchim::render::ShapeRenderer::drawSolid` | Verified |
| Linear Gradient (arbitrary angle) | `web/src/gradients/` | `catchim::render::ShapeRenderer::drawLinearGradient` | Verified |
| Radial Gradient (center to edge) | `web/src/gradients/` | `catchim::render::ShapeRenderer::drawRadialGradient` | Verified |
| Vector Rectangle with Corner Radius & Stroke | `web/src/graphics/definitions/rectangle.ts` | `catchim::render::ShapeRenderer::drawRectangle` | Verified |
| Vector Ellipse / Circle with Stroke | `web/src/graphics/definitions/ellipse.ts` | `catchim::render::ShapeRenderer::drawEllipse` | Verified |

---

## 12. Advanced Audio & Image Filters
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| Audio Linear Resampling | `web/src/media/audio.ts` | `catchim::audio::AudioResampler::resampleLinear` | Verified |
| Audio Fade In / Fade Out Ramping | `web/src/timeline/components/audio-volume-line.tsx` | `catchim::audio::AudioResampler::applyFadeRamp` | Verified |
| Shape Alpha Masking (Rect / Ellipse + Feather) | `web/src/masks/` | `catchim::render::MaskEngine::generateMask` | Verified |
| Crop Box & Anchor Point Transform | `web/src/canvas/` | `catchim::render::Transform::crop`, `anchorX/Y` | Verified |
| Extended Color Grading (Exposure, Temp, Tint, Gamma) | `web/src/native/opencut-wasm-compat.ts` | `catchim::render::Compositor::applyExtendedColorGrading` | Verified |
| Invert & Sepia Filters | `web/src/native/opencut-wasm-compat.ts` | `catchim::render::Compositor::applyInvert`, `applySepia` | Verified |
| Project Migrator (Schema < v31 -> v31) | `web/src/services/storage/migrations/` | `catchim::storage::ProjectMigrator::migrate` | Verified |
| Timeline Zoom & Viewport Presets | `web/src/timeline/zoom-utils.ts` | `catchim::editor::TimelineZoomController` | Verified |

---

## 13. Audio Mastering & Dynamics Limiter
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| Peak Amplitude Detection | `web/src/media/audio-mastering.ts` | `catchim::audio::AudioMastering::computePeak` | Verified |
| Stereo Downmixing (to mono) | `web/src/media/audio.ts` | `catchim::audio::AudioMastering::downmixStereo` | Verified |
| Stereo Field Panning | `web/src/media/audio.ts` | `catchim::audio::AudioMastering::applyPanning` | Verified |
| Brickwall Master Limiter (Threshold, Ratio, Headroom) | `web/src/media/audio-mastering.ts` | `catchim::audio::AudioMastering::applyLimiter` | Verified |
| Peak Normalization | `web/src/media/audio-mastering.ts` | `catchim::audio::AudioMastering::normalizePeak` | Verified |

---

## 14. Canvas Guides & Platform Safe Zones
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| Rule-of-Thirds & Crosshair Grid | `web/src/guides/grid.ts` | `catchim::render::SafeZoneGuide` | Verified |
| TikTok Safe Zone Overlay | `web/src/guides/definitions/tiktok-layout.tsx` | `catchim::render::SafeZoneGuide::renderGuideOverlay` | Verified |
| Instagram Reels Safe Zone | `web/src/guides/definitions/platforms.tsx` | `catchim::render::SafeZoneGuide::getMargins` | Verified |
| YouTube Shorts Safe Zone | `web/src/guides/definitions/platforms.tsx` | `catchim::render::SafeZoneGuide::getMargins` | Verified |

---

## 15. Video Transition Engine
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| Crossfade (Alpha cross-dissolve) | `web/src/` | `catchim::render::TransitionEngine::blend` | Verified |
| Fade to Black / Fade to White | `web/src/` | `catchim::render::TransitionEngine::blend` | Verified |
| Spatial Wipe (Left / Right) | `web/src/` | `catchim::render::TransitionEngine::blend` | Verified |
| Slide Transition (Left / Right) | `web/src/` | `catchim::render::TransitionEngine::blend` | Verified |
| Center Zoom In Transition | `web/src/` | `catchim::render::TransitionEngine::blend` | Verified |

---

## 16. Multi-Track Ripple Editing Pipeline
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| Downstream Clip Shifting | `web/src/timeline/update-pipeline.ts` | `catchim::editor::RipplePipeline::shiftClipsAfter` | Verified |
| Ripple Insert (Single & All Tracks) | `web/src/timeline/update-pipeline.ts` | `catchim::editor::RippleInsertCommand` | Verified |
| Ripple Move & Gap Maintenance | `web/src/timeline/update-pipeline.ts` | `catchim::editor::RippleMoveCommand` | Verified |

---

## 17. Project Bundling & Media Archiver
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| Media Reference Validation | `web/src/media/` | `catchim::storage::ProjectBundle::validateMediaReferences` | Verified |
| Portable Bundle Creation | `web/src/storage/` | `catchim::storage::ProjectBundle::createBundle` | Verified |
| Media Relinker & Path Migration | `web/src/storage/` | `catchim::storage::ProjectBundle::relinkMedia` | Verified |

---

## 18. Waveform Peak Bucketing Engine
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| Peak Buckets Calculation | `web/src/media/waveform-summary.ts` | `catchim::media::WaveformBucketer::computePeakBuckets` | Verified |
| Multi-channel Peak Reduction | `web/src/media/waveform-summary.ts` | `catchim::media::WaveformBucketer::computeMultiChannelPeakBuckets` | Verified |
| WaveformData Generation & Cache | `web/src/media/waveform-summary.ts` | `catchim::media::WaveformBucketer::generateWaveformData` | Verified |

---

## 19. Canvas Hit Testing & Transform Gizmo Handles
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| AABB Point-in-Rect Hit Test | `web/src/selection/hit-testing.ts` | `catchim::render::HitTesting::containsPoint` | Verified |
| Rotated OBB Point Hit Test | `web/src/selection/hit-testing.ts` | `catchim::render::HitTesting::containsPointRotated` | Verified |
| 8 Gizmo Handles & Rotate Handle | `web/src/selection/` | `catchim::render::HitTesting::hitTestGizmoHandle` | Verified |
| Marquee Box Element Selection | `web/src/selection/hit-testing.ts` | `catchim::render::HitTesting::resolveElementIntersections` | Verified |

---

## 20. Export Geometry & Bitrate Resolver
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| Even Dimension Constraint (roundToEven) | `web/src/export/index.ts` | `catchim::exporting::ExportGeometryResolver::roundToEven` | Verified |
| Aspect Ratio Preserving Presets (480p-8K) | `web/src/export/index.ts` | `catchim::exporting::ExportGeometryResolver::resolveDimensions` | Verified |
| Target Bitrate Calculation (bpp model) | `web/src/export/` | `catchim::exporting::ExportGeometryResolver::calculateVideoBitrate` | Verified |

---

## 21. Project Diagnostics & Gap Detector
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| Timeline Statistics & Resource Audit | `web/src/diagnostics/` | `catchim::editor::ProjectDiagnostics::analyze` | Verified |
| Track Clip Gap Detection | `web/src/timeline/` | `catchim::editor::ProjectDiagnostics::findGapsOnTrack` | Verified |
| Overlapping Clips Validation | `web/src/timeline/` | `catchim::editor::ProjectDiagnostics::analyze` | Verified |

---

## 22. Still Image & Poster Frame Exporter
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| Timeline Frame RGBA Renderer | `web/src/export/` | `catchim::exporting::StillImageExporter::renderFrameRgba` | Verified |
| 32-bit BMP Direct Encoder | `web/src/export/` | `catchim::exporting::StillImageExporter::encodeBmp` | Verified |
| Disk Snapshot & Thumbnail Writer | `web/src/export/` | `catchim::exporting::StillImageExporter::saveSnapshot` | Verified |

---

## 23. Timeline Ruler & ViewModel Subsystem
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| Ruler Subdivision & Interval Math | `web/src/timeline/ruler-utils.ts` | `catchim::editor::RulerEngine::getRulerConfig` | Verified |
| Timestamp & Frame Label Formatting | `web/src/timeline/ruler-utils.ts` | `catchim::editor::RulerEngine::formatRulerLabel` | Verified |
| Viewport Scrolling & Time Mapping | `web/src/timeline/pixel-utils.ts` | `catchim::editor::TimelineViewModel` | Verified |
| Playhead Auto-Scroll & Viewport Tracking | `web/src/timeline/controllers/` | `catchim::editor::TimelineViewModel::ensurePlayheadVisible` | Verified |
| Track Layout Geometries & Marquee Selection | `web/src/timeline/` | `catchim::editor::TimelineViewModel::getTrackLayouts`, `finishMarquee` | Verified |

---

## 24. Preview Canvas Snapping & Interaction ViewModel
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| Canvas Center & Boundary Snapping | `web/src/preview/preview-snap.ts` | `catchim::render::PreviewSnap::snapPosition` | Verified |
| 90-degree Increment Rotation Snapping | `web/src/preview/preview-snap.ts` | `catchim::render::PreviewSnap::snapRotation` | Verified |
| Uniform & Non-Uniform Scale Snapping | `web/src/preview/preview-snap.ts` | `catchim::render::PreviewSnap::snapScale`, `snapScaleAxes` | Verified |
| Canvas-to-Screen Transform & ZoomToFit | `web/src/preview/preview-coords.ts` | `catchim::render::PreviewViewModel::zoomToFit`, `canvasToScreen` | Verified |
| 8 Gizmo Handles & Interactive Dragging | `web/src/preview/components/` | `catchim::render::PreviewViewModel::getGizmoHandles`, `updateDrag` | Verified |

---

## 25. Real-Time Audio Playback Pipeline
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| dB to Linear Gain Curves | `web/src/timeline/audio-state.ts` | `catchim::audio::AudioPlaybackEngine::dBToLinear`, `clampDb` | Verified |
| Keyframe Volume Automation Resolution | `web/src/timeline/audio-state.ts` | `catchim::audio::AudioPlaybackEngine::resolveEffectiveAudioGain` | Verified |
| Downsampled Waveform Gain Profiles | `web/src/timeline/audio-state.ts` | `catchim::audio::AudioPlaybackEngine::buildWaveformGainSamples` | Verified |
| Real-time Interleaved Slice Mixer & Limiting | `web/src/core/managers/audio-manager.ts` | `catchim::audio::AudioPlaybackEngine::renderAudioSlice` | Verified |

---

## 26. Video Frame Cache Ring Buffer
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| Decoded Video Frame Caching | `web/src/services/video-cache/service.ts` | `catchim::media::VideoFrameCache::getFrameAt` | Verified |
| Seek Generation Cancellation | `web/src/services/video-cache/service.ts` | `catchim::media::VideoFrameCache::incrementGeneration` | Verified |
| Bounded Memory & LRU Frame Eviction | `web/src/services/video-cache/service.ts` | `catchim::media::VideoFrameCache::evictIfNeededLocked` | Verified |
| Stream Invalidation & Cache Reset | `web/src/services/video-cache/service.ts` | `catchim::media::VideoFrameCache::invalidateMedia`, `clear` | Verified |

---

## 27. Properties Inspector ViewModel
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| Multi-Selection Inspector State | `web/src/components/editor/panels/properties/` | `catchim::editor::PropertiesViewModel::getSelectionState` | Verified |
| Dynamic Property Tabs Registry | `web/src/components/editor/panels/properties/registry.tsx` | `catchim::editor::PropertiesViewModel::getAvailableTabs` | Verified |
| Tab Persistence per Element Type | `web/src/components/editor/panels/properties/stores/` | `catchim::editor::PropertiesViewModel::setActiveTab` | Verified |
| Bidirectional Property Mutation Binding | `web/src/components/editor/panels/properties/` | `catchim::editor::PropertiesViewModel::setPositionX`, `setSpeed`, etc. | Verified |

---

## 28. Subtitles Transcription & Dynamic Caption Generator
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| Whisper Output JSON Parser | `web/src/transcription/caption.ts` | `catchim::subtitles::TranscriptionEngine::parseWhisperJson` | Verified |
| Dynamic Word Chunking & Pacing | `web/src/transcription/caption.ts` | `catchim::subtitles::TranscriptionEngine::buildCaptionChunks` | Verified |
| Subtitle Minimum Duration Constraint | `web/src/transcription/caption.ts` | `catchim::subtitles::TranscriptionEngine::buildCaptionChunks` | Verified |

---

## 29. Sticker & Graphic Asset Registry
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| Preset Categorization (Arrows, Emojis, Badges, etc.) | `web/src/stickers/` | `catchim::media::StickerRegistry::getCategories`, `getStickersByCategory` | Verified |
| Aspect Ratio & Intrinsic Size Presets | `web/src/stickers/` | `catchim::media::StickerItem::aspectRatio` | Verified |
| Sticker-to-Timeline Clip Factory | `web/src/stickers/` | `catchim::media::StickerRegistry::createStickerClip` | Verified |

---

## 30. Spatial Motion Path & Catmull-Rom Trajectory
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| 2D Motion Path Waypoints | `web/src/animation/` | `catchim::editor::SpatialMotionPath::addPoint` | Verified |
| Catmull-Rom Smooth Spline Evaluation | `web/src/animation/` | `catchim::editor::SpatialMotionPath::evaluateAt` | Verified |
| Tangent Heading / Auto-Orient Angle | `web/src/animation/` | `catchim::editor::EvaluatedPose::headingDegrees` | Verified |
| Arc-Length Along Path Integration | `web/src/animation/` | `catchim::editor::SpatialMotionPath::totalLength` | Verified |

---

## 31. Lift / Gamma / Gain Color Wheels Engine
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| Shadow / Midtone / Highlight Wheels | `web/src/native/opencut-wasm-compat.ts` | `catchim::render::ColorWheelGrade` | Verified |
| Fast 256-entry RGBA LUT Generation | `web/src/native/opencut-wasm-compat.ts` | `catchim::render::ColorWheelEngine::applyColorWheelGrading` | Verified |
| Master & Per-Channel Wheel Adjustments | `web/src/native/opencut-wasm-compat.ts` | `catchim::render::ColorWheelEngine` | Verified |

---

## 32. Intelligent Audio Ducking
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| Voice Activity Energy Detection | `web/src/core/managers/audio-manager.ts` | `catchim::audio::AudioDucker::applyDucking` | Verified |
| Attack / Hold / Release Envelope Follower | `web/src/core/managers/audio-manager.ts` | `catchim::audio::DuckingConfig`, `AudioDucker` | Verified |
| Background Music Gain Attenuation | `web/src/core/managers/audio-manager.ts` | `catchim::audio::AudioDucker::applyDucking` | Verified |

---

## 33. Preset & Template Asset Manager
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| Aspect Ratio Geometry Presets (16:9, 9:16, 1:1, etc.) | `web/src/canvas/` | `catchim::media::PresetManager::aspectRatios` | Verified |
| Text Title Styles & Formatting Presets | `web/src/services/renderer/nodes/text-node.ts` | `catchim::media::PresetManager::textPresets` | Verified |
| Video Transition Duration & Type Presets | `web/src/transitions/` | `catchim::media::PresetManager::transitionPresets` | Verified |
| Audio Mastering Dynamic Ceiling Presets | `web/src/core/managers/audio-manager.ts` | `catchim::media::PresetManager::audioPresets` | Verified |

---

## 34. Headless Scripting & CLI Engine Runner
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| Project Diagnostics & Integrity CLI | CLI tool | `catchim::app::Application::run` (`--diagnose`) | Verified |
| Headless Frame Rendering Snapshot CLI | CLI tool | `catchim::app::Application::run` (`--render-frame`) | Verified |
| Engine Version & Schema Introspection CLI | CLI tool | `catchim::app::Application::run` (`--version`) | Verified |

---

## 35. End-to-End Pipeline Verification Suite
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| Multi-track Timeline Assembly (Video/Text/Audio/Graphic) | Full system | `tests/catchim_tests.cpp::runEndToEndPipelineIntegrationTests` | Verified |
| Cross-subsystem Motion + Ducking + Grading Integration | Full system | `tests/catchim_tests.cpp::runEndToEndPipelineIntegrationTests` | Verified |
| Atomic Save/Load Parity & Still Frame RGBA Export | Full system | `tests/catchim_tests.cpp::runEndToEndPipelineIntegrationTests` | Verified |

---

## 36. Multi-Clip Group Move & Timeline Placement
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| Anchored Group Construction & Offsets | `web/src/timeline/group-move/build-group.ts` | `catchim::editor::GroupMoveEngine::buildMoveGroup` | Verified |
| Relative Multi-track Translation Clamping | `web/src/timeline/group-move/resolve-move.ts` | `catchim::editor::GroupMoveEngine::resolveGroupMove` | Verified |
| Undoable Atomic Group Move Command | `web/src/timeline/group-move/` | `catchim::editor::GroupMoveCommand` | Verified |

---

## 37. Text-To-Speech Voices & WAV RIFF Encoder
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| Vietnamese & Multi-lingual Voice Catalog | `web/src/services/tts/voices.ts` | `catchim::audio::TtsEngine::voices` | Verified |
| Speech Duration & Syllable Estimation | `web/src/services/tts/tts-service.ts` | `catchim::audio::TtsEngine::estimateSpeechDuration` | Verified |
| Standard 16-bit PCM RIFF WAV Binary Writer | `web/src/services/tts/tts-service.ts` | `catchim::audio::TtsEngine::encodePcm16Wav` | Verified |

---

## 38. Sound Effects & Audio FX Library
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| Sfx Categories (Whoosh, Impact, Transition, UI, Ambience) | `web/src/sounds/` | `catchim::media::SoundEffectsRegistry::getCategories` | Verified |
| Sound Search & Tag Matching | `web/src/sounds/` | `catchim::media::SoundEffectsRegistry::searchEffects` | Verified |
| Audio SFX Timeline Clip Generator | `web/src/sounds/` | `catchim::media::SoundEffectsRegistry::createSfxClip` | Verified |

---

## 39. Typography & Font Fallback Resolver
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| System & Display Font Catalog | `web/src/fonts/system-fonts.ts` | `catchim::render::FontRegistry::availableFonts` | Verified |
| Case-Insensitive Name & ID Matching | `web/src/fonts/` | `catchim::render::FontRegistry::findFont` | Verified |
| Graceful Fallback Family Resolution | `web/src/fonts/` | `catchim::render::FontRegistry::resolveFontFamily` | Verified |
| Nearest Font Weight Clamping | `web/src/fonts/` | `catchim::render::FontRegistry::resolveNearestWeight` | Verified |

---

## 40. Rational Frame Rate & Video FPS Elevation
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| Standard Frame Rates (23.976, 29.97, 59.94, etc.) | `web/src/fps/presets.ts` | `catchim::core::RationalFrameRateHelper::standardRates` | Verified |
| Float to Exact Rational Rate (GCD Reduction) | `web/src/fps/utils.ts` | `catchim::core::RationalFrameRateHelper::fromFloat` | Verified |
| Rational Frame Ticks Rounding & Floor | `web/src/fps/utils.ts` | `catchim::core::RationalFrameRateHelper::roundFrameTicks` | Verified |
| Automatic Project FPS Elevation on Media Import | `web/src/fps/utils.ts` | `catchim::core::RationalFrameRateHelper::getRaisedProjectFpsForImportedMedia` | Verified |

---

## 41. Multi-Clip Group Resize Engine
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| Multi-clip Left/Right Trimming & Frame Snapping | `web/src/timeline/group-resize/compute-resize.ts` | `catchim::editor::GroupResizeEngine::computeGroupResize` | Verified |
| Neighbor Collision & Minimum 1-Frame Bound | `web/src/timeline/group-resize/compute-resize.ts` | `catchim::editor::GroupResizeEngine::computeGroupResize` | Verified |
| Source Media Duration Extension Constraint | `web/src/timeline/group-resize/compute-resize.ts` | `catchim::editor::GroupResizeEngine::computeGroupResize` | Verified |
| Atomic Undo/Redo Group Resize Command | `web/src/timeline/group-resize/` | `catchim::editor::GroupResizeCommand` | Verified |

---

## 42. Track Placement & Magnetic Boundary Engine
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| O(log N) Track Collision Check with Max Bound Array | `web/src/timeline/placement/overlap.ts` | `catchim::editor::PlacementEngine::canPlaceTimeSpansOnTrack` | Verified |
| Earliest Available Gap Scanner | `web/src/native/opencut-core.ts` (`findAvailableGap`) | `catchim::editor::PlacementEngine::findAvailableGap` | Verified |
| Magnetic Main Track Start Alignment (Snap to t=0) | `web/src/timeline/placement/main-track.ts` | `catchim::editor::PlacementEngine::enforceMainTrackStart` | Verified |
| Element-to-Track Compatibility Matrix | `web/src/timeline/placement/compatibility.ts` | `catchim::editor::PlacementEngine::canClipGoOnTrack` | Verified |
| Preferred Layer Hierarchy Track Placement | `web/src/timeline/placement/insert-index.ts` | `catchim::editor::PlacementEngine::resolvePreferredTrackPlacement` | Verified |

---

## 43. Audio DSP Biquad Filters & 3-Band Parametric Equalizer
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| RBJ Audio EQ Cookbook Biquad Filter (Low/Highpass) | DSP Engine | `catchim::audio::BiquadFilter` | Verified |
| Peaking Bell & Low/High Shelf Filters | DSP Engine | `catchim::audio::BiquadFilter::configure` | Verified |
| 3-Band Parametric Equalizer (Bass, Mid, Treble) | DSP Engine | `catchim::audio::ThreeBandEqualizer` | Verified |
| Stereo In-Place Direct Form II Transposed Processing | DSP Engine | `catchim::audio::ThreeBandEqualizer::processStereo` | Verified |

---

## 44. Color Space & CSS Extraction Utils
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| Hex <-> RGB / HSV / HSL Conversions | `web/src/utils/color.ts` | `catchim::core::ColorUtils::hexToRgb`, `hexToHsv`, `hexToHsl` | Verified |
| Alpha Channel Parsing & Hex8 Formatting | `web/src/utils/color.ts` | `catchim::core::ColorUtils::parseHexAlpha`, `appendAlpha` | Verified |
| Text & CSS Noise Stripping Color Parser | `web/src/utils/color.ts` | `catchim::core::ColorUtils::extractColorFromText` | Verified |
| Multiformat Color Input Parser & Formatter | `web/src/utils/color.ts` | `catchim::core::ColorUtils::formatColorValue`, `parseColorInput` | Verified |

---

## 45. Timeline Layout & Keyframe Lanes Metrics
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| Track Heights (Video 65px, Audio 50px, Text/Graphic 25px) | `web/src/timeline/components/layout.ts` | `catchim::editor::TimelineLayoutEngine::getTrackHeight` | Verified |
| Cumulative Heights & Vertical Scroll Offsets | `web/src/timeline/components/track-layout.ts` | `catchim::editor::TimelineLayoutEngine::getTrackLayoutOffsets` | Verified |
| Expanded Track Height Calculation | `web/src/timeline/components/track-layout.ts` | `catchim::editor::TimelineLayoutEngine::getExpandedTrackHeight` | Verified |
| Animated Property Rows & Labels Resolver | `web/src/timeline/components/expanded-layout.ts` | `catchim::editor::TimelineLayoutEngine::getExpandedRows` | Verified |

---

## 46. Audio Display Metrics & Logarithmic Waveform Mapping
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| Volume Line Non-linear Slider Quadratic Curve | `web/src/timeline/audio-display.ts` | `catchim::audio::AudioDisplayMetrics::getLinePosFromDb` | Verified |
| Inverse Line Percentage to dB Converter | `web/src/timeline/audio-display.ts` | `catchim::audio::AudioDisplayMetrics::getDbFromLinePos` | Verified |
| Power-Law Logarithmic Waveform Bar Fraction | `web/src/timeline/audio-display.ts` | `catchim::audio::AudioDisplayMetrics::getBarFractionFromOutputAmplitude` | Verified |

---

## 47. Advanced Multi-Source Snapping Engine
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| Zoom-Adaptive Snap Threshold (Ticks/Pixel) | `web/src/timeline/snapping/threshold.ts` | `catchim::editor::AdvancedSnapEngine::getTimelineSnapThresholdInTicks` | Verified |
| Animation Keyframe Snap Point Generator | `web/src/timeline/animation-snap-points.ts` | `catchim::editor::AdvancedSnapEngine::collectAllSnapPoints` | Verified |
| Fast O(log N) Binary Search Timeline Snapping | `web/src/timeline/snapping/resolve.ts` | `catchim::editor::AdvancedSnapEngine::resolveSortedTimelineSnap` | Verified |

---

## 48. Batch & Tracks Snapshot Transaction Commands
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| Atomic Batch Command Execution & Rollback | `web/src/commands/batch-command.ts` | `catchim::editor::BatchCommand` | Verified |
| Track Hierarchy Snapshots | `web/src/timeline/` | `catchim::editor::TimelineTracksSnapshot`, `Timeline::createSnapshot`, `restoreSnapshot` | Verified |
| Whole-Timeline Tracks State Restoration | `web/src/commands/timeline/tracks-snapshot.ts` | `catchim::editor::TracksSnapshotCommand` | Verified |

---

## 49. Element Factory & Element Utilities
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| Standard Element Builders (Video, Audio, Image, Text, Sticker, Graphic, Effect) | `web/src/timeline/creation.ts`, `element-utils.ts` | `catchim::editor::ElementFactory::build*` | Verified |
| Element Type Predicates (isVisual, canHaveAudio, isMaskable, isRetimable) | `web/src/timeline/element-utils.ts` | `catchim::editor::ElementUtils::*` | Verified |
| Active Elements at Time Query | `web/src/timeline/element-utils.ts` | `catchim::editor::ElementUtils::getElementsAtTime` | Verified |
| Font Families Extraction | `web/src/timeline/element-utils.ts` | `catchim::editor::ElementUtils::getElementFontFamilies` | Verified |

---

## 50. Animation Channels & Keyframe Clipboard Engine
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| Multi-Channel Keyframe Storage on Clip | `web/src/timeline/` | `catchim::editor::Clip::animationChannels`, `getOrCreateAnimationChannel` | Verified |
| Multi-Channel Keyframe Copy with Relative Normalization | `web/src/clipboard/handlers/keyframes.ts` | `catchim::editor::ClipboardKeyframeEngine::copy` | Verified |
| Boundary-Clamped Multi-Channel Keyframe Paste with Undo/Redo | `web/src/commands/timeline/clipboard/paste-keyframes.ts` | `catchim::editor::PasteKeyframesCommand`, `ClipboardKeyframeEngine::paste` | Verified |

---

## 51. Track Lifecycle & Clip Duplication Commands
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| Indexed Track Insertion & Deletion | `web/src/commands/timeline/track/add-track.ts`, `remove-track.ts` | `catchim::editor::AddTrackCommand`, `RemoveTrackCommand` | Verified |
| Track Mute & Visibility Toggling | `web/src/commands/timeline/track/toggle-track-mute.ts`, `toggle-track-visibility.ts` | `catchim::editor::ToggleTrackMuteCommand`, `ToggleTrackVisibilityCommand` | Verified |
| Whole-Track & Selected Clip Duplication | `web/src/commands/timeline/element/duplicate-elements.ts` | `catchim::editor::DuplicateElementsCommand` | Verified |

---

## 52. Multi-Pass Separable Gaussian Blur Pipeline
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| Intensity to Relative Sigma Scaling | `web/src/effects/definitions/blur.ts` | `catchim::render::BlurEffect::intensityToSigma` | Verified |
| Multi-Pass Iteration & Downsampling Step Decomposer | `web/src/effects/definitions/blur.ts` | `catchim::render::BlurEffect::buildGaussianBlurPasses` | Verified |
| Discrete 1D Normalized Gaussian Kernel Generator | `web/src/effects/definitions/blur.ts` | `catchim::render::BlurEffect::compute1DGaussianKernel` | Verified |

---

## 53. Parametric Vector Graphic Geometry & Aligned Strokes
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| Parametric N-Point Star Geometry Generator | `web/src/graphics/definitions/star.ts` | `catchim::render::StarGeometry::generateVertices` | Verified |
| Regular N-Gon Polygon Geometry Generator | `web/src/graphics/definitions/polygon.ts` | `catchim::render::PolygonGeometry::generateVertices` | Verified |
| Center / Inside / Outside Aligned Stroke Metrics | `web/src/graphics/stroke.ts` | `catchim::render::AlignedStrokeMetrics::getEffectiveStrokeWidth`, `computeExpandedBounds` | Verified |

---

## 54. Interactive Keyframe Editing Commands
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| Clamped Keyframe Upsertion & Channel Backup | `web/src/commands/timeline/element/keyframes/upsert-keyframe.ts` | `catchim::editor::UpsertKeyframeCommand` | Verified |
| Exact Keyframe Removal & State Rollback | `web/src/commands/timeline/element/keyframes/remove-keyframe.ts` | `catchim::editor::RemoveKeyframeCommand` | Verified |
| Keyframe Retiming & Timestamp Clamping | `web/src/commands/timeline/element/keyframes/retime-keyframe.ts` | `catchim::editor::RetimeKeyframeCommand` | Verified |
| Keyframe Interpolation Curve & Tangent Update | `web/src/commands/timeline/element/keyframes/update-scalar-keyframe-curve.ts` | `catchim::editor::UpdateKeyframeCurveCommand` | Verified |

---

## 55. Multi-Line Text Layout & Typography Engine
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| Multi-Line Text Measurement & Visual Center Offset | `web/src/text/layout.ts` | `catchim::render::TextLayoutEngine::measureTextBlock` | Verified |
| Text Alignment Offsets (Left, Center, Right) | `web/src/text/layout.ts` | `catchim::render::TextLayoutEngine::getTextRect` | Verified |
| Padded Text Background Bounding Box | `web/src/text/layout.ts` | `catchim::render::TextLayoutEngine::computeTextBackgroundRect` | Verified |
| Word-Bound Word-Wrapping Algorithm | `web/src/text/layout.ts`, `primitives.ts` | `catchim::render::TextLayoutEngine::wrapTextToLines` | Verified |

---

## 56. Freeform Bezier Path Mask Geometry & Subdivision
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| Freeform Tangent Points JSON Serialization | `web/src/masks/freeform/path.ts` | `catchim::render::FreeformMaskGeometry::parseFreeformPath`, `serializeFreeformPath` | Verified |
| Cubic Bezier Evaluation on Spline Segments | `web/src/masks/freeform/path.ts` | `catchim::render::FreeformMaskGeometry::evaluateCubicBezier` | Verified |
| de Casteljau Segment Subdivision & Continuous In/Out Tangents | `web/src/commands/timeline/element/masks/insert-custom-mask-point.ts` | `catchim::render::FreeformMaskGeometry::insertPointOnSegment` | Verified |
| Centroid Normalization & Path Recentering | `web/src/masks/freeform/path.ts` | `catchim::render::FreeformMaskGeometry::recenterPath` | Verified |

---

## 57. Mask Editing & Freeform Path Point Commands
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| Insert Freeform Mask Point on Spline Segment | `web/src/commands/timeline/element/masks/insert-custom-mask-point.ts` | `catchim::editor::InsertCustomMaskPointCommand` | Verified |
| Delete Freeform Mask Points & Auto Closed State | `web/src/commands/timeline/element/masks/delete-custom-mask-points.ts` | `catchim::editor::DeleteCustomMaskPointsCommand`, `FreeformMaskGeometry::removeFreeformPathPoints` | Verified |
| Remove Mask from Element | `web/src/commands/timeline/element/masks/remove-mask.ts` | `catchim::editor::RemoveMaskCommand` | Verified |
| Toggle Mask Inverted Parameter | `web/src/commands/timeline/element/masks/toggle-mask-inverted.ts` | `catchim::editor::ToggleMaskInvertedCommand` | Verified |

---

## 58. Visual Clip Effect Editing Commands & Serialization
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| Add Visual Clip Effect | `web/src/commands/timeline/element/effects/add-effect.ts` | `catchim::editor::AddClipEffectCommand` | Verified |
| Remove Visual Clip Effect | `web/src/commands/timeline/element/effects/remove-effect.ts` | `catchim::editor::RemoveClipEffectCommand` | Verified |
| Toggle Visual Clip Effect Enabled | `web/src/commands/timeline/element/effects/toggle-effect.ts` | `catchim::editor::ToggleClipEffectCommand` | Verified |
| Reorder Visual Clip Effects Stack | `web/src/commands/timeline/element/effects/reorder-effect.ts` | `catchim::editor::ReorderClipEffectsCommand` | Verified |
| Update / Patch Visual Clip Effect Params | `web/src/commands/timeline/element/effects/update-effect-params.ts` | `catchim::editor::UpdateClipEffectParamsCommand` | Verified |
| Clip Effects & Masks JSON Serialization Roundtrip | `web/src/timeline/types.ts` | `catchim::editor::ProjectSerializer`, `Clip::effects`, `Clip::masks` | Verified |

---

## 59. Canvas Viewport Pan-Zoom & Coordinate Geometry Controller
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| Viewport Fit Scale & Zoom Clamping | `web/src/preview/zoom.ts`, `preview-viewport.tsx` | `catchim::editor::CanvasViewportController::getFitScale`, `clampZoom` | Verified |
| Viewport Center Clamping during Pan | `web/src/preview/components/preview-viewport.tsx` | `catchim::editor::CanvasViewportController::clampViewportCenter` | Verified |
| Screen-to-Canvas & Canvas-to-Overlay Transformations | `web/src/preview/preview-coords.ts` | `catchim::editor::CanvasViewportController::screenToCanvas`, `canvasToOverlay` | Verified |
| Position-to-Overlay & Logical Threshold Resolution | `web/src/preview/preview-coords.ts` | `catchim::editor::CanvasViewportController::positionToOverlay`, `screenPixelsToLogicalThreshold` | Verified |
| Element Bounds Corner & Edge Handle Geometric Placement | `web/src/preview/element-bounds.ts` | `catchim::editor::ElementBounds::getCornerPosition`, `getEdgeHandlePosition` | Verified |

---

## 60. Multi-Element Atomic Deletion & Batch Parameter Updating
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| Multi-Track Atomic Element Deletion | `web/src/commands/timeline/element/delete-elements.ts` | `catchim::editor::DeleteElementsCommand` | Verified |
| Multi-Track Batch Element Property Updating | `web/src/commands/timeline/element/update-elements.ts` | `catchim::editor::UpdateElementsCommand` | Verified |

---

## 61. Effect Parameter Animation Engine & Keyframe Commands
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| Effect Param Path Builder & Parser (`effects.<id>.params.<key>`) | `web/src/animation/effect-param-channel.ts` | `catchim::editor::EffectParamAnimationEngine::buildEffectParamPath`, `parseEffectParamPath` | Verified |
| Real-Time Dynamic Effect Parameter Curve Evaluation | `web/src/animation/effect-param-channel.ts` | `catchim::editor::EffectParamAnimationEngine::resolveEffectParamsAtTime` | Verified |
| Upsert Keyframe on Effect Parameter Channel | `web/src/commands/timeline/element/keyframes/upsert-effect-param-keyframe.ts` | `catchim::editor::UpsertEffectParamKeyframeCommand` | Verified |
| Remove Keyframe from Effect Parameter Channel | `web/src/commands/timeline/element/keyframes/remove-effect-param-keyframe.ts` | `catchim::editor::RemoveEffectParamKeyframeCommand` | Verified |

---

## 62. Interactive Transform Handle Gizmo Session & Real-Time Snapping
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| Corner Uniform Scale Session with Anchor Distance | `web/src/preview/controllers/transform-handle-controller.ts` | `catchim::render::TransformHandleSession::startCornerScale`, `update` | Verified |
| Edge Directional Scale Session along Local Rotated Axes | `web/src/preview/controllers/transform-handle-controller.ts` | `catchim::render::TransformHandleSession::startEdgeScale`, `update` | Verified |
| Angular Rotation Tracking with 45°/90° Step Snapping | `web/src/preview/controllers/transform-handle-controller.ts` | `catchim::render::TransformHandleSession::startRotation`, `update` | Verified |
| Commit Handle Session to Element Transform Patch | `web/src/preview/controllers/transform-handle-controller.ts` | `catchim::render::HandleTransformResult::toPatchParams` | Verified |

---

## 63. Multi-Track Element Split Command & Animation Curve Division
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| Multi-Track Atomic Element Splitting | `web/src/commands/timeline/element/split-elements.ts` | `catchim::editor::SplitElementsCommand` | Verified |
| Retain Side Modes (Both, Left, Right) | `web/src/commands/timeline/element/split-elements.ts` | `catchim::editor::RetainSide` (Both, Left, Right) | Verified |
| Animation Channel Splitting & Keyframe Interpolation | `web/src/animation/keyframes.ts` | `catchim::editor::AnimationChannel::splitAt` | Verified |
| Retimed Source Span & Trim Preservation | `web/src/retime/split.ts` | `catchim::editor::SplitElementsCommand::execute` | Verified |
| Atomic Snapshot Undo Restoration | `web/src/commands/timeline/element/split-elements.ts` | `catchim::editor::SplitElementsCommand::undo` | Verified |

---

## 64. Interactive Preview Hit Testing & Selection Resolver
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| Rotated Rect (OBB) Geometric Hit Testing | `web/src/preview/hit-test.ts` | `catchim::render::PreviewInteractionEngine::pointInRotatedRect` | Verified |
| Reverse Z-Index Layer Hit Resolution | `web/src/preview/hit-test.ts` | `catchim::render::PreviewInteractionEngine::getHitElements` | Verified |
| Preferred Hit Selection Prioritization | `web/src/preview/hit-test.ts` | `catchim::render::PreviewInteractionEngine::resolvePreferredHit` | Verified |
| Drag Gesture Session with Real-Time Snapping | `web/src/preview/controllers/preview-interaction-controller.ts` | `catchim::render::PreviewInteractionEngine::onPointerMove` | Verified |
| Text Element Double-Click Edit Detection | `web/src/preview/controllers/preview-interaction-controller.ts` | `catchim::render::PreviewInteractionEngine::onDoubleClick` | Verified |

---

## 65. Audio Retiming & Pitch Preservation Engine
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| Rate Clamping & Pitch Feasibility Constraints | `web/src/retime/rate.ts` | `catchim::audio::AudioRetimeEngine::clampRate`, `canMaintainPitch` | Verified |
| Linear Resampled Buffer Generation | `web/src/retime/audio-stretch.ts` | `catchim::audio::AudioRetimeEngine::renderResampledBuffer` | Verified |
| Pitch-Preserved WSOLA Time Stretching | `web/src/retime/audio-stretch.ts` | `catchim::audio::AudioRetimeEngine::renderPitchPreservedBuffer` | Verified |
| Multi-Channel Phase-Synchronized Synthesis | `web/src/retime/audio-stretch.ts` | `catchim::audio::AudioRetimeEngine::renderPitchPreservedBuffer` | Verified |
| Retimed Audio Buffer Dispatcher | `web/src/retime/audio-stretch.ts` | `catchim::audio::AudioRetimeEngine::renderRetimedBuffer` | Verified |

---

## 66. Multi-Track Element Move & Dynamic Track Allocation Command
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| Multi-Track Atomic Element Relocation | `web/src/commands/timeline/element/move-elements.ts` | `catchim::editor::MoveElementsCommand` | Verified |
| Dynamic On-Demand Track Creation | `web/src/commands/timeline/element/move-elements.ts` | `catchim::editor::PlannedTrackCreation`, `Timeline::insertTrack` | Verified |
| Track & Clip Compatibility Validation | `web/src/timeline/placement.ts` | `catchim::editor::PlacementEngine::canClipGoOnTrack` | Verified |
| Atomic Snapshot Undo Restoration | `web/src/commands/timeline/element/move-elements.ts` | `catchim::editor::MoveElementsCommand::undo` | Verified |

---

## 67. Timeline Element Insertion & Project Canvas Auto-Fit Command
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| Explicit & Auto Track Placement Engine | `web/src/commands/timeline/element/insert-element.ts` | `catchim::editor::InsertElementCommand`, `InsertElementPlacement` | Verified |
| First-Media Canvas Resolution & FPS Auto-Fitting | `web/src/commands/timeline/element/insert-element.ts` | `catchim::editor::InsertElementCommand::execute`, `Project::settings` | Verified |
| Dynamic Collision-Free Overlay Track Allocation | `web/src/commands/timeline/element/insert-element.ts` | `catchim::editor::InsertElementCommand`, `Timeline::addTrack` | Verified |
| Multi-Domain Atomic Snapshot Undo | `web/src/commands/timeline/element/insert-element.ts` | `catchim::editor::InsertElementCommand::undo` | Verified |

---

## 68. Project Settings & Scene Bookmark History Commands
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| Project Settings Update & Undo | `web/src/commands/project/update-project-settings.ts` | `catchim::editor::UpdateProjectSettingsCommand` | Verified |
| Timeline Bookmark Toggle at Timestamp | `web/src/commands/scene/toggle-bookmark.ts` | `catchim::editor::ToggleBookmarkCommand` | Verified |
| Timeline Bookmark Temporal Relocation | `web/src/commands/scene/move-bookmark.ts` | `catchim::editor::MoveBookmarkCommand` | Verified |
| Timeline Bookmark Metadata (Note & Color) Update | `web/src/commands/scene/update-bookmark.ts` | `catchim::editor::UpdateBookmarkCommand` | Verified |
| Timeline Bookmark Deletion by ID | `web/src/commands/scene/remove-bookmark.ts` | `catchim::editor::RemoveBookmarkCommand` | Verified |

---

## 69. Multi-Scene Management History Commands
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| Scene Creation & Active Scene Selection | `web/src/commands/scene/create-scene.ts` | `catchim::editor::CreateSceneCommand` | Verified |
| Safe Scene Deletion & Fallback Scene Resolution | `web/src/commands/scene/delete-scene.ts`, `timeline/scenes.ts` | `catchim::editor::DeleteSceneCommand`, `SceneUtils::getFallbackSceneAfterDelete` | Verified |
| Scene Rename & Undo Rollback | `web/src/commands/scene/rename-scene.ts` | `catchim::editor::RenameSceneCommand` | Verified |
| Scene Duplication with Complete Timeline Cloning | `web/src/timeline/scenes.ts`, `scenes-manager.ts` | `catchim::editor::DuplicateSceneCommand` | Verified |

---

## 70. Media Asset Library Management History Commands
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| Add Media Asset & Automatic Project FPS Ratchet | `web/src/commands/media/add-media-asset.ts` | `catchim::editor::AddMediaAssetCommand` | Verified |
| Remove Media Asset & Multi-Track Orphaned Clip Cleanup | `web/src/commands/media/remove-media-asset.ts` | `catchim::editor::RemoveMediaAssetCommand` | Verified |
| Media Asset Path / Metadata Update with Rollback | `web/src/media/types.ts` | `catchim::editor::UpdateMediaAssetPathCommand` | Verified |

---

## 71. Interactive Live Preview Snapshot Tracker
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| Generic Snapshot Lifecycle Tracker | `web/src/commands/preview-tracker.ts` | `catchim::editor::PreviewTracker<T>` | Verified |
| Idempotent Snapshot Capture (`begin`, `isActive`) | `web/src/commands/preview-tracker.ts` | `catchim::editor::PreviewTracker::begin`, `isActive` | Verified |
| Interactive Session Commitment & Cancellation | `web/src/commands/preview-tracker.ts` | `catchim::editor::PreviewTracker::end`, `cancel` | Verified |

---

## 72. Source Audio Separation Engine & Unified Toggle Command
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| Audio Extraction & Recovery Feasibility Validation | `web/src/timeline/audio-separation/index.ts` | `catchim::editor::AudioSeparationEngine::canExtractSourceAudio`, `canRecoverSourceAudio` | Verified |
| Dynamic UI Action Label ("Extract audio" vs "Recover audio") | `web/src/timeline/audio-separation/index.ts` | `catchim::editor::AudioSeparationEngine::getSourceAudioActionLabel` | Verified |
| Separated Audio Clip Synthesis with Retime & Volume Curve Cloning | `web/src/timeline/audio-separation/index.ts` | `catchim::editor::AudioSeparationEngine::buildSeparatedAudioClip` | Verified |
| Bidirectional Audio Separation Toggle & Placement Resolution | `web/src/commands/timeline/element/toggle-source-audio-separation.ts` | `catchim::editor::ToggleSourceAudioSeparationCommand` | Verified |

---

## 73. Timeline Drag Session & Drop Payload Resolution
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| In-Progress Timeline Drag Session Manager | `web/src/timeline/drag-source.ts` | `catchim::editor::TimelineDragSource` | Verified |
| Multi-Type Drag Payloads (Media, Text, Sticker, Graphic, Effect) | `web/src/timeline/drag.ts` | `catchim::editor::TimelineDragData`, `DragPayloadType` | Verified |
| Track Drop Feasibility Validation & Collision-Free Placement | `web/src/timeline/drag.ts` | `catchim::editor::TimelineDragSource::canDropOnTrack`, `resolveDropOnTrack` | Verified |
| Clip Target Drop Resolution (Visual Effects Attachment) | `web/src/timeline/drag.ts` | `catchim::editor::TimelineDragSource::canDropOnClip`, `resolveDropOnClip` | Verified |

---

## 74. Unified Multi-Kind Editor Selection State & Hierarchy Resolver
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| Multi-Tier Selection Hierarchy (MaskPoints > Keyframes > Elements) | `web/src/selection/editor-selection.ts` | `catchim::editor::EditorSelectionKind`, `EditorSelection` | Verified |
| Active Selection Tier Resolution & Mutually Exclusive Switching | `web/src/core/managers/selection-manager.ts` | `catchim::editor::EditorSelection::getActiveSelectionKind` | Verified |
| Cascading Selection Dismissal (`clearMostSpecificSelection`) | `web/src/core/managers/selection-manager.ts` | `catchim::editor::EditorSelection::clearMostSpecificSelection` | Verified |
| Selection Snapshot Capture, Patching & Restoration | `web/src/selection/editor-selection.ts` | `catchim::editor::EditorSelectionSnapshot`, `EditorSelectionPatch` | Verified |

---

## 75. Audio Waveform Summary & UI Bucket Sampling Engine
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| Multi-Channel Peak & RMS Bucket Computation | `web/src/media/waveform-summary.ts` | `catchim::media::WaveformSummaryEngine::computePeakBuckets`, `computeRmsBuckets` | Verified |
| Retime-Aware Timeline UI Bar Waveform Sampling | `web/src/media/waveform-summary.ts` | `catchim::media::WaveformSummaryEngine::buildWaveformSampleBuckets` | Verified |
| Waveform Summary Subsampling Cache & Key Generation | `web/src/media/waveform-summary.ts` | `catchim::media::WaveformSummaryEngine::sampleSourceWaveformSummary`, `buildWaveformSourceKey` | Verified |

---

## 76. Canvas Background Presets & Aspect Ratio Geometry Engine
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| Background Color, Preset Blur Strengths (Light, Med, Heavy) | `web/src/types/canvas.ts` | `catchim::render::CanvasBackgroundEngine::getBlurSigma` | Verified |
| Standard Aspect Ratio Resolution & Dimension Snapping | `web/src/types/canvas.ts` | `catchim::render::CanvasBackgroundEngine::resolveAspectRatio` | Verified |
| Contain vs Cover Bounding Box Transformation | `web/src/canvas/` | `catchim::render::CanvasBackgroundEngine::computeFittedRect` | Verified |

---

## 77. Project Manager Facade, Normalized Presets & Duration Aggregator
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| Standard Project Factory Presets & Normalized Resolution | `web/src/project/` | `catchim::editor::ProjectManager::createDefaultProject` | Verified |
| Multi-Scene Timeline Duration Aggregator | `web/src/core/managers/project-manager.ts` | `catchim::editor::ProjectManager::getProjectDurationFromScenes` | Verified |
| Project Collection Multi-Criteria Sorting (Updated, Created, Name, Duration) | `web/src/services/storage/` | `catchim::editor::ProjectManager::sortProjects` | Verified |
| Dirty Flag Tracking & Project Schema Version Query | `web/src/core/managers/project-manager.ts` | `catchim::editor::ProjectManager::markDirty`, `isDirty`, `clearDirty` | Verified |

---

## 78. Audio State, Gain Resolution & Waveform Gain Automation Engine
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| Decibel Range Clamping & Bidirectional Gain Conversion (dB $\leftrightarrow$ Linear) | `web/src/timeline/audio-state.ts` | `catchim::audio::AudioStateEngine::clampDb`, `dBToLinear`, `linearToDb` | Verified |
| Track & Element Mute Resolution & Keyframe Volume Interpolation | `web/src/timeline/audio-state.ts` | `catchim::audio::AudioStateEngine::resolveEffectiveAudioGain` | Verified |
| Retime-Aware Timeline UI Bar Waveform Gain Sampling | `web/src/timeline/audio-state.ts` | `catchim::audio::AudioStateEngine::buildWaveformGainSamples` | Verified |
| Audio Playback Automation Point Sequence Generation | `web/src/timeline/audio-state.ts` | `catchim::audio::AudioStateEngine::buildAudioGainAutomation` | Verified |

---

## 79. Preview Viewport Coordinate Transformer
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| Viewport Geometry & Logical Canvas Origin Calculation | `web/src/preview/preview-coords.ts` | `catchim::render::PreviewCoordinateTransformer::getCanvasOrigin` | Verified |
| Physical Screen/Client to Logical Canvas Coordinate Mapping | `web/src/preview/preview-coords.ts` | `catchim::render::PreviewCoordinateTransformer::screenToCanvas` | Verified |
| Logical Canvas to Overlay Space Mapping | `web/src/preview/preview-coords.ts` | `catchim::render::PreviewCoordinateTransformer::canvasToOverlay`, `positionToOverlay` | Verified |
| Physical Screen Tolerance to Logical Threshold Conversion | `web/src/preview/preview-coords.ts` | `catchim::render::PreviewCoordinateTransformer::screenPixelsToLogicalThreshold` | Verified |

---

## 80. Element Bounds Calculation, Handle Geometry & Selection-Prioritized Hit Testing
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| Multi-Type Element Visual Bounds Calculation with Animated Transforms | `web/src/preview/element-bounds.ts` | `catchim::render::ElementBoundsEngine::computeVisualBounds` | Verified |
| Corner, Edge & Offset Rotation Handle Position Resolvers | `web/src/preview/element-bounds.ts` | `catchim::render::ElementBoundsEngine::getCornerPosition`, `getEdgeHandlePosition`, `getRotationHandlePosition` | Verified |
| Z-Order Track-Reversed Visible Elements Extractor | `web/src/preview/element-bounds.ts` | `catchim::render::ElementBoundsEngine::getVisibleElementsWithBounds` | Verified |
| Rotated Rect Point-in-Polygon Hit Testing & Selection-Prioritized Resolution | `web/src/preview/hit-test.ts` | `catchim::render::ElementBoundsEngine::pointInRotatedRect`, `getHitElements`, `resolvePreferredHit` | Verified |

---

## 81. Keybinding Engine & Standard Action Shortcut Normalizer
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| Shortcut String Normalization (`ctrl+alt+shift+key`), macOS Aliases (`cmd`, `opt`) | `web/src/actions/keybinding.ts` | `catchim::editor::KeybindingEngine::normalizeShortcut` | Verified |
| Single-Char vs Modifier-Based Shortcut Categorization & Validation | `web/src/actions/keybinding.ts` | `catchim::editor::KeybindingEngine::isSingleCharacterShortcut`, `isModifierBasedShortcut` | Verified |
| Shortcut Assignment Conflict Detection | `web/src/actions/keybindings-store.ts` | `catchim::editor::KeybindingEngine::validateKeybinding` | Verified |
| Comprehensive Default Action Shortcuts & JSON Configuration Portability | `web/src/actions/definitions.ts` | `catchim::editor::KeybindingEngine::getDefaultShortcuts`, `exportConfig`, `importConfig` | Verified |

---

## 82. Project Autosave Engine with Debounce & Safe Flush
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| Debounced Autosave Timer (800ms) with Dirty Flag Tracking | `web/src/core/managers/save-manager.ts` | `catchim::editor::ProjectAutosaveEngine::markDirty`, `isDirty` | Verified |
| Save Session Pause & Resume Flow with Deferred Execution | `web/src/core/managers/save-manager.ts` | `catchim::editor::ProjectAutosaveEngine::pause`, `resume` | Verified |
| Safe Save Execution Guard (Active Project, Migration & Loading Audits) | `web/src/core/managers/save-manager.ts` | `catchim::editor::ProjectAutosaveEngine::canSaveNow` | Verified |
| Synchronous Forced Flush & Lifecycle Status Dispatchers | `web/src/core/managers/save-manager.ts` | `catchim::editor::ProjectAutosaveEngine::flush`, `beginSave`, `endSave` | Verified |

---

## 83. Multi-Scene Manager Facade & FPS-Aware Bookmark System
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| Project Scene Lifecycle Management (Create, Rename, Switch, Timestamping) | `web/src/core/managers/scenes-manager.ts` | `catchim::editor::ScenesManager::createScene`, `renameScene`, `switchToScene` | Verified |
| Invariant Guarantee: Mandatory Single Main Scene Preservation | `web/src/core/managers/scenes-manager.ts` | `catchim::editor::ScenesManager::ensureMainScene` | Verified |
| Safe Scene Deletion Constraint Enforcement & Auto-Fallback Activation | `web/src/core/managers/scenes-manager.ts`, `timeline/scenes.ts` | `catchim::editor::ScenesManager::canDeleteScene`, `deleteScene` | Verified |
| Active Scene Frame-Aligned Bookmark Query, Toggle & Removal | `web/src/core/managers/scenes-manager.ts` | `catchim::editor::ScenesManager::isBookmarkedAtTime`, `getBookmarkAtTime`, `toggleBookmark` | Verified |

---

## 84. Safe Mathematical Expression Parsing, Numeric Stepping & Decimal Formatting
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| Recursive Descent Math Parser (+, -, *, /, parentheses, unary minus, decimals) | `web/src/utils/math.ts` | `catchim::core::MathExpressionEvaluator::evaluateMathExpression` | Verified |
| Precision Step Snapping (`snapToStep`) | `web/src/utils/math.ts` | `catchim::core::MathExpressionEvaluator::snapToStep` | Verified |
| Clean UI Decimal Formatting without Trailing Zeros (`formatNumberForDisplay`) | `web/src/utils/math.ts` | `catchim::core::MathExpressionEvaluator::formatNumberForDisplay` | Verified |
| Floating-Point Approximate Equality Comparison (`isNearlyEqual`) | `web/src/utils/math.ts` | `catchim::core::MathExpressionEvaluator::isNearlyEqual` | Verified |

---

## 85. Timeline Track Capability Resolution & Predicate-Based Element Queries
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| Track Audio & Visibility Capability Resolution (`canTrackHaveAudio`, `canTrackBeHidden`) | `web/src/timeline/tracks.ts` | `catchim::editor::TrackCapabilityEngine::canTrackHaveAudio`, `canTrackBeHidden` | Verified |
| Timeline Cross-Track Element Filter & Predicate Queries (`findClips`, `findClip`) | `web/src/timeline/tracks.ts` | `catchim::editor::TrackCapabilityEngine::findClips`, `findClip` | Verified |
| Conditional Element Property Update Propagation (`updateClipsWhere`) | `web/src/timeline/tracks.ts` | `catchim::editor::TrackCapabilityEngine::updateClipsWhere` | Verified |

---

## 86. Storage Quota Evaluation & Byte Units Formatting
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| Adaptive Byte Units Formatting (`formatStorageBytes`: B, KB, MB, GB, TB) | `web/src/services/storage/quota.ts` | `catchim::storage::StorageQuotaEngine::formatStorageBytes` | Verified |
| Storage Capacity Check with Headroom & 50MB Safety Reserve | `web/src/services/storage/quota.ts` | `catchim::storage::StorageQuotaEngine::evaluateStorageCapacity` | Verified |

---

## 87. Rule-Based Timeline Element Update Pipeline
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| Rule-Based Element Property Patch Pipeline | `web/src/timeline/update-pipeline.ts` | `catchim::editor::TimelineElementUpdatePipeline::applyElementUpdate` | Verified |
| Retime-Driven Duration Derivation (`clampRetimeRate`, `sourceDuration`, `trim`) | `web/src/timeline/update-pipeline.ts` | `catchim::editor::TimelineElementUpdatePipeline::applyElementUpdate` | Verified |
| Animation Keyframe Truncation & Clamping to Duration | `web/src/timeline/update-pipeline.ts`, `animation/keyframes.ts` | `catchim::editor::TimelineElementUpdatePipeline::clampAnimationsToDuration` | Verified |
| Non-negative Start Time & Main Track Zero-Start Invariant Enforcement | `web/src/timeline/update-pipeline.ts` | `catchim::editor::TimelineElementUpdatePipeline::applyElementUpdate` | Verified |

---

## 88. Export Canvas Geometry Resolver & Video Codec Dimension Snapping
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| Even Dimension Snapping for Video Codecs (`roundToEven`) | `web/src/export/index.ts` | `catchim::exporting::ExportCanvasGeometryResolver::roundToEven` | Verified |
| Resolution Preset Geometry Calculation (Source, 480p - 4320p) | `web/src/export/index.ts` | `catchim::exporting::ExportCanvasGeometryResolver::resolveExportCanvasSize` | Verified |
| Preset Target Height Mapping & String Conversion | `web/src/export/index.ts` | `catchim::exporting::ExportCanvasGeometryResolver::getPresetTargetHeight`, `resolutionPresetToString` | Verified |
| Export Format MIME Type and File Extension Resolution | `web/src/export/mime-types.ts`, `export/index.ts` | `catchim::exporting::ExportCanvasGeometryResolver::getExportMimeType`, `getExportFileExtension` | Verified |

---

## 89. Media Asset Inspector & Constrained Thumbnail Sizer
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| Media Audio Track Support Resolution (`mediaSupportsAudio`) | `web/src/media/media-utils.ts` | `catchim::media::MediaAssetInspector::mediaSupportsAudio` | Verified |
| Media Type Inference from MIME Types & Extensions | `web/src/media/media-utils.ts` | `catchim::media::MediaAssetInspector::getMediaTypeFromMimeType`, `getMediaTypeFromExtension` | Verified |
| Constrained Thumbnail Bounds Sizing (Max 1280x720) with Aspect Ratio Retention | `web/src/media/thumbnail.ts` | `catchim::media::MediaAssetInspector::calculateThumbnailSize` | Verified |
| Unsupported Codec Warnings & Safe Storage Limit Diagnostics | `web/src/media/processing.ts` | `catchim::media::MediaAssetInspector::getUnsupportedVideoDescription`, `getStorageLimitDescription` | Verified |

---

## 90. Timeline Coordinate & Pixel Engine
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| Bidirectional Timeline Time to Pixels Mapping | `web/src/timeline/pixel-utils.ts`, `scale.ts` | `catchim::editor::TimelineCoordinateEngine::timelineTimeToPixels`, `pixelsToTimelineTime` | Verified |
| Zoom-Scaled Density & Pixel Bounds Clamping | `web/src/timeline/scale.ts`, `pixel-utils.ts` | `catchim::editor::TimelineCoordinateEngine::getTimelinePixelsPerSecond` | Verified |
| HiDPI Device Pixel Grid Snapping | `web/src/timeline/pixel-utils.ts` | `catchim::editor::TimelineCoordinateEngine::snapPixelToDeviceGrid`, `timelineTimeToSnappedPixels` | Verified |
| Centered Indicator Line Geometry & ClientX-to-Time Conversion | `web/src/timeline/pixel-utils.ts`, `drag-utils.ts` | `catchim::editor::TimelineCoordinateEngine::getCenteredLineLeft`, `getMouseTimeFromClientX` | Verified |

---

## 91. Project Frame Rate & Media Auto-Raise Engine
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| Standard FPS UI Presets & Defaults | `web/src/fps/presets.ts`, `defaults.ts` | `catchim::core::ProjectFrameRateEngine::getFpsPresets`, `kDefaultFps` | Verified |
| Rational & Floating-Point NTSC Frame Rate Conversion (`floatToFrameRate`) | `web/src/fps/utils.ts` | `catchim::core::ProjectFrameRateEngine::floatToFrameRate`, `frameRateToFloat` | Verified |
| Imported Media Video FPS Scanner (`getHighestImportedVideoFps`) | `web/src/fps/utils.ts` | `catchim::core::ProjectFrameRateEngine::getHighestImportedVideoFps` | Verified |
| Automatic Project Frame Rate Elevation for High-FPS Media (`getRaisedProjectFpsForImportedMedia`) | `web/src/fps/utils.ts` | `catchim::core::ProjectFrameRateEngine::getRaisedProjectFpsForImportedMedia` | Verified |

---

## 92. Guide Overlay & Platform Safe Zone Engine
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| Multi-Platform Social Media Safe Zones (TikTok, Reels, Shorts, Spotlight) | `web/src/guides/definitions/platforms.tsx` | `catchim::render::GuideOverlayEngine::calculateSafeZone` | Verified |
| Dynamic Interior Grid Lines Generator (Rule of Thirds, Custom Grid) | `web/src/guides/definitions/grid.tsx`, `grid.ts` | `catchim::render::GuideOverlayEngine::calculateGridLines` | Verified |
| Guide Overlay Registry & Safe Zone Resolution by Identifier | `web/src/guides/registry.tsx`, `index.ts` | `catchim::render::GuideOverlayEngine::getAllGuides`, `getGuideById`, `calculateSafeZoneById` | Verified |

---

## 93. Element Param Registry & Blend Mode Engine
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| Standard 17 Blend Mode Specifications & String Mapping | `web/src/params/`, `rendering/` | `catchim::editor::ElementParamRegistry::blendModeToString`, `parseBlendMode` | Verified |
| Parameter Definitions for Visual, Audio & Text Elements | `web/src/params/element-params.ts` | `catchim::editor::ElementParamRegistry::getVisualParamDefinitions`, `getAudioParamDefinitions`, `getTextParamDefinitions` | Verified |
| Default Param Values Generator & Type Registry | `web/src/params/element-params.ts` | `catchim::editor::ElementParamRegistry::buildDefaultParamValues` | Verified |

---

## 94. Text Rendering Primitives & Typography Scaler
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| CSS Font Specification Formatter & Safe Font Name Quoting | `web/src/rendering/text/`, `text/font.ts` | `catchim::render::TextRenderingPrimitives::buildTextFontString`, `quoteFontFamily` | Verified |
| Canvas-Proportional Typography Metric Scaler (1080p Base) | `web/src/rendering/text/`, `text/scale.ts` | `catchim::render::TextRenderingPrimitives::resolveTextLayoutMetrics` | Verified |
| Proportional Background Box Corner Radius Computation | `web/src/rendering/text/`, `text/background.ts` | `catchim::render::TextRenderingPrimitives::calculateCornerRadiusPx` | Verified |

---

## 95. Timeline Element Builder & Factory Pipeline
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| Factory for Media Timeline Elements (Video, Audio, Image) | `web/src/timeline/element-builder.ts` | `catchim::editor::TimelineElementBuilder::buildElementFromMedia` | Verified |
| Factory for Rich Text Elements with Layout Attributes | `web/src/timeline/element-builder.ts` | `catchim::editor::TimelineElementBuilder::buildTextElement` | Verified |
| Parameter Invariant Guarantee & Override Merge Pipeline | `web/src/timeline/element-builder.ts` | `catchim::editor::TimelineElementBuilder::mergeParamValues` | Verified |

---

## 96. ASS Subtitle Parser & Subtitle Element Builder
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| Advanced SubStation Alpha (.ass/.ssa) Parser | `web/src/subtitles/ass.ts` | `catchim::editor::AssSubtitleParser::parse` | Verified |
| Script Info, V4/V4+ Styles, BGR Colors & Alignment 1-9 Mapping | `web/src/subtitles/ass.ts` | `catchim::editor::AssSubtitleParser::parseColor`, `parseTimestamp` | Verified |
| Subtitle Text Element Factory & Geometry Sizer (Margins & Wrapping) | `web/src/subtitles/build-subtitle-text-element.ts` | `catchim::editor::SubtitleElementBuilder::buildSubtitleTextElement`, `wrapSubtitleText` | Verified |

---

## 97. CSS Gradient & Background Engine
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| Multi-layer CSS Background Splitter & Gradient Parser | `web/src/gradients/parser.ts`, `canvas.ts` | `catchim::render::CssGradientEngine::splitCssLayers`, `parseBackgroundLayers`, `parseGradient` | Verified |
| Linear & Radial Geometry Resolvers (Ray endpoints & Extents) | `web/src/gradients/canvas.ts` | `catchim::render::CssGradientEngine::resolveLinearPoints`, `resolveRadialDimensions` | Verified |
| Color Stop Normalizer, Transparent Fixer & Pattern Craft Presets | `web/src/gradients/canvas.ts`, `data/colors/pattern-craft.ts` | `catchim::render::CssGradientEngine::normalizeColorStops`, `fixTransparentStops`, `getPatternCraftGradients` | Verified |

---

## 98. Canvas Transform Pipeline & Matrix Engine
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| 2D Transform Parameter Reader & Animator at Local Time | `web/src/rendering/index.ts`, `animation-values.ts` | `catchim::render::CanvasTransformPipeline::buildTransformFromParams`, `resolveTransformAtTime` | Verified |
| 3x3 2D Affine Transformation Matrix (TRS Composition) | `web/src/rendering/` | `catchim::render::Matrix3x3::compose`, `Matrix3x3::transformPoint` | Verified |
| Invertible Coordinate Mapping (Screen/Canvas to Element Local) | `web/src/rendering/` | `catchim::render::Matrix3x3::inverse`, `Matrix3x3::inverseTransformPoint` | Verified |

---

## 99. Render Performance Profiler & Diagnostics Registry
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| Rolling Render Performance Profiler (Spans & Counters) | `web/src/diagnostics/render-perf.ts` | `catchim::render::RenderPerformanceProfiler::recordSpan`, `measureSpan`, `incrementCounter` | Verified |
| Cadence Flush & Performance Percentiles (Mean, P50, P95, Max) | `web/src/diagnostics/render-perf.ts` | `catchim::render::RenderPerformanceProfiler::onFrameComplete`, `getSpanSummaries`, `getCounterSummaries` | Verified |
| Scoped Diagnostic Rules & Event Subscriptions | `web/src/diagnostics/types.ts`, `core/managers/diagnostics-manager.ts` | `catchim::editor::DiagnosticsRegistry::registerRule`, `getActiveDiagnostics`, `subscribe` | Verified |

---

## 100. Canvas Preset Engine & Aspect Ratio Sizer
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| Standard Canvas Dimensions & Presets (16:9, 9:16, 1:1, 4:3, 4K) | `web/src/canvas/sizes.ts`, `project/types.ts` | `catchim::render::CanvasPresetEngine::getStandardPresets`, `getDefaultPreset` | Verified |
| Aspect Ratio Fraction & String Resolution (GCD) | `web/src/canvas/sizes.ts` | `catchim::render::CanvasPresetEngine::calculateAspectRatioFraction`, `getAspectRatioString` | Verified |
| Aspect-Preserving Media Canvas Fitter with Even Dimension Snapping | `web/src/canvas/sizes.ts` | `catchim::render::CanvasPresetEngine::fitCanvasToMedia`, `roundToEven` | Verified |

---

## 101. Euclidean Frame Snapper & Seek Alignment
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| Euclidean Integer Division & Remainder (`divEuclid`, `remEuclid`) | `web/src/native/opencut-wasm-compat.ts` | `catchim::core::EuclideanFrameSnapper::divEuclid`, `remEuclid` | Verified |
| Fractional Frame Duration in Ticks Resolution | `web/src/native/opencut-wasm-compat.ts` | `catchim::core::EuclideanFrameSnapper::getTicksPerFrame` | Verified |
| Round, Floor, Last Frame & Snapped Seek Time Alignment | `web/src/native/opencut-wasm-compat.ts` | `catchim::core::EuclideanFrameSnapper::roundToFrame`, `floorToFrame`, `lastFrameTime`, `snappedSeekTime` | Verified |

---

## 102. Builtin Mask Geometry Engine
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| Shape Geometries (Split, Rect, Ellipse, Cinematic Bars, Diamond, Heart, Star) | `web/src/masks/builtin/definitions/` | `catchim::render::BuiltinMaskGeometry::build*` | Verified |
| Split Line Geometry, Normal Snapping & Polygon Clipping | `web/src/masks/builtin/definitions/split.ts` | `catchim::render::BuiltinMaskGeometry::splitLineGeometry`, `buildSplitPolygonVertices` | Verified |
| Interactive Parameter Updates (Position, Rotation, Feather, Edge, Corner, Scale) | `web/src/masks/builtin/box-like.ts`, `param-update.ts` | `catchim::render::BuiltinMaskGeometry::computeBoxMaskParamUpdate`, `computeSplitMaskParamUpdate` | Verified |

---

## 103. Mask Snap Engine
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| Local & Global Coordinate Snapping | `web/src/masks/geometry.ts` | `catchim::render::MaskSnapEngine::toGlobalMaskSnapLines`, `getMaskLocalCenter`, `setMaskLocalCenter` | Verified |
| Box Mask Position, Rotation & Scaling Snap | `web/src/masks/snap.ts` | `catchim::render::MaskSnapEngine::snapBoxMaskInteraction` | Verified |
| Split Mask Position & Rotation Snap | `web/src/masks/snap.ts` | `catchim::render::MaskSnapEngine::snapSplitMaskInteraction` | Verified |

---

## 104. Mask Interaction & Registry Engine
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| Line & Box Mask Interactive Handle Positions & Cursors | `web/src/masks/handle-positions.ts` | `catchim::render::MaskInteractionEngine::getLineMaskHandlePositions`, `getBoxMaskHandlePositions` | Verified |
| Mask Rect & Shape Overlays | `web/src/masks/handle-positions.ts` | `catchim::render::MaskInteractionEngine::getBoxMaskRectOverlay`, `getBoxMaskShapeOverlay` | Verified |
| Mask Registry & Type Definitions | `web/src/masks/registry.ts`, `types.ts` | `catchim::render::MaskRegistry::getAllMaskTypes`, `buildDefault`, `isActive` | Verified |

---

## 105. Scene Graph Node Hierarchy
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| Scene Graph Nodes Base & Polymorphic Hierarchy | `web/src/services/renderer/nodes/base-node.ts` | `catchim::render::SceneNode`, `RootNode`, `ColorNode`, `BlurBackgroundNode`, `EffectLayerNode` | Verified |
| Visual Media Clip Nodes (Video, Image, Text, Sticker, Graphic) | `web/src/services/renderer/nodes/` | `catchim::render::VisualNode`, `VideoNode`, `ImageNode`, `TextNode`, `StickerNode`, `GraphicNode` | Verified |
| Hierarchy Management (Parent-Child, Add, Remove, Clear) | `web/src/services/renderer/nodes/base-node.ts` | `catchim::render::SceneNode::addChild`, `removeChild`, `clearChildren` | Verified |

---

## 106. Scene Builder & Tree Resolver
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| Scene Construction from Tracks & Background Strategy | `web/src/services/renderer/scene-builder.ts` | `catchim::render::SceneBuilder::buildScene`, `buildTrackNodes`, `buildBlurBackgroundNodes` | Verified |
| Scaled Transform & Viewport Projection from Clip | `web/src/services/renderer/scene-builder.ts` | `catchim::render::SceneBuilder::buildScaledTransformFromClip` | Verified |
| Dynamic Node State Resolution at Timeline Time $t$ | `web/src/services/renderer/resolve.ts` | `catchim::render::SceneTreeResolver::resolveVisualNode`, `resolveTextNode`, `resolveBlurBackgroundNode`, `resolveEffectLayerNode` | Verified |
| Effect Pass Groups Resolution with Parameter Animation | `web/src/services/renderer/resolve.ts` | `catchim::render::SceneTreeResolver::resolveEffectPassGroups` | Verified |

---

## 107. Frame Descriptor Builder
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| Frame Descriptor & Compositor Item Generation | `web/src/services/renderer/compositor/frame-descriptor.ts` | `catchim::render::FrameDescriptorBuilder::buildFrameDescriptor` | Verified |
| Visual Quad Transform & Full Canvas Transform Projection | `web/src/services/renderer/compositor/frame-descriptor.ts` | `catchim::render::FrameDescriptorBuilder::fullCanvasTransform`, `computeVisualTransform` | Verified |
| Transform Hashing & Texture Upload Descriptors (Content Cache) | `web/src/services/renderer/compositor/frame-descriptor.ts` | `catchim::render::FrameDescriptorBuilder::transformHash`, `TextureUploadDescriptor` | Verified |

---

## 108. Render Surface Buffer & 2D Compositing
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| 32-bit RGBA Offscreen Surface Buffer Management | `web/src/services/renderer/canvas-utils.ts` | `catchim::render::RenderSurface` | Verified |
| Surface Pixel Operations (Clear, Fill, Set/Get, Resize, Copy) | `web/src/services/renderer/canvas-utils.ts` | `catchim::render::RenderSurface::clear`, `fill`, `copyFrom`, `resize` | Verified |
| Affine Quad Sampling & Blending (Normal, Multiply, Screen, Overlay) | `web/src/services/renderer/canvas-utils.ts` | `catchim::render::RenderSurface::blendOver` | Verified |

---

## 109. Texture Cache Manager & Sync Pipeline
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| Texture Cache Slots (External & Rendered) | `web/src/services/renderer/compositor/wasm-compositor.ts` | `catchim::render::TextureCacheManager`, `TextureCacheEntry` | Verified |
| Frame Texture Synchronization & Unreferenced Eviction | `web/src/services/renderer/compositor/wasm-compositor.ts` | `catchim::render::TextureCacheManager::syncTextures` | Verified |
| ContentHash Validation & Render Performance Telemetry | `web/src/services/renderer/compositor/wasm-compositor.ts` | `catchim::render::TextureCacheManager::rasterizeRenderedTexture`, `cacheHits`, `cacheMisses` | Verified |

---

## 110. Canvas Renderer & Effect Preview Service
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| End-to-End Canvas Frame Rendering Coordinator | `web/src/services/renderer/canvas-renderer.ts` | `catchim::render::CanvasRenderer::render`, `setSize` | Verified |
| Multi-Span Frame Performance Profiling Integration | `web/src/services/renderer/canvas-renderer.ts` | `catchim::render::CanvasRenderer` + `RenderPerformanceProfiler` | Verified |
| Standard 160x160 Effect Thumbnail Preview Service | `web/src/services/renderer/effect-preview.ts` | `catchim::render::EffectPreviewService::renderPreview`, `getTestSource` | Verified |

---

## 111. Timeline Snapping Engine
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| Multi-Source Timeline Snapping Core | `web/src/timeline/snapping/types.ts`, `build.ts` | `catchim::editor::TimelineSnappingEngine::buildTimelineSnapPoints`, `buildSortedTimelineSnapPoints` | Verified |
| Binary Search & Linear Snap Resolution | `web/src/timeline/snapping/resolve.ts` | `catchim::editor::TimelineSnappingEngine::resolveSortedTimelineSnap`, `resolveTimelineSnapLinear` | Verified |
| Zoom-Dependent Snap Threshold Calculation | `web/src/timeline/snapping/threshold.ts` | `catchim::editor::TimelineSnappingEngine::getTimelineSnapThresholdInTicks` | Verified |

---

## 112. Timeline Snap Point Sources
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| Element Edge Snap Point Extraction with Exclusions | `web/src/timeline/element-snap-source.ts` | `catchim::editor::TimelineSnapPointSources::getElementEdgeSnapPoints` | Verified |
| Playhead Snap Point Extraction | `web/src/timeline/playhead-snap-source.ts` | `catchim::editor::TimelineSnapPointSources::getPlayheadSnapPoints` | Verified |
| Animation Keyframe Snap Point Extraction | `web/src/timeline/animation-snap-points.ts` | `catchim::editor::TimelineSnapPointSources::getAnimationKeyframeSnapPoints` | Verified |

---

## 113. Animation Target Resolver
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| Clip Transform & Element Parameter Resolution | `web/src/timeline/animation-targets.ts` | `catchim::editor::AnimationTargetResolver::resolveAnimationTarget` | Verified |
| Graphic Geometry Parameter Target Resolution | `web/src/timeline/animation-targets.ts` | `catchim::editor::AnimationTargetResolver::resolveAnimationTarget` (`graphics.*`) | Verified |
| Effect Parameter Path Target Resolution | `web/src/timeline/animation-targets.ts` | `catchim::editor::AnimationTargetResolver::resolveAnimationTarget` (`effects.<id>.params.<key>`) | Verified |

---

## 114. Timeline Element Defaults & Duration Resolver
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| Default Element Duration & Duration Normalizer | `web/src/timeline/creation.ts` | `catchim::editor::TimelineDefaults::defaultNewElementDuration`, `toElementDurationTicks` | Verified |
| Default Transform, Opacity, Blend Mode & Volume | `web/src/timeline/defaults.ts` | `catchim::editor::TimelineDefaults::defaultTransform`, `defaultOpacity`, `defaultBlendMode`, `defaultVolume` | Verified |
| Default Text Element Parameters & View State | `web/src/timeline/defaults.ts` | `catchim::editor::TimelineDefaults::buildDefaultTextElementParams`, `defaultTimelineViewState` | Verified |

---

## 115. Animation Transform Resolver
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| Dynamic Clip Transform Evaluation at Local Time | `web/src/rendering/animation-values.ts` | `catchim::editor::AnimationTransformResolver::resolveTransformAtTime` | Verified |
| Dynamic Animated Opacity Evaluation & Clamping | `web/src/rendering/animation-values.ts` | `catchim::editor::AnimationTransformResolver::resolveOpacityAtTime` | Verified |
| Dynamic Animated Volume Evaluation & Clamping | `web/src/rendering/animation-values.ts` | `catchim::editor::AnimationTransformResolver::resolveVolumeAtTime` | Verified |

---

## 116. Preview Overlay Manager
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| Overlay Definitions & Hierarchy Registration | `web/src/preview/overlays.ts` | `catchim::render::PreviewOverlayManager::registerDefinition` | Verified |
| Overlay Visibility State & Toggle Controls | `web/src/preview/overlays.ts` | `catchim::render::PreviewOverlayManager::isOverlayVisible`, `setOverlayVisible`, `getControls` | Verified |
| Multi-Source Overlay Deduplication & Merging | `web/src/preview/overlays.ts` | `catchim::render::PreviewOverlayManager::mergeSources` | Verified |

---

## 117. Interaction Cancellation Registry
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| Active Interaction Cancellation Callback Registration | `web/src/editor/cancel-interaction.ts` | `catchim::editor::InteractionCancellationRegistry::registerCanceller` | Verified |
| Interaction Canceller Unregistration & Scoped RAII | `web/src/editor/cancel-interaction.ts` | `catchim::editor::InteractionCancellationRegistry::unregisterCanceller`, `ScopedRegistration` | Verified |
| Global/Batch Interaction Cancellation Execution | `web/src/editor/cancel-interaction.ts` | `catchim::editor::InteractionCancellationRegistry::cancelInteraction`, `cancelAll` | Verified |

---

## 118. Panel Layout Manager & Store
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| Default Panel Dimensions & Layout Configuration | `web/src/panels/layout.ts` | `catchim::editor::PanelLayoutManager::defaultSizes`, `PANEL_CONFIG` | Verified |
| Individual & Bulk Panel Sizing with Notifications | `web/src/editor/panel-store.ts` | `catchim::editor::PanelLayoutManager::setPanel`, `setPanels`, `resetPanels` | Verified |
| JSON Serialization & v1 Legacy Schema Migration | `web/src/editor/panel-store.ts` | `catchim::editor::PanelLayoutManager::toJson`, `fromJson`, `migrateJson` | Verified |

---

## 119. Media Thumbnail Engine & Media Utilities
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| Aspect-Ratio Preserving Thumbnail Dimension Calculation | `web/src/media/thumbnail.ts` | `catchim::media::MediaThumbnailEngine::calculateThumbnailSize` | Verified |
| Audio Support Detection & File/MIME Type Detection | `web/src/media/media-utils.ts` | `catchim::media::MediaThumbnailEngine::supportsAudio`, `detectMediaTypeFromMime`, `detectMediaTypeFromPath` | Verified |
| Bilinear Downsampling Thumbnail Surface Generation | `web/src/media/thumbnail.ts` | `catchim::media::MediaThumbnailEngine::createThumbnailSurface` | Verified |

---

## 120. Rendering Params & Blend Mode Resolver
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| Render Transform Extraction from Params | `web/src/rendering/index.ts` | `catchim::render::RenderingParamsResolver::buildTransformFromParams` | Verified |
| Opacity & Blend Mode Extraction with Fallback | `web/src/rendering/index.ts` | `catchim::render::RenderingParamsResolver::readOpacityFromParams`, `readBlendModeFromParams` | Verified |
| 17 Standard Blend Modes Validation & Mapping | `web/src/rendering/index.ts` | `catchim::render::RenderingParamsResolver::isBlendMode`, `blendModeToString`, `blendModeFromString` | Verified |

---

## 121. Track Defaults & Audio Volume Constants
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| Default Track Names by TrackType | `web/src/timeline/tracks.ts` | `catchim::editor::TrackDefaults::getDefaultTrackName`, `defaultVideoTrackName` | Verified |
| Audio Decibel Limits & Clamping (-60dB to +20dB) | `web/src/timeline/audio-constants.ts` | `catchim::editor::TrackDefaults::VOLUME_DB_MIN`, `VOLUME_DB_MAX`, `clampDb`, `clampLinear` | Verified |
| Decibel to Linear Volume Conversion Utilities | `web/src/timeline/audio-constants.ts` | `catchim::editor::TrackDefaults::linearToDb`, `dbToLinear` | Verified |

---

## 122. Math Formatting & Aspect Ratio Utilities
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| Step Snapping & Fraction Digits Calculation | `web/src/utils/math.ts` | `catchim::core::MathFormattingUtils::getFractionDigitsForStep`, `snapToStep` | Verified |
| Near Equality & Display Number Formatting | `web/src/utils/math.ts` | `catchim::core::MathFormattingUtils::isNearlyEqual`, `formatNumberForDisplay`, `clampRound` | Verified |
| Aspect Ratio String Calculation with GCD | `web/src/utils/geometry.ts` | `catchim::core::MathFormattingUtils::dimensionToAspectRatio` | Verified |

---

## 123. Export Options & MIME Defaults
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| Default Export Options Preset (mp4, high, source) | `web/src/export/defaults.ts` | `catchim::exporting::ExportDefaults::defaultExportOptions` | Verified |
| Export MIME Types Mapping (mp4, webm) | `web/src/export/mime-types.ts` | `catchim::exporting::ExportDefaults::mimeTypeMp4`, `mimeTypeWebm`, `getMimeTypeForFormat` | Verified |
| Quality & Resolution Presets Catalog | `web/src/export/defaults.ts` | `catchim::exporting::ExportDefaults::qualityPresets`, `resolutionPresets`, `supportedFormats` | Verified |

---

## 124. Timeline Bookmark Engine & Snap Source
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| Bookmark Array Operations (Find, Toggle, Move, Update) | `web/src/timeline/bookmarks/utils.ts` | `catchim::editor::BookmarkEngine::findBookmarkIndex`, `toggleBookmarkInArray`, `moveBookmarkInArray` | Verified |
| Active Time Range Query for Duration Bookmarks | `web/src/timeline/bookmarks/utils.ts` | `catchim::editor::BookmarkEngine::getBookmarksActiveAtTime` | Verified |
| Bookmark Snap Points Extraction with Exclusions | `web/src/timeline/bookmarks/snap-source.ts` | `catchim::editor::BookmarkEngine::getBookmarkSnapPoints` | Verified |

---

## 125. Timeline UI Store & Persistence
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| Snapping & Ripple Editing UI State Management | `web/src/timeline/timeline-store.ts` | `catchim::editor::TimelineUiStore::isSnappingEnabled`, `isRippleEditingEnabled`, `toggleSnapping` | Verified |
| Expanded Element IDs Tracking | `web/src/timeline/timeline-store.ts` | `catchim::editor::TimelineUiStore::isElementExpanded`, `toggleElementExpanded` | Verified |
| Timeline UI Store JSON Serialization & Hydration | `web/src/timeline/timeline-store.ts` | `catchim::editor::TimelineUiStore::toJson`, `fromJson` | Verified |

---

## 126. Track Compatibility Engine
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| Element to Track Type Mapping (ELEMENT_TRACK_MAP) | `web/src/timeline/placement/compatibility.ts` | `catchim::editor::TrackCompatibilityEngine::getTrackTypeForElementType` | Verified |
| Track Placement Acceptance Verification | `web/src/timeline/placement/compatibility.ts` | `catchim::editor::TrackCompatibilityEngine::canElementGoOnTrack` | Verified |
| User-Facing Compatibility Error Message Generation | `web/src/timeline/placement/compatibility.ts` | `catchim::editor::TrackCompatibilityEngine::validateElementTrackCompatibility` | Verified |

---

## 127. Track Insert Resolver
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| Default & Highest Track Insert Index Calculation | `web/src/timeline/placement/insert-index.ts` | `catchim::editor::TrackInsertResolver::getDefaultInsertIndexForTrack`, `getHighestInsertIndexForTrack` | Verified |
| Preferred New Track Placement & Direction Resolution | `web/src/timeline/placement/insert-index.ts` | `catchim::editor::TrackInsertResolver::resolvePreferredNewTrackPlacement` | Verified |

---

## 128. Group Move Snap & Dynamic Track Allocation Engine
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| Track Placement Mapping & Display Index Order | `web/src/timeline/group-move/track-placement.ts` | `catchim::editor::GroupMoveSnapEngine::getDisplayTrackPlacements`, `getTrackPlacementById`, `getTrackPlacementByDisplayIndex` | Verified |
| Anchored Group Construction & Member Offsets | `web/src/timeline/group-move/build-group.ts` | `catchim::editor::GroupMoveSnapEngine::buildMoveGroup` | Verified |
| Multi-Track Edge & Keyframe Snapping | `web/src/timeline/group-move/snap.ts` | `catchim::editor::GroupMoveSnapEngine::buildMoveGroupSnapPoints`, `snapGroupEdges` | Verified |
| Existing & New Track Move Resolution with Collision Checking | `web/src/timeline/group-move/resolve-move.ts` | `catchim::editor::GroupMoveSnapEngine::resolveGroupMove` | Verified |

---

## 129. Interval-Based Ripple Diff & Multi-Track Adjustment Engine
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| Mathematical Interval Normalization & Set Subtraction | `web/src/ripple/diff.ts` | `catchim::editor::RippleDiffEngine::normalizeIntervals`, `subtractSingleInterval`, `subtractIntervalSets` | Verified |
| Vacated & Joined Track Intervals Extraction | `web/src/ripple/diff.ts` | `catchim::editor::RippleDiffEngine::computeTrackRippleAdjustments` | Verified |
| Cross-Track Ripple Adjustment Computation | `web/src/ripple/diff.ts` | `catchim::editor::RippleDiffEngine::computeRippleAdjustments` | Verified |
| Descending Multi-Track Ripple Shift Execution | `web/src/ripple/apply.ts`, `shift.ts` | `catchim::editor::RippleDiffEngine::applyRippleAdjustments` | Verified |

---

## 130. Functional Track & Element Hierarchy Updater
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| Track Lookup Across Hierarchy Sections | `web/src/timeline/track-element-update.ts` | `catchim::editor::TrackElementUpdateEngine::findTrackInTimeline` | Verified |
| In-Place Functional Track Updating | `web/src/timeline/track-element-update.ts` | `catchim::editor::TrackElementUpdateEngine::updateTrackInTimeline` | Verified |
| Predicate-Guarded Element Mutation in Track | `web/src/timeline/track-element-update.ts` | `catchim::editor::TrackElementUpdateEngine::updateElementInTrack` | Verified |
| Scene-Wide Predicate-Guarded Element Mutation | `web/src/timeline/track-element-update.ts` | `catchim::editor::TrackElementUpdateEngine::updateElementInTimeline` | Verified |

---

## 131. Keybindings Schema Migration Engine & State Validator
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| Schema v2 to v7 Sequential Migration Pipeline | `web/src/actions/keybindings/migrations/index.ts` | `catchim::editor::KeybindingMigrationEngine::runMigrations` | Verified |
| Action Renaming (split, paste-copied, escape, split-element) | `web/src/actions/keybindings/migrations/v2-to-v3.ts` ... `v6-to-v7.ts` | `catchim::editor::KeybindingMigrationEngine::v2ToV3` ... `v6ToV7` | Verified |
| Persisted Keybindings State Parser & Serializer | `web/src/actions/keybindings/persisted-state.ts` | `catchim::editor::KeybindingMigrationEngine::parsePersistedKeybindingsState`, `serializePersistedKeybindingsState` | Verified |

---

## 132. Animation Property Groups & Keyframe Query Engine
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| Animation Property Paths & Group Mappings | `web/src/animation/types.ts` | `catchim::editor::AnimationKeyframeQueryEngine::getAllAnimationPropertyPaths`, `getPropertyPathsForGroup` | Verified |
| Group Keyframe Query & Temporal Existence Check | `web/src/animation/property-groups.ts` | `catchim::editor::AnimationKeyframeQueryEngine::getGroupKeyframesAtTime`, `hasGroupKeyframeAtTime` | Verified |
| Multi-Channel Keyframe Extraction & Path Lookup | `web/src/animation/keyframe-query.ts` | `catchim::editor::AnimationKeyframeQueryEngine::getElementKeyframes`, `hasKeyframesForPath`, `getKeyframeAtTime` | Verified |

---

## 133. Command Reactor & Empty Track Auto-Pruning Pipeline
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| Command Lifecycle Reactor Registration & Notification | `web/src/core/index.ts` | `catchim::editor::CommandReactorPipeline::registerReactor`, `unregisterReactor`, `notifyReactors` | Verified |
| Empty Overlay & Audio Tracks Auto-Pruning | `web/src/core/index.ts` | `catchim::editor::CommandReactorPipeline::pruneEmptyTracks`, `createAutoPruneReactor` | Verified |

---

## 134. Timeline Pixel & Device Grid Utilities
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| Time to Pixel Conversion & Zoom Scaling | `web/src/timeline/pixel-utils.ts`, `scale.ts` | `catchim::editor::TimelinePixelUtils::getTimelinePixelsPerSecond`, `timelineTimeToPixels` | Verified |
| HiDPI Device Grid Snapping & Centering | `web/src/timeline/pixel-utils.ts` | `catchim::editor::TimelinePixelUtils::snapPixelToDeviceGrid`, `timelineTimeToSnappedPixels`, `getCenteredLineLeft` | Verified |

---

## 135. Timeline Zoom Levels & Exponential Slider Utilities
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| Zoom To Fit Minimum Level & Dynamic Container Padding | `web/src/timeline/zoom-utils.ts` | `catchim::editor::TimelineZoomUtils::getTimelineZoomMin`, `getTimelinePaddingPx`, `getZoomPercent` | Verified |
| Exponential Zoom to Linear Slider Bidirectional Mapping | `web/src/timeline/zoom-utils.ts` | `catchim::editor::TimelineZoomUtils::sliderToZoom`, `zoomToSlider` | Verified |

---

## 136. Timeline Drag Data & Element Creation Defaults
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| Default New Element Duration (5s) | `web/src/timeline/creation.ts` | `catchim::editor::TimelineDragEngine::toElementDurationTicks`, `DEFAULT_NEW_ELEMENT_DURATION` | Verified |
| ClientX to Timeline Time Conversion | `web/src/timeline/drag-utils.ts` | `catchim::editor::TimelineDragEngine::getMouseTimeFromClientX` | Verified |
| Drag Data Variant Hierarchy (Media, Text, Sticker, Graphic, Effect) | `web/src/timeline/drag.ts` | `catchim::editor::TimelineDragData`, `TimelineDragEngine::getDragDataId`, `getDragDataName`, `getDragDataType` | Verified |

---

## 137. Canvas Background Blur & Color Presets
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| Background Blur Intensity Presets (Light, Medium, Heavy) | `web/src/background/blur.ts` | `catchim::render::BackgroundPresets::getBlurPresets`, `findBlurPreset`, `clampBlurIntensity` | Verified |
| Default Canvas Background Color (#000000) | `web/src/background/color.ts` | `catchim::render::BackgroundPresets::DEFAULT_BACKGROUND_COLOR` | Verified |

---

## 138. Param Channel Layout & sRGB/Linear Conversion Engine
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| IEC 61966-2-1 sRGB to Linear RGBA Conversion | `web/src/params/index.ts` | `catchim::editor::ParamChannelLayoutEngine::srgbToLinear`, `linearToSrgb` | Verified |
| Linear RGBA Color Parsing & Hex8 Formatting | `web/src/params/index.ts` | `catchim::editor::ParamChannelLayoutEngine::parseColorToLinearRgba`, `formatLinearRgba` | Verified |
| Param Value Range Clamping & Stepped Coercion | `web/src/params/index.ts` | `catchim::editor::ParamChannelLayoutEngine::coerceParamValueNumber`, `coerceParamValueSelect` | Verified |

---

## 139. Internationalization (I18n) Engine
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| Multi-Locale Dictionary (en, vi) & Hierarchy Lookup | `web/src/i18n/` | `catchim::core::I18nEngine::t`, `hasKey`, `supportedLocales`, `setLocale` | Verified |
| Dynamic Template Parameter Interpolation ({name}, {count}) | `web/src/i18n/` | `catchim::core::I18nEngine::interpolate` | Verified |

---

## 140. Retime Resolution Engine
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| Retime Rate Bounds Clamping (0.1x - 50.0x) | `web/src/timeline/retime.ts` | `catchim::editor::RetimeResolutionEngine::clampRetimeRate` | Verified |
| Audio Pitch Maintenance Feasibility Bounds (0.25x - 4.0x) | `web/src/timeline/retime.ts` | `catchim::editor::RetimeResolutionEngine::canMaintainPitch` | Verified |
| Bidirectional Time & Span Mapping at Retime Rate | `web/src/timeline/retime.ts` | `catchim::editor::RetimeResolutionEngine::getSourceTimeAtClipTime`, `getClipTimeAtSourceTime`, `getTimelineDurationForSourceSpan`, `getSourceSpanAtClipTime` | Verified |

---

## 141. Effect Definition Registry & Gaussian Blur Shaders
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| Gaussian Blur Two-Pass Multi-Sample Kernel Resolution | `web/src/effects/definitions/blur.ts` | `catchim::render::EffectDefinitionRegistry::intensityToSigma`, `buildGaussianBlurPasses`, `GAUSSIAN_BLUR_SHADER` | Verified |
| Effect Definitions Registry & Default Instance Builder | `web/src/effects/index.ts` | `catchim::render::EffectDefinitionRegistry::definitions`, `findDefinition`, `buildDefaultEffectInstance` | Verified |
| Effect Pass Resolution Pipeline | `web/src/effects/index.ts` | `catchim::render::EffectDefinitionRegistry::resolveEffectPasses` | Verified |

---

## 142. Canvas Size Presets
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| Standard Canvas Dimensions Presets (1080p, 9:16, 1:1, 4:3) | `web/src/canvas/sizes.ts` | `catchim::render::CanvasSizePresets::defaultCanvasPresets`, `DEFAULT_CANVAS_SIZE` | Verified |
| Preset Membership & Aspect Ratio Utilities | `web/src/canvas/sizes.ts` | `catchim::render::CanvasSizePresets::isDefaultPreset`, `getAspectRatio` | Verified |

---

## 143. Scene Hierarchy & Main Scene Utilities
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| Main Scene Lookup & Invariant Enforcement | `web/src/timeline/scenes.ts` | `catchim::editor::SceneHierarchyUtils::getMainScene`, `ensureMainScene`, `buildDefaultScene` | Verified |
| Scene Deletion Validation & Fallback Selection | `web/src/timeline/scenes.ts` | `catchim::editor::SceneHierarchyUtils::canDeleteScene`, `getFallbackSceneAfterDelete`, `findCurrentScene` | Verified |
| Scene Timeline Total Duration Calculation | `web/src/timeline/scenes.ts`, `timeline/index.ts` | `catchim::editor::SceneHierarchyUtils::calculateTotalDuration`, `getProjectDurationFromScenes` | Verified |

---

## 144. RFC 4122 v4 UUID Generator
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| RFC 4122 Version 4 UUID Generation | `web/src/utils/id.ts` | `catchim::core::UuidGenerator::generateUUID` | Verified |
| UUID Format & Version/Variant Validation | `web/src/utils/id.ts` | `catchim::core::UuidGenerator::isValidUUID` | Verified |

---

## 145. String & Platform Key Utilities
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| String Capitalization & Case Transformation | `web/src/utils/string.ts` | `catchim::core::StringUtils::capitalizeFirstLetter`, `uppercase`, `lowercase` | Verified |
| Platform-Specific Modifier Keys Detection (⌘ / Ctrl, ⌥ / Alt) | `web/src/utils/platform.ts` | `catchim::core::StringUtils::isAppleDevice`, `getPlatformSpecialKey`, `getPlatformAlternateKey` | Verified |

---

## 146. Geometry & Aspect Ratio Utilities
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| Greatest Common Divisor (GCD) Calculation | `web/src/utils/geometry.ts` | `catchim::core::GeometryUtils::gcd` | Verified |
| Dimension to Aspect Ratio String Formatting (16:9, 4:3, 1:1) | `web/src/utils/geometry.ts` | `catchim::core::GeometryUtils::dimensionToAspectRatio` | Verified |

---

## 147. Date Formatting Utilities
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| Standard en-US Date Formatting ("Sep 20, 2026") | `web/src/utils/date.ts` | `catchim::core::DateUtils::formatDate` | Verified |

---

## 148. Render Param & Transform Resolvers
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| 17 Standard Blend Modes Validation & Fallback | `web/src/rendering/index.ts` | `catchim::render::RenderParamResolvers::isBlendMode`, `readBlendModeFromParams` | Verified |
| Opacity & Transform Parameter Extraction | `web/src/rendering/index.ts` | `catchim::render::RenderParamResolvers::readOpacityFromParams`, `buildTransformFromParams` | Verified |
| Dynamic Animated Transform Evaluation at Time | `web/src/rendering/animation-values.ts` | `catchim::render::RenderParamResolvers::resolveTransformAtTime` | Verified |

---

## 149. Animation Value Resolvers & Curve Bridge
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| Opacity, Number, & Color Evaluation at Local Time | `web/src/animation/values.ts` | `catchim::editor::AnimationValueResolvers::resolveOpacityAtTime`, `resolveNumberAtTime`, `resolveColorAtTime` | Verified |
| Normalized Cubic Bezier Segment Conversion | `web/src/animation/curve-bridge.ts` | `catchim::editor::AnimationValueResolvers::getNormalizedCubicBezierForScalarSegment` | Verified |
| Normalized Bezier to Curve Handles Reconstruction | `web/src/animation/curve-bridge.ts` | `catchim::editor::AnimationValueResolvers::getCurveHandlesForNormalizedCubicBezier` | Verified |

---

## 150. Timeline Track Names & Audio Defaults
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| Default Track Names by Type (Video, Text, Audio, Graphic, Effect) | `web/src/timeline/tracks.ts` | `catchim::editor::TimelineTrackDefaults::getDefaultTrackName` | Verified |
| Audio Decibel Range Bounds (-60 dB to +20 dB) | `web/src/timeline/audio-constants.ts` | `catchim::editor::TimelineTrackDefaults::clampVolumeDb`, `VOLUME_DB_MIN`, `VOLUME_DB_MAX` | Verified |
| Playhead Snap Point Generator | `web/src/timeline/playhead-snap-source.ts` | `catchim::editor::TimelineTrackDefaults::getPlayheadSnapPoints` | Verified |

---

## 151. TTS Voice Catalog Registry
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| Natural Vietnamese Voice Catalog (Hoài My, Nam Minh, GPT-SoVITS, Chị Google...) | `web/src/services/tts/voices.ts` | `catchim::media::TtsVoiceRegistry::getAllVoices`, `findVoiceById` | Verified |
| Voice Filtering by Language, Category, and Gender | `web/src/services/tts/voices.ts`, `types.ts` | `catchim::media::TtsVoiceRegistry::getVoicesByLanguage`, `getVoicesByCategory`, `getVoicesByGender` | Verified |

---

## 152. Core Playback Manager
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| Playback Control (play, pause, toggle, seek, stepForward, stepBackward) | `web/src/core/managers/playback-manager.ts` | `catchim::editor::PlaybackManager::play`, `pause`, `toggle`, `seek`, `stepForward`, `stepBackward` | Verified |
| Volume & Mute Management | `web/src/core/managers/playback-manager.ts` | `catchim::editor::PlaybackManager::setVolume`, `mute`, `unmute`, `toggleMute` | Verified |
| Scrubbing State & Timeline Scope Reconciliation | `web/src/core/managers/playback-manager.ts` | `catchim::editor::PlaybackManager::setScrubbing`, `reconcileTimelineScope` | Verified |

---

## 153. Core Timeline Manager
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| Track Operations (addTrack, removeTrack, toggleMute, toggleVisibility) | `web/src/core/managers/timeline-manager.ts` | `catchim::editor::TimelineManager::addTrack`, `removeTrack`, `toggleTrackMute`, `toggleTrackVisibility` | Verified |
| Element Operations (insertElement, deleteElements, duplicateElements, splitElements, moveElement) | `web/src/core/managers/timeline-manager.ts` | `catchim::editor::TimelineManager::insertElement`, `deleteElements`, `duplicateElements`, `splitElements`, `moveElement` | Verified |
| Total Duration & Last Frame Time Calculation | `web/src/core/managers/timeline-manager.ts` | `catchim::editor::TimelineManager::getTotalDuration`, `getLastFrameTime` | Verified |

---

## 154. Core Renderer Manager
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| Snapshot Generation & File Naming | `web/src/core/managers/renderer-manager.ts` | `catchim::render::RendererManager::createSnapshot` | Verified |
| Project Export Pipeline & Progress/Cancel Tracking | `web/src/core/managers/renderer-manager.ts` | `catchim::render::RendererManager::exportProject` | Verified |
| Performance Degraded State Notification | `web/src/core/managers/renderer-manager.ts` | `catchim::render::RendererManager::isDegraded`, `setDegraded` | Verified |

---

## 155. Core Save Manager
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| Debounced Autosave Lifecycle (start, stop, pause, resume) | `web/src/core/managers/save-manager.ts` | `catchim::editor::SaveManager::start`, `stop`, `pause`, `resume` | Verified |
| Dirty State & Immediate Flush | `web/src/core/managers/save-manager.ts` | `catchim::editor::SaveManager::markDirty`, `flush`, `isDirty` | Verified |

---

## 156. Core Media Manager
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| Media Asset Catalog & Lookup | `web/src/core/managers/media-manager.ts` | `catchim::media::MediaManager::getAssets`, `findAsset` | Verified |
| Add, Remove & Clear Asset Operations | `web/src/core/managers/media-manager.ts` | `catchim::media::MediaManager::addMediaAsset`, `removeMediaAsset`, `removeMediaAssets`, `clearAllAssets` | Verified |
| Loading State Tracking & Change Listeners | `web/src/core/managers/media-manager.ts` | `catchim::media::MediaManager::isLoadingMedia`, `setIsLoading`, `subscribe` | Verified |

---

## 157. Core Audio Manager
| Tính năng Web | Source File Web | Thiết kế C++ App | Trạng thái |
|---|---|---|---|
| Master Volume & Mute State | `web/src/core/managers/audio-manager.ts` | `catchim::audio::AudioManager::setMasterVolume`, `setMuted` | Verified |
| Track Mute & Track Solo Management | `web/src/core/managers/audio-manager.ts` | `catchim::audio::AudioManager::setTrackMute`, `isTrackMuted`, `setTrackSolo`, `isTrackSolo`, `hasSoloTracks` | Verified |
| Active Audio Clips Collection & Effective Gain Calculation | `web/src/core/managers/audio-manager.ts` | `catchim::audio::AudioManager::collectActiveAudioClips` | Verified |





