// Weather is a bad name unless we add Barometric pressure

#include <SPI.h>
#include <BLEPeripheral.h>

// define pins for Adafruit Bluefruit LE 
// https://github.com/sandeepmistry/arduino-BLEPeripheral#pinouts
#define BLE_REQ 10
#define BLE_RDY 2
#define BLE_RST 9

BLEPeripheral blePeripheral = BLEPeripheral(BLE_REQ, BLE_RDY, BLE_RST);
BLEService weatherService = BLEService("BBB0");
// TODO normalize descriptor text Temperature, Humidity, Pressure
// TODO are there standard Bluetooth descriptors for units?
BLEFloatCharacteristic temperatureCharacteristic = BLEFloatCharacteristic("BBB1", BLERead | BLENotify);
BLEDescriptor temperatureDescriptor = BLEDescriptor("2901", "Temperature");
BLEFloatCharacteristic humidityCharacteristic = BLEFloatCharacteristic("BBB2", BLERead | BLENotify);
BLEDescriptor humidityDescriptor = BLEDescriptor("2901", "Humidity");
BLEFloatCharacteristic pressureCharacteristic = BLEFloatCharacteristic("BBB3", BLERead | BLENotify);
BLEDescriptor pressureDescriptor = BLEDescriptor("2901", "Pressure");

#include "DHT.h"
#define DHTPIN 7        // what pin we're connected to
#define DHTTYPE DHT22   // DHT 22  (AM2302)
DHT dht(DHTPIN, DHTTYPE);

// TODO add pressure sensor

long previousMillis = 0;  // will store last time sensor was read
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
}

void loop()
{
  // Tell the bluetooth radio to do whatever it should be working on
  blePeripheral.poll();
  
  // limit how often we read the sensor
  if(millis() - previousMillis > interval) {
    pollSensor();
    previousMillis = millis();
  }
}

void pollSensor()
{
  
  float temperature = dht.readTemperature(true);
  float humidity = dht.readHumidity();

  // only set the characteristic value if the temperature has changed
  if (temperatureCharacteristic.value() != temperature) {
    temperatureCharacteristic.setValue(temperature);
    Serial.print("Temperature ");
    Serial.println(temperature);
  }

  if (humidityCharacteristic.value() != humidity) {
    humidityCharacteristic.setValue(humidity);
    Serial.print("Humidity ");
    Serial.println(humidity);
  }

}

