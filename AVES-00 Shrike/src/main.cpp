#include <Arduino.h>
#include <optional>
#include "BmpManager.h"

#define BUFFER_SIZE 100

enum class State: uint8_t {
    PAD_IDLE,
    BOOST,
    COAST,
    DROGUE_DEPLOYMENT,
    MAIN_DEPLOYMENT,
    LANDED
};

BmpManager bmp;
bool radio_active = false;
State current_state = State::PAD_IDLE;

double ground_altitude_m;

void setup() {
    Serial.begin(115200);    // USB Serial for Monitor
    while (!Serial) delay(10);      // !Important! [REMOVE FOR FLIGHT] Waits for serial

    if (!bmp.begin()) {
        Serial.println("--BMP FAILED TO START--");
    }

    bmp.setMode(BMP5XX_POWERMODE_CONTINUOUS, BMP5XX_ODR_240_HZ);
    bmp.setTempOSR(BMP5XX_OVERSAMPLING_1X);
    bmp.setPressureOSR(BMP5XX_OVERSAMPLING_1X);

    // check for a bmp update
    while (!bmp.update()) {
        delay(10);
    }

    BmpData bmp_data = bmp.getBmpData();
    ground_altitude_m = bmp_data.altitude;
}

double sps;

int counter = 0;
unsigned long last_print_time = 0;

void loop() {
    bool bmp_update = false;
    bool printed = false;
    
    // gather raw altitude data
    BmpData raw_bmp_data;
    if (bmp.update()) {
        raw_bmp_data = bmp.getBmpData();
        bmp_update = true;
        
    }
    if (bmp_update) {
        // enter current state
        switch (current_state) {
            case State::PAD_IDLE:
                break;
            case State::BOOST:
                break;
            case State::COAST:
                break;
            case State::DROGUE_DEPLOYMENT:
                break;
            case State::MAIN_DEPLOYMENT:
                break;
            case State::LANDED:
                break;
            default:
                break;
        }
        counter++;
    }
    
    if (printed) {
        Serial.println("==================");
    }

    static unsigned long last_ping = 0;
    if (millis() - last_ping >= 1000) {
        last_ping = millis();
        Serial.printf("Alive Ping: %d\n", millis() / 1000);
    }

}