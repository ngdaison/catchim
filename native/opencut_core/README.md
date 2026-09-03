# OpenCut Native Core (C++20)

Thư viện lõi xử lý hiệu năng cao của Catchim, được viết hoàn toàn bằng **C++20** và biên dịch sang **WebAssembly** thông qua Emscripten.

---

## 📌 Các Phân Hệ Cốt Lõi

1. **`opencut::time`** (`include/opencut/time.hpp`, `src/time.cpp`):
   - Đơn vị thời gian media ticks chuẩn: `TICKS_PER_SECOND = 120_000`.
   - Chuẩn khung hình Rational framerate (`FrameRate`) hỗ trợ mọi chuẩn fps phổ biến.
   - Định dạng và phân tích Timecode SMPTE (`MM:SS`, `HH:MM:SS`, `HH:MM:SS:CS`, `HH:MM:SS:FF`).
   - Căn chỉnh khung hình: `round_to_frame`, `floor_to_frame`, `snapped_seek_time`.

2. **`opencut::timeline`** (`include/opencut/timeline.hpp`, `src/timeline.cpp`):
   - Cấu trúc timeline đa track, quản lý chỉ mục khoảng thời gian (`TimeRange`, `Clip`, `Track`).
   - Kiểm tra va chạm (`can_place_clip`, `find_first_overlapping_clip`).
   - Các lệnh thao tác clip: thêm mới, xóa, di chuyển, cắt ngắn, chia đôi clip.
   - Thuật toán hít nam châm thông minh (`snap_time`).
   - Di chuyển nhiều clip đồng thời theo nhóm (`check_group_move`).

3. **`opencut::compositor`** (`include/opencut/compositor.hpp`, `src/compositor.cpp`):
   - Biến đổi ma trận 3x3 (`QuadTransform`, `Matrix3x3`, tịnh tiến, xoay, phóng to/thu nhỏ, lật ảnh).
   - 17 chế độ hòa trộn màu sắc (Blend Modes: `Normal`, `Darken`, `Multiply`, `ColorBurn`, `Lighten`, `Screen`, `Overlay`,...).
   - Tính toán hòa trộn alpha compositing `blend_colors`.

4. **`opencut::masks`** (`include/opencut/masks.hpp`, `src/masks.cpp`):
   - Giải tích Signed Distance Fields (SDF) cho hình chữ nhật, hình tròn, elip, mặt phẳng tuyến tính, đa giác.
   - Làm mờ biên độ mềm (analytical feathering) và tính toán giá trị alpha.

5. **`opencut::effects`** (`include/opencut/effects.hpp`, `src/effects.cpp`):
   - Tinh chỉnh thông số màu sắc (nhiệt độ màu, phơi sáng, tương phản, độ bão hòa, vignette, gamma).
   - Tạo nhân ma trận làm mờ 1D Gaussian kernel.

6. **`opencut::audio`** (`include/opencut/audio.hpp`, `src/audio.cpp`):
   - Đường bao âm lượng keyframe envelope.
   - Làm mượt âm lượng vào/ra (fade-in / fade-out).
   - Thuật toán trích xuất đỉnh sóng âm thanh (waveform decimation).

7. **`opencut::animation`** (`include/opencut/animation.hpp`, `src/animation.cpp`):
   - Giải phương trình tham số bậc 3 Cubic Bezier.
   - Quản lý kênh KeyframeChannel (Hold, Linear, Bezier).
   - Đánh giá nhóm kênh TransformChannelGroup (Position, Scale, Rotation, Opacity, Anchor).

8. **`opencut::speed`** (`include/opencut/speed.hpp`, `src/speed.cpp`):
   - Đánh giá SpeedCurve đa điểm.
   - Tích phân giải tích đoạn thẳng ánh xạ chính xác timeline offset sang source media offset.

9. **C ABI & WebAssembly Exports** (`include/opencut/c_api.h`, `src/c_api.cpp`):
   - C ABI tương thích WebAssembly chuẩn.

---

## 🛠️ Hướng Dẫn Biên Dịch & Kiểm Thử

### Biên dịch và chạy Unit Test (Native C++)

```bash
cmake -S native/opencut_core -B native/opencut_core/build -DCMAKE_BUILD_TYPE=Release
cmake --build native/opencut_core/build --config Release
ctest --test-dir native/opencut_core/build --output-on-failure
```

### Biên dịch sang WebAssembly

```bash
# Linux/macOS
npm run build:cpp:wasm

# Windows
npm run build:cpp:wasm:win
```
Tệp đầu ra `opencut_core.js` sẽ được đưa vào `apps/web/public/wasm/`.
