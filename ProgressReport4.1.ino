// Blynk Template info
#define BLYNK_TEMPLATE_ID "TMPL6spo8yn6d"
#define BLYNK_TEMPLATE_NAME "Environment Control and Monitor"
#define BLYNK_AUTH_TOKEN "xS7bnuD-HaeMqGp8_TcGPVPSCT8O9Sgw"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>
#include <SPI.h>
#include <MFRC522.h>
#include <ESP32Servo.h>

char ssid[] = "C.A.";
char pass[] = "CeeCeeEy";

#define TEMP_DHT_PIN 15
#define HUMID_DHT_PIN 4
#define DHTTYPE DHT11

#define SS_PIN 21
#define RST_PIN 22
#define SERVO_PIN 12

#define LDR_PIN 34
#define GAS_PIN 35
#define SOIL_PIN 32
#define BUZZER_PIN 14
#define LED_STRIP_PIN 27
#define VALVE_PIN 26
#define ATOMIZER_PIN 25
#define FAN_PIN 33
#define PUMP_PIN 13

DHT dhtTemp(TEMP_DHT_PIN, DHTTYPE);
DHT dhtHumid(HUMID_DHT_PIN, DHTTYPE);
BlynkTimer timer;
MFRC522 mfrc522(SS_PIN, RST_PIN);
Servo servo;

float temp = 0, humid = 0, gas = 0, soil = 0;
float temp_p1 = 0, temp_p2 = 0, temp_p3 = 0;
float humid_p1 = 0, humid_p2 = 0, humid_p3 = 0;
float gas_p1 = 0, gas_p2 = 0, gas_p3 = 0;
float soil_p1 = 0, soil_p2 = 0, soil_p3 = 0;

float dTemp = 0, dHumid = 0, dGas = 0, dSoil = 0;
float sumTemp = 0, sumHumid = 0, sumGas = 0, sumSoil = 0;
float emaTemp = 0, emaHumid = 0, emaGas = 0, emaSoil = 0;
float totalTimeElapsed = 0;

float dt = 1.0;
float alpha = 0.3;

bool overrideBuzzer = false;
bool overrideLedStrip = false;
bool overrideValve = false;
bool overrideAtomizer = false;
bool overrideFan = false;
bool overridePump = false;
bool servoOpen = false;

byte validUID[4] = {0xDE, 0xAD, 0xBE, 0xEF}; // Sample UID (change as needed)

void readSensors() {
  float temperature = dhtTemp.readTemperature();
  float humidity = dhtHumid.readHumidity();

  Serial.println("----- SENSOR READINGS -----");

  if (!isnan(temperature)) {
    Blynk.virtualWrite(V0, temperature);
  }

  if (!isnan(humidity)) {
    Blynk.virtualWrite(V1, humidity);
  }

  int ldrValue = analogRead(LDR_PIN);
  int lightBinary = (ldrValue < 2000) ? 1 : 0;
  Blynk.virtualWrite(V3, lightBinary);

  int gasRaw = analogRead(GAS_PIN);
  int gasLevel = map(gasRaw, 0, 4095, 0, 100);
  Blynk.virtualWrite(V4, gasLevel);

  int soilRaw = analogRead(SOIL_PIN);
  int soilMoisture = map(soilRaw, 3000, 1200, 0, 100);
  Blynk.virtualWrite(V5, soilMoisture);

  float gasPPM = map(gasRaw, 0, 4095, 0, 1000);
  float soilPercent = constrain(map(soilRaw, 3000, 1200, 0, 100), 0, 100);

  dTemp = (temperature - temp_p2) / (2 * dt);
  dHumid = (humidity - humid_p2) / (2 * dt);
  dGas = (gasRaw - gas_p2) / (2 * dt);
  dSoil = (soilRaw - soil_p2) / (2 * dt);

  sumTemp += (dt / 2.0) * (temperature + temp_p3);
  sumHumid += (dt / 2.0) * (humidity + humid_p3);
  sumGas += (dt / 2.0) * (gasPPM + gas_p3);
  sumSoil += (dt / 2.0) * (soilPercent + soil_p3);

  totalTimeElapsed += dt;

  float avgTemperature = sumTemp / totalTimeElapsed;
  float avgHumidity = sumHumid / totalTimeElapsed;
  float avgGasPPM = sumGas / totalTimeElapsed;
  float avgSoilMoisture = sumSoil / totalTimeElapsed;

  emaTemp = alpha * temperature + (1 - alpha) * emaTemp;
  emaHumid = alpha * humidity + (1 - alpha) * emaHumid;
  emaGas = alpha * gasRaw + (1 - alpha) * emaGas;
  emaSoil = alpha * soilRaw + (1 - alpha) * emaSoil;

  temp_p2 = temp_p1; temp_p1 = temperature; temp_p3 = temperature;
  humid_p2 = humid_p1; humid_p1 = humidity; humid_p3 = humidity;
  gas_p2 = gas_p1; gas_p1 = gasRaw; gas_p3 = gasPPM;
  soil_p2 = soil_p1; soil_p1 = soilRaw; soil_p3 = soilPercent;

  
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


  if (!overrideLedStrip) digitalWrite(LED_STRIP_PIN, HIGH);
  else digitalWrite(LED_STRIP_PIN, (ldrValue < 2000) ? HIGH : LOW);

  if (!overrideValve) digitalWrite(VALVE_PIN, HIGH);
  else digitalWrite(VALVE_PIN, (gasLevel < 44) ? LOW : HIGH);

  if (!overrideAtomizer) digitalWrite(ATOMIZER_PIN, HIGH);
  else digitalWrite(ATOMIZER_PIN, (humidity < 80) ? LOW : HIGH);

  if (!overrideFan) digitalWrite(FAN_PIN, HIGH);
  else digitalWrite(FAN_PIN, (temperature > 33) ? LOW : HIGH);

  if (!overridePump) digitalWrite(PUMP_PIN, HIGH);
  else digitalWrite(PUMP_PIN, (soilMoisture < 70) ? LOW : HIGH);

  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    bool isValid = true;
    for (byte i = 0; i < 4; i++) {
      if (mfrc522.uid.uidByte[i] != validUID[i]) {
        isValid = false;
        break;
      }
    }
    if (isValid) {
      servoOpen = !servoOpen;
      servo.write(servoOpen ? 180 : 0);
    } else {
      digitalWrite(BUZZER_PIN, LOW);
      delay(3000);
      digitalWrite(BUZZER_PIN, HIGH);
    }
    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();
  }
}

void setup()
 {
  Serial.begin(115200);
  delay(10000);
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  timer.setInterval(1000L, readSensors);

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

  SPI.begin();
  mfrc522.PCD_Init();
  servo.attach(SERVO_PIN);
  servo.write(0);

  dhtTemp.begin();
  dhtHumid.begin();
}


BLYNK_WRITE(V6) { overrideBuzzer = param.asInt(); readSensors(); }
BLYNK_WRITE(V7) { overrideLedStrip = param.asInt(); readSensors(); }
BLYNK_WRITE(V8) { overrideValve = param.asInt(); readSensors(); }
BLYNK_WRITE(V9) { overrideAtomizer = param.asInt(); readSensors(); }
BLYNK_WRITE(V10) { overrideFan = param.asInt(); readSensors(); }
BLYNK_WRITE(V11) { overridePump = param.asInt(); readSensors(); }

void loop() 
{
  Blynk.run();
  timer.run();
}
