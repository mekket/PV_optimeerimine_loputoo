#include <ESP8266WiFi.h>
#include <PubSubClient.h>

//WIFI
const char* ssid = "";
const char* password = "";

//MQTT
const char* mqtt_broker = "";
const char* mqtt_user = "";
const char* mqtt_pass = "";

const int relayPin = D5;  //relee active-low

//mqtt teemad
const char* mqtt_nordpool_topic = "nordpool"; //nordpool hinnad homeassistantist
const char* mqtt_threshold_topic = "elektrihind2"; //kasutaja seadistatud hinnapiir
const char* mqtt_solar_l1_topic = "home/energy/l1";//L1 faas
const char* mqtt_solar_l2_topic = "home/energy/l2"; //L2 faas
const char* mqtt_solar_l3_topic = "home/energy/l3"; //L3 faas
const char* mqtt_phase_selector_topic = "faasivalik2"; //0=OFF   1=L1   2=L2      3=L3
const char* heater_power_topic = "tarbimine2"; //tarbija nimivõimsus homeassistantist
const char* heater_hysteresis_topic = "hysteresis2"; //hüsterees homeassistantist

//default
float nordpoolprice = 0.0;
float hinnapiir = 100.0;

int solarL1 = 0;
int solarL2 = 0;
int solarL3 = 0;

int faas = 0;
bool faasinitial = false;

int nimivoimsus = 0;

unsigned long switchdelay = 1000;//1s default
unsigned long switchtime = 0;

int state = 0;// 0 = OFF, 1 = ON

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  Serial.begin(9600);
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, HIGH);//active low ehk HIGH = off

  client.setServer(mqtt_broker, 1883);
  client.setCallback(callback);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting WiFi...");
  }
  Serial.println("WiFi connected");
}


void updatestate(int stateUUS) {
  if (stateUUS != state) {
    unsigned long now = millis();

    state = stateUUS;
    digitalWrite(relayPin, (state == 1) ? LOW : HIGH);//active-low
    switchtime = now;
  }
}


void heater() {
  unsigned long now = millis();

//faasivalik
  int loetudPV = 0;
  if (faas == 1) loetudPV = solarL1;
  else if (faas == 2) loetudPV = solarL2;
  else if (faas == 3) loetudPV = solarL3;

//debugid
  Serial.print("faas=");
  Serial.print(faas);
  Serial.print(" PV=");
  Serial.print(loetudPV);
  Serial.print(" nimivõimsus=");
  Serial.print(nimivoimsus);
  Serial.print(" hind=");
  Serial.print(nordpoolprice);
  Serial.print(" piir=");
  Serial.print(hinnapiir);
  Serial.print(" olek=");
  Serial.println(state);

  int stateUUS = state;

  if (state == 0) {
    if (nordpoolprice < hinnapiir && loetudPV <= -nimivoimsus) {
    stateUUS = 1;
    }
  } else {
    if (nordpoolprice >= hinnapiir || loetudPV >= 0) {
    stateUUS = 0;
    }
  }

  // hüsterees (aeg)
  if ((stateUUS != state) && ((now - switchtime) > switchdelay)) {
  updatestate(stateUUS);
  }
}


void callback(char* topic, byte* payload, unsigned int length) {
  String message;
  for (unsigned int i = 0; i < length; ++i) message += (char)payload[i];

  bool run = false;

  if (strcmp(topic, mqtt_nordpool_topic) == 0) {
    nordpoolprice = message.toFloat();
    run = true;
  } else if (strcmp(topic, mqtt_threshold_topic) == 0) {
    hinnapiir = message.toFloat();
    run = true;
  } else if (strcmp(topic, mqtt_solar_l1_topic) == 0) {
    solarL1 = message.toInt();
    if (faas == 1) run = true;
  } else if (strcmp(topic, mqtt_solar_l2_topic) == 0) {
    solarL2 = message.toInt();
    if (faas == 2) run = true;
  } else if (strcmp(topic, mqtt_solar_l3_topic) == 0) {
    solarL3 = message.toInt();
    if (faas == 3) run = true;
  } else if (strcmp(topic, mqtt_phase_selector_topic) == 0) {
    int faasUUS = message.toInt();
    if (!faasinitial) {



    faasinitial = true;
    faas = faasUUS;
    run = true;



    } else if (faasUUS != faas) {
      faas = faasUUS;
      Serial.println("reboot faasivahetuseks");
      delay(500);
      ESP.restart();
        }
  } else if (strcmp(topic, heater_power_topic) == 0) {
    nimivoimsus = message.toInt();
    run = true;

  } else if (strcmp(topic, heater_hysteresis_topic) == 0) {
    switchdelay = (unsigned long)message.toInt() * 1000UL;
    run = true;
  }

  if (run) {
  heater();
  }
}

void reconnect() {
  while (!client.connected()) {
    if (client.connect("PV_Relee_Seade")) {
      client.subscribe(mqtt_nordpool_topic);
      client.subscribe(mqtt_threshold_topic);
      client.subscribe(mqtt_solar_l1_topic);
      client.subscribe(mqtt_solar_l2_topic);
      client.subscribe(mqtt_solar_l3_topic);
      client.subscribe(mqtt_phase_selector_topic);
      client.subscribe(heater_power_topic);
      client.subscribe(heater_hysteresis_topic);
    } else {
      Serial.print("Failed to connect to MQTT broker, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}
