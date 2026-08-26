#include "HX711.h"
#include <BLE2902.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>

constexpr int LOAD_CELL_DOUT_PIN = 4;
constexpr int LOAD_CELL_SCK_PIN = 5;
constexpr unsigned long SAMPLE_INTERVAL_MS = 100;

constexpr char DEVICE_NAME[] = "Pas_Tensometryczny";
constexpr char SERVICE_UUID[] = "4fafc201-1fb5-459e-8fcc-c5c9c331914b";
constexpr char CHARACTERISTIC_UUID[] = "beb5483e-36e1-4688-b7f5-ea07361b26a8";

HX711 scale;
BLEServer* server = nullptr;
BLECharacteristic* characteristic = nullptr;
bool deviceConnected = false;
bool oldDeviceConnected = false;
unsigned long lastSampleTime = 0;

class ServerCallbacks : public BLEServerCallbacks {
 public:
  void onConnect(BLEServer*) override { deviceConnected = true; }
  void onDisconnect(BLEServer*) override { deviceConnected = false; }
};

void setup() {
  Serial.begin(115200);
  scale.begin(LOAD_CELL_DOUT_PIN, LOAD_CELL_SCK_PIN);
  scale.set_scale();
  scale.tare();

  BLEDevice::init(DEVICE_NAME);
  BLEDevice::setMTU(512);
  server = BLEDevice::createServer();
  server->setCallbacks(new ServerCallbacks());

  BLEService* service = server->createService(SERVICE_UUID);
  characteristic = service->createCharacteristic(
      CHARACTERISTIC_UUID,
      BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
  characteristic->setValue("0");
  characteristic->addDescriptor(new BLE2902());
  service->start();

  BLEAdvertising* advertising = BLEDevice::getAdvertising();
  advertising->addServiceUUID(SERVICE_UUID);
  advertising->setScanResponse(true);
  BLEAdvertisementData data;
  data.setFlags(0x04);
  data.setName(DEVICE_NAME);
  advertising->setAdvertisementData(data);
  advertising->start();
}

void loop() {
  const unsigned long now = millis();
  if (deviceConnected && now - lastSampleTime >= SAMPLE_INTERVAL_MS) {
    lastSampleTime = now;
    if (scale.is_ready()) {
      const long raw = scale.read();
      char payload[20];
      snprintf(payload, sizeof(payload), "%ld", raw);
      characteristic->setValue(payload);
      characteristic->notify();
      Serial.println(payload);
    }
  }

  if (!deviceConnected && oldDeviceConnected) {
    delay(500);
    server->startAdvertising();
    oldDeviceConnected = false;
  }
  if (deviceConnected && !oldDeviceConnected) {
    oldDeviceConnected = true;
  }
  delay(5);
}
