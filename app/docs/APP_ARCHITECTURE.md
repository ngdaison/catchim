# APP ARCHITECTURE - KIẾN TRÚC HỆ THỐNG CATCHIM DESKTOP C++

Tài liệu này đặc tả kiến trúc phân tầng của ứng dụng desktop native C++20, luồng phụ thuộc một chiều, nguyên tắc tách rời UI và Core logic.

---

## 1. Sơ đồ Phân tầng & Chiều Phụ thuộc (Dependency Direction)

```
                            [ Catchim App ]
                                   │
                                   ▼
                             [ C++ UI ]
                     (Qt Widgets & Custom QPainters)
                                   │
                                   ▼
                          [ EditorEngine ]
                (Command dispatch & Event publisher)
                                   │
         ┌─────────────────────────┼─────────────────────────┐
         ▼                         ▼                         ▼
   [ Project ]               [ Timeline ]               [ History ]
(Model, Settings)         (Tracks, Clips, Time)        (Undo / Redo)
         │                         │
         └────────────┬────────────┘
                      │
                      ▼
                 [ Render ]
           (Compositor, Shaders)
                      │
         ┌────────────┴────────────┐
         ▼                         ▼
     [ Media ]                 [ Audio ]
  (Decoders, Cache)       (Mixer, Resampler)
         │                         │
         └────────────┬────────────┘
                      │
                      ▼
            [ Platform / Hardware ]
          (FFmpeg libs, GPU Context)
```

### Nguyên tắc Bất biến:
1. **Một chiều tuyệt đối**: Tầng trên gọi xuống tầng dưới. Không có circular dependency (ví dụ: Timeline không biết UI hay MainWindow tồn tại).
2. **Core độc lập**: Toàn bộ Core (`core/`, `editor/project`, `editor/timeline`, `editor/history`, `core/time`) được viết bằng **C++20 thuần túy** (`std::vector`, `std::string`, `std::optional`, `std::chrono`, `int64_t ticks`). Không ép dùng `QString`, `QVector` trong core data model để đảm bảo tính module hóa và hiệu năng tối đa.
3. **UI không sở hữu Logic**: UI (`ui/`) chỉ gửi lệnh yêu cầu sang `EditorEngine` (ví dụ: `editor.splitClip(clipId, time)`), sau đó lắng nghe các sự kiện (`ProjectChanged`, `TimelineChanged`, `ClipMoved`, `PlayheadChanged`) để repaint đúng vùng dirty region.

---

## 2. Các Module Chính & Trách nhiệm

### 2.1. Module `core`
- `core/time/TimelineTime`: Biểu diễn thời gian bằng số nguyên ticks 64-bit với $120,000\text{ ticks/sec}$. Các hàm cộng, trừ, làm tròn khung hình, clamp, tính giây.
- `core/time/Timecode`: Chuyển đổi hai chiều giữa ticks và chuỗi Timecode (`HH:MM:SS:FF`, `HH:MM:SS:CS`, `MM:SS`, `HH:MM:SS`).
- `core/ids/Ids`: Quản lý ID ổn định (`ProjectId`, `TrackId`, `ClipId`, `MediaId`, `EffectId`, `BookmarkId`).
- `core/logging/Logger`: Ghi log phân cấp (Trace, Debug, Info, Warning, Error, Critical), thread-safe.
- `core/errors/Errors`: Hệ thống mã lỗi và thông báo chi tiết.

### 2.2. Module `editor`
- `editor/EditorEngine`: Trái tim điều phối của toàn bộ editor. Nắm giữ con trỏ tới `Project`, `Timeline`, `CommandHistory`, `PlaybackController`, `MediaLibrary`.
- `editor/project/Project`: Đại diện cho dự án biên tập (Settings: FPS, CanvasSize, Background; Scenes; ViewState).
- `editor/timeline/Timeline`: Quản lý danh sách tracks (`SceneTracks`: overlay, main video, audio) và bookmarks.
- `editor/timeline/Track`: Mỗi track lưu loại track (`video`, `audio`, `text`, `graphic`, `effect`), thuộc tính mute, hide và mảng các `Clip`.
- `editor/timeline/Clip`: Lưu trữ `timelineStart`, `timelineDuration`, `sourceStart`, `sourceDuration`, `mediaId`, `params` (Transform, Opacity, Blend, Volume, Mute, Text params).
- `editor/history/CommandHistory`: Ngăn xếp Undo / Redo quản lý các `EditorCommand`.

### 2.3. Module `media`
- `media/MediaAsset`: Chứa thông tin siêu dữ liệu video/audio/ảnh (đường dẫn, thời lượng, độ phân giải, fps, codec, kênh âm thanh, sample rate).
- `media/probe/MediaProbe`: Sử dụng FFmpeg để phân tích tập tin media.
- `media/decoder/VideoDecoder`: Giải mã từng khung hình video theo timestamp seek request.
- `media/thumbnail/ThumbnailGenerator`: Trích xuất và lưu bộ nhớ đệm thumbnail dạng dải film (filmstrip) trong nền (background thread).
- `media/waveform/WaveformGenerator`: Tính toán mẫu biên độ âm thanh (RMS / Peak) chia bucket để vẽ sóng âm.

### 2.4. Module `render`
- `render/Compositor`: Ghép các layer từ dưới lên trên theo thứ tự z-index của scene.
- `render/Transform`: Tính toán ma trận biến đổi tọa độ (center X/Y, scale X/Y, rotation, flip X/Y).
- `render/effects/`: Các shader và thuật toán xử lý điểm ảnh (Gaussian blur, color grading, vignette, chroma key).
- `render/masks/`: Cắt lớp theo hình dạng (chữ nhật, elip, ngôi sao, trái tim, vệt tự do) và làm mờ mép (feathering).

### 2.5. Module `audio`
- `audio/AudioMixer`: Trộn các kênh âm thanh đang active tại thời điểm timeline, áp dụng volume, mute, fade in/out.
- `audio/AudioResampler`: Chuyển đổi sample rate của nguồn media về sample rate đầu ra chuẩn (ví dụ 44.1kHz hoặc 48kHz).

### 2.6. Module `ui`
- `ui/MainWindow`: Khung cửa sổ chính với Custom TitleBar và bố cục panel chia ngăn.
- `ui/theme/Theme`: Bộ bảng màu, kích thước, phông chữ đồng bộ 100% với `globals.css` của web.
- `ui/timeline/TimelineTracksWidget`: Custom widget tự vẽ toàn bộ timeline bằng `QPainter`, tối ưu hóa hiệu năng render hàng ngàn clip và zoom mượt mà.
- `ui/preview/PreviewWidget`: Custom widget hiển thị khung hình preview kèm handles điều khiển biến đổi.

---

## 3. Quy trình Khởi động & Thoát (Startup & Shutdown Lifecycle)

```
main()
  │
  ▼
Catchim::Application app(argc, argv)
  │
  ├── Init Logging
  ├── Init Config & Settings
  ├── Init FFmpeg Runtime
  ├── Create MediaLibrary & AudioEngine
  ├── Create RenderEngine
  ├── Create EditorEngine
  ├── Create MainWindow (áp dụng Theme & UI Tokens)
  └── Show MainWindow
        │
        ▼
   Qt Event Loop (app.exec())
        │
        ▼
Shutdown:
  ├── Dừng PlaybackClock & AudioOutput
  ├── Hủy các Background Task (Waveform, Thumbnail)
  ├── Giải phóng Decoders & Media Buffers
  ├── Lưu Settings & Autosave (nếu có)
  └── Đóng MainWindow & Thoát an toàn
```
