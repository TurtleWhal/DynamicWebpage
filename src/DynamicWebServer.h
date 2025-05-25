#ifndef DYNAMIC_WEBSERVER_H
#define DYNAMIC_WEBSERVER_H

#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>

class DynamicElement; // Forward declaration

class DynamicWebServer {
private:
    int port;
    AsyncWebServer server;
    AsyncWebSocket ws;
    String injectScript(String html);
    void handleMessage(const String &message, AsyncWebSocketClient *client);

public:
    DynamicWebServer(int port) : port(port), server(port), ws("/ws") {}

    void begin();

    DynamicElement *registerElement(String htmlid);

    void send(const String &message);
};

#endif // DYNAMIC_WEBSERVER_H