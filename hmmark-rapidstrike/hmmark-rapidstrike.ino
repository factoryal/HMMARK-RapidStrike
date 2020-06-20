#include "pin_definition.h"

#include <SoftwareSerial.h>

SoftwareSerial BT(PIN_BT_RX, PIN_BT_TX);

void setup() {
    Serial.begin(9600);
    BT.begin(9600);
    Serial.println("service ready.");

    pinMode(PIN_BTN_1, INPUT_PULLUP);
    pinMode(PIN_BTN_2, INPUT_PULLUP);
    pinMode(PIN_LED_1, OUTPUT);
    pinMode(PIN_LED_2, OUTPUT);
    pinMode(PIN_BUZ, OUTPUT);

}

void loop() {
    
}