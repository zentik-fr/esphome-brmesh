#include <BLEDevice.h>
#include <BLEScan.h>

BLEScan* pBLEScan;

// optionnel : filtre RSSI pour réduire le bruit
const int RSSI_MIN = -55;
unsigned int lasttime = millis();

// Cherche dans l'advertising un AD structure Manufacturer (0xFF) avec Company ID = 0xFFF0 (bytes F0 FF)
bool hasCompanyIdFFF0(const uint8_t* payload, size_t len) {
  size_t i = 0;

  while (i < len) {
    uint8_t adLen = payload[i];
    if (adLen == 0) break;                 // fin
    if (i + adLen >= len) break;           // sécurité

    uint8_t adType = payload[i + 1];

    // Manufacturer Specific Data
    if (adType == 0xFF) {
      // format: [len][type=FF][company_id_L][company_id_H]...
      if (adLen >= 4) {
        uint8_t cidL = payload[i + 2];
        uint8_t cidH = payload[i + 3];
        if (cidL == 0xF0 && cidH == 0xFF) return true;
      }
    }

    i += (adLen + 1);
  }

  return false;
}

class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice d) {
    int rssi = d.getRSSI();
    if (rssi < RSSI_MIN) return;

    uint8_t* p = d.getPayload();
    size_t n = d.getPayloadLength();

    if (!hasCompanyIdFFF0(p, n)) return;

    Serial.print("🎯 MATCH F0 FF - ");
    Serial.println(millis() - lasttime);
    lasttime=millis();
    
    Serial.print(".                    RAW: ");
    for (size_t i = 0; i < n; i++) Serial.printf("%02X ", p[i]);
    Serial.print("\n------------------\n");
  }
};

void setup() {
  Serial.begin(500000);
  BLEDevice::init("");

  pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks(), true); // duplicates
  pBLEScan->setActiveScan(false); // en général suffisant pour manufacturer data
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(90);
}

void loop() {
  pBLEScan->start(5, false);
  pBLEScan->clearResults();
}
