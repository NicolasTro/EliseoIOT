#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <addons/TokenHelper.h>
#include <addons/RTDBHelper.h>

#define WIFI_SSID "@TheAtomicLab2 2.4GHz"
#define WIFI_PASSWORD "superheroes1"

#define API_KEY "AIzaSyCj8crR3cLk8ZI3_aywJVVwMSu_Hae9Trw"
#define DATABASE_URL "https://eliseoiot-default-rtdb.firebaseio.com/"

#define USER_EMAIL "eliseoctd@gmail.com"
#define USER_PASSWORD "eliseo"

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;

const int releA = 27;
const int releB = 14;
const int buttonPin = 26;
const int pirPin = 13;  // Pin al que está conectado el sensor PIR
bool pirStateActive = false; // Indica si el PIR está activo
unsigned long pirTimerStart = 0; // Guarda el tiempo de activación del PIR
const unsigned long pirActiveDuration = 10000; // Duración de 10 segundos (en milisegundos) para mantener el PIR activo

volatile bool buttonState = false; // Variable para el estado del botón
const unsigned long debounceDelay = 50; // Tiempo de debounce en milisegundos
unsigned long lastButtonPress = 0; // Tiempo del último cambio de estado del botón

// Función de interrupción para el botón
void IRAM_ATTR handleButton() {
  unsigned long currentMillis = millis();
  // Solo cambiar el estado si ha pasado el tiempo de debounce
  if (currentMillis - lastButtonPress > debounceDelay) {
    buttonState = !buttonState; // Invierte el estado del botón al presionar
    lastButtonPress = currentMillis; // Actualiza el tiempo del último cambio de estado
  }
}

void setup() {
  Serial.begin(115200);   // Inicializar el puerto serie
  pinMode(pirPin, INPUT); // Configura el pin PIR como entrada
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(releA, OUTPUT);
  pinMode(releB, OUTPUT);
  Serial.println("Monitoring PIR Sensor in real-time...");
  attachInterrupt(digitalPinToInterrupt(buttonPin), handleButton, FALLING); // Interrupción para el botón (detecta cuando se presiona)

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  config.api_key = API_KEY;
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;
  config.database_url = DATABASE_URL;
  config.token_status_callback = tokenStatusCallback;
  Firebase.reconnectNetwork(true);

  fbdo.setBSSLBufferSize(4096, 1024);
  fbdo.setResponseSize(2048);
  Firebase.begin(&config, &auth);
  Firebase.setDoubleDigits(5);
  config.timeout.serverResponse = 10 * 1000;
}

void loop() {
  int pirState = digitalRead(pirPin); // Leer el estado actual del pin PIR

  // Verificar si el PIR detectó movimiento y si no está en estado activo por el temporizador
  if (pirState == HIGH && !pirStateActive) {
    Serial.println("PIR Pin: HIGH (Motion detected)");
    Firebase.RTDB.setBool(&fbdo, F("/house/pir-state"), true);
    pirStateActive = true; // Marcar el PIR como activo
    pirTimerStart = millis(); // Iniciar el temporizador de 10 segundos
  }

  // Si el PIR está activo, comprobar si han pasado los 10 segundos
  if (pirStateActive && (millis() - pirTimerStart >= pirActiveDuration)) {
    Serial.println("PIR Pin: LOW (No motion - 10s passed)");
    Firebase.RTDB.setBool(&fbdo, F("/house/pir-state"), false);
    pirStateActive = false; // Reiniciar el estado del PIR
  }
  Serial.printf("Button state: %s\n", Firebase.RTDB.setBool(&fbdo, F("/house/button-state"), buttonState) ? "ok" : "no ok");

 if (Firebase.ready() && (millis() - sendDataPrevMillis > 1000 || sendDataPrevMillis == 0))
  {
    sendDataPrevMillis = millis();

   int releState;
   if(Firebase.RTDB.getInt(&fbdo, "/rele1", &releState)){
    digitalWrite(releA, !releState);
   }
   
  
   int releStateB;
   if(Firebase.RTDB.getInt(&fbdo, "/rele2", &releStateB)){
    digitalWrite(releB, !releStateB);
   }
   
  }

  delay(300);  // Ajustar el retardo según la frecuencia deseada (en milisegundos)
}