/*
  Main server, AP setup, serial to PC
*/

#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>

template<typename Data>
class Vector {
   size_t d_size;
   size_t d_capacity;
   Data *d_data;
   public:
     Vector() : d_size(0), d_capacity(0), d_data(0) {};
     Vector(Vector const &other) : d_size(other.d_size), d_capacity(other.d_capacity), d_data(0) { d_data = (Data *)malloc(d_capacity*sizeof(Data)); memcpy(d_data, other.d_data, d_size*sizeof(Data)); };
     ~Vector() { free(d_data); }; // Destructor
     Vector &operator=(Vector const &other) { free(d_data); d_size = other.d_size; d_capacity = other.d_capacity; d_data = (Data *)malloc(d_capacity*sizeof(Data)); memcpy(d_data, other.d_data, d_size*sizeof(Data)); return *this; };
     void push_back(Data const &x) {
      for(size_t i = 0; i < d_size;i ++)
      {
        if(x.id == d_data[i].id){
        d_data[i] = x;
        return;
        }
      }
        if (d_capacity == d_size) resize();
        d_data[d_size++] = x;   
      };
     size_t size() const { return d_size; };
     Data const &operator[](size_t idx) const { return d_data[idx]; };
     Data &operator[](size_t idx) { return d_data[idx]; };
   private:
     void resize() { d_capacity = d_capacity ? d_capacity*2 : 1; Data *newdata = (Data *)malloc(d_capacity*sizeof(Data)); memcpy(newdata, d_data, d_size * sizeof(Data)); free(d_data); d_data = newdata; };
};

const char *ssid = "ESP32Shap";
const char *password = "123456789";

const int port = 50000;
WebServer server(port);

struct CLIENT{
  bool bright;
  float temp;
  float hum;
  int id;
};

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

String printCLIENTS(Vector<CLIENT>& someCLIENTS){
  String temp;
  temp+="CLIENTS:\n";
  for (size_t i =0; i < someCLIENTS.size();i++){
  temp+=printCLIENT(someCLIENTS[i]) + "\n";
  }
  return temp;
}

int loss = 0;
int total_loss = 0;

int getStats(Vector<CLIENT>& someCLIENTS, Vector<CLIENT>& lastCLIENTS){

  int temp = 0;
  
  for (size_t i =0; i < someCLIENTS.size();i++){
    for (size_t j =0; j < lastCLIENTS.size();j++){
      if(((lastCLIENTS[j].id == 3) && (someCLIENTS[i].id == 3)) || ((lastCLIENTS[j].id == 4) && (someCLIENTS[i].id == 4))){
        if(lastCLIENTS[j].hum == someCLIENTS[i].hum)
          temp++;
      }
    }
  }
  
  if(temp!=0)
    loss += temp;
  else
    loss = 0;

  total_loss += temp;
  return loss;
}

Vector<CLIENT> CLIENTS;
Vector<CLIENT> CLIENTS_LAST;

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
    <h1>Hello from Top Server!</h1>\
    <p>Uptime: %02d:%02d:%02d</p>\
  </body>\
</html>",

           hr, min % 60, sec % 60
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

int requests1 = 0;
int requests2 = 0;

void handleData() {

  if(server.args()!=0){
    
    String message = "Collected data is:\n";
    
    for (uint8_t i = 0; i < server.args(); i++) {
      message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
    }

    uint8_t lastArg = server.args();
    if(server.argName(lastArg) == "requests1")
      requests1 = (server.arg(lastArg)).toInt();
    else
      requests2 = (server.arg(lastArg)).toInt();

    if( requests1 > requests2 ){
      message += " suggested IP:" + String("132");
      requests1 -= requests2;      
    }
    else if( requests1 < requests2 ){
      message += " suggested IP:" + String("131");
      requests2 -= requests1; 
    }
    else
      message += " suggested IP:" + (((bool)(esp_random()%2)) ? String("131") : String("132"));
    
    for(size_t i = 0; i < server.args() / 4; i++){
      CLIENT newClient;
      newClient.id = server.arg((i*4)+0).toInt();
      newClient.temp = server.arg((i*4)+1).toFloat();
      newClient.hum = server.arg((i*4)+2).toFloat();
      newClient.bright = server.arg((i*4)+3).toInt();
      CLIENTS.push_back(newClient);
    }
    
    server.send(200, "text/plain", message);
    
  }
  else{
  server.send(200, "text/plain", printCLIENTS(CLIENTS));
  }

}

void setup() {

  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.softAP(ssid, password,8,0,8);
  Serial.println("");

  Serial.print("AP IP address: ");
  Serial.println(WiFi.softAPIP());
  
  Serial.println("");
  
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.print("Server port: ");
  Serial.println(port);

  server.on("/", handleRoot);
  server.on("/data", handleData);
  server.onNotFound(handleNotFound);
  
  server.begin();
  Serial.println("HTTP server started");
}

unsigned long previousMillisServer = 0; 
unsigned long previousMillisPrint = 0; 
unsigned long previousMillisStat = 0; 

void loop() {

  if (millis() - previousMillisServer >= 50) {
    previousMillisServer = millis();
    
    server.handleClient();
  }
  
  if (millis() - previousMillisPrint >= 2000) {
    previousMillisPrint = millis();

    Serial.println(printCLIENTS(CLIENTS));
  }

  if (millis() - previousMillisStat >= 375) {
    previousMillisStat = millis();

    Serial.println(total_loss);
    //Serial.println(getStats(CLIENTS,CLIENTS_LAST));
    CLIENTS_LAST=Vector<CLIENT>(CLIENTS);
  }
}
