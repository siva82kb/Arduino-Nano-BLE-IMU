/*
 * Simple program to stream IMU data through BLE.
 */

#include <ArduinoBLE.h>
#include <Arduino_LSM6DS3.h>

#define DEBUG false

// These UUIDs have been randomly generated. - they must match between the Central and Peripheral devices
// Any changes you make here must be suitably made in the Python program as well

BLEService nanoIMUService("13012F00-F8C3-4F4A-A8F4-15CD926DA146"); // BLE Service
// Accelerometer and Gyroscope characteristics
BLECharacteristic acclGyroCharacteristic("13012F01-F8C3-4F4A-A8F4-15CD926DA146", BLENotify, 28);

// IMU data packet
typedef union {
  float timeaccgyr[7];
  uint8_t bytes[28];
} imuunion_t;

typedef union {
    unsigned long data;
    uint8_t bytes[4];
} ulongunion_t;

ulongunion_t _time;
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
  acclGyroCharacteristic.writeValue(_imudata.bytes, 28);

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
            // Get current time 
            _time.data = micros();
            // Update first four bytes of IMU data union.
            for (int i = 0; i < 4;i ++) {
                _imudata.bytes[i] = _time.bytes[i];
            }
            IMU.readAcceleration(_imudata.timeaccgyr[1], _imudata.timeaccgyr[2], _imudata.timeaccgyr[3]);
            IMU.readGyroscope(_imudata.timeaccgyr[4], _imudata.timeaccgyr[5], _imudata.timeaccgyr[6]);
            acclGyroCharacteristic.writeValue(_imudata.bytes, 28);
        }
    }
    
    // when the central disconnects, print it out:
    Serial.print(F("Disconnected from central: "));
    Serial.println(central.address());
  }
}
