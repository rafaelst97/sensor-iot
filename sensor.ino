#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>

// Configurações Wi-Fi
const char* ssid = "Rafael-mobile";
const char* password = "aqbbh3gh7k3j8vj";

// Configurações do Broker MQTT
const char* mqtt_server = "test.mosquitto.org"; // Broker alternativo para testes
const int mqtt_port = 1883;

// Configuração do DHT11
#define DHTPIN 2
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

WiFiClient espClient;
PubSubClient client(espClient);

/*Este controle com o Flag abaixo é para que contorne o problema de não enviar a segunda publicação, assim a cada loop ele envia uma publicação diferente*/
//Controle de envio: 0 - Envio de tempratura; 1 - Envio de umidade
int temp_umi = 0;

// Tópicos MQTT
const char* topic_temp = "graduacao/iot/grupo_2/temperatura";
const char* topic_humi = "graduacao/iot/grupo_2/umidade";

// Variáveis de controle
unsigned long lastMsg = 0;
const long interval = 500; // Intervalo de 5 segundos entre as publicações

void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  //client.setBufferSize(512); // Ajusta o buffer para suportar payloads maiores
  //client.setKeepAlive(60); //Mantém conexão aberta por mais tempo
  dht.begin();
}

void setup_wifi() {
  delay(10);
  Serial.print("Conectando a ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi conectado");
  Serial.print("Endereço IP: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Conectando ao MQTT...");
    if (client.connect("ESP8266Client")) {
      Serial.println("Conectado!");
      // Inscreve-se nos tópicos, se necessário
      // client.subscribe(topic_temp);
      // client.subscribe(topic_humi);
    } else {
      Serial.print("Falha, rc=");
      Serial.print(client.state());
      Serial.println(" tentando novamente em 5 segundos");
      delay(5000);
    }
  }
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  unsigned long now = millis();
  if (now - lastMsg > interval) {
    lastMsg = now;

    // Leitura do sensor
    float h = dht.readHumidity();
    float t = dht.readTemperature();

    if (isnan(h) || isnan(t)) {
      Serial.println("Falha na leitura do sensor!");
      return;
    }

    // Converte os valores para string
    char tempString[8];
    char humString[8];
    dtostrf(t, 1, 2, tempString);
    dtostrf(h, 1, 2, humString);

    // Publica temperatura
    if (temp_umi == 0){ //Temperatura
      Serial.print("[DEBUG] Publicando temperatura: ");
      Serial.print(tempString);
      Serial.print(" no tópico: ");
      Serial.println(topic_temp);
    }else{ //Umidade
      Serial.print("[DEBUG] Publicando umidade: ");
      Serial.print(humString);
      Serial.print(" no tópico: ");
      Serial.println(topic_humi);
    }

    if (temp_umi == 0){ //Temperatura
      if (client.publish(topic_temp, tempString, true)){
        Serial.println("Temperatura publicada!");
      }else{
        Serial.println("Falha ao publicar a temperatura!");
      }
      
      temp_umi = 1; //Muda para que o próximo envio seja de umidade
      
    }else{ //Umidade
      if (client.publish(topic_humi, humString, true)){
        Serial.println("Umidade publicada!");
      }else{
        Serial.println("Falha ao publicar a umidade!");
      }
      
      temp_umi = 0; //Muda para que o próximo envio seja de temperatura
    }
  }
}
