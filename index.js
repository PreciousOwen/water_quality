const express = require('express');
const bodyParser = require('body-parser');
const { Pool } = require('pg');

const app = express();
app.set('view engine', 'ejs');
app.use(bodyParser.json());

// PostgreSQL connection setup
const pool = new Pool({
  user: 'postgres',
  host: 'localhost',
  database: 'water_quality',
  password: 'OWEN',
  port: 5433,
});

async function ensureDatabaseExists() {
  const { Pool } = require('pg');
  const tempPool = new Pool({
    user: 'postgres',
    host: 'localhost',
    database: 'postgres',
    password: 'OWEN',
    port: 5433,
  });
  try {
    const res = await tempPool.query("SELECT 1 FROM pg_database WHERE datname='water_quality'");
    if (res.rowCount === 0) {
      console.log('[DEBUG] Database water_quality does not exist. Creating...');
      await tempPool.query('CREATE DATABASE water_quality');
      console.log('[DEBUG] Database water_quality created.');
    } else {
      console.log('[DEBUG] Database water_quality already exists.');
    }
  } catch (err) {
    console.error('[DEBUG] Error checking/creating database:', err);
  } finally {
    await tempPool.end();
  }
}

(async () => {
  await ensureDatabaseExists();
  console.log('[DEBUG] Connecting to PostgreSQL...');
  pool.connect((err, client, release) => {
    if (err) {
      return console.error('[DEBUG] PostgreSQL connection error:', err.stack);
    }
    console.log('[DEBUG] PostgreSQL connection established');
    // Create table if not exists
    client.query(`CREATE TABLE IF NOT EXISTS sensor_data (
      id SERIAL PRIMARY KEY,
      turbidity REAL,
      ph REAL,
      ultrasonic REAL,
      water_status VARCHAR(20),
      created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
    )`, (err) => {
      release();
      if (err) {
        return console.error('[DEBUG] Error creating table:', err.stack);
      }
      console.log('[DEBUG] Table sensor_data is ready');
    });
  });
})();

// POST endpoint for sensor data
app.post('/api/sensor', async (req, res) => {
  console.log('[DEBUG] Received POST /api/sensor with body:', req.body);
  const { turbidity, ph, ultrasonic } = req.body;
  if (typeof turbidity !== 'number' || typeof ph !== 'number' || typeof ultrasonic !== 'number') {
    console.warn('[DEBUG] Invalid data format:', req.body);
    return res.status(400).json({ error: 'Invalid data format' });
  }
  const waterStatus = ultrasonic < 200 ? 'insufficient' : 'sufficient';
  console.log(`[DEBUG] Calculated waterStatus: ${waterStatus}`);
  try {
    const result = await pool.query(
      'INSERT INTO sensor_data (turbidity, ph, ultrasonic, water_status) VALUES ($1, $2, $3, $4) RETURNING *',
      [turbidity, ph, ultrasonic, waterStatus]
    );
    console.log('[DEBUG] Data saved to PostgreSQL:', result.rows[0]);
    res.json({ message: 'Data saved', data: result.rows[0] });
  } catch (err) {
    console.error('[DEBUG] Error saving data:', err);
    res.status(500).json({ error: 'Database error' });
  }
});

// View data trends
app.get('/management', async (req, res) => {
  console.log('[DEBUG] GET /management called');
  try {
    const result = await pool.query('SELECT * FROM sensor_data ORDER BY created_at DESC');
    console.log(`[DEBUG] Retrieved ${result.rows.length} records for management view`);
    res.render('management', { allData: result.rows });
  } catch (err) {
    console.error('[DEBUG] Error retrieving data for management:', err);
    res.status(500).send('Error loading management view');
  }
});

const PORT = process.env.PORT || 3000;
app.listen(PORT, () => console.log(`[DEBUG] Server running on port ${PORT}`));
