#include <Adafruit_BME280.h>
#include <Adafruit_Sensor.h>
#include <BLE2902.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <Wire.h>

constexpr int SDA_PIN = 5;
constexpr int SCL_PIN = 4;
constexpr uint8_t BME_ADDR = 0x76;
constexpr unsigned long SAMPLE_INTERVAL_MS = 100;

constexpr char DEVICE_NAME[] = "Pas_Barometryczny";
constexpr char SERVICE_UUID[] = "4fafc201-1fb5-459e-8fcc-c5c9c331914b";
constexpr char CHARACTERISTIC_UUID[] = "beb5483e-36e1-4688-b7f5-ea07361b26a8";

Adafruit_BME280 bme;
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
  Wire.begin(SDA_PIN, SCL_PIN);
  if (!bme.begin(BME_ADDR)) {
    while (true) delay(10);
  }

  BLEDevice::init(DEVICE_NAME);
  BLEDevice::setMTU(512);
  server = BLEDevice::createServer();
  server->setCallbacks(new ServerCallbacks());

  BLEService* service = server->createService(SERVICE_UUID);
  characteristic = service->createCharacteristic(
      CHARACTERISTIC_UUID,
      BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
  characteristic->setValue("0.00");
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
    const float pressureHpa = bme.readPressure() / 100.0F;
    char payload[20];
    snprintf(payload, sizeof(payload), "%.2f", pressureHpa);
    characteristic->setValue(payload);
    characteristic->notify();
    Serial.println(payload);
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
