# Stegmotorstyrning

En minimal ESP32-boilerplate med en inbyggd webbserver.

## Bygg och ladda upp

Öppna projektet i PlatformIO och bygg samt ladda upp miljön `esp32dev`. Välj rätt
seriell port i PlatformIO innan uppladdning.

## Anslut till webbservern

- Med Wi-Fi: ange nätverksnamn och lösenord i `src/main.cpp`, i `wifiSsid` och
  `wifiPassword`. Efter uppstart visas ESP32:ans IP-adress i seriell monitor
  (115200 baud).
- Utan Wi-Fi: anslut till accesspunkten `Stegmotorstyrning` som ESP32 startar
  och öppna `http://192.168.5.5`.

Webbsidan visar att servern är igång. Ingen stegmotorutgång är konfigurerad ännu.
