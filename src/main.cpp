#include <Arduino.h>
#include <LittleFS.h>
#include "DynamicWebServer.h"
#include "DynamicElement.h"
#include "WiFi.h"
#include <ESPmDNS.h>

const char *ssid = "NetworkOfIOT";
const char *password = "40961024";

DynamicWebServer server(80); // Create a web server on port 80

DynamicElement *bootStatus;

void setup()
{
  Serial.begin(115200);
  delay(100);

  pinMode(0, INPUT_PULLUP); // Set GPIO 0 as input with pull-up resistor

  if (!LittleFS.begin(true))
  {
    Serial.println("An error has occurred while mounting LittleFS");
  }
  Serial.println("LittleFS mounted successfully");

  Serial.println("Connecting to ");
  Serial.println(ssid);

  // connect to your local wi-fi network
  WiFi.begin(ssid, password);

  // check wi-fi is connected to wi-fi network
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected..!");

  // Initialize mDNS
  if (!MDNS.begin("dynamic"))
  {
    Serial.println("Error setting up MDNS responder!");
  }
  Serial.println("mDNS responder started");

  Serial.println("Starting Dynamic Web Server...");
  server.begin();

  bootStatus = server.registerElement("bootstatus");
}

void loop()
{
  // Serial.println("loop");
  // delay(100);

  static bool buttonPressed = false;
  if (digitalRead(0) == LOW && !buttonPressed) // Button on GPIO 0 pressed
  {
    buttonPressed = true;
    bootStatus->setText("Pressed");
  }
  else if (digitalRead(0) == HIGH && buttonPressed) // Button released
  {
    buttonPressed = false;
    bootStatus->setText("Not Pressed");
  }
}