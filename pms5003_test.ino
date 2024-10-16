#include <ESP8266WiFi.h>
#include <SoftwareSerial.h>
#include "HTTPSRedirect.h"

#define PMS_SET_PIN 0 // ESP8266 GPIO0 <-> PMS5003 SET
SoftwareSerial pmsSerial(2, 3); // RX, TX (ESP8266 GPIO2 <-> PMS5003 TXD; ignore the PMS5003 RXD)

bool pretendToWakeAndSleep = false; // for debugging PMS5003
const int secsToSleepBetweenLogs = 30; // for PMS5003
const int logsToAverageOver = 30; // one log each second for PMS5003

/*Put your SSID & Password*/
const char* ssid = "YourSSID"; // TODO: Enter SSID here
const char* password = "YourPassword"; // TODO: Enter Password here

// For writing to Google Sheets
const char *GScriptId = ""; // TODO

// Enter command (insert_row or append_row) and your Google Sheets sheet name (default is Sheet1):
String payload_base =  "{\"command\": \"append_row\", \"sheet_name\": \"Anole\", \"values\": ";
String payload = "";

// Google Sheets setup (do not edit)
const char* host = "script.google.com";
const int httpsPort = 443;
const char* fingerprint = "";
String url = String("/macros/s/") + GScriptId + "/exec";
HTTPSRedirect* client = nullptr;

void connectToWifi() {
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

void setup() {
  Serial.println("\nBeginning setup...");
  Serial.begin(74880); // Serial for debugging

  connectToWifi();

  pmsSerial.begin(9600); // PMS5003 operates at 9600 baud rate
  if (!pretendToWakeAndSleep) {
    // Start PMS5003 in sleep mode
    pinMode(PMS_SET_PIN, OUTPUT);
    digitalWrite(PMS_SET_PIN, LOW);
  }
  Serial.println("\nPMS5003 setup complete, and initialized in sleep mode.");
}

struct pms5003data {
  uint16_t framelen;
  uint16_t pm10_standard, pm25_standard, pm100_standard;
  uint16_t pm10_env, pm25_env, pm100_env;
  uint16_t particles_03um, particles_05um, particles_10um, particles_25um, particles_50um, particles_100um;
  uint16_t unused;
  uint16_t checksum;
};
 
struct pms5003data data;
unsigned long previousMillis = 0;
bool isAsleep = true;
int logCount = 0;
uint16_t curData[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

// notes: 
// the "warm up" time seems to be more about actually reading data from the stream?
// it's like we need to ignore the first ten data reads,
// which is 10 secs, but just calling delay(10*1000) isn't enough

void pmsWake() {
  // Wake up PMS5003 and then wait to stabilize
  if (!pretendToWakeAndSleep) {
    digitalWrite(PMS_SET_PIN, HIGH);
  }
  isAsleep = false;
  resetDataPoint();
}

void pmsSleep() {
  if (!pretendToWakeAndSleep) {
    digitalWrite(PMS_SET_PIN, LOW);
  }
  isAsleep = true;
  logCount = 0;
}

bool pmsAllEmptyData() {
  return curData[9] == 0;
}

void loop() {
  unsigned long currentMillis = millis();
  
  if (isAsleep && (currentMillis - previousMillis >= 1000*secsToSleepBetweenLogs)) {
    pmsWake();
    Serial.println("\nPMS5003 is awake.");

  } else if (!isAsleep) {
    // Read and print data from PMS5003
    if (readPMSdata(&pmsSerial)) {
      
      // ignore data until we've seen one nonzero value
      if (dataIsEmpty() && pmsAllEmptyData()) {
        // Serial.println("Empty data.");
      } else {
        addDataPoint();
        printData();
      }

      logCount++;
      if (logCount >= logsToAverageOver) {
        previousMillis = currentMillis;
        postToSheets();
        pmsSleep();
        Serial.println("\nPMS5003 is asleep.");
      }
    }
  }
}

bool dataIsEmpty() {
  return (data.particles_03um == 0) && (data.particles_05um == 0) && (data.particles_10um == 0) && (data.particles_25um == 0) && (data.particles_50um == 0) && (data.particles_100um == 0);
}

void resetDataPoint() {
  curData[0] = 0;
  curData[1] = 0;
  curData[2] = 0;
  curData[3] = 0;
  curData[4] = 0;
  curData[5] = 0;
  curData[6] = 0;
  curData[7] = 0;
  curData[8] = 0;
  curData[9] = 0;
}

void addDataPoint() {
  curData[0] += data.particles_03um;
  curData[1] += data.particles_05um;
  curData[2] += data.particles_10um;
  curData[3] += data.particles_25um;
  curData[4] += data.particles_50um;
  curData[5] += data.particles_100um;
  curData[6] += data.pm10_standard;
  curData[7] += data.pm25_standard;
  curData[8] += data.pm100_standard;
  curData[9] += 1;
}

void printData() {

  Serial.println("\n---------------------------------------");
  Serial.println(logCount);
  Serial.print("\nCurrent reading: "); Serial.print(" ");
  Serial.print(data.particles_03um); Serial.print(" ");
  Serial.print(data.particles_05um); Serial.print(" ");
  Serial.print(data.particles_10um); Serial.print(" ");
  Serial.print(data.particles_25um); Serial.print(" ");
  Serial.print(data.particles_50um); Serial.print(" ");
  Serial.print(data.particles_100um); Serial.print(" ");
  Serial.print(data.pm10_standard); Serial.print(" ");
  Serial.print(data.pm25_standard); Serial.print(" ");
  Serial.print(data.pm100_standard); Serial.print(" ");

  if (curData[9] > 0) {
    Serial.print("\nAverage reading: "); Serial.print(" ");
    Serial.print(curData[0] / curData[9]); Serial.print(" ");
    Serial.print(curData[1] / curData[9]); Serial.print(" ");
    Serial.print(curData[2] / curData[9]); Serial.print(" ");
    Serial.print(curData[3] / curData[9]); Serial.print(" ");
    Serial.print(curData[4] / curData[9]); Serial.print(" ");
    Serial.print(curData[5] / curData[9]); Serial.print(" ");
    Serial.print(curData[6] / curData[9]); Serial.print(" ");
    Serial.print(curData[7] / curData[9]); Serial.print(" ");
    Serial.print(curData[8] / curData[9]); Serial.print(" ");
    Serial.print("n="); Serial.print(curData[9]);
  }
}

void postToSheets() {
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
  payload = payload_base + "[" + curData[0]/curData[9] + "," + curData[1]/curData[9] + "," + curData[2]/curData[9] + "," + curData[3]/curData[9] + "," + curData[4]/curData[9] + "," + curData[5]/curData[9] + "," + curData[6]/curData[9] + "," + curData[7]/curData[9] + "," + curData[8]/curData[9] + "," + curData[9] + "]}";
  
  // Publish data to Google Sheets
  Serial.println("\nPublishing data...");
  Serial.println(payload);
  if(client->POST(url, host, payload)){ 
    // do stuff here if publish was successful
    Serial.println("Successfully published");
  }
  else{
    // do stuff here if publish was not successful
    Serial.println("Error while connecting");
  }
}

boolean readPMSdata(Stream *s) {
  if (! s->available()) {
    return false;
  }
  
  // Read a byte at a time until we get to the special '0x42' start-byte
  if (s->peek() != 0x42) {
    s->read();
    return false;
  }
 
  // Now read all 32 bytes
  if (s->available() < 32) {
    return false;
  }
    
  uint8_t buffer[32];    
  uint16_t sum = 0;
  s->readBytes(buffer, 32);
 
  // get checksum ready
  for (uint8_t i=0; i<30; i++) {
    sum += buffer[i];
  }
 
  /* debugging
  for (uint8_t i=2; i<32; i++) {
    Serial.print("0x"); Serial.print(buffer[i], HEX); Serial.print(", ");
  }
  Serial.println();
  */
  
  // The data comes in endian'd, this solves it so it works on all platforms
  uint16_t buffer_u16[15];
  for (uint8_t i=0; i<15; i++) {
    buffer_u16[i] = buffer[2 + i*2 + 1];
    buffer_u16[i] += (buffer[2 + i*2] << 8);
  }
 
  // put it into a nice struct :)
  memcpy((void *)&data, (void *)buffer_u16, 30);
 
  if (sum != data.checksum) {
    Serial.println("Checksum failure");
    return false;
  }
  // success!
  return true;
}
