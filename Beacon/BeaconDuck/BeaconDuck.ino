/**
 * @file BeaconDuck.ino
 * @brief A MamaDuck that sends the message "QUACK QUACK QUACK QUACK" repeatedly.
 */

std::string deviceId("QUACKER1"); // DuckID - NEEDS to be 8 characters
const int INTERVAL_MS = 30000; // for sending the counter message

#include <string>
#include <arduino-timer.h>
#include <CDP.h>
#include "FastLED.h"

// Setup for W2812 (LED)
#define LED_TYPE WS2812
#define DATA_PIN 4
#define NUM_LEDS 1
#define COLOR_ORDER GRB
#define BRIGHTNESS  128
#include <pixeltypes.h>
CRGB leds[NUM_LEDS];

#ifdef SERIAL_PORT_USBVIRTUAL
#define Serial SERIAL_PORT_USBVIRTUAL
#endif

//GPS
#include <TinyGPS++.h>
TinyGPSPlus tgps;
HardwareSerial GPS(1);

bool sendData(std::vector<byte> message, topics value);
bool runSensor(void *);
bool runGPS(void *);
String getGPSData();
static void smartDelay(unsigned long ms);

// create a built-in mama duck
MamaDuck duck;

// create a timer with default settings
auto timer = timer_create_default();

int counter = 1;
bool setupOK = false;

void setup() {
  // LED Intial
  FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalSMD5050 );
  FastLED.setBrightness(  BRIGHTNESS );
  leds[0] = CRGB::Red;
  FastLED.show();

  //The Device ID must be exactly 8 bytes otherwise it will get rejected
  std::vector<byte> devId;
  devId.insert(devId.end(), deviceId.begin(), deviceId.end());
  
  if (duck.setupWithDefaults(devId) != DUCK_ERR_NONE) {
    Serial.println("[MAMA] Failed to setup MamaDuck");
    return;
  }

  //Setup GPS
  GPS.begin(9600, SERIAL_8N1, 34, 12);

  // Initialize the timer for telemetry
  timer.every(INTERVAL_MS, runSensor);

  // LED Complete
  leds[0] = CRGB::Gold;
  FastLED.show();

  setupOK = true;

  Serial.println("[MAMA] Setup OK!");
}

std::vector<byte> stringToByteVector(const String& str) {
    std::vector<byte> byteVec;
    byteVec.reserve(str.length());

    for (unsigned int i = 0; i < str.length(); ++i) {
        byteVec.push_back(static_cast<byte>(str[i]));
    }

    return byteVec;
}

void loop() {
  if (!setupOK) {
    return; 
  }
  timer.tick();
  // Use the default run(). The Mama duck is designed to also forward data it receives
  // from other ducks, across the network. It has a basic routing mechanism built-in
  // to prevent messages from hoping endlessly.
  duck.run();
}

bool runSensor(void *) {
  
  bool result;

  String gpsData = getGPSData();

  String statusMessage = String("QUACK QUACK QUACK QUACK") + " " + gpsData;
  Serial.print("[MAMA] status data: ");
  Serial.println(statusMessage);

  result = sendData(stringToByteVector(statusMessage), location);
  if (result) {
     Serial.println("[MAMA] runSensor ok.");
  } else {
     Serial.println("[MAMA] runSensor failed.");
  }

  return result;
}

bool sendData(std::vector<byte> message, topics value) {
  bool sentOk = false;
  
  int err = duck.sendData(value, message);
  if (err == DUCK_ERR_NONE) {
     counter++;
     sentOk = true;
  }
  if (!sentOk) {
    Serial.println("[MAMA] Failed to send data. error = " + String(err));
  }
  return sentOk;
}

static void smartDelay(unsigned long ms)
{
  unsigned long start = millis();
  do
  {
    while (GPS.available())
      tgps.encode(GPS.read());
  } while (millis() - start < ms);
}

// Getting GPS data
String getGPSData() {

  // Encoding the GPS
  smartDelay(5000);
  
  // Printing the GPS data
  Serial.println("[MAMA] --- GPS ---");
  Serial.print("[MAMA] Latitude  : ");
  Serial.println(tgps.location.lat(), 5);  
  Serial.print("[MAMA] Longitude : ");
  Serial.println(tgps.location.lng(), 4);
  Serial.print("[MAMA] Altitude  : ");
  Serial.print(tgps.altitude.feet() / 3.2808);
  Serial.println("M");
  Serial.print("[MAMA] Satellites: ");
  Serial.println(tgps.satellites.value());
  Serial.print("[MAMA] Time      : ");
  Serial.print(tgps.time.hour());
  Serial.print(":");
  Serial.print(tgps.time.minute());
  Serial.print(":");
  Serial.println(tgps.time.second());
  Serial.print("[MAMA] Speed     : ");
  Serial.println(tgps.speed.kmph());
  Serial.println("[MAMA] **********************");
  
  // Creating a message of the Latitude and Longitude
  String sensorVal = "Lat:" + String(tgps.location.lat(), 5) + " Lng:" + String(tgps.location.lng(), 4) + " Alt:" + String(tgps.altitude.feet() / 3.2808) + " Time: " + String(tgps.time.hour())+":"+String(tgps.time.minute())+":"+String(tgps.time.second());

  // Check to see if GPS data is being received
  if (millis() > 5000 && tgps.charsProcessed() < 10)
  {
    Serial.println(F("[MAMA] No GPS data received: check wiring"));
  }

  return sensorVal;
}