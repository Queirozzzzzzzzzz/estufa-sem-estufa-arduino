#include <DHT.h>
#include <DHT_U.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#define DHTPIN D5
#define DHTTYPE DHT11

// Wi-Fi
WiFiClient espClient;
PubSubClient client(espClient);
const char* ssid = "wifi_ssid_here";
const char* password = "wifi_password_here";

// Website
const char* webUrl = "website_url_here";
const char* webUser = "website_user_here";
const char* webPassword = "website_user_password_here";

// Humidity sensor
DHT dht (DHTPIN, DHTTYPE);

void setupWifi() {
  delay(100);

  Serial.println("");
  Serial.println("Connecting to Wi-Fi: ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print("...");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("Connected to Wi-Fi.");
}

void setup() {
  Serial.begin(115200);

  // Sensors
  dht.begin();

  setupWifi();

  delay(2000);
}

String getHumidity() {
  float humidity = dht.readHumidity();

  if(isnan(humidity)) {
    Serial.println("Falha ao ler do DHT11!");
    return "";
  }

  return String(humidity);
}

void sendData() {
  String humidity = getHumidity();
  Serial.println(humidity);
}

void loop() {
  sendData();

  delay(1000);
}