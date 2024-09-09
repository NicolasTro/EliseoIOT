#include <ESP8266WiFi.h>

const char* ssid = "WIFI";
const char* password = "pass";

WiFiServer server(80);
const int relayPin = D3; 

void setup() {
  Serial.begin(115200);
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, HIGH); // Relé apagado inicialmente

  // Conectar a WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConectado a la red WiFi");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  server.begin();
}

void loop() {
  WiFiClient client = server.available();
  if (client) {
    String request = client.readStringUntil('\r');
    client.flush();
    
  

    if (request.indexOf("/prender") != -1) {
      digitalWrite(relayPin, LOW);  // Enciende el relé
      client.println("HTTP/1.1 200 OK");
      client.println("Content-Type: text/html");
      client.println();
      client.println("Relay encendido");
    } 
    else if (request.indexOf("/apagar") != -1) {
      digitalWrite(relayPin, HIGH);   // Apaga el relé
      client.println("HTTP/1.1 200 OK");
      client.println("Content-Type: text/html");
      client.println();
      client.println("Relay apagado");
    } else {
      client.println("HTTP/1.1 404 Not Found");
      client.println("Content-Type: text/html");
      client.println();
      client.println("Comando no encontrado");
    }
    client.stop();
  }
}