/*
 * Simple program to stream IMU data through BLE.
 */

#include <ArduinoBLE.h>
#include <Arduino_LSM6DS3.h>

#define DEBUG true

// These UUIDs have been randomly generated. - they must match between the Central and Peripheral devices
// Any changes you make here must be suitably made in the Python program as well

BLEService nanoIMUService("13012F00-F8C3-4F4A-A8F4-15CD926DA146"); // BLE Service
// Accelerometer and Gyroscope characteristics
BLECharacteristic acclGyroCharacteristic("13012F01-F8C3-4F4A-A8F4-15CD926DA146", BLENotify, 24);

// IMU data packet
typedef union {
  float accgyr[6];
  uint8_t bytes[24];
} imuunion_t;

imuunion_t _imudata;

void setup() {
  Serial.begin(9600);

  if (DEBUG) {
    while (!Serial);
  }

  if (!IMU.begin()) {
    Serial.println("Failed to initialize IMU!");
    while (1);
  }

  // begin initialization
  if (!BLE.begin()) {
    Serial.println("Starting BLE failed!");
    while (1);
  }

  // set advertised local name and service UUID:
  BLE.setLocalName("Arduino Nano 33 BLE Sense");
  BLE.setAdvertisedService(nanoIMUService);

  // add the characteristic to the service
  nanoIMUService.addCharacteristic(acclGyroCharacteristic);
  
  // add service
  BLE.addService(nanoIMUService);

  // set the initial value for the characeristic:
  acclGyroCharacteristic.writeValue(_imudata.bytes, 24);

  // start advertising
  BLE.advertise();
  delay(100);
  Serial.println("ProtoStax Arduino Nano BLE LED Peripheral Service Started");
}

void loop() {
  // listen for BLE centrals to connect:
  BLEDevice central = BLE.central();

  // if a central is connected to peripheral:
  if (central) {
    Serial.print("Connected to central: ");
    // print the central's MAC address:
    Serial.println(central.address());

    // while the central is still connected to peripheral:
    while (central.connected()) {
        if (IMU.accelerationAvailable() && IMU.gyroscopeAvailable()) {
            IMU.readAcceleration(_imudata.accgyr[0], _imudata.accgyr[1], _imudata.accgyr[2]);
            IMU.readGyroscope(_imudata.accgyr[3], _imudata.accgyr[4], _imudata.accgyr[5]);
            acclGyroCharacteristic.writeValue(_imudata.bytes, 24);
        }
    }
    
    // when the central disconnects, print it out:
    Serial.print(F("Disconnected from central: "));
    Serial.println(central.address());
  }
}
