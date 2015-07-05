#include <SPI.h>
#include <BLEPeripheral.h>

// define pins for Adafruit Bluefruit LE
// https://github.com/sandeepmistry/arduino-BLEPeripheral#pinouts
#define BLE_REQ 10
#define BLE_RDY 2
#define BLE_RST 9

BLEPeripheral blePeripheral = BLEPeripheral(BLE_REQ, BLE_RDY, BLE_RST);
BLEService weatherService = BLEService("BBB0");
BLEFloatCharacteristic temperatureCharacteristic = BLEFloatCharacteristic("BBB1", BLERead | BLENotify);
//BLEDescriptor temperatureDescriptor = BLEDescriptor("2901", "Temperature (deg F)");
BLEFloatCharacteristic humidityCharacteristic = BLEFloatCharacteristic("BBB2", BLERead | BLENotify);
//BLEDescriptor humidityDescriptor = BLEDescriptor("2901", "Humidity (%)");
BLEUnsignedLongCharacteristic pressureCharacteristic = BLEUnsignedLongCharacteristic("BBB3", BLERead | BLENotify);
BLEDescriptor pressureDescriptor = BLEDescriptor("2901", "Pressure"); // (pascal)

// Display Format https://developer.bluetooth.org/gatt/descriptors/Pages/DescriptorViewer.aspx?u=org.bluetooth.descriptor.gatt.characteristic_presentation_format.xml
// format - 8 (32-bit unsigned integer)
// exponent - 0
// unit - 0x2724 pascal
// namespace - 1 Bluetooth SIG
// namespace description - 0000
const uint8_t displayFormat[] = { 8, 0, 0x24, 0x27, 1, 0, 0 };
BLEDescriptor pressureDisplayDescriptor = BLEDescriptor("2904", displayFormat, sizeof(displayFormat));

#include "DHT.h"
#define DHTPIN 7        // what pin we're connected to
#define DHTTYPE DHT22   // DHT 22  (AM2302)
DHT dht(DHTPIN, DHTTYPE);

#include <Wire.h>
#include <Adafruit_BMP085.h>
Adafruit_BMP085 bmp;

long previousMillis = 0;  // will store last time sensor was read
long interval = 2000;     // interval at which to read sensor (milliseconds)

void setup()
{
  Serial.begin(9600);
  Serial.println(F("Bluetooth Low Energy Weather Station"));

  if (!bmp.begin()) {
    Serial.println(F("Could not find a valid BPM180 sensor, check wiring!"));
    while (1) {}
  }

  // set advertised name and service
  blePeripheral.setLocalName("Weather");
  blePeripheral.setDeviceName("Weather");
  blePeripheral.setAdvertisedServiceUuid(weatherService.uuid());

  // add service and characteristic
  blePeripheral.addAttribute(weatherService);
  blePeripheral.addAttribute(temperatureCharacteristic);
//  blePeripheral.addAttribute(temperatureDescriptor);
  blePeripheral.addAttribute(humidityCharacteristic);
//  blePeripheral.addAttribute(humidityDescriptor);
  blePeripheral.addAttribute(pressureCharacteristic);
  blePeripheral.addAttribute(pressureDescriptor);
  blePeripheral.addAttribute(pressureDisplayDescriptor);

  blePeripheral.begin();
}

void loop()
{
  // Tell the bluetooth radio to do whatever it should be working on
  blePeripheral.poll();

  // limit how often we read the sensor
  if (millis() - previousMillis > interval) {
    pollSensors();
    previousMillis = millis();
  }
}

void pollSensors()
{

  Serial.println(F("Polling"));
  float temperature = dht.readTemperature(true); // fahrenheight
  float humidity = dht.readHumidity();

  // http://www.srh.noaa.gov/images/epz/wxcalc/pressureConversion.pdf
  // To convert between inches of mercury (inHg) and millibars (mb) or hectopascals (hPa)
  // P inHg = 0.0295300 x P mb
  // Dividing by 100 to convert pascals to hectopascals
  //float pressure = bmp.readPressure() * 0.0295300 / 100.0;
  int32_t pressure = bmp.readPressure();

  if (isnan(temperature)) {
    Serial.println(F("Temp NaN"));
  }
  if (isnan(humidity)) {
    Serial.println(F("Humidity NaN"));
  }

  // only set the characteristic value if the temperature has changed
  if (!isnan(temperature) && temperatureCharacteristic.value() != temperature) {
    temperatureCharacteristic.setValue(temperature);
    Serial.print(F("Temperature "));
    Serial.println(bmp.readTemperature());
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

