# Stegmotorstyrning

ESP32-webbpanel för en DM542-stegmotordrivare. GPIO18 ger STEP-pulser och
GPIO19 ställer riktningen. Panelen kan starta och stoppa motorn, välja riktning
när motorn står stilla och ändra pulshastigheten.

## Bygg och ladda upp

Öppna projektet i PlatformIO och bygg samt ladda upp miljön `esp32dev`. Välj rätt
seriell port i PlatformIO innan uppladdning.

## Anslut till webbservern

- Med Wi-Fi: ange nätverksnamn och lösenord i `src/main.cpp`, i `wifiSsid` och
  `wifiPassword`. Efter uppstart visas ESP32:ans IP-adress i seriell monitor
  (115200 baud).
- Utan Wi-Fi: anslut till accesspunkten `Stegmotorstyrning` som ESP32 startar
  och öppna `http://192.168.5.5`.

## Motorpanelen

- Panelen visar varvtal mellan 7,5 och 750 varv/min, räknat med 0,15 varv/min
  per puls/s (400 pulser/varv) enligt de värden du angav. Hastigheten rampas upp
  från 7,5 varv/min och ändras stegvis för att minska risken för att motorn
  tappar steg. Stoppet avbryter pulserna direkt.
- Framåt/Bakåt väljer låg/hög signal på GPIO19. Vilket håll motorn faktiskt går
  åt beror på fasernas inkoppling.
- Om DM542:ans mikrosteg ändras behöver omräkningen till varv/min ändras också.
- Stoppknappen stoppar rörelsen men kopplar inte bort DM542:ans hållström.

Kontrollera DM542-modellens inkopplingsschema och att dess STEP/DIR-ingångar
fungerar med ESP32:ans 3,3 V-signaler innan du ansluter. Börja med låg hastighet
och kontrollera att motorn kan röra sig fritt.
