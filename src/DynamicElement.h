#ifndef DYNAMIC_ELEMENT_H
#define DYNAMIC_ELEMENT_H

#include <Arduino.h>
#include "DynamicWebServer.h"

typedef struct
{
    String event; // Event type (e.g., "click", "change")
    std::function<void(String)> callback;
} DynamicEvent;

class DynamicElement
{
private:
    DynamicWebServer *server; // Pointer to the server instance

    DynamicEvent events[8]; // Event for click actions

    String html;
    // String text;
    String value;

public:
    String htmlid;

    DynamicElement(String htmlid)
        : htmlid(htmlid) {}

    void setServer(DynamicWebServer *srv)
    {
        server = srv;
    }

    // Set Methods
    // void setText(String text)
    // {
    //     Serial.printf("Setting text for element: %s to %s\n", htmlid.c_str(), text.c_str());
    //     server->send(String("{\"type\":\"setText\",\"id\":\"") + htmlid + "\"" + ",\"text\":\"" + text + "\"}");
    //     this->text = text;
    // }

    void setHtml(String html)
    {
        Serial.printf("Setting HTML for element: %s to %s\n", htmlid.c_str(), html.c_str());
        server->send(String("{\"type\":\"setHtml\",\"id\":\"") + htmlid + "\"" + ",\"html\":\"" + html + "\"}");
        this->html = html;
    }

    void setValue(String value)
    {
        if (!value.isEmpty())
        {
            Serial.printf("Setting value for element: %s to %s\n", htmlid.c_str(), value.c_str());
            server->send(String("{\"type\":\"setValue\",\"id\":\"") + htmlid + "\"" + ",\"value\":\"" + value + "\"}");
            this->value = value;
        }
    }

    // Callback Methods
    void addEventListener(String event, std::function<void(String)> callback)
    {
        DynamicEvent newEvent;
        newEvent.event = event;
        newEvent.callback = callback;

        for (int i = 0; i < 8; i++)
        {
            if (events[i].event.isEmpty())
            {
                events[i] = newEvent;
                Serial.printf("Added event listener for %s on element %s\n", event.c_str(), htmlid.c_str());
                return;
            }
        }
        Serial.println("Event listener array is full, cannot add more events.");
    }

    void sendStatus()
    {
        setHtml(html);
        // setText(text);
        setValue(value);

        // Serial.printf("Sending events for element: %s\n", htmlid.c_str());
        for (int i = 0; i < 8; i++)
        {
            if (!events[i].event.isEmpty())
            {
                String message = String("{\"type\":\"addEvent\",\"id\":\"") + htmlid + "\"" + ",\"event\":\"" + events[i].event + "\"}";
                server->send(message);
            }
        }
    }

    void triggerEvent(String event, String value)
    {
        for (int i = 0; i < 8; i++)
        {
            if (events[i].event == event && events[i].callback)
            {
                Serial.printf("Triggering event %s for element %s\n", event.c_str(), htmlid.c_str());
                this->value = value; // Update the value before calling the callback
                events[i].callback(value); // Call the callback function
                return;
            }
        }
    }
};

#endif // DYNAMIC_ELEMENT_H