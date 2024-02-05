#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer_Generic.h>
#include <FS.h>
#include "SPIFFS.h"
#include <WiFiGeneric.h>
#define MAX_MASTERS 4
const char *ssid = "Galaxy S10+d6db";
const char *password = "nhnr9088";

const char *hotspotPass = "87654321";          // pass for hotspot
const char *deviceName = "kinetick-balls-mom"; // name of device in networt

IPAddress staticIP(192, 168, 1, 200); // for esp
IPAddress gateWay(192, 168, 1, 1);    // for router
IPAddress subnet(255, 255, 255, 0);
IPAddress dns(8, 8, 8, 8);

WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);
enum DataTypes
{
    ID = 0,
    HEIGHT = 1,
    TIME = 2,
    RGB = 3,
};
struct MasterData
{
    uint16_t rgb;
    uint8_t id;
    uint8_t height;
    uint8_t time;
};

struct State
{
    bool masterConnections[MAX_MASTERS];

    // 0 to 3(index of the array) is the id of the master micros and the value is websocket connection number of the master
    uint8_t masterWebsocketConnectionNumbers[MAX_MASTERS];
    bool sentCommand = false;

} _state;

void setup()
{
    Serial.begin(115200);
    SPIFFS.begin();
    startSoftAP();
    startHTTPServer();
    startWebSocket();
}

void loop()
{
    if (!_state.sentCommand &&
        _state.masterConnections[0] &&
        _state.masterConnections[1] &&
        _state.masterConnections[2] &&
        _state.masterConnections[3])
    {
        readAndSendFile();
    }
    server.handleClient();
    webSocket.loop();
}

void startSoftAP()
{
    // WiFi.mode(WIFI_OFF);
    Serial.println("starting soft AP");
    WiFi.enableSTA(false);
    WiFi.enableAP(true);
    WiFi.mode(WIFI_AP);
    WiFi.softAP(deviceName, hotspotPass, 6);
    WiFi.softAPConfig(IPAddress(192, 168, 1, 200), gateWay, subnet);
    WiFi.softAPIP();
    Serial.println(WiFi.softAPIP());
}
/////////////////////////////////////////HTTP SERVER//////////////////////////////////////////////////////////////////////
bool handleFileRead(String path)
{ // send the right file to the client (if it exists)
    Serial.println(path);
    Serial.println(SPIFFS.exists(path));
    if (path.endsWith("/"))
        path += "index.html";                  // If a folder is requested, send the index file
    String contentType = getContentType(path); // Get the MIME type
    if (SPIFFS.exists(path))
    {                                                       // If the file exists
        File file = SPIFFS.open(path, "r");                 // Open it
        size_t sent = server.streamFile(file, contentType); // And send it to the client
        file.close();                                       // Then close the file again
        return true;
    }

    return false; // If the file doesn't exist, return false
}
String getContentType(String filename)
{ // return the type of the file passed to this finction
    if (filename.endsWith(".htm"))
        return "text/html";
    else if (filename.endsWith(".html"))
        return "text/html";
    else if (filename.endsWith(".css"))
        return "text/css";
    else if (filename.endsWith(".jpg"))
        return "image/jpeg";
    else if (filename.endsWith(".js"))
        return "text/javascript";
    else if (filename.endsWith(".svg"))
        return "image/svg+xml";
    else if (filename.endsWith(".ico"))
        return "image/x-icon";
    return "text/plain";
}
void startHTTPServer()
{
    server.on("/", HTTP_GET, []()
              { server.send(200, "text/html", "web page"); });
    server.onNotFound([]() {                                  // If the client requests any URI
        if (!handleFileRead(server.uri()))                    // send it if it exists
            server.send(404, "text/plain", "404: Not Found"); // otherwise, respond with a 404 (Not Found) error
    });
    server.begin(); // Start web server
}
/////////////////////////////////////////////WEBSOCKET SERVER///////////////////////////////////////////////////////
void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length)
{ // get message
    switch (type)
    {
    case WStype_DISCONNECTED:
        int8_t dcNumber = -1;
        for (size_t i = 0; i < 4; i++)
        {
            if (_state.masterWebsocketConnectionNumbers[i] == num)
            {
                dcNumber = i;
            }
        }
        if (dcNumber != -1)
        {
            _state.masterWebsocketConnectionNumbers[dcNumber] = false;
        }
        break;
    case WStype_CONNECTED:
    {
        IPAddress ip = webSocket.remoteIP(num);
    }
    break;
    case WStype_TEXT:
        String msg = (char *)payload;
        proccessWebsocketMessage(msg, num);
        break;
    }
}
void proccessWebsocketMessage(String msg, uint8_t num)
{ // figure out the message

    Serial.println(msg);
    if (msg.startsWith("id:"))
    {
        uint8_t id = msg.substring(3).toInt();
        _state.masterConnections[id] = true;
    }
}
void startWebSocket()
{
    webSocket.begin();
    webSocket.onEvent(webSocketEvent); // if there's an incoming websocket message, go to function 'webSocketEvent'
}
void setMasterStatus(uint8_t masterNumber, bool status)
{
    _state.masterConnections[masterNumber] = status;
}
////////////////////////////////////////STORAGE///////////////////////////////////////////////////////////
void readAndSendFile()
{
    File image3D = SPIFFS.open("/saved3DImage.txt", "r");
    String currentReadingValue = "";
    uint16_t currentRow = 0;

    for (uint16_t i = 0; i < image3D.size(); i++)
    {
        char readingCharacter = (char)image3D.read();
        if (readingCharacter == '\n')
        {
            webSocket.sendTXT(_state.masterWebsocketConnectionNumbers[currentRow / 36],
                              String("dataRow:") + currentReadingValue);
        }
        else
        {
            currentReadingValue += readingCharacter;
        }
    }
}