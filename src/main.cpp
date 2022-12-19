#if defined(ESP32)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif
// include the firebase client library
#include <Firebase_ESP_Client.h>
// Provide the token generation process info.
#include <addons/TokenHelper.h>
// Provide the RTDB payload printing info and other helper functions.
#include <addons/RTDBHelper.h>
// initialize temperature sensor
#include "DHT.h"
#define DHTPIN 22
#define DHTTYPE DHT11   // DHT 11  (AM2302), AM2321
DHT dht(DHTPIN, DHTTYPE);
// Define wiFi credentials
#define WIFI_SSID "TECNO SPARK 8C"
#define WIFI_PASSWORD "test1234"
// Define API Key
#define API_KEY "AIzaSyBtQ0uTWLkNVuK7e_ZGQ9UHxebQnD1WZLs"
// Define RTDB URL
#define DATABASE_URL "smarthome-6f50e-default-rtdb.firebaseio.com"
// Define the user email and password authentificated
#define USER_EMAIL "sonwaroberto@gmail.com"
#define USER_PASSWORD "677070023sS"
// create firebase Data object
FirebaseData fbdo;
// initialize auth in firebase
FirebaseAuth auth;
FirebaseConfig config;
unsigned long sendDataPrevMillis = 0;
// assign led pin to their deferent ports
int pinLivingRoom = 5;
int pinBedRoom = 3;
int pinGuestRoom = 4 ;
int pinKitchen = 15;

void setup() {
  Serial.begin(115200);
  pinMode(pinLivingRoom, OUTPUT);
  pinMode(pinBedRoom, OUTPUT);
  pinMode(pinGuestRoom, OUTPUT);
  pinMode(pinKitchen, OUTPUT);
  dht.begin();
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();
  Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);
  /* Assign the api key (required) */
  config.api_key = API_KEY;
  /* Assign the user sign in credentials */
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;
   /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;
  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; // see addons/TokenHelper.h
  #if defined(ESP8266)
    // In ESP8266 required for BearSSL rx/tx buffer for large data handle, increase Rx size as needed.
    fbdo.setBSSLBufferSize(2048 /* Rx buffer size in bytes from 512 - 16384 */, 2048 /* Tx buffer size in bytes from 512 - 16384 */);
  #endif
  // Limit the size of response payload to be collected in FirebaseData
  fbdo.setResponseSize(2048);
  Firebase.begin(&config, &auth);
  // connect or pass false value when WiFi reconnection will control by your code or third party library
  Firebase.reconnectWiFi(true);
  Firebase.setDoubleDigits(5);
  config.timeout.serverResponse = 10 * 1000;
  Serial.print(F("DHTxx test!!!"));
}
void loop() {
  // Firebase.ready() loop continuously to handle auth status
  if (Firebase.ready() && (millis() - sendDataPrevMillis > 15000 || sendDataPrevMillis == 0))
  {
    sendDataPrevMillis = millis();
    bool temperatureSensor;
    bool livingRoomBob;
    bool bedRooomBob;
    bool guestRoomBob;
    bool kitchenBob;
    // get bob status and update based on previous values
    Serial.printf("Get living room bob... %s\n", Firebase.RTDB.getBool(&fbdo, F("/smartHome/livingRoom/bob"), &livingRoomBob) ? livingRoomBob ? "true" : "false" : fbdo.errorReason().c_str());
    livingRoomBob ? digitalWrite(pinLivingRoom, HIGH) : digitalWrite(pinLivingRoom, LOW);
    Serial.printf("Get bed room bob... %s\n", Firebase.RTDB.getBool(&fbdo, F("/smartHome/bedRoom/bob"), &bedRooomBob) ? bedRooomBob ? "true" : "false" : fbdo.errorReason().c_str());
    bedRooomBob ? digitalWrite(pinBedRoom, HIGH) : digitalWrite(pinBedRoom, LOW);
    Serial.printf("Get guest room bob... %s\n", Firebase.RTDB.getBool(&fbdo, F("/smartHome/guestRoom/bob"), &guestRoomBob) ? guestRoomBob ? "true" : "false" : fbdo.errorReason().c_str());
    guestRoomBob ? digitalWrite(pinGuestRoom, HIGH) : digitalWrite(pinGuestRoom, LOW);
    Serial.printf("Get kitchen bob... %s\n", Firebase.RTDB.getBool(&fbdo, F("/smartHome/kitchen/bob"), &kitchenBob) ? kitchenBob ? "true" : "false" : fbdo.errorReason().c_str());
    kitchenBob ? digitalWrite(pinKitchen, HIGH) : digitalWrite(pinKitchen, LOW);
    Serial.println();
  }
  delay(2000);
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  float f = dht.readTemperature(true);
  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  } else {
    float hic = dht.computeHeatIndex(t, h, false);
    Serial.printf("Set temperature ... %s\n", Firebase.RTDB.setFloat(&fbdo, F("/smartHome/temperature/value"), t) ? "ok" : fbdo.errorReason().c_str());
    Serial.printf("Set humidity ... %s\n", Firebase.RTDB.setFloat(&fbdo, F("/smartHome/humidity/value"), h) ? "ok" : fbdo.errorReason().c_str());
    Serial.printf("Set heat index ... %s\n", Firebase.RTDB.setFloat(&fbdo, F("/smartHome/heatIndex/value"), hic) ? "ok" : fbdo.errorReason().c_str());
  }
}