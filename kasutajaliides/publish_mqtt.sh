#!/usr/bin/env bash

#Home Assistanti MQTT topicute discovery skript.

BROKER=localhost

mosquitto_pub -h "$BROKER" -t "homeassistant/sensor/esp32/l1_power/config" -m '{
  "name":"L1 Power",
  "state_topic":"home/energy/l1",
  "unit_of_measurement":"W",
  "value_template":"{{ value }}"
}' -r

mosquitto_pub -h "$BROKER" -t "homeassistant/sensor/esp32/l2_power/config" -m '{
  "name":"L2 Power",
  "state_topic":"home/energy/l2",
  "unit_of_measurement":"W",
  "value_template":"{{ value }}"
}' -r

mosquitto_pub -h "$BROKER" -t "homeassistant/sensor/esp32/l3_power/config" -m '{
  "name":"L3 Power",
  "state_topic":"home/energy/l3",
  "unit_of_measurement":"W",
  "value_template":"{{ value }}"
}' -r

mosquitto_pub -h "$BROKER" -t "homeassistant/sensor/esp32/total_power/config" -m '{
  "name":"Total Power",
  "state_topic":"home/energy/total",
  "unit_of_measurement":"W",
  "value_template":"{{ value }}"
}' -r
echo "MQTT discovery OK."
