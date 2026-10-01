# Occupancy-Aware Smart Classroom for Comfort and Energy Optimization (Project 09)

Mã nguồn đầy đủ cho dự án **Occupancy-Aware Smart Classroom**, thuộc chuỗi đề tài IoT môn học / Đồ án tốt nghiệp. Hệ thống tích hợp đầy đủ từ vi điều khiển ESP32, MQTT Broker HiveMQ, Backend Node.js lưu trữ cơ sở dữ liệu Neon PostgreSQL, đến Web Dashboard hiển thị thời gian thực trên Vercel.

---

## 🏗️ Cấu Trúc Tổng Quan Hệ Thống (Architecture)

```
[ESP32 Device]  <--- (MQTT 1883) --->  [HiveMQ Broker]  <--- (MQTT WSS) --->  [Vercel Frontend]
(PIR, Radar, DHT22,                       ^                                        |
 BH1750, INA219, Relay)                   | (MQTT Ingest)                          | (REST API)
                                 [Render Backend API]  <--- (SQL) --->  [Neon Postgres DB]
```

---

## 📂 Cấu Trúc Thư Mục Repository

```
Smart_Classroom_Project_09/
├── firmware_esp32/             # Mã nguồn C++ cho ESP32 (PlatformIO)
│   ├── platformio.ini          # Cấu hình thư viện & Board
│   └── src/
│       └── main.cpp            # Mã nguồn chính điều khiển cảm biến, Hysteresis, MQTT
├── backend_render/             # Backend Node.js Express API (Deploy Render.com)
│   ├── package.json
│   ├── server.js               # MQTT Subscriber & REST APIs cho Neon DB
│   └── .env.example            # Mẫu biến môi trường
├── frontend_vercel/            # Web Dashboard HTML/CSS/JS (Deploy Vercel.com)
│   └── index.html              # Giao diện điều khiển real-time & Biểu đồ Chart.js
└── database_neon/              # Cơ sở dữ liệu PostgreSQL (Neon.tech)
    └── schema.sql              # SQL Schema tạo bảng sensor_telemetry
```

---

## ⚡ 1. Sơ Đồ Chân Cắm Phần Cứng ESP32 (Hardware Pinout)

| Linh Kiện | Chân Linh Kiện | Chân ESP32 | Chức Năng |
| :--- | :--- | :--- | :--- |
| **PIR HC-SR501** | `OUT` | **GPIO 12** | Đọc hiện diện chuyển động hồng ngoại |
| **Radar RCWL-0516** | `OUT` | **GPIO 13** | Đọc hiện diện vi sóng vi mô |
| **DHT22** | `DATA` | **GPIO 14** | Đọc Nhiệt độ & Độ ẩm (Trở kéo 10kΩ lên 5V) |
| **BH1750** | `SDA` / `SCL` | **GPIO 21 / 22** | Đọc cường độ ánh sáng Lux (I2C) |
| **INA219** | `SDA` / `SCL` | **GPIO 21 / 22** | Đọc điện năng tiêu thụ thực tế (I2C) |
| **Relay Đèn** | `IN1` | **GPIO 25** | Đóng ngắt hệ thống chiếu sáng |
| **Relay Quạt** | `IN2` | **GPIO 26** | Đóng ngắt hệ thống quạt thông gió |

---

## 🚀 2. Hướng Dẫn Triển Khai (Deployment Guide)

### Bước 1: Khởi Tạo Neon PostgreSQL Database
1. Truy cập [Neon.tech](https://neon.tech), tạo cơ sở dữ liệu mới.
2. Mở **SQL Editor** và chạy toàn bộ câu lệnh trong file `database_neon/schema.sql`.
3. Sao chép chuỗi **Connection String** (`postgres://...`).

### Bước 2: Deploy Backend lên Render.com
1. Đẩy thư mục `backend_render` lên kho chứa GitHub.
2. Đăng nhập [Render.com](https://render.com) -> Chọn **New Web Service**.
3. Cấu hình:
   * **Build Command:** `npm install`
   * **Start Command:** `node server.js`
4. Thêm biến môi trường (Environment Variable):
   * `DATABASE_URL` = Chuỗi Connection String lấy từ Neon DB.

### Bước 3: Deploy Web Dashboard lên Vercel.com
1. Đăng nhập [Vercel.com](https://vercel.com) -> Tạo dự án mới từ thư mục `frontend_vercel`.
2. Mở file `index.html`, cập nhật biến `RENDER_API` thành URL Web Service Render của bạn.
3. Bấm **Deploy** để nhận đường dẫn trang Web chính thức.

### Bước 4: Nạp Code cho ESP32 bằng VS Code (PlatformIO)
1. Mở thư mục `firmware_esp32` trên **VS Code**.
2. Cập nhật thông tin `WIFI_SSID` và `WIFI_PASSWORD` trong file `src/main.cpp`.
3. Kết nối ESP32 qua cáp Micro-USB, bấm **Build** (✓) và **Upload** (➔) trên thanh công cụ PlatformIO.

---

## 👥 Tác Giả & Bản Quyền
* **Dự án:** Occupancy-Aware Smart Classroom for Comfort and Energy Optimization (Project 09)
* **Phiên bản:** 1.0.0 (Production Release)
