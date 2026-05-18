#include <Arduino.h>
#include <optional>
#include "BmpManager.h"
#include "Altimeter.h"

#define BOOST_TIME_MS 4000 // seconds to ignore pressure for apogee readings
#define DROGUE_DROP_M 3 // the meters vertically down from apogee to deploy drogue chute
#define FIRE_TIME_MS 2000
#define MAIN_DEPLOYMENT_ALT 150 // meters AGL to deploy main chute at

#define DROGUE_PIN 13
#define MAIN_PIN 13

enum class State: uint8_t {
    PAD_IDLE,
    BOOST,
    COAST,
    DROGUE_DEPLOYMENT,
    DROGUE_DESCENT,
    MAIN_DEPLOYMENT,
    MAIN_DESCENT,
    LANDED
};
State current_state = State::PAD_IDLE;

BmpManager bmp;
Altimeter altimeter;

void handlePadIdle(double agl);
void handleBoost(double agl);
void handleCoast(double agl);
void handleDrogueDeployment(double agl);
void handleDrogueDescent(double agl);
void handleMainDeployment(double agl);
void handleMainDescent(double agl);
void handleLanded(double agl);
void transitionState(State next_state);

void setup() {
    Serial.begin(115200);    // USB Serial for Monitor
    while (!Serial) delay(10);      // !Important! [REMOVE FOR FLIGHT] Waits for serial

    if (!bmp.begin()) {
        Serial.println("--BMP FAILED TO START--");
    }

    bmp.setMode(BMP5XX_POWERMODE_CONTINUOUS, BMP5XX_ODR_240_HZ);
    bmp.setTempOSR(BMP5XX_OVERSAMPLING_1X);
    bmp.setPressureOSR(BMP5XX_OVERSAMPLING_1X);
    bmp.setIIRCoeff(BMP5XX_IIR_FILTER_COEFF_3);

    altimeter.setGroundMode(true);
}

int stateCounter = 0;
bool stateInit = false;
unsigned long stateStart_ms = 0;

double apogee = 0.0;

void loop() {
    bool bmp_update = false;
    
    // gather raw altitude data
    BmpData raw_bmp_data;
    if (bmp.update()) {
        raw_bmp_data = bmp.getBmpData();
        altimeter.update(raw_bmp_data.altitude);
    }
    double agl = altimeter.getAltitudeAGL();

    // enter current state
    switch (current_state) {
        case State::PAD_IDLE:               handlePadIdle(agl);             break;
        case State::BOOST:                  handleBoost(agl);               break;
        case State::COAST:                  handleCoast(agl);               break;
        case State::DROGUE_DEPLOYMENT:      handleDrogueDeployment(agl);    break;
        case State::DROGUE_DESCENT:         handleDrogueDescent(agl);       break;
        case State::MAIN_DEPLOYMENT:        handleMainDeployment(agl);      break;
        case State::MAIN_DESCENT:           handleMainDescent(agl);         break;
        case State::LANDED:                 handleLanded(agl);              break;
        default:                            break;
    }

    static unsigned long last_ping = 0;
    if (millis() - last_ping >= 1000) {
        last_ping = millis();
        Serial.printf("Alive Ping: %d\n", millis() / 1000);
    }
}

// ---------------------
// State Logic Functions
// ---------------------

void handlePadIdle(double agl) {
    if (agl > 15) {
        stateCounter++;
        if (stateCounter >= 3) {
            transitionState(State::BOOST);
        }
    } else {
        stateCounter = 0;
    }
}


void handleBoost(double agl) {
    if (millis() - stateStart_ms > BOOST_TIME_MS) {
        transitionState(State::COAST);
    }
}


void handleCoast(double agl) {
    if (agl > apogee) {
        apogee = agl;
    }

    if (agl < (apogee - DROGUE_DROP_M)) {
        stateCounter++;
        if (stateCounter >= 3) {
            transitionState(State::DROGUE_DEPLOYMENT);
        }
    } else {
        stateCounter = 0;
    }
}


void handleDrogueDeployment(double agl) {
    if (!stateInit) {
        digitalWrite(DROGUE_PIN, HIGH);
        stateStart_ms = millis();
        stateInit = true;

    } else if (millis() - stateStart_ms >= FIRE_TIME_MS) {
        digitalWrite(DROGUE_PIN, LOW);
        transitionState(State::DROGUE_DESCENT);
    }
}


void handleDrogueDescent(double agl) {
    if (agl < MAIN_DEPLOYMENT_ALT) {
        stateCounter++;
        if (stateCounter >= 3) {
            transitionState(State::MAIN_DEPLOYMENT);
        }
    } else {
        stateCounter = 0;
    }
}


void handleMainDeployment(double agl) {
    if (!stateInit) {
        digitalWrite(MAIN_PIN, HIGH);
        stateStart_ms = millis();
        stateInit = true;

    } else if (millis() - stateStart_ms >= FIRE_TIME_MS) {
        digitalWrite(MAIN_PIN, LOW);
        transitionState(State::MAIN_DESCENT);
    }
}


void handleMainDescent(double agl) {

}


void handleLanded(double agl) {

}

void transitionState(State next_state) {
    stateCounter = 0;
    stateStart_ms = millis();
    stateInit = false;

    current_state = next_state;
    Serial.printf("STATE-CHANGE: %d\n", static_cast<int>(next_state));
}