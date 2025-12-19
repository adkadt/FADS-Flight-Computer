#include <Arduino.h>
#include "GpsManager.h"

// GPS using Serial4 pins (TX4 17, RX4 16)
#define GPS_SERIAL Serial4

GpsManager gps(GPS_SERIAL, 9600);

void setup() {
    Serial.begin(115200);      // USB Serial for Monitor
    gps.Begin();    // starts gps
    
    Serial.println("Teensy 4.1 GPS Initialized");
}

void loop() {
    // updates gps and if sucessful prints data
    if (gps.Update()) {
        locationData gps_data = gps.GetLocationData();
        
        if (gps_data.is_valid) {
            Serial.printf("Lat %.6f, Lng %.6f, Alt %.2fft, Sats %d\n", gps_data.lat, gps_data.lng, gps_data.alt, gps_data.sats);
        } else {
            Serial.println("Waiting for FIX");
        }
    }

}