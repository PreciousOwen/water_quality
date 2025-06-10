const mongoose = require('mongoose');

const SensorDataSchema = new mongoose.Schema({
  turbidity: Number,
  ph: Number,
  ultrasonic: Number,
  waterStatus: {
    type: String,
    enum: ['sufficient', 'insufficient'],
    required: true
  },
  createdAt: {
    type: Date,
    default: Date.now
  }
});

module.exports = mongoose.model('SensorData', SensorDataSchema);
