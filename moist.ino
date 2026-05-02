#define BLYNK_TEMPLATE_ID "TMPL31VR1-9FR"
#define BLYNK_TEMPLATE_NAME "water management"
#define BLYNK_AUTH_TOKEN "l9GKSRCMhXe0uGHvjk6ErVUuvo-tjaxC"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>

// WiFi
char ssid[] = "Vivo Y58";
char pass[] = "gothilla";

// Twilio Credentials
const char* account_sid = "YOUR_SID";
const char* auth_token  = "YOUR_AUTH_TOKEN";

String from_number = "+1XXXXXXXXXX";   // Twilio number
String to_number   = "+91XXXXXXXXXX";  // Your number

bool alertSent = false;

// TFT Pins
#define TFT_CS   5
#define TFT_RST  4
#define TFT_DC   2

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

// Flow Sensor
#define FLOW_PIN 27
volatile int pulseCount = 0;

float flowRate = 0.0;
float totalLitres = 0.0;

unsigned long lastTime = 0;

// Interrupt
void IRAM_ATTR pulseCounter() {
  pulseCount++;
}

// 🔥 Twilio SMS Function
void sendSMS() {
  WiFiClientSecure client;
  client.setInsecure();

  HTTPClient http;

  String url = "https://api.twilio.com/2010-04-01/Accounts/" + String(account_sid) + "/Messages.json";

  String message = "🚿 Water reached 5 Litres!";
  String postData = "To=" + to_number +
                    "&From=" + from_number +
                    "&Body=" + message;

  http.begin(client, url);
  http.setAuthorization(account_sid, auth_token);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");

  int response = http.POST(postData);

  Serial.print("SMS Response: ");
  Serial.println(response);

  http.end();
}

void setup() {
  Serial.begin(115200);

  // SPI Fix
  SPI.begin(18, -1, 23, 5);
  SPI.setFrequency(4000000);

  // TFT Init
  tft.initR(INITR_BLACKTAB);
  tft.setRotation(1);
  tft.fillScreen(ST7735_BLACK);

  tft.setTextColor(ST7735_GREEN);
  tft.setTextSize(2);
  tft.setCursor(10, 10);
  tft.println("Water Monitor");

  // Flow Sensor
  pinMode(FLOW_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(FLOW_PIN), pulseCounter, FALLING);

  // WiFi + Blynk
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  lastTime = millis();
}

void loop() {
  Blynk.run();

  if (millis() - lastTime >= 1000) {

    detachInterrupt(FLOW_PIN);

    // Calculate Flow
    flowRate = pulseCount / 7.5;
    float litresPerSec = flowRate / 60.0;
    totalLitres += litresPerSec;

    Serial.print("Flow: ");
    Serial.print(flowRate);
    Serial.print(" L/min | Total: ");
    Serial.println(totalLitres);

    // Send to Blynk
    Blynk.virtualWrite(V0, flowRate);
    Blynk.virtualWrite(V1, totalLitres);

    // 🚨 SMS Trigger
    if (totalLitres >= 5 && !alertSent) {
      sendSMS();
      alertSent = true;
    }

    // Display
    tft.fillScreen(ST7735_BLACK);

    tft.setTextColor(ST7735_GREEN);
    tft.setTextSize(2);
    tft.setCursor(10, 10);
    tft.println("Water Monitor");

    tft.setTextColor(ST7735_WHITE);

    tft.setCursor(10, 50);
    tft.print("Flow: ");
    tft.print(flowRate);
    tft.println(" L/min");

    tft.setCursor(10, 90);
    tft.print("Total: ");
    tft.print(totalLitres);
    tft.println(" L");

    pulseCount = 0;
    lastTime = millis();

    attachInterrupt(digitalPinToInterrupt(FLOW_PIN), pulseCounter, FALLING);
  }
}