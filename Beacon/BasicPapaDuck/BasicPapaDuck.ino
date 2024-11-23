/**
 * @file PapaDuck-Basic.ino
 * @author Amir Nathoo
 * @brief 
 *  This basic PapaDuck example can communicate with other ducks but does connect to the cloud.
 *  It will instead output messages to the serial monitor in JSON format.
 * 
 * 
 * @date 02-28-2024
 *
 */

#include <CDP.h>
#include "FastLED.h"

#include <ArduinoJson.h>
#include <arduino-timer.h>
#include <string>

#include <queue>
#include <iomanip>
#include <sstream>

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

std::string toTopicString(byte topic);
String convertToHex(byte* data, int size);
int toJSON(CdpPacket packet);
static void smartDelay(unsigned long ms);
String getGPSData();

// const int bufferSize = 4 * JSON_OBJECT_SIZE(4);
JsonDocument doc;

bool setupOK = false;

// Use pre-built papa duck
PapaDuck duck;

auto timer = timer_create_default();


std::string toTopicString(byte topic) 
{

  std::string topicString;

  switch (topic) {
    case topics::status:
      topicString = "status";
      break;
    case topics::cpm:
      topicString = "portal";
      break;
    case topics::sensor:
      topicString = "sensor";
      break;
    case topics::alert:
      topicString = "alert";
      break;
    case topics::location:
      topicString = "gps";
      break;
    case topics::health:
      topicString = "health";
      break;
    case topics::bmp180:
      topicString = "bmp180";
      break;
    case topics::pir:
      topicString = "pir";
      break;
    case topics::dht11:
      topicString = "dht";
      break;
    case topics::bmp280:
      topicString = "bmp280";
      break;
    case topics::mq7:
      topicString = "mq7";
      break;
    case topics::gp2y:
      topicString = "gp2y";
      break;
    case reservedTopic::ack:
      topicString = "ack";
      break;
    default:
      topicString = "status";
  }

  return topicString;
}

String convertToHex(byte* data, int size) 
{
  String buf = "";
  buf.reserve(size * 2); // 2 digit hex
  const char* cs = "0123456789ABCDEF";
  for (int i = 0; i < size; i++) {
    byte val = data[i];
    buf += cs[(val >> 4) & 0x0F];
    buf += cs[val & 0x0F];
  }
  return buf;
}

int toJSON(CdpPacket packet) 
{

  String gpsData = getGPSData();
  
  std::stringstream ss;

  // convert the dduid to a string. We should probably do this for all fields
  // using stringstream because print will interpret 00 as a null terminator
  for (auto &c : packet.dduid) {
    ss << std::hex << std::setw(2) << std::setfill('0') << (int)c;
  }
  
  std::string payload(packet.data.begin(), packet.data.end());
  std::string sduid(packet.sduid.begin(), packet.sduid.end());
  std::string dduid = ss.str();
  std::string muid(packet.muid.begin(), packet.muid.end());

  Serial.println("[PAPA] topic:   " + String(toTopicString(packet.topic).c_str()));
  
  Serial.println("[PAPA] sduid:   " + String(sduid.c_str()));
  Serial.println("[PAPA] dduid:   " + String(dduid.c_str()));

  Serial.println("[PAPA] muid:    " + String(muid.c_str()));
  Serial.println("[PAPA] data:    " + String(payload.c_str()));
  Serial.println("[PAPA] hops:    " + String(packet.hopCount));
  Serial.println("[PAPA] duck:    " + String(packet.duckType));

  doc["DeviceId"] = sduid;
  doc["topic"].set(toTopicString(packet.topic));
  doc["MessageId"] = muid;
  doc["Payload"].set(payload);
  doc["hops"].set(packet.hopCount);
  doc["duckType"].set(packet.duckType);

  Serial.println("[PAPA] GPS:     " + gpsData);

  String jsonstat;
  serializeJson(doc, jsonstat);
  serializeJsonPretty(doc, Serial);
  Serial.print("\n[PAPA] --------------------------------------------------------------------------------------\n\n");

  
  return 1;
}

// The callback method simply takes the incoming packet and
// converts it to a JSON string, before sending it out over WiFi
void handleDuckData(std::vector<byte> packetBuffer)
{
  Serial.print("\n[PAPA] --------------------------------------------------------------------------------------\n");
  Serial.println("[PAPA] got packet: " + convertToHex(packetBuffer.data(), packetBuffer.size()));

  CdpPacket packet = CdpPacket(packetBuffer);
  toJSON(packet);
}

void setup() 
{
  
  // LED Intial
  FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalSMD5050 );
  FastLED.setBrightness(  BRIGHTNESS );
  leds[0] = CRGB::Red;
  FastLED.show();

  std::string deviceId("PAPA0004");
  std::vector<byte> devId;
  devId.insert(devId.end(), deviceId.begin(), deviceId.end());
  if (duck.setupWithDefaults(devId) != DUCK_ERR_NONE) {
    Serial.println("[PAPA] Failed to setup MamaDuck");
    setupOK = false;
    return;
  }
  
  //Setup GPS
  GPS.begin(9600, SERIAL_8N1, 34, 12);

  // register a callback to handle incoming data from duck in the network
  duck.onReceiveDuckData(handleDuckData);

  setupOK = true;

  Serial.println("[PAPA] Setup OK! ");

  // LED Complete
  leds[0] = CRGB::Green;
  FastLED.show();

  duck.enableAcks(false);
}

void loop() 
{
  if (!setupOK) {
    return;
  }
  duck.run();
  timer.tick();
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