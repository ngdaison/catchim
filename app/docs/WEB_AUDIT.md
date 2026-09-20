# WEB AUDIT - KIỂM TOÁN KIẾN TRÚC ỨNG DỤNG WEB CATCHIM

**Nguồn đối chiếu**: `D:\DATA\source\catchim\web`  
**Ngày kiểm toán**: 2026-09-20  
**Phiên bản Web**: `@opencut/web v0.1.0` (Next.js 16.1.3, React 19, TypeScript 5.8.3, TailwindCSS v4)

---

## 1. Tổng quan Công nghệ Web
| Thành phần | Công nghệ / Thư viện | Mô tả vai trò |
|---|---|---|
| **Framework & Router** | Next.js 16 (App Router), React 19 | Điều phối trang và quản lý vòng đời view |
| **State Management** | Zustand v5 (kèm middleware `persist`) | Quản lý state cục bộ cho Editor, Assets panel, Properties, Timeline, Preview, Keybindings |
| **Data Models & Schema** | TypeScript + Zod 4.3.6 | Định nghĩa kiểu dữ liệu chặt chẽ cho Project, Scenes, Tracks, Elements |
| **Time System** | `MediaTime` (Integer ticks) | $120,000\text{ ticks/giây}$ (`TICKS_PER_SECOND = 120_000`). Tuyệt đối tránh sai số dấu phẩy động |
| **Media Handling** | WebCodecs, Mediabunny v1.29.1 | Demux, decode video frame và audio samples |
| **Audio Processing** | Web Audio API (`AudioContext`), SoundTouchJS v0.3.0 | Phát lại, mixing âm thanh đa track, điều chỉnh volume, time-stretching / pitch-shifting |
| **Waveform Visualization** | Wavesurfer.js v7.9.8, Cache phân cụm | Trích xuất biên độ min/max theo bucket để vẽ dạng sóng |
| **Canvas & Rendering** | HTML5 Canvas 2D + WebGL / wgpu shader pass | Render scene tree theo z-index, transform matrix, color grading, blur, vignette, masks |
| **Icons & UI Primitives** | Radix UI, HugeIcons, Lucide Icons | Nút bấm, Dialogs, DropdownMenu, ContextMenu, Tooltips, Sliders |
| **Animation & Easing** | Cubic-bezier curves, Bezier solver | Interpolation cho keyframes các thuộc tính position, scale, rotate, opacity, volume |
| **Storage & Persistence** | IndexedDB, File System / OPFS, LocalStorage | Lưu dự án, lịch sử phiên bản (Migration Runner v0 đến v31) |

---

## 2. Cấu trúc Cây Thư mục Mã nguồn Web (`src/`)
```
web/src/
├── actions/              # Hệ thống Action registry & Phím tắt bàn phím (Keybindings)
├── animation/            # Keyframe animations, cubic-bezier math, easing functions
├── app/                  # Next.js App Router (editor/[project_id], api, globals.css)
├── canvas/               # Overlay, Transform gizmos, bounding box interaction
├── commands/             # Command Pattern: Timeline, Track, Element, Project commands (Undo/Redo)
├── components/           # UI Components
│   ├── editor/           # EditorHeader, ExportButton, ScenesView, Panels (Assets, Properties)
│   └── ui/               # Radix UI wrappers (Button, Dialog, Select, Slider, Tooltip, etc.)
├── core/managers/        # Core Editor Managers (Project, Timeline, Playback, Audio, Renderer, etc.)
├── effects/              # Định nghĩa hiệu ứng (Gaussian Blur, Color Grading, Vignette, Chroma Key)
├── export/               # Cấu hình và pipeline xuất video (Resolution, Quality, Mime, Format)
├── masks/                # Hệ thống Mask (Rectangle, Ellipse, Heart, Star, Cinematic Bars, Freeform)
├── media/                # Quản lý tài nguyên media (Asset metadata, probe, waveform summary)
├── preview/              # Viewport preview, Toolbar điều khiển, Overlays, Context menu
├── project/              # Project types, Metadata, Dialogs (Rename, Delete, Migration)
├── rendering/            # Scene Builder, Node tree, Compositor, Shaders
├── services/storage/     # IndexedDB adapter, Storage migration runner (v0 -> v31)
├── speed/                # Retime, tốc độ phát và đường cong tốc độ (Speed curve)
├── text/                 # Định dạng văn bản, phông chữ, Text-to-Speech
└── timeline/             # Cốt lõi Timeline: Tracks, Clips, Playhead, Ruler, Snapping, Bookmarks
```

---

## 3. Mô hình Dữ liệu Chuẩn (Data Models Spec)

### 3.1. Đơn vị Thời gian (`MediaTime`)
- Bản chất là số nguyên 64-bit (`int64_t` trong C++):
  $$\text{ticks} = \text{seconds} \times 120,000$$
- Ưu điểm: Chia hết chính xác cho các tốc độ khung hình phổ biến:
  - $24\text{ fps} \rightarrow 5,000\text{ ticks/frame}$
  - $25\text{ fps} \rightarrow 4,800\text{ ticks/frame}$
  - $30\text{ fps} \rightarrow 4,000\text{ ticks/frame}$
  - $48\text{ fps} \rightarrow 2,500\text{ ticks/frame}$
  - $50\text{ fps} \rightarrow 2,400\text{ ticks/frame}$
  - $60\text{ fps} \rightarrow 2,000\text{ ticks/frame}$
  - $120\text{ fps} \rightarrow 1,000\text{ ticks/frame}$
- Timecode Formats: `"HH:MM:SS:FF"`, `"HH:MM:SS:CS"`, `"MM:SS"`, `"HH:MM:SS"`.

### 3.2. Cấu trúc Dự án (`TProject` - Version 31)
```json
{
  "version": 31,
  "metadata": {
    "id": "uuid/nanoid",
    "name": "Tên dự án",
    "thumbnail": "data-url/path",
    "duration": 0,
    "createdAt": "ISO-date",
    "updatedAt": "ISO-date"
  },
  "settings": {
    "fps": { "numerator": 30, "denominator": 1 },
    "canvasSize": { "width": 1920, "height": 1080 },
    "canvasSizeMode": "preset",
    "background": { "type": "color", "color": "#000000" }
  },
  "currentSceneId": "scene-1",
  "scenes": [
    {
      "id": "scene-1",
      "name": "Main scene",
      "isMain": true,
      "bookmarks": [],
      "tracks": {
        "overlay": [],
        "main": {
          "id": "main-track",
          "name": "Video",
          "type": "video",
          "elements": [],
          "muted": false,
          "hidden": false
        },
        "audio": []
      }
    }
  ],
  "timelineViewState": {
    "zoomLevel": 1.0,
    "scrollLeft": 0,
    "playheadTime": 0
  }
}
```

### 3.3. Các loại Track và Element
1. **VideoTrack**: Chứa `VideoElement`, `ImageElement`.
2. **AudioTrack**: Chứa `AudioElement` (upload hoặc sound library).
3. **TextTrack**: Chứa `TextElement` (phông chữ, kích thước, căn lề, màu nền, viền).
4. **GraphicTrack**: Chứa `StickerElement`, `GraphicElement` (shapes, icons).
5. **EffectTrack**: Chứa `EffectElement` (filter/effect độc lập kéo dài trên timeline).

---

## 4. Hành vi & Tương tác Cốt lõi (Core Behaviors)
- **Tương tác Timeline**:
  - Di chuyển clip (Move): Kéo clip ngang thay đổi `startTime`, kéo dọc đổi track tương thích. Tự động tìm gap hoặc áp dụng ripple.
  - Cắt clip (Split): Chia clip tại vị trí playhead thành 2 clip với `sourceDuration` và `trimStart`/`trimEnd` tương ứng.
  - Cắt trái/phải (`Split-left`, `Split-right`): Cắt và loại bỏ ngay đoạn clip bên trái hoặc bên phải playhead.
  - Thu phóng (Zoom): Ctrl + Con lăn chuột zoom theo con trỏ chuột; thanh trượt Slider mượt mà.
  - Nam châm hít (Snapping): Hít vào Playhead, mép đầu clip (start), mép cuối clip (end), và Marker/Bookmark.
  - Marker/Bookmark: Đánh dấu điểm lưu ý trên thước timeline, đổi màu, thêm ghi chú.
- **Hệ thống Lệnh (Commands / Undo - Redo)**:
  - Tất cả các thao tác thay đổi timeline, track, element, metadata đều qua Command Pattern.
  - Hỗ trợ hoàn tác (Ctrl+Z) và làm lại (Ctrl+Shift+Z, Ctrl+Y).
- **Phát lại & Preview**:
  - Điều khiển qua `PlaybackManager`: Play/Pause, Seek, Step frame forward/backward.
  - Preview Canvas: Fit to screen hoặc chọn zoom tỷ lệ cố định (25%, 50%, 75%, 100%, 150%, 200%). Hỗ trợ pan khung nhìn.
- **Xuất Video (Export Pipeline)**:
  - Định dạng: MP4, WebM.
  - Chất lượng: Low, Medium, High, Very High.
  - Độ phân giải: Source, 480p, 720p, 1080p, 1440p, 2160p (4K), 4320p (8K).
  - Tùy chọn kèm audio hoặc tắt tiếng.
