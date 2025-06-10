# Water Quality Detection

This Node.js project provides an API to receive water quality sensor data (turbidity, pH, ultrasonic distance), stores it in MongoDB, and allows management to view data trends via a web interface.

## Features
- POST `/api/sensor`: Submit sensor data as JSON
- GET `/management`: View all sensor data and water status
- Data stored in MongoDB with status (sufficient/insufficient) based on ultrasonic value

## Setup
1. Install dependencies: `npm install`
2. Start MongoDB locally (default URI: mongodb://localhost:27017/water_quality)
3. Start the server: `node index.js`
4. POST sensor data to `/api/sensor` as JSON
5. View management dashboard at `http://localhost:3000/management`

## Example POST Body
```json
{
  "turbidity": 5.2,
  "ph": 7.1,
  "ultrasonic": 180
}
```

## License
MIT
