/*
  Middle server, serial to PC
*/

#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <HTTPClient.h>

#define HARDCODEID 0

template<typename Data>
class Vector {
  size_t d_size;
  size_t d_capacity;
  Data *d_data;
  public:
    Vector() : d_size(0), d_capacity(0), d_data(0) {};
    Vector(Vector const &other) : d_size(other.d_size), d_capacity(other.d_capacity), d_data(0) { d_data = (Data *)malloc(d_capacity*sizeof(Data)); memcpy(d_data, other.d_data, d_size*sizeof(Data)); };
    ~Vector() { free(d_data); };
    Vector &operator=(Vector const &other) { free(d_data); d_size = other.d_size; d_capacity = other.d_capacity; d_data = (Data *)malloc(d_capacity*sizeof(Data)); memcpy(d_data, other.d_data, d_size*sizeof(Data)); return *this; };
    void push_back(Data const &x) {
      for(size_t i = 0; i < d_size;i ++){
        if(x.id == d_data[i].id){
          d_data[i] = x;
          return;
        }
      }
      if (d_capacity == d_size)
        resize();
      d_data[d_size++] = x;   
    }; 
    size_t size() const { return d_size; };
    Data const &operator[](size_t idx) const { return d_data[idx]; };
    Data &operator[](size_t idx) { return d_data[idx]; };
  private:
    void resize() { d_capacity = d_capacity ? d_capacity*2 : 1; Data *newdata = (Data *)malloc(d_capacity*sizeof(Data)); memcpy(newdata, d_data, d_size * sizeof(Data)); free(d_data); d_data = newdata; };
};

struct CLIENT{
  bool bright;
  float temp;
  float hum;
  int id;
};

Vector<CLIENT> CLIENTS;

const char *ssid = "ESP32Shap";
const char *password = "123456789";

IPAddress staticIP(192, 168, 4, 130 + HARDCODEID);
IPAddress gateway(192, 168, 4, 1);
IPAddress subnet(255, 255, 0, 0);

String serverName = "http://192.168.4.1:50000/data";
int suggestedIP = 131;

WebServer server(50000);

String printCLIENT(CLIENT someClient){
  String temp;
  temp+="Station id is ";
  temp+=someClient.id;
  temp+=", temperature is ";
  temp+=someClient.temp;
  temp+=", humidity is ";
  temp+=someClient.hum;
  temp+=", there is ";
  temp+=(someClient.bright) ? "bright" : "dark";
  return temp;
}

String printCLIENTS(const Vector<CLIENT>& someCLIENTS){
  String temp;
  temp+="CLIENTS:\n";
  for (size_t i =0; i < someCLIENTS.size();i++){
    temp+=printCLIENT(someCLIENTS[i]) + "\n";
  }
  return temp;
}

void handleRoot() {
  char temp[400];
  int sec = millis() / 1000;
  int min = sec / 60;
  int hr = min / 60;

  snprintf(temp, 400,

           "<html>\
  <head>\
    <meta http-equiv='refresh' content='5'/>\
    <title>Middle Server</title>\
    <style>\
      body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\
    </style>\
  </head>\
  <body>\
    <h1>Hello from Middle Server #%d!</h1>\
    <p>Uptime: %02d:%02d:%02d</p>\
  </body>\
</html>",

           HARDCODEID, hr, min % 60, sec % 60
          );
  server.send(200, "text/html", temp);
}

void handleNotFound() {
  String message = "Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";

  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }

  server.send(404, "text/plain", message);
  Serial.println(message);
}

int requests = 0;

void handleData(){

  if(server.args()!=0){
    
    String message = "Collected data is:\n";
    
    for (uint8_t i = 0; i < server.args(); i++) {
      message += " " + server.argName(i) + ":" + server.arg(i) + "\n";
    }
    message += " suggested IP:" + String(suggestedIP);
    
    CLIENT newClient;
    newClient.id = server.arg(0).toInt();
    newClient.temp = server.arg(1).toFloat();
    newClient.hum = server.arg(2).toFloat();
    newClient.bright = server.arg(3).toInt();
    CLIENTS.push_back(newClient);
    
    server.send(200, "text/plain", message);
    Serial.println(message);
    requests++;
  }
  else{
    server.send(200, "text/plain", printCLIENTS(CLIENTS));
  }
}

String parsePayload(String& payload){
  String temp = payload.substring(payload.lastIndexOf(":")+1);
  temp.trim();
  return temp;
}

void sendData(Vector<CLIENT>& someCLIENTS){
  if(someCLIENTS.size() != 0){
    if(WiFi.status()== WL_CONNECTED){
      HTTPClient http;
      String serverPath = serverName;
    
      for(size_t i = 0; i < someCLIENTS.size(); i++){
        serverPath += "&id=" + String(someCLIENTS[i].id);
        serverPath += "&temp=" + String(someCLIENTS[i].temp);
        serverPath += "&hum=" + String(someCLIENTS[i].hum);
        serverPath += "&bright=" + String(someCLIENTS[i].bright) ;
      }
      
      serverPath += "&requests" + String(HARDCODEID) + "=" + String(requests);
        serverPath.setCharAt(serverPath.indexOf("&"),'?');
      
      Serial.print("URL: ");
      Serial.println(serverPath);
    
      http.begin(serverPath.c_str());
      
      int httpResponseCode = http.GET();
      
      if (httpResponseCode>0) {
        Serial.print("HTTP Response code: ");
        Serial.println(httpResponseCode);
        String payload = http.getString();
        Serial.println(payload);
        
        requests = 0;
        someCLIENTS = Vector<CLIENT>();
        suggestedIP = parsePayload(payload).toInt();
      }
      else {
        Serial.print("Error code: ");
        Serial.println(httpResponseCode);
      }
      // Free resources
      http.end();
    }
    else {
      Serial.println("WiFi Disconnected");
    }
  }
}

void setup() {

  Serial.begin(115200);
  
  //Attempt to get sitatic IP
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

  server.on("/", handleRoot);
  server.on("/data", handleData);
  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");
}

unsigned long previousMillisBot = 0; 
unsigned long previousMillisTop = 0; 
unsigned long previousMillisPrint = 0; 

void loop() {

  if (millis() - previousMillisBot >= 10) {
    previousMillisBot = millis();
    server.handleClient();
  }

  if (millis() - previousMillisTop >= 50) {
    previousMillisTop = millis();
    sendData(CLIENTS);
  }
  
  if (millis() - previousMillisPrint >= 100) {
    previousMillisPrint = millis();
    Serial.println(printCLIENTS(CLIENTS));
  }
  
}
