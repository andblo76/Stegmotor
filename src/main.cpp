#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>

// Add your Wi-Fi network details here. Leave both values empty to use the ESP32 access point.
const char* wifiSsid = "";
const char* wifiPassword = "";

const IPAddress apIp(192, 168, 5, 5);
const IPAddress apGateway(192, 168, 5, 5);
const IPAddress apSubnet(255, 255, 255, 0);

WebServer server(80);

const char page[] PROGMEM = R"html(
<!doctype html>
<html lang="sv">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <meta name="theme-color" content="#111827">
  <title>Stegmotorstyrning</title>
  <style>
    body {
      align-items: center;
      background: #111827;
      color: #f9fafb;
      display: flex;
      font-family: system-ui, sans-serif;
      justify-content: center;
      margin: 0;
      min-height: 100vh;
      text-align: center;
    }
    main { padding: 2rem; }
    h1 { font-size: clamp(2rem, 8vw, 4rem); margin-bottom: 0.5rem; }
    p { color: #9ca3af; font-size: 1.1rem; }
    .status { color: #34d399; }
  </style>
</head>
<body>
  <main>
    <h1>Stegmotorstyrning</h1>
    <p class="status">Webbservern är igång</p>
    <p>ESP32 är redo för stegmotorstyrning.</p>
  </main>
</body>
</html>
)html";

void handleNotFound()
{
    server.send(404, "text/plain; charset=utf-8", "Sidan hittades inte");
}

void setup()
{
    Serial.begin(115200);

    if (wifiSsid[0] != '\0') {
        WiFi.mode(WIFI_STA);
        WiFi.begin(wifiSsid, wifiPassword);

        Serial.print("Ansluter till Wi-Fi");
        const unsigned long timeout = millis() + 15000;
        while (WiFi.status() != WL_CONNECTED && millis() < timeout) {
            delay(250);
            Serial.print(".");
        }
    }

    if (WiFi.status() != WL_CONNECTED) {
        WiFi.mode(WIFI_AP);
        if (!WiFi.softAPConfig(apIp, apGateway, apSubnet)) {
            Serial.println("\nKunde inte konfigurera ESP32-accesspunktens IP-adress.");
            return;
        }
        if (!WiFi.softAP("Stegmotorstyrning")) {
            Serial.println("\nKunde inte starta ESP32-accesspunkten.");
            return;
        }
        Serial.println("\nAnslut till Wi-Fi-nätverket Stegmotorstyrning.");
        Serial.print("Öppna http://");
        Serial.println(WiFi.softAPIP());
    } else {
        Serial.println("\nAnsluten till Wi-Fi.");
        Serial.print("Öppna http://");
        Serial.println(WiFi.localIP());
    }

    server.on("/", []() {
        server.send_P(200, "text/html; charset=utf-8", page);
    });
    server.onNotFound(handleNotFound);

    server.begin();
    Serial.println("Webbservern är igång.");
}

void loop()
{
    server.handleClient();
}
