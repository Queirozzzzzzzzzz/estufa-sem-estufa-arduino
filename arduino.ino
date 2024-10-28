#include <DHT.h>
#include <DHT_U.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

#define DHTPIN D5
#define DHTTYPE DHT11

// Wi-Fi
WiFiClient espClient;
PubSubClient client(espClient);
const char* ssid = "ecogarden";
const char* password = "camposss";

// Local server
ESP8266WebServer server(80);

HTTPClient http;

// Website
const char* webUrl = "https://ecogarden.vercel.app";
const char* webToken = "website_token_here";

// Humidity sensor
DHT dht(DHTPIN, DHTTYPE);

void definirWifi() {
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
  Serial.println(WiFi.localIP());
}

void lidarServidorRemotoNaoEncontrado() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nHeaders: ";
  message += server.headers();
  message += "\n\n";
  server.send(404, "text/plain", message);
}

void definirServidorRemoto() {
  server.on("/", []() {
    server.send(200, "text/html", "<h1>Eco Garden</h1>");
  });

  server.on("/api/v1/token", HTTP_POST, []() {
    String token = server.arg("plain");
    
    const char* webTokenStr = token.c_str();
    webToken = webTokenStr;
    
    server.send(200, "application/json", "{\"message\": \"Token received\"}");
  }, lidarComServidorRemotoNaoEncontrado);

  server.onNotFound(lidarComServidorRemotoNaoEncontrado);
  server.begin();
}

void setup() {
  Serial.begin(115200);

  // Sensors
  dht.begin();

  definirWifi();
  definirServidorRemoto();

  delay(2000);
}

String lerHumidade() {
  float humidity = dht.readHumidity();

  if (isnan(humidity)) {
    Serial.println("Falha ao ler do DHT11!");
    return "";
  }

  return String(humidity);
}

bool humidadeEstaValida(float humidity) {
  // Checar se humidity é válida
  return true;
}

void enviarDados() {
  DynamicJsonDocument dados(2048);

  float humidity = lerHumidade().toFloat();

  if(humidadeEstaValida(humidity)) {
    dados["humidity"] = humidity;
  }

  fazerRequisicao(dados);
}

void fazerRequisicao(DynamicJsonDocument dados) {
  String url = webUrl;
  url += "/api/v1/data";
  http.begin(espClient, url);
  http.addHeader("Cookie", "token=" + String(webToken));
  http.addHeader("Content-Type", "application/json");

  String requestBody;
  serializeJson(dados, requestBody);

  int httpResponseCode = http.POST(requestBody);

  if (httpResponseCode == 200) {
    String response = http.getString();
    Serial.println("HTTP POST sent successfully");
    Serial.println(response);
  } else {
    Serial.printf("Error sending HTTP POST request: %d\n", httpResponseCode);
  }

  http.end();
}


void loop() {
  if (WiFi.status() == WL_CONNECTED && webToken != "website_token_here") {
    enviarDados();
  }

  server.handleClient();

  delay(1000);
}