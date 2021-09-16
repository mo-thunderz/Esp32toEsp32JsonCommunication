// ---------------------------------------------------------------------------------------
//
// Code for ESP32 to ESP32 communication using Websockets (device used for tests: ESP32-WROOM-32D).
// >>> SERVER CODE <<<
//
// This is the code for the server ESP32 that sets up a local access point. It then generates
// two random numbers and broadcasts these on through the Websocket with JSON encapsulation. 
// the clients answer with some other data that is collected on the master. 
//
// For installation, the following libraries need to be installed:
// * Websockets by Markus Sattler (can be tricky to find -> search for "Arduino Websockets"
// * ArduinoJson by Benoit Blanchon
//
// Written by mo thunderz (last update: 11.09.2021)
//
// ---------------------------------------------------------------------------------------

#include <WiFi.h>                                     // needed to connect to setup an accesspoint
#include <WebSocketsServer.h>                         // needed for instant communication between client and server through Websockets
#include <ArduinoJson.h>                              // needed for JSON encapsulation (send multiple variables with one string)

// SSID and password that are going to be used for the Access Point you will create -> DONT use the SSID/Password of your router:
const char* ssid = "TYPE_YOUR_SSID_HERE";
const char* password = "TYPE_YOUR_PASSWORD_HERE";

// The JSON library uses static memory, so this will need to be allocated:
StaticJsonDocument<200> doc_tx;                       // provision memory for about 200 characters
StaticJsonDocument<200> doc_rx;

// We want to periodically send values to the clients, so we need to define an "interval" and remember the last time we sent data to the client (with "previousMillis")
int interval = 1000;                                  // send data to the client every 1000ms -> 1s
unsigned long previousMillis = 0;                     // we use the "millis()" command for time reference and this will output an unsigned long

// Initialization of websocket
WebSocketsServer webSocket = WebSocketsServer(81);    // the websocket uses port 81 (standard port for websockets

// Configure IP addresses of the local access point
IPAddress local_IP(192,168,1,22);
IPAddress gateway(192,168,1,5);
IPAddress subnet(255,255,255,0);

void setup() {
  Serial.begin(115200);                               // init serial port for debugging
 
  Serial.print("Setting soft-AP configuration ... ");
  Serial.println(WiFi.softAPConfig(local_IP, gateway, subnet) ? "Ready" : "Failed!");

  Serial.print("Setting soft-AP ... ");
  Serial.println(WiFi.softAP(ssid, password) ? "Ready" : "Failed!");

  Serial.print("Soft-AP IP address = ");
  Serial.println(WiFi.softAPIP());
  
  webSocket.begin();                                  // start websocket
  webSocket.onEvent(webSocketEvent);                  // define a callback function -> what does the ESP32 need to do when an event from the websocket is received? -> run function "webSocketEvent()"
}

void loop() {
  webSocket.loop();                                   // Update function for the webSockets 
  
  unsigned long now = millis();                       // read out the current "time" ("millis()" gives the time in ms since the Arduino started)
  if ((unsigned long)(now - previousMillis) > interval) { // check if "interval" ms has passed since last time the clients were updated
    
    String jsonString = "";                           // create a JSON string for sending data to the client
    JsonObject object = doc_tx.to<JsonObject>();      // create a JSON Object
    object["rand1"] = random(100);                    // write data into the JSON object -> I used "rand1" and "rand2" here, but you can use anything else
    object["rand2"] = random(100);
    serializeJson(doc_tx, jsonString);                // convert JSON object to string
    // Serial.println(jsonString);                       // print JSON string to console for debug purposes (you can comment this out)
    webSocket.broadcastTXT(jsonString);               // send JSON string to clients
    
    previousMillis = now;                             // reset previousMillis
  }
}

void webSocketEvent(byte num, WStype_t type, uint8_t * payload, size_t length) {      // the parameters of this callback function are always the same -> num: id of the client who send the event, type: type of message, payload: actual data sent and length: length of payload
  switch (type) {                                     // switch on the type of information sent
    case WStype_DISCONNECTED:                         // if a client is disconnected, then type == WStype_DISCONNECTED
      Serial.println("Client " + String(num) + " disconnected");
      break;
    case WStype_CONNECTED:                            // if a client is connected, then type == WStype_CONNECTED
      Serial.println("Client " + String(num) + " connected");
      // optionally you can add code here what to do when connected
      break;
    case WStype_TEXT:                                 // if a client has sent data, then type == WStype_TEXT
      // try to decipher the JSON string received
      DeserializationError error = deserializeJson(doc_rx, payload);
      if (error) {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
        return;
      }
      else {
        // JSON string was received correctly, so information can be retrieved:
        const char* g_brand = doc_rx["brand"];
        const char* g_type = doc_rx["type"];
        const int g_year = doc_rx["year"];
        const char* g_color = doc_rx["color"];
        Serial.println("Received guitar info from user: " + String(num));
        Serial.println("Brand: " + String(g_brand));
        Serial.println("Type: " + String(g_type));
        Serial.println("Year: " + String(g_year));
        Serial.println("Color: " + String(g_color));
      }
      Serial.println("");
      break;
  }
}
