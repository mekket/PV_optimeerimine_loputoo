# Sümmeetrilise PV inverteri energiatarbe optimeerimine

See repositoorium sisaldab bakalaureusetöö raames loodud lahenduse lähtekoodi ja dokumentatsiooni. Töö eesmärk on sümmeetrilise väljundiga PV inverteri abil toodetud energia kasutamise optimeerimine kasutades Home Assistant tarkvara, ESP32 ja ESP8266 mikrokontrollereid ning MQTT protokolli.

## Repositooriumi struktuur

- `esp32andmehoive.ino` – ESP32-põhise andmehõiveseadme kood, mis loeb andmeid CHINT DTSU-666 nutiarvestilt.
- `esp8266juhtseade.ino` – ESP8266-põhise elektritarbijate juhtseadme kood, mis juhib releesid vastavalt Home Assistantist saadud MQTT sõnumitele.
- `configuration.yaml` – Home Assistanti konfiguratsioonifail.
- `automations.yaml` – Automatisatsioonide konfiguratsioon Home Assistantis.
- `kasutajaliides.yaml` – Kasutajaliidese seadistused Home Assistantis.
- `publish_mqtt.sh` – Skript MQTT teemade loomiseks Home Assistanti MQTT Discovery jaoks.
- `andmehoive.png` – ESP32-põhise andmehõiveseadme elektriskeem.
 - `juhtseade.png` – ESP8266-põhise juhtseadme elektriskeem.

## Nõuded
- Home Assistant seadistatud ja töötav MQTT brokeriga (Eclipse Mosquitto).
- CHINT DTSU-666 nutiarvesti.
- ESP32 ja ESP8266 mikrokontrolleritega seadmed ühendatud Wi-Fi võrku.

