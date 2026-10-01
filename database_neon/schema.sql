-- Schema cho Neon PostgreSQL Database
-- Dự án: Occupancy-Aware Smart Classroom (Project 09)

CREATE TABLE IF NOT EXISTS sensor_telemetry (
    id SERIAL PRIMARY KEY,
    device_id VARCHAR(50) DEFAULT 'ESP32_Room9',
    temperature REAL NOT NULL,
    humidity REAL NOT NULL,
    lux REAL NOT NULL,
    occupied BOOLEAN NOT NULL,
    auto_mode BOOLEAN NOT NULL,
    light_state BOOLEAN NOT NULL,
    fan_state BOOLEAN NOT NULL,
    energy_kwh DOUBLE PRECISION NOT NULL,
    created_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP
);

-- Index tối ưu hóa truy vấn dữ liệu lịch sử cho biểu đồ Dashboard
CREATE INDEX IF NOT EXISTS idx_telemetry_created_at ON sensor_telemetry(created_at DESC);
