// Complete Instructions to Get and Change ESP MAC Address: https://RandomNerdTutorials.com/get-change-esp32-esp8266-mac-address-arduino/

#include <esp_now.h>
#include <WiFi.h>
#include <WebServer.h>

/* Put your SSID & Password */
const char* ssid = "ESP32";  // Enter SSID here
const char* password = "12345678";  //Enter Password here

/* Put IP Address details */
IPAddress local_ip(192,168,1,1);
IPAddress gateway(192,168,1,1);
IPAddress subnet(255,255,255,0);

WebServer server(80);

uint8_t broadcastAddress[] = {0xB0, 0xA7, 0x32, 0x23, 0x5A, 0xF0};

bool Relay3 = false;
bool Relay4 = false;

bool ChangeRelay3;
bool ChangeRelay4;

// Variable to store if sending data was successful
String success;




//Structure example to send data
//Must match the receiver structure
typedef struct struct_message {
bool CR3;
bool CR4;
} struct_message;

typedef struct struct_message_temp {
float Temp;
float humid;
} struct_message_temp;


// Create a struct_message to hold relay data
struct_message relayData;

// Create a struct_message to hold incoming relay change
struct_message incomingRelayChange;

struct_message_temp incommingHumidTempChange;



esp_now_peer_info_t peerInfo;


void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
  if (status ==0){
    success = "Delivery Success :)";
  }
  else{
    success = "Delivery Fail :(";
  }
}

void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&incomingRelayChange, incomingData, sizeof(incomingRelayChange));
  Serial.print("Bytes received: ");
  Serial.println(len);
  ChangeRelay3 = incomingRelayChange.CR3;
  ChangeRelay4 = incomingRelayChange.CR4;
}

void setup() {
  // Init Serial Monitor
  Serial.begin(115200);

// Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);
  WiFi.softAP(ssid, password);
  WiFi.softAPConfig(local_ip, gateway, subnet);
  
  server.on("/", handle_OnConnect);
  server.on("/relayClose", handle_relayClose);
  server.on("/relayOpen", handle_relayOpen);

  server.onNotFound(handle_NotFound);
  
  server.begin();
  Serial.println("HTTP server started");

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

    // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_register_send_cb(OnDataSent);

// Register peer
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;


  // Add peer        
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
  // Register for a callback function that will be called when data is received
  esp_now_register_recv_cb(OnDataRecv);
}


void loop() {
  server.handleClient();

  relayData.CR3 = Relay3;
  relayData.CR4 = Relay4;
  Serial.println("Relay3:" + String(Relay3));
  Serial.println("Relay4:" + String(Relay4));

// Send message via ESP-NOW
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &relayData, sizeof(relayData));
  if (result == ESP_OK) {
    Serial.println("Sent with success");
  }
  else {
    Serial.println("Error sending the data");
  }
  delay(1000);
}

void handle_OnConnect() {
  relayData.CR3 = 0;
  relayData.CR4 = 0;
  Serial.println("GPIO4 Status: OFF | GPIO5 Status: OFF");
  server.send(200, "text/html", SendHTML(true)); 
}
void handle_relayClose() {
  Relay3 = 1;
  Relay4 = 0;
  Serial.println("GPIO4 Status: ON");
  server.send(200, "text/html", SendHTML(true)); 
}

void handle_relayOpen() {
  Relay3 = 0;
  Relay4 = 1;
  Serial.println("GPIO4 Status: OFF");
  server.send(200, "text/html", SendHTML(false)); 
}


void handle_NotFound(){
  server.send(404, "text/plain", "Not found");
}

String SendHTML(uint8_t led1stat){
  String ptr = "<!DOCTYPE html> <html>\n";
  ptr +="<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  ptr +="<title>LED Control</title>\n";
  ptr +="<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n";
  ptr +="body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;} h3 {color: #444444;margin-bottom: 50px;}\n";
  ptr +=".button {display: block;width: 80px;background-color: #3498db;border: none;color: white;padding: 13px 30px;text-decoration: none;font-size: 25px;margin: 0px auto 35px;cursor: pointer;border-radius: 4px;}\n";
  ptr +=".button-on {background-color: #3498db;}\n";
  ptr +=".button-on:active {background-color: #2980b9;}\n";
  ptr +=".button-off {background-color: #34495e;}\n";
  ptr +=".button-off:active {background-color: #2c3e50;}\n";
  ptr +="p {font-size: 14px;color: #888;margin-bottom: 10px;}\n";
  ptr +="</style>\n";
  ptr +="</head>\n";
  ptr +="<body>\n";
  ptr +="<h1>ESP32 Web Server</h1>\n";
  ptr +="<h3>Using Access Point(AP) Mode</h3>\n";
  
   if(led1stat)
  {ptr +="<p>window Status: closed</p><a class=\"button button-off\" href=\"/relayOpen\">Closed</a>\n";}
  else
  {ptr +="<p>window Status: open</p><a class=\"button button-on\" href=\"/relayClose\">Open</a>\n";}


  ptr +="</body>\n";
  ptr +="</html>\n";
  return ptr;
}
