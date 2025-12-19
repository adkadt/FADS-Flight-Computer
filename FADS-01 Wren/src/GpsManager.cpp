#include "GpsManager.h"

// constructor sets serial port and serial baud rate to gps
GpsManager::GpsManager(HardwareSerial& serial_port, uint32_t baud) 
    : serial_port_(&serial_port),
      baud_(baud)
{ }

// starts gps serial port
void GpsManager::Begin() {
    serial_port_->begin(baud_);
}

// updates tinygps with the latest data
bool GpsManager::Update() {
    bool updated_status = false;
    // loops until serial port is cleared
    while (serial_port_->available() > 0) {
        // reads data from serial port and encodes it in tinygps
        if (gps_.encode(serial_port_->read())) {
            updated_status = gps_.location.isUpdated();
        }
    }
    return updated_status;
}

// gets the location data from tinygps
locationData GpsManager::GetLocationData() {
    locationData data;
    data.lat = gps_.location.lat();
    data.lng = gps_.location.lng();
    data.sats = gps_.satellites.value();
    data.is_valid = gps_.location.isValid();
    return data;
}

// will add time information

// can add other functions for other info