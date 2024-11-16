#include <WiFi.h> // Para ESP32
#include <PubSubClient.h>
#include <DHT.h>

// Configurações Wi-Fi
const char* ssid = "Rafael";          // Substitua pelo SSID da sua rede Wi-Fi
const char* password = "5050.1000";  // Substitua pela senha da sua rede Wi-Fi

// Configurações do Broker MQTT
const char* mqtt_server = "test.mosquitto.org";
const int mqtt_port = 1883;

// Configuração do DHT11
#define DHTPIN 2   // Pino conectado ao DHT11 (GPIO2 no ESP32)
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

WiFiClient espClient;
PubSubClient client(espClient);

// Tópicos MQTT para o grupo 2
const char* topic_temp = "graduacao/iot/grupo_2/temperatura";
const char* topic_humi = "graduacao/iot/grupo_2/umidade";

void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  dht.begin();
}

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Conectando a ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi conectado");
  Serial.print("Endereço IP: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  // Tenta reconectar ao MQTT
  while (!client.connected()) {
    Serial.print("Conectando ao MQTT...");
    if (client.connect("ESP32Client")) {
      Serial.println("Conectado!");
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

  // Leitura do sensor
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (isnan(h) || isnan(t)) {
    Serial.println("Falha na leitura do sensor!");
    return;
  }

  // Publica os dados no broker MQTT
  char tempString[8];
  char humString[8];
  dtostrf(t, 1, 2, tempString);
  dtostrf(h, 1, 2, humString);

  client.publish(topic_temp, tempString);
  client.publish(topic_humi, humString);

  // Exibe no monitor serial
  Serial.print("Temperatura: ");
  Serial.print(tempString);
  Serial.print(" °C\tUmidade: ");
  Serial.print(humString);
  Serial.println(" %");

  delay(5000); // Aguarda 5 segundos para a próxima leitura
}
