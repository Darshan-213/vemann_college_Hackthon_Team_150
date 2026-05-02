#define BLYNK_TEMPLATE_ID "TMPL31VR1-9FR"
#define BLYNK_TEMPLATE_NAME "water management"
#define BLYNK_AUTH_TOKEN "l9GKSRCMhXe0uGHvjk6ErVUuvo-tjaxC"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>

// WiFi
char ssid[] = "Vivo Y58";
char pass[] = "gothilla";

// Pins
#define SOIL_PIN 35
#define TDS_PIN 34
#define TRIG_PIN 5
#define ECHO_PIN 18

#define TANK_HEIGHT 100  

BlynkTimer timer;

// ---------- Soil ----------
int readSoil() {
  int value = analogRead(SOIL_PIN);
  int moisture = map(value, 4095, 0, 0, 100);
  moisture = constrain(moisture, 0, 100);
  return moisture;
}

// ---------- Ultrasonic ----------
float readTankLevel() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 30000); // timeout added

  if (duration == 0) return -1; // no reading

  float distance = duration * 0.034 / 2;
  float level = ((TANK_HEIGHT - distance) / TANK_HEIGHT) * 100;

  return constrain(level, 0, 100);
}

// ---------- TDS ----------
float readTDS() {
  int raw = analogRead(TDS_PIN);
  float voltage = raw * (3.3 / 4095.0);

  float tds = (133.42 * voltage * voltage * voltage 
              - 255.86 * voltage * voltage 
              + 857.39 * voltage) * 0.5;

  return tds;
}

// ---------- Send ----------
void sendData() {
  int soil = readSoil();
  float tank = readTankLevel();
  float tds = readTDS();

  Blynk.virtualWrite(V5, soil);
  Blynk.virtualWrite(V7, tank);
  Blynk.virtualWrite(V8, tds);

  Serial.print("Soil: "); Serial.print(soil);
  Serial.print(" % | Tank: "); Serial.print(tank);
  Serial.print(" % | TDS: "); Serial.println(tds);
}

void setup() {
  Serial.begin(115200);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  timer.setInterval(2000L, sendData);
}

void loop() {
  Blynk.run();
  timer.run();
}