#include <WiFi.h>
#include <HTTPClient.h>

#define HARDCODEID 4
//Sensors emulation
#define VIRTUAL

#define INTERRUPTPIN 13
#define INTERRUPTLEDPIN 27

String garbageString;

const char* ssid = "ESP32Shap";
const char* password = "123456789";

//Variables to get static IP
IPAddress staticIP(192, 168, 4, 150 + HARDCODEID);
IPAddress gateway(192, 168, 4, 1);
IPAddress subnet(255, 255, 0, 0);

// Set timer to 100ms (100)
unsigned long defaultDelay = 100;
unsigned long timerDelay = 100;
bool debug=0;

#ifndef VIRTUAL

    #include "DHT.h"
      
    //DHT pin and type
    #define DHTPIN 4     // GPIO 4, 5, 12, 13 or 14
    #define DHTTYPE DHT11
    
    //DHT initialization
    DHT dht(DHTPIN, DHTTYPE);
    
    //GPIO Light sensor pin
    #define LIGHTPIN 5  // GPIO 4, 5, 12, 13 or 14

    float getHum(){
      if(!isnan(dht.readHumidity()))
        return dht.readHumidity();
      else{
        Serial.println(F("Failed to read humidity from DHT sensor!"));
        return 0.0;
      }
    }
    
    float getTemp(){
      if(!isnan(dht.readTemperature()))
        return dht.readTemperature();
      else{
        Serial.println(F("Failed to read temperature from DHT sensor!"));
        return 0.0;
      }
    }
    
    bool getLight(){
        return digitalRead(LIGHTPIN)?0:1;
    }
    
#endif

#ifdef VIRTUAL
  
  float getHum(){
    return ((float)(esp_random()%5900)+2000.00)/100.00;
  }
  
  float getTemp(){
    return ((float)(esp_random()%700)+2000.00)/100.00;
  }
  
  bool getLight(){
    return (bool)(esp_random()%2);
  }
  
#endif

void IRAM_ATTR doFlood() {
  if(timerDelay == 0){
    debug=0;
    timerDelay=defaultDelay;
    digitalWrite(INTERRUPTLEDPIN,LOW);
  }
  else{
    debug=1;
    timerDelay=0;
    digitalWrite(INTERRUPTLEDPIN,HIGH);
  }
}

//Destination URL
String start_URL = "http://192.168.4.";
String default_mid_URL = "131";
String end_URL = ":50000/data";
String suggested_mid_URL = "";
int lastResponse = 0;

String constructURL(){
  String temp = start_URL;
  if((lastResponse == 200) && (suggested_mid_URL != ""))
    temp += suggested_mid_URL;
  else
    temp += default_mid_URL;
  temp += end_URL;
  return temp;
}

String parsePayload(String& payload){
  String temp = payload.substring(payload.lastIndexOf(":")+1);
  temp.trim();
  return temp;
}

void sendData(bool DEBUG){
  if(!DEBUG){
  //Check WiFi connection status
    if(WiFi.status()== WL_CONNECTED){
      HTTPClient http;

      String serverPath = constructURL();
      serverPath += "?id=" + String(HARDCODEID);
      serverPath += "&temp=" + String(getTemp());
      serverPath += "&hum=" + String(getHum());
      serverPath += "&bright=" + String(getLight());
      
      Serial.print("\nURL: ");
      Serial.println(serverPath);
      
      //Your Domain name with URL path or IP address with path
      http.begin(serverPath.c_str());
      
      //Send HTTP GET request
      int httpResponseCode = http.GET();
      lastResponse = httpResponseCode;
      
      if (httpResponseCode>0) {
        String payload = http.getString();
        Serial.print("HTTP Response code: ");
        Serial.println(httpResponseCode);
        Serial.println(payload);
        suggested_mid_URL = parsePayload(payload);
      }
      else {
        Serial.print("Error code: ");
        Serial.println(httpResponseCode);
      }
      http.end();   
    }
    else {
      Serial.println("WiFi Disconnected");
    }
  }
  else{
    HTTPClient http;
    String serverPath = constructURL();
    http.begin(serverPath.c_str());
    http.sendRequest("GET", garbageString);
  }
}

void setup() {
  Serial.begin(115200); 
  // Configures static IP address
  if (!WiFi.config(staticIP, gateway, subnet)) {
    Serial.println("STA Failed to configure");
  }
  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());

  pinMode(INTERRUPTLEDPIN,OUTPUT);


  attachInterrupt(INTERRUPTPIN, doFlood, CHANGE);

  for(size_t i=0;i<100;i++)
  garbageString+="123456789";
  
}

void loop() {

  sendData(debug);
  delay(timerDelay);
  
}
