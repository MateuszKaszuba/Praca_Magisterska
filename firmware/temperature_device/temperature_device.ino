#include <Adafruit_BME280.h>
#include <BLE2902.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <Wire.h>

constexpr int SDA_PIN = 8;
constexpr int SCL_PIN = 9;
constexpr uint8_t TCA_ADDR = 0x70;
constexpr uint8_t BME_ADDR = 0x76;
constexpr uint8_t SENSOR_CHANNELS[4] = {0, 1, 2, 7};
constexpr unsigned long SAMPLE_INTERVAL_MS = 100;

constexpr char DEVICE_NAME[] = "ESP32S3_BME280_4x";
constexpr char SERVICE_UUID[] = "4fafc201-1fb5-459e-8fcc-c5c9c331914b";
constexpr char CHARACTERISTIC_UUID[] = "beb5483e-36e1-4688-b7f5-ea07361b26a8";

Adafruit_BME280 sensors[4];
bool sensorAvailable[4] = {false, false, false, false};
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

void selectMultiplexerChannel(uint8_t channel) {
  if (channel > 7) return;
  Wire.beginTransmission(TCA_ADDR);
  Wire.write(1 << channel);
  Wire.endTransmission();
}

float readTemperature(uint8_t sensorIndex) {
  if (!sensorAvailable[sensorIndex]) return NAN;
  selectMultiplexerChannel(SENSOR_CHANNELS[sensorIndex]);
  return sensors[sensorIndex].readTemperature();
}

float readHumidity(uint8_t sensorIndex) {
  if (!sensorAvailable[sensorIndex]) return NAN;
  selectMultiplexerChannel(SENSOR_CHANNELS[sensorIndex]);
  return sensors[sensorIndex].readHumidity();
}

void setup() {
  Serial.begin(115200);
  Wire.begin(SDA_PIN, SCL_PIN);

  for (uint8_t sensorIndex = 0; sensorIndex < 4; ++sensorIndex) {
    selectMultiplexerChannel(SENSOR_CHANNELS[sensorIndex]);
    sensorAvailable[sensorIndex] = sensors[sensorIndex].begin(BME_ADDR, &Wire);
  }

  BLEDevice::init(DEVICE_NAME);
  BLEDevice::setMTU(512);
  server = BLEDevice::createServer();
  server->setCallbacks(new ServerCallbacks());

  BLEService* service = server->createService(SERVICE_UUID);
  characteristic = service->createCharacteristic(
      CHARACTERISTIC_UUID,
      BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
  characteristic->setValue("nan,nan,nan,nan,nan,nan,nan,nan");
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

    // t0/h0: usta, t1/h1: prawe nozdrze, t2/h2: lewe nozdrze,
    // tAmb/hAmb: czujnik odniesienia.
    const float t0 = readTemperature(0);
    const float h0 = readHumidity(0);
    const float t1 = readTemperature(1);
    const float h1 = readHumidity(1);
    const float t2 = readTemperature(2);
    const float h2 = readHumidity(2);
    const float tAmbient = readTemperature(3);
    const float hAmbient = readHumidity(3);

    char payload[128];
    snprintf(payload, sizeof(payload), "%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f",
             t0, h0, t1, h1, t2, h2, tAmbient, hAmbient);
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
