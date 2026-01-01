#include <Arduino.h>
#include <optional>
#include "GpsManager.h"
#include "BmpManager.h"

// GPS using Serial4 pins (TX4 17, RX4 16)
#define GPS_SERIAL Serial5

GpsManager gps(GPS_SERIAL, 115200);
BmpManager bmp;

double zero_altitude;

void setup() {
    Serial.begin(115200);    // USB Serial for Monitor
    while (!Serial) delay(10);      // Wait for Serial Monitor to open [REMOVE FOR FLIGHT]

    // set up i2c
    Wire.begin();
    Wire.setClock(1000000);

    gps.Begin();    // starts gps

    if (!bmp.Begin()) {
        Serial.println("--BMP FAILED TO START--");
    }
    bmp.SetMode(BMP5XX_POWERMODE_CONTINUOUS, BMP5XX_ODR_240_HZ);
    bmp.SetTempOSR(BMP5XX_OVERSAMPLING_1X);
    bmp.SetPressureOSR(BMP5XX_OVERSAMPLING_4X);

    while (!bmp.Update()) {
        delay(10);
    }
    bmpData bmp_data = bmp.GetBmpData();
    zero_altitude = bmp_data.altitude;

    Serial.println("Teensy 4.1 GPS Initialized");
}

int last_gps_time = 0;
int last_bmp_time = -1;
double sps;

int counter = 0;

void loop() {
    bool update = false;
    
    // updates gps and if sucessful prints data
    if (gps.Update()) {
        update = true;
        locationData gps_data = gps.GetLocationData();
        
        if (gps_data.is_valid) {
            Serial.printf("Lat %.6f, Lng %.6f, Alt %.2fft, Sats %d\n", gps_data.lat, gps_data.lng, gps_data.alt, gps_data.sats);
        } else {
            Serial.println("Waiting for FIX");
        }

        if(last_gps_time != 0 && counter % 100 == 0){
            sps = 1 / ((millis() - last_gps_time) / 1000.0);
            Serial.printf("GPS SPS: %.1f\n", sps);
        }
        last_gps_time = millis();
    }

    if (bmp.Update()) {
        update = true;
        bmpData bmp_data = bmp.GetBmpData();
        Serial.printf("Pressure: %.1fhPa, Altitude %.2fm, Temperature: %.1fC\n", bmp_data.pressure, bmp_data.altitude-zero_altitude, bmp_data.temperature);
    
        if(last_bmp_time != -1 && counter % 1 == 0){
            sps = 1 / ((micros() - last_bmp_time) / 1000000.0);
            Serial.printf("BMP SPS: %.1f\n", sps);
            last_bmp_time = micros();
        }else if(last_bmp_time == -1){
            last_bmp_time = micros();
        }
    }

    if (update) {
        Serial.println("==================");
        counter++;
    }

}