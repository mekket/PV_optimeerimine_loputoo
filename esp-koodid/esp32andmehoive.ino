#include <ModbusMaster.h>
#include <WiFi.h>
#include <PubSubClient.h>

//WIFI
#define WIFI_SSID    ""
#define WIFI_PASS    ""
//MQTT

#define MQTT_SERVER  ""
#define MQTT_PORT    1883
#define MQTT_USER    ""
#define MQTT_PASS    ""


#define MODBUS_DIR_PIN   4  //rs485 DE/RE
#define MODBUS_RX_PIN   16
#define MODBUS_TX_PIN   17
#define MODBUS_BAUD     9600
#define SLAVE_ID        1  // DTSU666 aadress


const uint16_t register_start = 8212;//0x2014 (8212 dec)
const uint8_t  register_count   = 6; //Pa,Pb,Pc  (3 × 2 16-bit registrit)



const unsigned long busidle = 10;// minimum vaikus enne saatmist
const unsigned int  busdelay = 4000; // vaikusedelay

WiFiClient      espClient;
PubSubClient    mqtt(espClient);
ModbusMaster    node;

unsigned long lastRx = 0;

void wifiConnect() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) delay(500);
}


void mqttConnect() {
  while (!mqtt.connected()) {
    mqtt.connect("DTSU666_ESP32", MQTT_USER, MQTT_PASS);
    if (!mqtt.connected()) delay(2000);
  }
}



void preTransmission()  {
  uint32_t diff = millis() - lastRx;
  
  if (diff < busidle) delay(busidle - diff);//oota vaikust
  digitalWrite(MODBUS_DIR_PIN, HIGH);//TX
}


void postTransmission() {
  digitalWrite(MODBUS_DIR_PIN, LOW);//RX
  delayMicroseconds(busdelay);
}

bool is_valid(float v) {  //0.1 väärtused viskame ära(vead)
return fabs(v + 0.1f) > 0.001f;
}

void mqtt_publish(const char* topic, float power) {
  char sonum[12]; dtostrf(power, 6, 2, sonum); mqtt.publish(topic, sonum);
}


void setup() {
  Serial.begin(115200);

  pinMode(MODBUS_DIR_PIN, OUTPUT);
  digitalWrite(MODBUS_DIR_PIN, LOW);

  wifiConnect();

  mqtt.setServer(MQTT_SERVER, MQTT_PORT);
  mqttConnect();

  Serial2.begin(MODBUS_BAUD, SERIAL_8N2, MODBUS_RX_PIN, MODBUS_TX_PIN);
  node.begin(SLAVE_ID, Serial2);
  node.preTransmission(preTransmission);
  node.postTransmission(postTransmission);
}


void loop() {
  if (!mqtt.connected()) mqttConnect();
  mqtt.loop();

  // rs485 vaba?
  if (Serial2.available()) lastRx = millis();

  // arvestile päring
  uint8_t rc = node.readInputRegisters(register_start, register_count);
  if (rc == node.ku8MBSuccess) {

    float phase[3];
    for (uint8_t i = 0; i < 3; ++i) {
      uint32_t raw = node.getResponseBuffer(i*2) << 16 |
                     node.getResponseBuffer(i*2 + 1);
      phase[i] = *(float*)&raw / 10.0f; 
    }

    if (is_valid(phase[0]) && is_valid(phase[1]) && is_valid(phase[2])) {
      char sonum[12];

      dtostrf(phase[0], 6, 2, sonum); 
      mqtt.publish("home/energy/l1", sonum);

      dtostrf(phase[1], 6, 2, sonum);
      mqtt.publish("home/energy/l2", sonum);

      dtostrf(phase[2], 6, 2, sonum); 
      mqtt.publish("home/energy/l3", sonum);

      dtostrf(phase[0] + phase[1] + phase[2], 7, 2, sonum);
      mqtt.publish("home/energy/total", sonum);
    }
  }

  delay(600);
}
