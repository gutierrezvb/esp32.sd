#include <PubSubClient.h>
#include <WiFi.h>
#include <DHT.h>


#define INTERVALO_ENVIO       20000
#define DHTPIN 4 //pino digital dht
#define DHTTYPE DHT11
//CONFIG WIFI
const char* ssid = "Rep Santa Casa";
const char* password = "rep1234567";
//CONFIG BROKER
const char* mqttServer = "broker.shiftr.io";
const int mqttPort = 1883;
const char* mqttUser = "5f1e1403";
const char* mqttPassword = "8cfcbda147e0d517";

const char* light = "/light";
int ultimoEnvioMQTT = 0;
int LED_BUILTIN = 2;
WiFiClient espClient;
PubSubClient client(espClient);
DHT dht(DHTPIN, DHTTYPE);

//SETUP
void setup(){
  
    pinMode(LED_BUILTIN, OUTPUT);

    Serial.begin(115200);
    dht.begin();
    WiFi.begin(ssid, password);
    //CONECTAR WIFI
    while (WiFi.status() != WL_CONNECTED){
        delay(500);
        Serial.print("Conectando na WI-FI : ");
        Serial.println(ssid);
    }

    Serial.println("Conexão realizada com sucesso");
    Serial.println("");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    //SERVIDOR BROKER PELO MQTT
    client.setServer(mqttServer, mqttPort);
    client.setCallback(callback);

    while (!client.connected()) {
        Serial.println("Conectando no MQTT…");
        //Criando um client ID random
        String clientId = "ESP32";
        clientId += String(random(0xffff), HEX);
        if (client.connect(clientId.c_str(), mqttUser, mqttPassword )) {
            Serial.println("Conectado");
        } else {
            Serial.print("Falha com o estado ");
            Serial.print(client.state());
            delay(2000);
        }
    }

    Serial.println("Tentando enviar a mensagem");
    client.publish("/light", "Hello from ESP32");
    client.subscribe("/light");
}

void callback(char* topic, byte* payload, unsigned int length){

    Serial.print("Mensagem recebida no topico : ");
    Serial.println(topic);
    String message;
    Serial.print("Mensagem:");
    for (int i = 0; i < length; i++) {
        //message = message + (char)payload[i];  //Conver *byte to String
        Serial.print((char)payload[i]);
    }

    if (payload[0] == '0'){
        Serial.println(" Desligando luz");
        digitalWrite(LED_BUILTIN, LOW);
    }

    if (payload[0] == '1'){
        Serial.println(" Ligando luz");
        digitalWrite(LED_BUILTIN, HIGH);
    }
    Serial.println();
    Serial.println(" — — — — — — — — — — — -");

}

void enviaDHT(){
  char MsgUmidadeMQTT[10];
  char MsgTemperatureMQTT[10];
  
  float umidade = dht.readHumidity();
  float temperature = dht.readTemperature();
  
 
  if (isnan(temperature) || isnan(umidade)) 
  {
    #ifdef DEBUG
    Serial.println("Falha na leitura do dht11...");
    #endif
  } 
  else 
  {
    #ifdef DEBUG
    Serial.print("Umidade: ");
    Serial.print(umidade);
    Serial.print(" \n"); //quebra de linha
    Serial.print("Temperatura: ");
    Serial.print(temperature);
    Serial.println(" °C");
    #endif
 
    //sprintf(MsgUmidadeMQTT,"%f",umidade);
    //client.publish("/umidade", MsgUmidadeMQTT);
    sprintf(MsgTemperatureMQTT,"%f",temperature);
    client.publish("/temperature", MsgTemperatureMQTT);
  }
}

void loop() {

    if ((millis() - ultimoEnvioMQTT) > INTERVALO_ENVIO)
  {
      enviaDHT();
      ultimoEnvioMQTT = millis();
  }
  
    client.loop();
    
}
