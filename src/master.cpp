#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <WiFi.h>
#include <WebServer.h>
#include <FS.h>
#include <WebSocketsClient_Generic.h> // WebSocket Client Library for WebSocket
#include "SPIFFS.h"

#define ID 0
#define MAX_SLAVES 36

const char *ssid = "kinetick-balls-mom";
const char *password = "87654321";

IPAddress staticIP(192, 168, 1, 200); // for esp
IPAddress gateWay(192, 168, 1, 1);    // for router
IPAddress subnet(255, 255, 255, 0);
IPAddress dns(8, 8, 8, 8);
/////////////////////////////////////SERVER/////////////////////////////////////////////////////////
WebSocketsClient webSocket; // websocket client class instance

struct SlaveData {
    uint16_t rgb;
    uint8_t height;
    uint8_t time;
};

SlaveData slavesData[MAX_SLAVES];

struct State
{
  bool acknowledged = false;
} _state;

void setup()
{
  Serial.begin(115200);
  SPIFFS.begin();
  startWifi();
  connectWebsocketClient();
}

void loop()
{
}

void startWifi()
{
  WiFi.mode(WIFI_STA);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.print("ESP32-CAM IP Address: ");
  Serial.println(WiFi.localIP());
  staticIP[2] = IPAddress(WiFi.gatewayIP())[2];
  WiFi.config(staticIP, IPAddress(WiFi.gatewayIP()), IPAddress(WiFi.subnetMask()), dns);
  WiFi.localIP();
}

void connectWebsocketClient()
{
  webSocket.begin("192.168.1.200", 81, "/");
  // WebSocket event handler
  webSocket.onEvent(webSocketEvent);
  // if connection failed retry every 5s
  webSocket.setReconnectInterval(5000);
}
void webSocketEvent(WStype_t type, uint8_t *payload, size_t length)
{ // get message
  switch (type)
  {
  case WStype_DISCONNECTED:
    break;
  case WStype_CONNECTED:
  {
    webSocket.sendTXT("id:" + String(ID));
  }
  break;
  case WStype_TEXT:
    String msg = (char *)payload;
    proccessWebsocketMessage(msg);
    break;
  }
}
void proccessWebsocketMessage(String msg)
{ // figure out the message
  if(msg.startsWith("data:")){
    decypherData(msg.substring(5));
  }
  Serial.println(msg);
}

void decypherData(String slaveData)
{
  char seperator = '-';
  uint8_t currentStep = 0;
  String rgb = "";
  String id = "";
  String time = "";
  String height = "";
  for (size_t i = 0; i < slaveData.length(); i++)
  {
    if (slaveData[i] != seperator)
    {
      if (currentStep == 0)
      {
        id += slaveData[i];
      }
      else if (currentStep == 1)
      {
        height += slaveData[i];
      }
      else if (currentStep == 2)
      {
        time += slaveData[i];
      }
      else if (currentStep == 3)
      {
        rgb += slaveData[i];
      }
    }
  }

  SlaveData data;
  int idNumber = id.toInt();
  data.height = id.toInt();
  data.time = id.toInt();
  data.rgb = id.toInt();

  slavesData[idNumber] = data;
}