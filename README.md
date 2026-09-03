# Catchim - Trình Chỉnh Sửa Video Trực Tuyến

**Catchim** là trình chỉnh sửa video mã nguồn mở hoạt động hoàn toàn trên trình duyệt web. Ứng dụng tập trung vào tính riêng tư (toàn bộ video và dữ liệu được xử lý trực tiếp trên máy của bạn), tốc độ xử lý mượt mà và giao diện trực quan, dễ sử dụng.

---

## 🏗️ Kiến Trúc Hệ Thống

Dự án được xây dựng dựa trên sự kết hợp giữa hiệu năng cao của C++ và tính linh hoạt của giao diện Web hiện đại:

* **Core Engine (`native/opencut_core/`) — Viết bằng C++20**:
  Toàn bộ logic tính toán nền tảng, thuật toán xử lý video/audio được chuyển sang C++20 độc lập với giao diện, sau đó biên dịch thành WebAssembly (thông qua Emscripten) để chạy trực tiếp trên trình duyệt:
  - **`opencut::time`**: Quản lý thời gian chính xác cao với hệ quy chiếu 120.000 ticks/giây, hỗ trợ toàn bộ các chuẩn khung hình (fps) và định dạng Timecode SMPTE.
  - **`opencut::timeline`**: Quản lý đa track, thêm, sửa, xóa, cắt ngắn, chia tách clip, tính toán va chạm và thuật toán hít nam châm (snapping).
  - **`opencut::compositor`**: Biến đổi hình học (ma trận 3x3), 17 chế độ hòa trộn lớp màu (blend modes) và kết xuất alpha compositing.
  - **`opencut::masks`**: Mặt nạ hình học sử dụng giải tích Signed Distance Field (SDF) và làm mờ biên độ mềm (feathering).
  - **`opencut::effects`**: Bộ chỉnh màu sắc (nhiệt độ màu, phơi sáng, tương phản, độ bão hòa, vignette, gamma) và nhân làm mờ Gaussian.
  - **`opencut::audio`**: Đường bao âm lượng keyframe, làm mượt âm lượng vào/ra (fade-in / fade-out) và thuật toán tạo biểu đồ sóng âm (waveform).
  - **`opencut::animation`**: Giải phương trình đường cong Bezier bậc 3, tìm kiếm nhị phân và nội suy keyframe mượt mà 60-120fps (Hold, Linear, Bezier) cho 8 kênh thuộc tính Transform 2D.
  - **`opencut::speed`**: Tích phân giải tích đường cong tốc độ (Speed Ramping) và ánh xạ thời gian timeline sang thời gian video gốc.
  - **`opencut::scene`**: Cây đồ họa Scene Graph & sinh danh sách vẽ (Display List) theo tiêu chuẩn Figma, tự động tính ma trận thế giới, cắt cúp ngoài tầm nhìn (view-frustum culling) và sắp xếp thứ tự hiển thị Z-index loại bỏ rác bộ nhớ JavaScript.

* **Giao Diện Người Dùng (`apps/web/`) — Next.js, React & TypeScript**:
  Đảm nhận phần hiển thị giao diện đồ họa, điều khiển tương tác chuột, phím tắt, timeline trực quan, các thanh công cụ và bảng cài đặt thuộc tính. Frontend giao tiếp trực tiếp với Core Engine C++ thông qua cầu nối WebAssembly tại `apps/web/src/native/`.

---

## 📁 Cấu Trúc Thư Mục

```text
catchim/
├── native/opencut_core/     # Mã nguồn C++20 của Core Engine
│   ├── include/opencut/     # Header files (time, timeline, compositor, masks, effects, audio, c_api)
│   ├── src/                 # Triển khai thuật toán C++
│   ├── tests/               # Bộ kiểm thử Unit Test cho C++ (CTest)
│   └── CMakeLists.txt       # Cấu hình biên dịch Native & WebAssembly
├── apps/web/                # Giao diện người dùng Web (Next.js 16, React 19, TailwindCSS)
│   ├── src/                 # Mã nguồn React components, stores, hooks
│   │   ├── native/          # Cầu nối gọi C++ WebAssembly
│   │   └── ...
│   └── public/wasm/         # Tệp nhúng WebAssembly đã biên dịch (opencut_core.js)
├── docs/                    # Tài liệu kỹ thuật
└── package.json             # Quản lý script và phụ thuộc dự án
```

---

## 🚀 Hướng Dẫn Cài Đặt & Khởi Chạy

### 1. Yêu cầu môi trường

- **Node.js** (phiên bản 20+ trở lên) hoặc **Bun**
- Trình duyệt web hiện đại có hỗ trợ WebAssembly (Chrome, Edge, Firefox, Safari)

*(Tùy chọn nếu muốn tự build lại mã nguồn C++):*
- **CMake** (3.24+)
- **Trình biên dịch C++20** (MSVC 2022, GCC 12+ hoặc Clang 15+)
- **Emscripten SDK** (để biên dịch C++ sang WebAssembly)

---

### 2. Cài đặt và chạy giao diện Web

1. **Clone repository về máy**:
   ```bash
   git clone https://github.com/0kiyo/catchim.git
   cd catchim
   ```

2. **Cài đặt các gói phụ thuộc**:
   ```bash
   npm install
   # hoặc dùng bun:
   bun install
   ```

3. **Tạo tệp cấu hình môi trường**:
   ```bash
   # Trên Linux/macOS
   cp apps/web/.env.example apps/web/.env.local

   # Trên Windows PowerShell
   Copy-Item apps/web/.env.example apps/web/.env.local
   ```

4. **Khởi chạy máy chủ phát triển (Development Server)**:
   ```bash
   npm run dev:web
   # hoặc:
   bun dev:web
   ```
   Mở trình duyệt và truy cập: `http://localhost:3000`

---

### 3. Đóng gói bản phát hành (Production Build)

Để đóng gói và tối ưu toàn bộ ứng dụng web:
```bash
npm run build:web
```

---

### 4. Biên dịch mã nguồn C++ Core (Dành cho nhà phát triển Engine)

Nếu bạn thực hiện thay đổi mã nguồn C++ trong `native/opencut_core/`:

* **Chạy bộ kiểm thử C++ Native**:
  ```bash
  cmake -S native/opencut_core -B native/opencut_core/build -DCMAKE_BUILD_TYPE=Release
  cmake --build native/opencut_core/build --config Release
  ctest --test-dir native/opencut_core/build --output-on-failure
  ```

* **Biên dịch sang WebAssembly**:
  ```bash
  # Trên Linux/macOS (khi đã active emsdk):
  npm run build:cpp:wasm

  # Trên Windows:
  npm run build:cpp:wasm:win
  ```
  File WebAssembly sau khi build sẽ tự động được đưa vào `apps/web/public/wasm/opencut_core.js`.

---

## ✨ Tính Năng Nổi Bật

- **Riêng tư và bảo mật**: Không tải video của người dùng lên server bên ngoài, dựng hình trực tiếp trên máy cục bộ.
- **Timeline chuyên nghiệp**: Hỗ trợ đa track video, audio, text, sticker với độ trễ cực thấp.
- **Hệ thống hít nam châm thông minh (Magnetic Snapping)**: Căn chỉnh mép clip, đường thời gian chính xác tới từng khung hình.
- **Bộ lọc và hiệu ứng đa dạng**: Chỉnh màu, làm mờ viền, tạo hình mặt nạ và hòa trộn 17 chế độ màu khác nhau.
- **Tương thích cao**: Hoạt động mượt mà trên máy tính để bàn và giao diện cảm ứng di động.

---

## 📄 Giấy Phép

Dự án được phát hành theo giấy phép mã nguồn mở [MIT License](LICENSE).
