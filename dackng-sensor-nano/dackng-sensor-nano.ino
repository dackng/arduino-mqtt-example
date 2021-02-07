
#include <PubSubClient.h>
#include <WiFiNINA.h> 

const char* ssid = "Wc_prod";
const char* ssidPassword = "01123581321fibo";
const char* mqttServer = "19X.16X.2X.3X";

int humidity = 0;// 0- 1023
int rainLevel = 0;// 0- 1023

unsigned long lastQuery = 0;
unsigned long queryTime = 60000;//60 seconds

const char HUMIDITY_TOPIC[] = "dackng/humidity-state";  
const char RAIN_TOPIC[] = "dackng/rain-state";
const String CLIENT_ID = "dackng-nano-pubsub-01";
const int VALUE_SIZE = 7; // size of char[] for converting from int to char

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

void reconnect() {
  
  while (!clientMqtt.connected()) {
    Serial.print("Attempting MQTT connection... with clientID ");
    Serial.println(CLIENT_ID);
    
    if (clientMqtt.connect(CLIENT_ID.c_str())) {
      Serial.println("connected");
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
  setup_wifi();
  clientMqtt.setServer(mqttServer, 1883);
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
 
  if (millis() < lastQuery) {
    
    lastQuery = millis();
  }
 
  if ((millis() - lastQuery) > queryTime) {
    
    lastQuery = millis();
    
    sendRain();
    sendHumidity();
  }
}

void sendRain(){
  rainLevel = analogRead(A0); 
  char value[VALUE_SIZE];
  sprintf(value, "%05d", rainLevel);

  int totalSize = CLIENT_ID.length() + VALUE_SIZE + 1;
  char payload[totalSize];
  createPayload(value, payload, totalSize);
 
  clientMqtt.publish(RAIN_TOPIC, payload);
  Serial.print("message of rain sensor sent successfully =>");
  Serial.println(payload);
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
