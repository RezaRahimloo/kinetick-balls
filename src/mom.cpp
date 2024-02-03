#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer_Generic.h>
#include <FS.h>
#include "SPIFFS.h"
#include <WiFiGeneric.h>

const char *ssid = "Galaxy S10+d6db";
const char *password = "nhnr9088";

IPAddress staticIP(192, 168, 1, 200); // for esp
IPAddress gateWay(192, 168, 1, 1);    // for router
IPAddress subnet(255, 255, 255, 0);
IPAddress dns(8, 8, 8, 8);

WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);

struct State
{
    bool masterConnections[4] = {false, false, false, false};
} _state;

void setup()
{
    Serial.begin(115200);
    SPIFFS.begin();
    startWifi();
    startHTTPServer();
    startWebSocket();
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
    staticIP[2] = IPAddress(WiFi.gatewayIP())[2];
    WiFi.config(staticIP, IPAddress(WiFi.gatewayIP()), IPAddress(WiFi.subnetMask()), dns);
    Serial.print("ESP32 IP Address: ");

    Serial.println(WiFi.localIP());
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
}
void startWebSocket()
{
    webSocket.begin();
    webSocket.onEvent(webSocketEvent); // if there's an incoming websocket message, go to function 'webSocketEvent'
}
