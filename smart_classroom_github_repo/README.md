# Occupancy-Aware Smart Classroom (Project 09)

Hệ thống quản lý phòng học thông minh tự động tối ưu hóa điện năng và đảm bảo sự thoải mái dựa trên công nghệ hiện diện kép (PIR + Radar) và cảm biến môi trường.

## 🏗 Kiến Trúc Hệ Thống (End-to-End)
- **Vi điều khiển:** ESP32 DevKit V1 (Arduino / PlatformIO)
- **Cảm biến:** PIR HC-SR501, Radar RCWL-0516, DHT22, BH1750, INA219, Relay 2 Kênh
- **MQTT Broker:** HiveMQ Cloud / HiveMQ Public (`broker.hivemq.com`)
- **Backend Service:** Node.js + Express (Deploy trên Render.com)
- **Database:** PostgreSQL (Deploy trên Neon.tech)
- **Frontend Web Dashboard:** HTML5 / Bootstrap 5 / Chart.js / MQTT WSS (Deploy trên Vercel.com)

---

## 🚀 Hướng Dẫn Cài Đặt & Triển Khai

### 1. Database (Neon.tech)
- Đăng nhập [Neon.tech](https://neon.tech), tạo database PostgreSQL mới.
- Mở **SQL Editor**, dán toàn bộ nội dung file `database_neon/schema.sql` và bấm **Run**.

### 2. Backend Service (Render.com)
- Đẩy thư mục `backend_render` lên GitHub.
- Đăng nhập [Render.com](https://render.com), chọn **New Web Service**.
- Trỏ tới Repository backend.
- Thêm biến môi trường (Environment Variable): `DATABASE_URL` = Chuỗi kết nối Postgres từ Neon.

### 3. Frontend Web Dashboard (Vercel.com)
- Đẩy thư mục `frontend_vercel` lên GitHub.
- Mở file `frontend_vercel/index.html`, thay hằng số `RENDER_API_URL` bằng URL app Render của bạn.
- Vào [Vercel.com](https://vercel.com), import Repository frontend và bấm **Deploy**.

### 4. Firmware ESP32 (VS Code / PlatformIO)
- Mở thư mục `firmware_esp32` bằng VS Code (đã cài extension PlatformIO).
- Cập nhật SSID và Password Wi-Fi trong `src/main.cpp`.
- Nhấn **Upload** (`Ctrl + Alt + U`) để nạp code xuống ESP32.
