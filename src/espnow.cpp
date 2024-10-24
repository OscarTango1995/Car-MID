#include <esp_now.h>
#include <WiFi.h>
extern "C"
{
#include "esp_wifi.h"
}
#include "espnow.h"
#include <sd.h>

#define STATUS_CMD 0xA5 // same as immobilizer expects
#define PARK_STATUS_REPORT_CMD 0xB2

extern StatusData lastStatus;
extern bool displayStatusReceived;
extern bool checkStatusReceived;
static uint8_t lastCommand = 0x00;
static uint8_t lastSequence = 0;
extern bool isPark;

struct CommandPacket
{
  uint8_t cmd;
  uint8_t seqNum;
  uint8_t payload;
};

typedef struct __attribute__((packed)) {
    float fuelUsedLiters;
} FuelUsagePacket;
FuelUsagePacket latestFuelUsage; // Global or static storage

uint8_t immobilizerMac[] = {0x6C, 0xC8, 0x40, 0x07, 0x27, 0x80}; // replace with real MAC

void sendStatusRequest()
{
  CommandPacket packet = {STATUS_CMD, ++lastSequence};
  esp_err_t result = esp_now_send(immobilizerMac, (uint8_t *)&packet, sizeof(packet));
  if (result == ESP_OK)
  {
    Serial.println("Status request sent to immobilizer");
  }
  else
  {
    Serial.println("Failed to send status request");
  }
}

void sendParkStatus()
{
  CommandPacket packet;
  packet.cmd = PARK_STATUS_REPORT_CMD;
  packet.seqNum = 0; // You can increment if you’re tracking
  packet.payload = isPark ? 1 : 0;

  esp_err_t result = esp_now_send(immobilizerMac, (uint8_t *)&packet, sizeof(packet));
  if (result == ESP_OK)
  {
    Serial.println("Parking status sent");
  }
  else
  {
    Serial.println("Failed to send Parking status");
  }
}

void onReceive(const uint8_t *mac, const uint8_t *incomingData, int len)
{
  if (len == sizeof(StatusData))
  {
    Serial.print("Status Data Received");
    memcpy(&lastStatus, incomingData, sizeof(StatusData));
    displayStatusReceived = true; // Signal main loop
    checkStatusReceived = true;   // Signal main loop
  }
  else if (len == sizeof(FuelUsagePacket))
  {
    memcpy(&latestFuelUsage, incomingData, sizeof(FuelUsagePacket));
    Serial.printf("Fuel Data Received: %.3f L\n", latestFuelUsage.fuelUsedLiters);
    updateTripTotalToCSV(latestFuelUsage.fuelUsedLiters);
  }
  else
  {
    Serial.println("Unknown data format received.");
  }
}

void initESPNow()
{
  WiFi.mode(WIFI_STA);
  esp_wifi_set_channel(0, WIFI_SECOND_CHAN_NONE); // Must match the immobilizer's channel

  if (esp_now_init() != ESP_OK)
  {
    Serial.println("ESP-NOW init failed");
    return;
  }

  esp_now_register_recv_cb(onReceive);

  // Add peer: the immobilizer
  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, immobilizerMac, 6);
  peerInfo.channel = 0; // same channel
  peerInfo.encrypt = false;

  if (!esp_now_is_peer_exist(immobilizerMac))
  {
    if (esp_now_add_peer(&peerInfo) != ESP_OK)
    {
      Serial.println("Failed to add peer (immobilizer)");
    }
  }
}
