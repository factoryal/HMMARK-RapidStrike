#include "pin_definition.h"

#include <SoftwareSerial.h>

SoftwareSerial BT(PIN_BT_RX, PIN_BT_TX);

void keyEvent(uint8_t id, uint8_t event) {
    switch(event) {
        case EVENT::PRESS:
        digitalWrite(id + 5, HIGH);
        break;

        case EVENT::RELEASE:
        digitalWrite(id + 5, LOW);
        break;
    }
}

void setup() {
    Serial.begin(9600);
    BT.begin(9600);
    Serial.println("service ready.");

    pinMode(PIN_BTN_1, INPUT_PULLUP);
    pinMode(PIN_BTN_2, INPUT_PULLUP);
    pinMode(PIN_LED_1, OUTPUT);
    pinMode(PIN_LED_2, OUTPUT);
    pinMode(PIN_BUZ, OUTPUT);

    btn[0].setEventCallback(keyEvent, 0);
    btn[1].setEventCallback(keyEvent, 1);

    delay(2000);
}

void loop() {
    
    btn[0].update();
    btn[1].update();
}