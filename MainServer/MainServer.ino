// Complete Instructions to Get and Change ESP MAC Address: https://RandomNerdTutorials.com/get-change-esp32-esp8266-mac-address-arduino/

#include <esp_now.h>
#include <WiFi.h>

uint8_t broadcastAddress[] = {0xB0, 0xA7, 0x32, 0x22, 0x58, 0x5C};

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


// Create a struct_message to hold relay data
struct_message relayData;

// Create a struct_message to hold incoming relay change
struct_message incomingRelayChange;



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

  pinMode(22, OUTPUT);
  pinMode(23, OUTPUT);
}


void loop() {
  getReadings();
  Serial.println(String(ChangeRelay3) + " :ChangeRelay3");
  Serial.println(String(ChangeRelay4) + " :ChangeRelay4");
  if (ChangeRelay3){
    digitalWrite(23, HIGH);
    digitalWrite(22, HIGH);
  } else{
    digitalWrite(23, LOW);
    digitalWrite(22, LOW);
  }


  delay(1000);
}

void getReadings(){
  Relay3 = incomingRelayChange.CR3;
  Relay4 = incomingRelayChange.CR4;
}
