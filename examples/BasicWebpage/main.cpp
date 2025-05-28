#include <Arduino.h>
#include <LittleFS.h>
#include "DynamicWebServer.h"
#include "DynamicElement.h"
#include "WiFi.h"
#include <ESPmDNS.h>
#include "Adafruit_NeoPixel.h"

const char *ssid = "YOUR_SSID"; // Replace with your Wi-Fi SSID
const char *password = "YOUR_PASSWORD"; // Replace with your Wi-Fi credentials

DynamicWebServer server(80); // Create a web server on port 80

DynamicElement *bootStatus;

Adafruit_NeoPixel led = Adafruit_NeoPixel(1, 48, NEO_GRB + NEO_KHZ800); // Create a NeoPixel object for one LED on GPIO 2

void setup()
{
  Serial.begin(115200);
  delay(100);

  pinMode(0, INPUT_PULLUP); // Set GPIO 0 as input with pull-up resistor

  led.begin();           // Initialize the NeoPixel LED
  led.setBrightness(50); // Set brightness to 50%
  led.setPixelColor(0, 0, 255, 0);
  led.show(); // Initialize all pixels to 'off'

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
  bootStatus->setHtml("Not Pressed");
  bootStatus->addEventListener("click", [](String value)
                               {
    Serial.println("Boot status element clicked");
    bootStatus->setHtml("Clicked"); });

  static uint8_t r = 0, g = 255, b = 0;
  static bool ledOn = true;

  DynamicElement *ledcolor = server.registerElement("ledcolor");
  ledcolor->addEventListener("input", [](String value)
                             {
    // value is a hex string like "#RRGGBB"
    Serial.printf("LED color changed to: %s\n", value.c_str());
    if (value.length() == 7 && value[0] == '#') {
      long color = strtol(value.substring(1).c_str(), nullptr, 16);
      r = (color >> 16) & 0xFF;
      g = (color >> 8) & 0xFF;
      b = color & 0xFF;
      Serial.printf("Setting LED color to R:%d G:%d B:%d\n", r, g, b);
      if (ledOn) {
        led.setPixelColor(0, r, g, b); // Set LED color based on value
        led.show();
      }
    } });
  ledcolor->setValue("#00FF00"); // Set initial color to green

  DynamicElement *ledstate = server.registerElement("ledstate");
  ledstate->addEventListener("change", [](String value)
                             {
    if (value == "on") {
      led.setPixelColor(0, r, g, b);
      led.show();
      ledOn = true; 
    } else if (value == "off") {
      led.setPixelColor(0, 0, 0, 0);
      led.show();
      ledOn = false; 
    } else {
      Serial.printf("Unknown LED state: %s\n", value.c_str());
    } });
  // ledstate->setValue("on"); // Set initial state to off
}

void loop()
{
  // Serial.println("loop");
  // delay(100);

  static bool buttonPressed = false;
  if (digitalRead(0) == LOW && !buttonPressed) // Button on GPIO 0 pressed
  {
    buttonPressed = true;
    bootStatus->setHtml("Pressed");
  }
  else if (digitalRead(0) == HIGH && buttonPressed) // Button released
  {
    buttonPressed = false;
    bootStatus->setHtml("Not Pressed");
  }
}