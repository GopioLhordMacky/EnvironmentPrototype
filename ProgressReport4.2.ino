// Blynk Template info
#define BLYNK_TEMPLATE_ID "TMPL6wS6p8PD3"
#define BLYNK_TEMPLATE_NAME "Environment Control and Monitoring Prototype"
#define BLYNK_AUTH_TOKEN "-LYQZERkQ_9v-WgTXd7jvNNts_xTyw7u"

// Include necessary libraries
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>
#include <SPI.h>
#include <MFRC522.h>
#include <ESP32Servo.h>


// Wi-Fi credentials
char ssid[] = "C.A.";
char pass[] = "CeeCeeEy";

// -------------------- Pins --------------------
#define TEMP_DHT_PIN 15
#define HUMID_DHT_PIN 4
#define DHTTYPE DHT11

#define SS_PIN 21
#define RST_PIN 22
#define SERVO_PIN 17   // Choose a free PWM-capable pin

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
MFRC522 mfrc522(SS_PIN, RST_PIN);
Servo myServo;
bool servoOpen = false;  // State tracking (false = 0°, true = 180°)

// Add these global variables near the top of your code
unsigned long buzzerStartTime = 0;
bool buzzerActive = false;


// Example valid UID (you can change this later)
// If your tag UID is: 1773A085
byte validUID[4] = {0x17, 0x73, 0xA0, 0x85};


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
  Serial.print("Light Status: ");
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
  if (temperature > 33)
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
  if (soilMoisture < 70) {
    digitalWrite(PUMP_PIN, LOW);
  } else {
    digitalWrite(PUMP_PIN, HIGH);
  }
}

// ---------- RFID - Servo & Buzzer Logic ----------
static unsigned long lastScanTime = 0;
const unsigned long scanCooldown = 500;  // Minimum time between scans (ms)

digitalWrite(SS_PIN, LOW);  // Manually select RFID

if (millis() - lastScanTime >= scanCooldown) {
  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    lastScanTime = millis();

    Serial.print("Card UID: ");
    for (byte i = 0; i < mfrc522.uid.size; i++) {
      Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
      Serial.print(mfrc522.uid.uidByte[i], HEX);
    }
    Serial.println();

    bool isValid = true;
    for (byte i = 0; i < 4; i++) {
      if (mfrc522.uid.uidByte[i] != validUID[i]) {
        isValid = false;
        break;
      }
    }

    if (isValid) {
      servoOpen = !servoOpen;
      myServo.write(servoOpen ? 90 : 0);
      Serial.println(servoOpen ? "Servo: OPEN (90°)" : "Servo: CLOSE (0°)");
    } else {
      Serial.println("Invalid RFID Tag!");
      digitalWrite(BUZZER_PIN, LOW); // Buzzer ON
      buzzerStartTime = millis();    // Start buzzer timer
      buzzerActive = true;
    }

    mfrc522.PICC_HaltA();          // Halt PICC
    mfrc522.PCD_StopCrypto1();     // Stop encryption
  }
}

digitalWrite(SS_PIN, HIGH); // Deselect RFID


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

  digitalWrite(BUZZER_PIN, HIGH);
  digitalWrite(LED_STRIP_PIN, HIGH);
  digitalWrite(VALVE_PIN, HIGH);
  digitalWrite(FAN_PIN, HIGH);
  digitalWrite(ATOMIZER_PIN, HIGH);
  digitalWrite(PUMP_PIN, HIGH);

  SPI.begin();               // Start SPI bus
  mfrc522.PCD_Init();        // Init RC522
  myServo.attach(17);   // Attach servo
  myServo.write(0);            // Initialize to 0°

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

  if (buzzerActive && millis() - buzzerStartTime >= 3000) {
  digitalWrite(BUZZER_PIN, HIGH); // Turn OFF buzzer
  buzzerActive = false;
}

}
