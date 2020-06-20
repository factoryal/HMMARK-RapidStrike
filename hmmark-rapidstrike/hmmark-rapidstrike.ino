#include "pin_definition.h"

#include <SoftwareSerial.h>

SoftwareSerial BT(PIN_BT_RX, PIN_BT_TX);

void setup() {
    Serial.begin(9600);
    BT.begin(9600);
}

void loop() {
    
}