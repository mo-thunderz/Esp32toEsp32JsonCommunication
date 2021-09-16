// ---------------------------------------------------------------------------------------
//
// Code for ESP32 to ESP32 communication using Websockets (device used for tests: ESP32-WROOM-32D).
// >>> CLIENT CODE <<<
//
// This is the code for the client ESP32 that logs in on the ESP32 server and receives two 
// randomly generated numbers. It then replies with information about my guitar. 
//
// For installation, the following libraries need to be installed:
// * Websockets by Markus Sattler (can be tricky to find -> search for "Arduino Websockets"
// * ArduinoJson by Benoit Blanchon
//
// Written by mo thunderz (last update: 11.09.2021)
//
// ---------------------------------------------------------------------------------------

#include <WiFi.h>                                     // needed to connect to WiFi
#include <WebSocketsClient.h>                         // needed for instant communication between client and server through Websockets
#include <ArduinoJson.h>                              // needed for JSON encapsulation (send multiple variables with one string)

// SSID and password of ESP32 Wifi server:
const char* ssid = "TYPE_YOUR_SSID_HERE";
const char* password = "TYPE_YOUR_PASSWORD_HERE";

WebSocketsClient webSocket; // websocket client class instance
StaticJsonDocument<200> doc_tx; // Allocate a static JSON document
StaticJsonDocument<200> doc_rx; // Allocate a static JSON document

void setup() {
  Serial.begin(115200);                               // init serial port for debugging
 
  WiFi.begin(ssid, password);                         // start WiFi interface
  Serial.println("Establishing connection to WiFi with SSID: " + String(ssid));     // print SSID to the serial interface for debugging
 
  while (WiFi.status() != WL_CONNECTED) {             // wait until WiFi is connected
    delay(1000);
    Serial.print(".");
  }
  
  Serial.print("Connected to network with IP address: ");
  Serial.println(WiFi.localIP());                     // show IP address that the ESP32 has received from router
  
  
  webSocket.begin("192.168.1.22", 81, "/");           // Connect to ESP32 server websocket (port 81)
  webSocket.onEvent(webSocketEvent);                  // Define a callback function -> what does the ESP32 need to do when an event from the websocket is received? -> run function "webSocketEvent()"
  webSocket.setReconnectInterval(5000);               // if connection failed retry every 5s
}


void loop() {
  webSocket.loop(); // Keep the socket alive
}


void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
  if (type == WStype_TEXT)                                 // if data is received from server then type == WStype_TEXT
  {
    DeserializationError error = deserializeJson(doc_rx, payload); // deserialize incoming Json String
    if (error) {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.f_str());
      return;
    }
    const int rand1 = doc_rx["rand1"];                    // first random variable sent
    const int rand2 = doc_rx["rand2"];                    // second random variable sent
    
    // Print the received data for debugging
    Serial.println("Rand1: " + String(rand1) + ", Rand2: " + String(rand2));

    // ----------------------------------- send data back to Master ESP32 -------------------------------------//
    String jsonString;                                    // Temporary storage for JSON String
    JsonObject object = doc_tx.to<JsonObject>();          // Create JSON object
    object["brand"] = "Gibson";                           // Write some data in the object
    object["type"] = "Les Paul Studio";
    object["year"] = 2010;
    object["color"] = "white";
    serializeJson(doc_tx, jsonString);                    // convert JSON object to string
    // Serial.println(jsonString);                           // print JSON string to console for debug purposes (you can comment this out)
    webSocket.sendTXT(jsonString);                        // send JSON string to clients
  }
}
