# Cắt Chim

Cắt Chim là ứng dụng chỉnh sửa video chạy trên web, được xây dựng từ mã nguồn OpenCut. Mục tiêu của dự án là cung cấp một trình chỉnh sửa video đơn giản, có thể chạy trên máy cá nhân, hỗ trợ thao tác với timeline, preview, phụ đề, sticker, âm thanh, hiệu ứng và xuất bản nội dung.

## Cấu Trúc Dự Án

```text
catchim/
├── apps/web/           # Ứng dụng web chính
├── native/opencut_core/ # Core engine xử lý tính toán C++ (WASM)
├── eslint/             # Rule ESLint riêng của dự án
├── docker-compose.yml  # Chạy database, Redis và web bằng Docker
├── package.json        # Script cấp root
├── bun.lock            # Lockfile dùng cho Bun
├── biome.json          # Cấu hình formatter/linter phụ trợ
├── eslint.config.mjs   # Cấu hình ESLint
├── turbo.json          # Cấu hình Turbo
└── README.md
```

Những phần phụ như cấu hình GitHub mẫu, desktop app thử nghiệm, ghi chú phát triển cũ, source Rust/native để build WASM thủ công và artifact build đã được lược bỏ để repo gọn hơn.

## Tính Năng Chính

- Chỉnh sửa video bằng timeline.
- Thêm và quản lý media trong dự án.
- Preview nội dung trực tiếp trong trình duyệt.
- Quản lý project trên web.
- Hỗ trợ text, phụ đề, sticker, hiệu ứng, mask, speed và audio.
- Có service worker và cache nội bộ để tối ưu trải nghiệm.

## Hướng Dẫn Cài Đặt & Khởi Chạy

### 1. Cài đặt dependencies

```bash
npm install
# hoặc dùng bun
bun install
```

### 2. Khởi chạy ứng dụng

```bash
npm run dev
# hoặc
bun run dev
```

Mở trình duyệt và truy cập: `http://localhost:3000`

### 3. Build sản phẩm

```bash
npm run build
```
