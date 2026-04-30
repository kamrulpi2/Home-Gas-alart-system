/*************************************************************
   ESP32-C3 Gas Leak Detector with DHT11 + Firebase
   Board: ESP32-C3
   Sensors: MQ-5 + DHT11
   Actuator: Relay + Buzzer
   Cloud: Firebase Realtime Database
*************************************************************/

#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <DHT.h>

// ====================== WiFi Credentials ======================
#define WIFI_SSID     "bubt"
#define WIFI_PASSWORD "bubt1234"

// ====================== Firebase Config ======================
// Replace with your own Firebase project details
#define API_KEY       "AIzaSyAxHjFOwi-_8MYHi0mwsHid6VBSSLHghNo"
#define DATABASE_URL  "https://dht22iot-5b3fd-default-rtdb.firebaseio.com"

// ====================== Pin Definitions ======================
#define MQ5_PIN       4
#define RELAY_PIN     2
#define BUZZER_PIN    7
#define DHT_PIN       3
#define DHTTYPE       DHT11

// ====================== Objects ======================
DHT dht(DHT_PIN, DHTTYPE);

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// ====================== Variables ======================
int sensorMin = 300;
int sensorMax = 4047;
int alertLevel = 30;

unsigned long lastSendTime = 0;
const long sendInterval = 2000;   // Send data every 2 seconds

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n=== ESP32-C3 Gas + DHT11 Firebase Monitor ===");

  // Pin Setup
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  
  digitalWrite(RELAY_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);

  dht.begin();

  // Connect to WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi");
  
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  
  Serial.println("\n✅ WiFi Connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Firebase Configuration
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  // Optional: Enable WiFi reconnection
  Firebase.reconnectWiFi(true);

  // Start Firebase
  Firebase.begin(&config, &auth);

  Serial.println("Firebase Started Successfully!");
  Serial.println("System Ready...\n");
}

void loop() {
  unsigned long currentMillis = millis();

  // Read Sensors
  int rawValue = analogRead(MQ5_PIN);
  int gasPercent = map(rawValue, sensorMin, sensorMax, 0, 100);
  gasPercent = constrain(gasPercent, 0, 100);

  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  // Check for invalid DHT reading
  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("❌ Failed to read from DHT sensor!");
    temperature = 0;
    humidity = 0;
  }

  bool alertStatus = (gasPercent >= alertLevel);

  // Control Relay and Buzzer
  if (alertStatus) {
    digitalWrite(RELAY_PIN, HIGH);
    digitalWrite(BUZZER_PIN, HIGH);
    delay(200);
    digitalWrite(BUZZER_PIN, LOW);
  } else {
    digitalWrite(RELAY_PIN, LOW);
    digitalWrite(BUZZER_PIN, LOW);
  }

  // Print to Serial Monitor
  Serial.printf("Gas: %3d%% | Temp: %.1f°C | Humidity: %.1f%% | Alert: %s\n",
                gasPercent, temperature, humidity, alertStatus ? "YES" : "NO");

  // Send data to Firebase every 2 seconds
  if (currentMillis - lastSendTime >= sendInterval) {
    lastSendTime = currentMillis;

    if (Firebase.RTDB.setInt(&fbdo, "/sensor/gas", gasPercent)) {
      Firebase.RTDB.setFloat(&fbdo, "/sensor/temperature", temperature);
      Firebase.RTDB.setFloat(&fbdo, "/sensor/humidity", humidity);
      Firebase.RTDB.setBool(&fbdo, "/sensor/alert", alertStatus);
      
      Serial.println("✅ Data sent to Firebase successfully");
    } else {
      Serial.print("❌ Firebase Error: ");
      Serial.println(fbdo.errorReason());
    }
  }

  delay(1000);   // Main loop delay (adjust as needed)
}
