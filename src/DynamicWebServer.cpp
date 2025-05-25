#include <LittleFS.h>
#include "ArduinoJson.h"

#include "DynamicWebServer.h"
#include "DynamicElement.h"

#include "DynamicClient.h"

uint8_t numElements = 0;                  // Counter for registered elements
DynamicElement *elements[64] = {nullptr}; // Array to hold pointers to registered elements

void DynamicWebServer::begin()
{
  server.on("/", HTTP_GET, [this](AsyncWebServerRequest *request)
            {
              Serial.println("Received request for /");

              // Send the index.html file from LittleFS
              if (LittleFS.exists("/index.html"))
              {
                Serial.println("Sending index.html");
                String index = LittleFS.open("/index.html", "r").readString();

                index = injectScript(index);

                request->send(200, "text/html", index);
              }
              else
              {
                Serial.println("index.html not found, sending 404");
                request->send(404, "text/plain", "File not found");
                return;
              } });

  server.on("/esp32dynamic.js", HTTP_GET, [this](AsyncWebServerRequest *request)
            { request->send(200, "text/js", javascript); });

  server.serveStatic("/", LittleFS, "/");

  ws.onEvent([this](AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len)
             {
    (void)len;

    if (type == WS_EVT_CONNECT) {
      // ws.textAll("new client connected");
      Serial.println("ws connect");
      client->setCloseClientOnQueueFull(false);
      client->ping();

    } else if (type == WS_EVT_DISCONNECT) {
      // ws.textAll("client disconnected");
      Serial.println("ws disconnect");

    } else if (type == WS_EVT_ERROR) {
      Serial.println("ws error");

    } else if (type == WS_EVT_PONG) {
      Serial.println("ws pong");

    } else if (type == WS_EVT_DATA) {
      AwsFrameInfo *info = (AwsFrameInfo *)arg;
      // Serial.printf("index: %" PRIu64 ", len: %" PRIu64 ", final: %" PRIu8 ", opcode: %" PRIu8 "\n", info->index, info->len, info->final, info->opcode);
      String msg = "";
      if (info->final && info->index == 0 && info->len == len) {
        if (info->opcode == WS_TEXT) {
          data[len] = 0;
          // Serial.printf("ws text: %s\n", (char *)data);
          msg = String((char *)data);
          handleMessage(msg, client);
        }
      }
    } });

  server.addHandler(&ws);
  server.begin();
  Serial.printf("Server started on port %d\n", port);
}

String DynamicWebServer::injectScript(String html)
{
  html += "<script src='esp32dynamic.js'></script>";
  return html;
}

void DynamicWebServer::handleMessage(const String &message, AsyncWebSocketClient *client)
{
  Serial.printf("Received message: %s\n", message.c_str());

  // Parse the message as JSON
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, message);

  if (error)
  {
    Serial.printf("Failed to parse JSON: %s\n", error.c_str());
    return;
  }

  if (doc["event"].is<String>())
  {
    String event = doc["event"];
    if (event == "load")
    {
      // client->text("load event received");
      for (uint8_t i = 0; i < numElements; i++)
      {
        if (elements[i] != nullptr)
        {
          elements[i]->sendStatus();
        }
      }
    }
    else if (event == "event")
    {
      for (uint8_t i = 0; i < numElements; i++)
      {
        if (elements[i]->htmlid == doc["id"].as<String>())
        {
          // Serial.printf("Event %s received for element %s\n", event.c_str(), elements[i]->htmlid.c_str());
          // Call the event callback if it exists
          elements[i]->triggerEvent(doc["type"].as<String>(), doc["value"].as<String>());
          return; // Exit after handling the event
        }
      }
    }
  }
  else
  {
    Serial.println("No event key in message");
  }
}

DynamicElement *DynamicWebServer::registerElement(String htmlid)
{
  DynamicElement *element = new DynamicElement(htmlid);
  element->setServer(this); // Set the server instance in the element

  if (numElements < 64)
  {
    elements[numElements++] = element; // Store the pointer to the element in the array
  }
  else
  {
    Serial.println("Maximum number of elements reached, cannot register more.");
    delete element; // Clean up if we can't register
    return nullptr;
  }

  Serial.printf("Registered element with HTML ID: %s\n", htmlid.c_str());
  return element;
}

void DynamicWebServer::send(const String &message)
{
  Serial.printf("Sending message: %s\n", message.c_str());
  ws.textAll(message);
}