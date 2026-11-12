#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>

const char* ssid = "2,4Ghz";
const char* password = "Walter2014";

#define RELAY_PIN 23

WebServer server(80);

void triggerRelay()
{
    digitalWrite(RELAY_PIN, LOW);
    delay(500);
    digitalWrite(RELAY_PIN, HIGH);

    server.sendHeader("Location", "/");
    server.send(303);
}

void handleRoot()
{
    String html =
    "<!DOCTYPE html>"
    "<html>"
    "<head>"
    "<meta charset='UTF-8'>"
    "<meta name='viewport' content='width=device-width, initial-scale=1'>"
    "<title>Garageport</title>"
    "</head>"
    "<body style='font-family:Arial;text-align:center;'>"

    "<h1>Garageport</h1>"

    "<p>Relästyrning via ESP32</p>"

    "<button onclick=\"location.href='/trigger'\" "
    "style='width:250px;height:80px;font-size:28px;'>"
    "Oppna / Stang"
    "</button>"

    "</body>"
    "</html>";

    server.send(200, "text/html", html);
}

void setup()
{
    pinMode(RELAY_PIN, OUTPUT);
    digitalWrite(RELAY_PIN, HIGH);

    Serial.begin(115200);

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }

    Serial.println();
    Serial.print("IP-adress: ");
    Serial.println(WiFi.localIP());

    server.on("/", handleRoot);
    server.on("/trigger", triggerRelay);

    server.begin();
}

void loop()
{
    server.handleClient();
}