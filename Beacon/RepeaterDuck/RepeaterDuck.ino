/**
 * @file MamaDuck.ino
 * @brief Uses the built in Mama Duck.
 */

std::string deviceId("REPEATR2"); // DuckID - NEEDS to be 8 characters
const int INTERVAL_MS = 10000; // for sending the counter message

#include <string>
#include <arduino-timer.h>
#include <CDP.h>
#include "FastLED.h"
// #include <DuckRadio.h>

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

bool sendData(std::vector<byte> message, topics value);
void handleDuckData(std::vector<byte> packetBuffer);

// create a built-in mama duck
MamaDuck duck;
// DuckRadio radio;

// create a timer with default settings
auto timer = timer_create_default();

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

  duck.onReceiveDuckData(handleDuckData);

  // LED Complete
  leds[0] = CRGB::Green;
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

void handleDuckData(std::vector<byte> packetBuffer) {
  
  CdpPacket packet = CdpPacket(packetBuffer);
  
  std::string payload(packet.data.begin(), packet.data.end());
  std::string sduid(packet.sduid.begin(), packet.sduid.end());
  std::string dduid(packet.dduid.begin(), packet.dduid.end());

  std::string muid(packet.muid.begin(), packet.muid.end());
  std::string path(packet.path.begin(), packet.path.end());

  Serial.println("[MAMA] Packet Received:");
  Serial.println("[MAMA] sduid:   " + String(sduid.c_str()));
  Serial.println("[MAMA] dduid:   " + String(dduid.c_str()));

  Serial.println("[MAMA] muid:    " + String(muid.c_str()));
  Serial.println("[MAMA] path:    " + String(path.c_str()));
  Serial.println("[MAMA] data:    " + String(payload.c_str()));
  Serial.println("[MAMA] hops:    " + String(packet.hopCount));
  Serial.println("[MAMA] duck:    " + String(packet.duckType));
}