#include "pin_definition.h"

#include <SoftwareSerial.h>
SoftwareSerial BT(PIN_BT_RX, PIN_BT_TX);

#include "ButtonController.h"
ButtonController btn[2] = { PIN_BTN_1, PIN_BTN_2 };

#include "LiquidCrystal_I2C.h"
LiquidCrystal_I2C lcd(0x27, 16, 2);

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

enum PHASE {
    INIT = 0x00,
    READY,
    STANDBY,
    MEASUREMENT,
    RESULT
};

enum PHASEMODE {
    SETUP = 0x00,
    LOOP
};

uint8_t phase = PHASE::INIT;

void setup() {
    lcd.begin();
    lcd.clear();
    lcd.backlight();
    lcd.setCursor(5, 0);
    lcd.print("HMMARK");
    lcd.setCursor(2, 1);
    lcd.print("Rapid Strike");
    Serial.begin(9600);
    BT.begin(9600);

    pinMode(PIN_BTN_1, INPUT_PULLUP);
    pinMode(PIN_BTN_2, INPUT_PULLUP);
    pinMode(PIN_LED_1, OUTPUT);
    pinMode(PIN_LED_2, OUTPUT);
    pinMode(PIN_BUZ, OUTPUT);

    btn[0].setEventCallback(keyEvent, 0);
    btn[1].setEventCallback(keyEvent, 1);

    delay(2000);
    phase = PHASE::READY;
}

void loop() {
    btn[0].update();
    btn[1].update();
    switch(phase) {
        case PHASE::READY:
        p_ready(LOOP);
        break;

        case PHASE::STANDBY:
        p_standby(LOOP);
        break;

        case PHASE::MEASUREMENT:
        p_measurement(LOOP);
        break;

        case PHASE::RESULT:
        p_result(LOOP);
        break;
    }
}

void p_ready(uint8_t mode) {
    switch(mode) {
        case PHASEMODE::SETUP:
        break;

        case PHASEMODE::LOOP:
        break;
    }
}

void p_standby(uint8_t mode) {
    switch(mode) {
        case PHASEMODE::SETUP:
        break;

        case PHASEMODE::LOOP:
        break;
    }
}

void p_measurement(uint8_t mode) {
    switch(mode) {
        case PHASEMODE::SETUP:
        break;

        case PHASEMODE::LOOP:
        break;
    }
}

void p_result(uint8_t mode) {
    switch(mode) {
        case PHASEMODE::SETUP:
        break;

        case PHASEMODE::LOOP:
        break;
    }
}