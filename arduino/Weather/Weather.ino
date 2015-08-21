//#include <Wire.h>
#include <SPI.h>
//#include <Adafruit_Sensor.h>
//#include <Adafruit_BME280.h>
#include <BLEPeripheral.h>

// define pins for Adafruit Bluefruit LE
// https://github.com/sandeepmistry/arduino-BLEPeripheral#pinouts
#define BLE_REQ 10
#define BLE_RDY 2
#define BLE_RST 9

BLEPeripheral blePeripheral = BLEPeripheral(BLE_REQ, BLE_RDY, BLE_RST);
BLEService weatherService = BLEService("BBB0");
BLEFloatCharacteristic temperatureCharacteristic = BLEFloatCharacteristic("BBB1", BLERead | BLENotify);
BLEDescriptor temperatureDescriptor = BLEDescriptor("2901", "Temp");
BLEFloatCharacteristic humidityCharacteristic = BLEFloatCharacteristic("BBB2", BLERead | BLENotify);
BLEDescriptor humidityDescriptor = BLEDescriptor("2901", "Humidity");
BLEFloatCharacteristic pressureCharacteristic = BLEFloatCharacteristic("BBB3", BLERead | BLENotify);
BLEDescriptor pressureDescriptor = BLEDescriptor("2901", "Pressure");

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
Adafruit_BME280 bme;

long previousMillis = 0;  // stores the last time sensor was read
long interval = 2000;     // interval at which to read sensor (milliseconds)

void setup()
{
  Serial.begin(9600);
  Serial.println(F("Bluetooth Low Energy Weather Station"));

  // set advertised name and service
  blePeripheral.setLocalName("Weather");
  blePeripheral.setDeviceName("Weather");
  blePeripheral.setAdvertisedServiceUuid(weatherService.uuid());

  // add service and characteristic
  blePeripheral.addAttribute(weatherService);
  blePeripheral.addAttribute(temperatureCharacteristic);
  blePeripheral.addAttribute(temperatureDescriptor);
  blePeripheral.addAttribute(humidityCharacteristic);
  blePeripheral.addAttribute(humidityDescriptor);
  blePeripheral.addAttribute(pressureCharacteristic);
  blePeripheral.addAttribute(pressureDescriptor);

  blePeripheral.begin();
  //bme.begin();
  if (!bme.begin()) {  
    Serial.println(F("Could not find a valid BME280 sensor, check wiring!"));
    while (1);
  }
}

void loop()
{
  // Tell the bluetooth radio to do whatever it should be working on
  blePeripheral.poll();

  // limit how often we read the sensor
  if (millis() - previousMillis > interval) {
    Serial.print("*");
    pollSensors();
    previousMillis = millis();
  }
}

void pollSensors()
{

  float temperature = bme.readTemperature();
  float humidity = bme.readHumidity();
  float pressure = bme.readPressure();

  // only set the characteristic value if the temperature has changed
  if (!isnan(temperature) && temperatureCharacteristic.value() != temperature) {
    temperatureCharacteristic.setValue(temperature);
    Serial.print(F("Temperature "));
    Serial.println(temperature);
  }

  // only set the characteristic value if the humidity has changed
  if (!isnan(humidity) && humidityCharacteristic.value() != humidity) {
    humidityCharacteristic.setValue(humidity);
    Serial.print(F("Humidity "));
    Serial.println(humidity);
  }

  // only set the characteristic value if the pressure has changed
  if (!isnan(pressure) && pressureCharacteristic.value() != pressure) {
    pressureCharacteristic.setValue(pressure);
    Serial.print(F("Pressure "));
    Serial.println(pressure);
  }

}

