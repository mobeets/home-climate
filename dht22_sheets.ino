#include <ESP8266WiFi.h>
#include "DHT.h"
#include "HTTPSRedirect.h"

// Uncomment one of the lines below for whatever DHT sensor type you're using!
//#define DHTTYPE DHT11   // DHT 11
//#define DHTTYPE DHT21   // DHT 21 (AM2301)
#define DHTTYPE DHT22  // DHT 22  (AM2302), AM2321

// DHT Sensor
uint8_t DHTPin = D2;

/*Put your SSID & Password*/
const char* ssid = "YourSSID"; // TODO: Enter SSID here
const char* password = "YourPassword"; // TODO: Enter Password here

// For writing to Google Sheets
const char *GScriptId = ""; // TODO

// Enter command (insert_row or append_row) and your Google Sheets sheet name (default is Sheet1):
String payload_base =  "{\"command\": \"insert_row\", \"sheet_name\": \"Sheet1\", \"values\": ";
String payload = "";

// Google Sheets setup (do not edit)
const char* host = "script.google.com";
const int httpsPort = 443;
const char* fingerprint = "";
String url = String("/macros/s/") + GScriptId + "/exec";
HTTPSRedirect* client = nullptr;

// Initialize DHT sensor.
DHT dht(DHTPin, DHTTYPE);

// declare sensor measurement variables
float Temperature;
float Humidity;

void setup() {
  Serial.begin(74880);
  delay(100);
  Serial.println("pinMode");

  pinMode(DHTPin, INPUT);

  Serial.println("dht.begin calling");
  dht.begin();
  delay(200);
  Serial.println("dht.begin called");

  Serial.println("Connecting to ");
  Serial.println(ssid);

  //connect to your local wi-fi network
  WiFi.begin(ssid, password);

  //check wi-fi is connected to wi-fi network
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected..!");
  Serial.print("Got IP: ");
  Serial.println(WiFi.localIP());

  // Use HTTPSRedirect class to create a new TLS connection
  client = new HTTPSRedirect(httpsPort);
  client->setInsecure();
  client->setPrintResponseBody(true);
  client->setContentTypeHeader("application/json");
  
  Serial.print("Connecting to ");
  Serial.println(host);

  // Try to connect for a maximum of 5 times
  bool flag = false;
  for (int i=0; i<5; i++){ 
    int retval = client->connect(host, httpsPort);
    if (retval == 1){
       flag = true;
       Serial.println("Connected");
       break;
    }
    else
      Serial.println("Connection failed. Retrying...");
  }
  if (!flag){
    Serial.print("Could not connect to server: ");
    Serial.println(host);
    return;
  }
  delete client;    // delete HTTPSRedirect object
  client = nullptr; // delete HTTPSRedirect object
  
}
void loop() {

  // Temperature = 0.0;
  // Humidity = 0.0;
  Temperature = (dht.readTemperature() * 9/5) + 32;  // Gets the values of the temperature
  Humidity = dht.readHumidity();        // Gets the values of the humidity
  Serial.println("Temperature: " + String(Temperature) + "°F, Humidity: " + String(Humidity) + "%");
  
  static bool flag = false;
  if (!flag){
    client = new HTTPSRedirect(httpsPort);
    client->setInsecure();
    flag = true;
    client->setPrintResponseBody(true);
    client->setContentTypeHeader("application/json");
  }
  if (client != nullptr){
    if (!client->connected()){
      client->connect(host, httpsPort);
    }
  }
  else{
    Serial.println("Error creating client object!");
  }
  
  // Create json object string to send to Google Sheets
  payload = payload_base + "\"" + Temperature + "," + Humidity + "\"}";
  
  // Publish data to Google Sheets
  Serial.println("Publishing data...");
  Serial.println(payload);
  if(client->POST(url, host, payload)){ 
    // do stuff here if publish was successful
    Serial.println("Successfully published");
  }
  else{
    // do stuff here if publish was not successful
    Serial.println("Error while connecting");
  }

  // a delay of several seconds is required before publishing again    
  delay(10*60*1000); // ten minutes
}
