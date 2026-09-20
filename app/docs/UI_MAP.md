# UI MAP - ÁNH XẠ GIAO DIỆN & STYLE TOKEN TỪ WEB SANG C++ APP

Tài liệu này xác định chi tiết từng vùng giao diện, tệp React/CSS gốc, giá trị thiết kế (metrics, màu sắc, font, icon) và lớp C++ tương ứng.

---

## 1. Hệ thống Màu sắc & Token (Color Tokens)
Trích xuất trực tiếp từ `web/src/app/globals.css`:

### 1.1. Chế độ Sáng (Light Theme)
- Background App: `hsl(0, 0%, 100%)` $\rightarrow$ `#FFFFFF`
- Panel Background: `hsl(210, 20%, 98%)` $\rightarrow$ `#F8FAFC`
- Text Primary: `hsl(0, 0%, 11%)` $\rightarrow$ `#1C1C1C`
- Text Muted: `hsl(0, 0%, 50%)` $\rightarrow$ `#7F7F7F`
- Primary Accent: `hsl(200, 90%, 52%)` $\rightarrow$ `#16A9F3`
- Secondary / Hover: `hsl(204, 100%, 97%)` $\rightarrow$ `#F0F8FF`
- Border: `hsl(0, 0%, 91%)` $\rightarrow$ `#E8E8E8`
- Panel Border: `hsl(0, 0%, 87%)` $\rightarrow$ `#DEDEDE`
- Destructive: `hsl(0, 83%, 50%)` $\rightarrow$ `#EA1616`

### 1.2. Chế độ Tối (Dark Theme - Mặc định)
- Background App: `hsl(0, 0%, 5%)` $\rightarrow$ `#0D0D0D`
- Panel Background: `hsl(0, 0%, 10%)` $\rightarrow$ `#1A1A1A`
- Text Primary: `hsl(0, 0%, 87%)` $\rightarrow$ `#DEDEDE`
- Text Muted: `hsl(0, 0%, 50%)` $\rightarrow$ `#808080`
- Primary Accent: `hsl(200, 90%, 52%)` $\rightarrow$ `#16A9F3`
- Secondary: `hsl(204, 100%, 12%)` $\rightarrow$ `#00223D`
- Border: `hsl(0, 0%, 16%)` $\rightarrow$ `#292929`
- Panel Border: `hsl(0, 0%, 18%)` $\rightarrow$ `#2E2E2E`
- Accent / Hover: `hsl(0, 0%, 14%)` $\rightarrow$ `#242424`
- Destructive: `hsl(0, 83%, 50%)` $\rightarrow$ `#EA1616`

### 1.3. Màu sắc Clip trên Timeline (`web/src/timeline/components/theme.ts`)
- `video`: Thumbnail strip nền transparent, viền tối mỏng
- `text`: `#5DBAA0` (Xanh ngọc lợt)
- `audio`: `#8F5DBA` (Tím nhạt), Sóng âm waveform: `rgba(255, 255, 255, 0.7)`
- `graphic`: `#BA5D7A` (Hồng cánh sen)
- `effect`: `#5D93BA` (Xanh dương nhạt)
- `bookmark`: `#009DFF` (Xanh dương sáng)
- `selected track row`: `rgba(255, 255, 255, 0.05)`

---

## 2. Thông số Kích thước Cốt lõi (Layout Metrics)
Trích xuất trực tiếp từ `web/src/timeline/components/layout.ts` & `editor-header.tsx`:
- Header Chiều cao: `3.4rem` = `54px`
- Panel Border Radius: `0.35rem` = `6px`
- Gap giữa các Panel: `0.19rem` = `3px`
- Timeline Track Chiều cao:
  - Video Track: `65px`
  - Audio Track: `50px`
  - Text Track: `25px`
  - Graphic Track: `25px`
  - Effect Track: `25px`
- Keyframe Lane Chiều cao: `20px`
- Keyframe Diamond Size: `14px`
- Timeline Track Gap: `6px`
- Timeline Track Labels Cột rộng: `112px`
- Timeline Ruler Chiều cao: `22px`
- Timeline Bookmarks Row Chiều cao: `16px`
- Timeline Scrollbar Chiều rộng/cao: `12px`
- Timeline Content Top Padding: `2px`

---

## 3. Bảng Ánh xạ Component Sang C++ Widget

### 3.1. Vỏ Ứng dụng & Header
| Thành phần Web | Tệp Mã nguồn Web | Lớp C++ Widget Tương đương |
|---|---|---|
| Main Window Shell | `web/src/app/editor/[project_id]/page.tsx` | `catchim::ui::MainWindow` |
| Editor Header | `web/src/components/editor/editor-header.tsx` | `catchim::ui::EditorHeader` |
| Project Dropdown Menu | `web/src/components/editor/editor-header.tsx` | `catchim::ui::ProjectMenu` |
| Editable Project Name | `web/src/components/editor/editor-header.tsx` | `catchim::ui::InlineProjectNameEdit` |
| Export Button & Dialog | `web/src/components/editor/export-button.tsx` | `catchim::ui::ExportButton`, `ExportDialog` |
| Shortcuts Dialog | `web/src/actions/components/shortcuts-dialog.tsx` | `catchim::ui::ShortcutsDialog` |

### 3.2. Assets Panel (Bảng Tài nguyên)
| Thành phần Web | Tệp Mã nguồn Web | Lớp C++ Widget Tương đương |
|---|---|---|
| Assets Panel Root | `web/src/components/editor/panels/assets/index.tsx` | `catchim::ui::AssetsPanel` |
| Vertical TabBar | `web/src/components/editor/panels/assets/tabbar.tsx` | `catchim::ui::AssetsTabBar` |
| Media View (Grid / List) | `web/src/components/editor/panels/assets/views/assets.tsx` | `catchim::ui::MediaAssetsView` |
| Draggable Item & Overlay | `web/src/components/editor/panels/assets/draggable-item.tsx` | `catchim::ui::MediaItemWidget` |
| Project Settings View | `web/src/components/editor/panels/assets/views/settings/` | `catchim::ui::ProjectSettingsView` |

### 3.3. Preview Panel (Bảng Xem trước)
| Thành phần Web | Tệp Mã nguồn Web | Lớp C++ Widget Tương đương |
|---|---|---|
| Preview Panel Root | `web/src/preview/components/index.tsx` | `catchim::ui::PreviewPanel` |
| Preview Viewport & Canvas | `web/src/preview/components/index.tsx` | `catchim::ui::PreviewWidget` (OpenGL/QPainter) |
| Preview Toolbar | `web/src/preview/components/toolbar.tsx` | `catchim::ui::PreviewToolbar` |
| Editable Timecode | `web/src/components/editable-timecode.tsx` | `catchim::ui::EditableTimecodeWidget` |
| Zoom Select Dropdown | `web/src/preview/components/toolbar.tsx` | `catchim::ui::PreviewZoomSelector` |
| Preview Context Menu | `web/src/preview/components/context-menu.tsx` | `catchim::ui::PreviewContextMenu` |

### 3.4. Properties Panel (Inspector Thuộc tính)
| Thành phần Web | Tệp Mã nguồn Web | Lớp C++ Widget Tương đương |
|---|---|---|
| Properties Panel Root | `web/src/components/editor/panels/properties/index.tsx` | `catchim::ui::PropertiesPanel` |
| Empty View | `web/src/components/editor/panels/properties/empty-view.tsx` | `catchim::ui::PropertiesEmptyView` |
| Property TabBar | `web/src/components/editor/panels/properties/index.tsx` | `catchim::ui::PropertiesTabBar` |
| Transform Tab | `web/src/components/editor/panels/properties/registry.tsx` | `catchim::ui::TransformPropertiesTab` |
| Audio Tab | `web/src/components/editor/panels/properties/registry.tsx` | `catchim::ui::AudioPropertiesTab` |
| Speed Tab | `web/src/speed/components/speed-tab.tsx` | `catchim::ui::SpeedPropertiesTab` |
| Masks Tab | `web/src/masks/components/masks-tab.tsx` | `catchim::ui::MasksPropertiesTab` |
| Effects Tab | `web/src/effects/components/effects-tab.tsx` | `catchim::ui::EffectsPropertiesTab` |
| Text Tab | `web/src/components/editor/panels/properties/registry.tsx` | `catchim::ui::TextPropertiesTab` |

### 3.5. Timeline (Dòng thời gian)
| Thành phần Web | Tệp Mã nguồn Web | Lớp C++ Widget Tương đương |
|---|---|---|
| Timeline Root Panel | `web/src/timeline/components/index.tsx` | `catchim::ui::TimelinePanel` |
| Timeline Toolbar | `web/src/timeline/components/timeline-toolbar.tsx` | `catchim::ui::TimelineToolbar` |
| Timeline Ruler | `web/src/timeline/components/timeline-ruler.tsx` | `catchim::ui::TimelineRulerWidget` |
| Bookmarks Row | `web/src/timeline/bookmarks/` | `catchim::ui::TimelineBookmarksWidget` |
| Track Labels Header | `web/src/timeline/components/index.tsx` | `catchim::ui::TimelineTrackHeaderWidget` |
| Timeline Tracks Area | `web/src/timeline/components/timeline-track.tsx` | `catchim::ui::TimelineTracksWidget` |
| Timeline Playhead | `web/src/timeline/components/timeline-playhead.tsx` | `catchim::ui::TimelinePlayheadOverlay` |
| Snapping Indicator Line | `web/src/timeline/components/snap-indicator.tsx` | `catchim::ui::TimelineSnapLine` |
| Drag Target Guide Line | `web/src/timeline/components/drag-line.tsx` | `catchim::ui::TimelineDragLine` |
| Marquee Selection Box | `web/src/selection/selection-box.tsx` | `catchim::ui::TimelineSelectionBox` |
