#include "RadioManager.h"

RadioManager::RadioManager() : rf95_(RFM95_CS, RFM95_INT) {
}

bool RadioManager::Begin() {
    pinMode(RFM95_RST, OUTPUT);
    digitalWrite(RFM95_RST, HIGH);

    // Manual reset
    digitalWrite(RFM95_RST, LOW);
    delay(10);
    digitalWrite(RFM95_RST, HIGH);
    delay(10);

    if (!rf95_.init()) {
        return false;
    }

    if (!rf95_.setFrequency(RF95_FREQ)) {
        return false;
    }

    // Tx power to 23 dBm (max supported by module)
    rf95_.setTxPower(23, false);

    return true;
}

void RadioManager::Transmit(String data) {
    rf95_.send((uint8_t *)data.c_str(), data.length());
    rf95_.waitPacketSent();
}
