// Arduino code for water quality sensor system
// This code reads ultrasonic, turbidity, and pH sensors, displays data on LCD, controls a pump, and sends data to a Node.js backend via ESP8266.
// Update WiFi credentials and backend IP as needed.

#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>

// LCD setup
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Sensor & motor pins
const int trigPin = 10;
const int echoPin = 9;
const int turbidityPin = A0;
const int phPin = A1;
const int motorPin = 7;

// ESP8266 Serial
SoftwareSerial espSerial(5, 6); // Rx, Tx (connected to ESP8266 Tx, Rx)

void setup() {
  Serial.begin(9600);
  espSerial.begin(9600);

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(motorPin, OUTPUT);

  lcd.begin();
  lcd.backlight();
  lcd.clear();
  lcd.print("System Init...");
  delay(2000);
  Serial.println("[INIT] Arduino started.");

  connectToWiFi();
}

void loop() {
  float distance = getDistance();
  float turbidity = getTurbidity();
  float ph = getPH();

  displayToLCD(distance, turbidity, ph);
  controlPump(distance);
  sendDataToServer(distance, turbidity, ph);

  delay(5000);
}

// ----- Sensor Functions -----

float getDistance() {
  digitalWrite(trigPin, LOW); delayMicroseconds(2);
  digitalWrite(trigPin, HIGH); delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  long duration = pulseIn(echoPin, HIGH);
  float distance = duration * 0.034 / 2;
  Serial.print("[SENSOR] Distance: "); Serial.println(distance);
  return distance;
}

float getTurbidity() {
  int raw = analogRead(turbidityPin);
  float voltage = raw * (5.0 / 1023.0);
  Serial.print("[SENSOR] Turbidity: "); Serial.println(voltage);
  return voltage;
}

float getPH() {
  int raw = analogRead(phPin);
  float ph = (raw * (5.0 / 1023.0)) * 3.5;
  Serial.print("[SENSOR] pH: "); Serial.println(ph);
  return ph;
}

// ----- LCD and Motor -----

void displayToLCD(float dist, float turb, float ph) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("D:");
  lcd.print(dist, 0);
  lcd.print(" T:");
  lcd.print(turb, 1);

  lcd.setCursor(0, 1);
  lcd.print("PH:");
  lcd.print(ph, 1);
}

void controlPump(float distance) {
  if (distance >= 500) {
    digitalWrite(motorPin, HIGH);
    Serial.println("[MOTOR] Water low → Pump ON");
  } else if (distance <= 100) {
    digitalWrite(motorPin, LOW);
    Serial.println("[MOTOR] Water high → Pump OFF");
  } else {
    Serial.println("[MOTOR] Water level stable.");
  }
}

// ----- WiFi & Networking -----

void connectToWiFi() {
  Serial.println("[WIFI] Connecting to WiFi...");

  sendAT("AT", 1000);
  sendAT("AT+CWMODE=1", 1000);
  sendAT("AT+CWJAP=\"YOUR_SSID\",\"YOUR_PASSWORD\"", 6000);

  Serial.println("[WIFI] Should be connected now.");
}

void sendDataToServer(float dist, float turb, float ph) {
  // The backend expects: turbidity, ph, ultrasonic
  String json = "{";
  json += "turbidity:" + String(turb, 2) + ",";
  json += "ph:" + String(ph, 2) + ",";
  json += "ultrasonic:" + String(dist, 1);
  json += "}";

  String cmd = "AT+CIPSTART=\"TCP\",\"192.168.1.142\",3000"; // Update IP to backend server
  sendAT(cmd, 2000);

  int length = json.length() + 46;
  cmd = "AT+CIPSEND=" + String(length);
  sendAT(cmd, 1000);

  String httpRequest =
    "POST /api/sensor HTTP/1.1\r\n"
    "Host: 192.168.1.142\r\n"
    "Content-Type: application/json\r\n"
    "Content-Length: " + String(json.length()) + "\r\n\r\n" +
    json;

  espSerial.print(httpRequest);
  Serial.println("[HTTP] Sent data:\n" + httpRequest);
  delay(2000);
  sendAT("AT+CIPCLOSE", 1000);
}

void sendAT(String command, unsigned long timeout) {
  espSerial.println(command);
  Serial.println("[AT] Sent: " + command);

  unsigned long time = millis();
  while (millis() - time < timeout) {
    while (espSerial.available()) {
      String response = espSerial.readStringUntil('\n');
      Serial.println("[ESP] " + response);
    }
  }
}
