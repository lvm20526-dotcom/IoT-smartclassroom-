const express = require('express');
const cors = require('cors');
const mqtt = require('mqtt');
const { Pool } = require('pg');
require('dotenv').config();

const app = express();
app.use(cors());
app.use(express.json());

const pool = new Pool({
    connectionString: process.env.DATABASE_URL,
    ssl: { rejectUnauthorized: false }
});

const MQTT_BROKER = 'mqtt://broker.hivemq.com:1883';
const TOPIC_TELEMETRY = 'classroom/room9/telemetry';
const TOPIC_COMMAND = 'classroom/room9/command';

const mqttClient = mqtt.connect(MQTT_BROKER);

mqttClient.on('connect', () => {
    console.log('✅ Connected to HiveMQ Broker');
    mqttClient.subscribe(TOPIC_TELEMETRY);
});

mqttClient.on('message', async (topic, message) => {
    if (topic === TOPIC_TELEMETRY) {
        try {
            const data = JSON.parse(message.toString());
            const query = `
                INSERT INTO sensor_telemetry 
                (temperature, humidity, lux, occupied, auto_mode, light_state, fan_state, energy_kwh)
                VALUES ($1, $2, $3, $4, $5, $6, $7, $8)
            `;
            const values = [
                data.temp || 0,
                data.hum || 0,
                data.lux || 0,
                data.occupied || false,
                data.autoMode !== undefined ? data.autoMode : true,
                data.light || false,
                data.fan || false,
                data.energyKwh || 0
            ];
            await pool.query(query, values);
            console.log('💾 Data saved to Neon DB');
        } catch (err) {
            console.error('❌ DB Error:', err.message);
        }
    }
});

app.get('/api/telemetry/latest', async (req, res) => {
    try {
        const result = await pool.query('SELECT * FROM sensor_telemetry ORDER BY created_at DESC LIMIT 1');
        res.json(result.rows[0] || {});
    } catch (err) {
        res.status(500).json({ error: err.message });
    }
});

app.get('/api/telemetry/history', async (req, res) => {
    try {
        const result = await pool.query('SELECT * FROM sensor_telemetry ORDER BY created_at DESC LIMIT 50');
        res.json(result.rows.reverse());
    } catch (err) {
        res.status(500).json({ error: err.message });
    }
});

app.post('/api/control', (req, res) => {
    const { mode, light, fan } = req.body;
    const payload = JSON.stringify({ mode, light, fan });
    mqttClient.publish(TOPIC_COMMAND, payload);
    res.json({ status: 'Command sent', payload: req.body });
});

const PORT = process.env.PORT || 3000;
app.listen(PORT, () => {
    console.log(`🚀 Server running on port ${PORT}`);
});
