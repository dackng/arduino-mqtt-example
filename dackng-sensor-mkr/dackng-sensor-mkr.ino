
#include <PubSubClient.h>
#include <WiFiNINA.h> 
#define PIN_VALVE 6 // turning on or off of valve in PIN D6

const char* ssid = "Wc_prod";
const char* ssidPassword = "01123581321fibo";
const char* mqttServer = "19X.16X.1Y.2Z";

int humidity = 0;// 0- 1023

unsigned long lastQuery = 0;
unsigned long queryTime = 60000;//60 seconds

const char HUMIDITY_TOPIC[] = "dackng/humidity-state"; 
const char OPEN_VALVE_TOPIC[] = "dackng/open-valve"; 
const char OPENED_VALVE_TOPIC[] = "dackng/opened-valve";
const char CLOSE_VALVE_TOPIC[] = "dackng/close-valve"; 
const char CLOSED_VALVE_TOPIC[] = "dackng/closed-valve";
const String CLIENT_ID = "dackng-mkr-pubsub-01";
const int VALUE_SIZE = 7; // size of char[] for converting from int to char
int valveSwitchState = 0;

WiFiClient wifiClient;
PubSubClient clientMqtt(wifiClient);

void setup_wifi() {
  delay(10);
  
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, ssidPassword);

  while (WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] with payload => ");
  for (int i=0;i<length;i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

   // Switch on the VALVE if 1 was received as first character
  char newPayload[1];
  if ( strcmp (OPEN_VALVE_TOPIC, topic) == 0 && (char)payload[0] == '1') {//turn on
    digitalWrite(PIN_VALVE, HIGH);   
    valveSwitchState = 1;
    itoa(valveSwitchState, newPayload, 10);
    clientMqtt.publish(OPENED_VALVE_TOPIC, newPayload);
    Serial.println("TURN ON!!");
  
  } else if( strcmp (CLOSE_VALVE_TOPIC, topic) == 0 && (char)payload[0] == '0'){//turn off
    digitalWrite(PIN_VALVE, LOW); ;
    valveSwitchState = 0;
    itoa(valveSwitchState, newPayload, 10);
    clientMqtt.publish(CLOSED_VALVE_TOPIC, newPayload);
    Serial.println("TURN OFF!!");
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!clientMqtt.connected()) {
    Serial.print("Attempting MQTT connection... with clientID ");
    
    Serial.println(CLIENT_ID);
    
    if (clientMqtt.connect(CLIENT_ID.c_str())) {
      Serial.println("connected");
      clientMqtt.subscribe(OPEN_VALVE_TOPIC);
      clientMqtt.subscribe(CLOSE_VALVE_TOPIC);
    } else {
      Serial.print("failed, rc=");
      Serial.print(clientMqtt.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  
  pinMode(PIN_VALVE,OUTPUT);

  setup_wifi();
  clientMqtt.setServer(mqttServer, 1883);
  clientMqtt.setCallback(callback);
}

void loop() {
  if (!clientMqtt.connected()) {
    reconnect();
  }
  delay(1000);
  clientMqtt.loop();

  timer();
}

void timer() {
  // Comprobar si se ha dado la vuelta
  if (millis() < lastQuery) {
    // Asignar un nuevo valor
    lastQuery = millis();
  }
 
  if ((millis() - lastQuery) > queryTime) {
    // Marca de tiempo
    lastQuery = millis();
    sendHumidity();
  }
}

void sendHumidity(){
  humidity = analogRead(A1); 
  char value[VALUE_SIZE];
  sprintf(value, "%05d", humidity);
  
  int totalSize = CLIENT_ID.length() + VALUE_SIZE + 1;
  char payload[totalSize];
  createPayload(value, payload, totalSize);
 
  clientMqtt.publish(HUMIDITY_TOPIC, payload);
  Serial.print("message of humidity sensor sent successfully => ");
  Serial.println(payload);
}

void createPayload(char* value, char* payload, int totalSize){
  CLIENT_ID.toCharArray(payload, totalSize);
  strcat(payload, ",");
  strcat(payload, value);
}
