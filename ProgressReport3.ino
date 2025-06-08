// Blynk Template info
#define BLYNK_TEMPLATE_ID "TMPL6lcz8tswR"
#define BLYNK_TEMPLATE_NAME "Environmental Control and Monitoring"
#define BLYNK_AUTH_TOKEN "dP_JAdLfrNK7u7dTH46nrUwsBxPAnDQy"

// Include necessary libraries
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>

// Wi-Fi credentials
char ssid[] = "C.A.";
char pass[] = "CeeCeeEy";

// -------------------- Pins --------------------
#define TEMP_DHT_PIN 15
#define HUMID_DHT_PIN 4
#define DHTTYPE DHT11

#define PIR_PIN 5
#define LDR_PIN 34
#define GAS_PIN 35
#define SOIL_PIN 32
#define BUZZER_PIN 14
#define LED_STRIP_PIN 27
#define VALVE_PIN 26
#define ATOMIZER_PIN 25
#define FAN_PIN 33
#define PUMP_PIN 13

// DHT sensor objects
DHT dhtTemp(TEMP_DHT_PIN, DHTTYPE);
DHT dhtHumid(HUMID_DHT_PIN, DHTTYPE);

BlynkTimer timer;


// -------------------- Override Flags --------------------
bool overrideBuzzer = false;
bool overrideLedStrip = false;
bool overrideValve = false;
bool overrideAtomizer = false;
bool overrideFan = false;
bool overridePump = false;

// -------------------- Sensor Reading &  Automation --------------------
void readSensors()

 {
  float temperature = dhtTemp.readTemperature();
  float humidity = dhtHumid.readHumidity();

  Serial.println("----- SENSOR READINGS -----");

  if (isnan(temperature)) {
    Serial.println("Temperature: ERROR");
  } else {
    Serial.print("Temperature: ");
    Serial.println(temperature);
    Blynk.virtualWrite(V0, temperature);
  }

  if (isnan(humidity)) {
    Serial.println("Humidity: ERROR");
  } else {
    Serial.print("Humidity: ");
    Serial.println(humidity);
    Blynk.virtualWrite(V1, humidity);
  }

  int ldrValue = analogRead(LDR_PIN);
  int lightBinary = (ldrValue < 2000) ? 1 : 0;
  Serial.print(" Light Status: ");
  Serial.println(lightBinary == 0 ? "DARK" : "LIGHT");
  Blynk.virtualWrite(V3, lightBinary);

  int gasRaw = analogRead(GAS_PIN);
  int gasLevel = map(gasRaw, 0, 4095, 0, 100);
  Serial.print("Gas: ");
  Serial.println(gasLevel);
  Blynk.virtualWrite(V4, gasLevel);

  int soilRaw = analogRead(SOIL_PIN);
  int soilMoisture = map(soilRaw, 4095, 0, 0, 100);
  Serial.print("Soil Moisture: ");
  Serial.println(soilMoisture);
  Blynk.virtualWrite(V5, soilMoisture);

  Serial.println("----------------------------");  



  // ---------- Automation Logic ----------

  // Motion Sensor - Buzzer
if (!overrideBuzzer) 
{
  digitalWrite(BUZZER_PIN, HIGH);
}
 else 
{
   digitalWrite(BUZZER_PIN, LOW);
}

// Light Sensor - LED Strip
bool isDark = (ldrValue < 2000);
if (!overrideLedStrip) 
{
      Serial.print(overrideLedStrip);
  // Override off → keep LED strip ON
  digitalWrite(LED_STRIP_PIN,HIGH);
} else 
{
  // Override on → automation controls LED strip
  digitalWrite(LED_STRIP_PIN, isDark ? HIGH : LOW);
}

// Gas Sensor - Valve and Fan
if (!overrideValve) {
  digitalWrite(VALVE_PIN, HIGH);
} else {
  if (gasLevel < 44) {
    digitalWrite(VALVE_PIN, LOW);
  } else {
    digitalWrite(VALVE_PIN, HIGH);
  }
}

// Humidity Sensor - Atomizer
if (!overrideAtomizer) {
  digitalWrite(ATOMIZER_PIN, HIGH);
} else {
  if (humidity < 80)
  //Purpose demonstration only. Value should be 40
   {
    digitalWrite(ATOMIZER_PIN, LOW);
  } else {
    digitalWrite(ATOMIZER_PIN, HIGH);
  }
}

// Temperature Sensor - Fan
if (!overrideFan)
 {
  digitalWrite(FAN_PIN, HIGH);
} else 
{
  if (temperature > 27)
    //Purpose demonstration only. Value should be 33
  {
    digitalWrite(FAN_PIN, LOW);
  } else 
  {
    digitalWrite(FAN_PIN, HIGH);
  }
}

// Soil Moisture Sensor - Pump
if (!overridePump) {
  digitalWrite(PUMP_PIN, HIGH);
} else {
  if (soilMoisture < 10) {
    digitalWrite(PUMP_PIN, LOW);
  } else {
    digitalWrite(PUMP_PIN, HIGH);
  }
}
}

// -------------------- Setup & Loop --------------------
void setup()
 {
  Serial.begin(115200);
  Serial.println("ESP32 is ready!");
  delay(10000);

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  timer.setInterval(1000L, readSensors);

  pinMode(PIR_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_STRIP_PIN, OUTPUT);
  pinMode(VALVE_PIN, OUTPUT);
  pinMode(FAN_PIN, OUTPUT);
  pinMode(ATOMIZER_PIN, OUTPUT);
  pinMode(PUMP_PIN, OUTPUT);

  digitalWrite(BUZZER_PIN, LOW);
  digitalWrite(LED_STRIP_PIN, LOW);
  digitalWrite(VALVE_PIN, LOW);
  digitalWrite(FAN_PIN, LOW);
  digitalWrite(ATOMIZER_PIN, LOW);
  digitalWrite(PUMP_PIN, LOW);


  dhtTemp.begin();
  dhtHumid.begin();

// Blynk.config(BLYNK_AUTH_TOKEN);
// WiFi.begin(ssid, pass);
// timer.setInterval(1000L, readSensors);


// // Wait for Wi-Fi connection with timeout
// int wifiTimeout = 10000; // 10 seconds
// unsigned long startAttemptTime = millis();

// while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < wifiTimeout) {
//   Serial.print(".");
//   delay(500);
// }

// if (WiFi.status() == WL_CONNECTED) {
//   Serial.println("\nWi-Fi Connected!");
// } else {
//   Serial.println("\nWi-Fi Connection Failed!");
// }

// Blynk.connect(10000); 
// // Try connecting to Blynk for up to 10s
//   Serial.println("Blynk Connected");

}
// -------------------- Manual Overrides --------------------
BLYNK_WRITE(V6) {
  overrideBuzzer = param.asInt();
  readSensors(); 
}

BLYNK_WRITE(V7) {
  overrideLedStrip = param.asInt();
  readSensors(); 
}

BLYNK_WRITE(V8) {
  overrideValve = param.asInt();
  readSensors(); 
}

BLYNK_WRITE(V9) {
  overrideAtomizer = param.asInt();
  readSensors(); 
}

BLYNK_WRITE(V10) {
  overrideFan = param.asInt();
  readSensors(); 
}

BLYNK_WRITE(V11) {
  overridePump = param.asInt();
  readSensors(); 
}


void loop() 
{
  Blynk.run();
  timer.run();
}
