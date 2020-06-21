#include "pin_definition.h"

#include <SoftwareSerial.h>
SoftwareSerial BT(PIN_BT_RX, PIN_BT_TX);

#include "ButtonController.h"
ButtonController btn[2] = { PIN_BTN_1, PIN_BTN_2 };

#include "LiquidCrystal_I2C.h"
LiquidCrystal_I2C lcd(0x27, 16, 2);

#include "CountController.h"
CountController counter;

void keyEvent(uint8_t id, uint8_t event) {
    switch(event) {
        case EVENT::PRESS:
        digitalWrite(id + PIN_LED_1, HIGH);
        counter.increaseOne();
        break;

        case EVENT::RELEASE:
        digitalWrite(id + PIN_LED_1, LOW);
        break;
    }
}

void keyNone(uint8_t id, uint8_t event) {}

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
uint32_t firstPressResponse = 0xffffffff;
uint32_t pressResult = 0;

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

    btn[0].setPreventChatteringTime(50);
    btn[1].setPreventChatteringTime(50);
    btn[0].setEventCallback(keyEvent, 0);
    btn[1].setEventCallback(keyEvent, 1);

    delay(2000);
    phase = PHASE::READY;
    p_ready(PHASEMODE::SETUP);
    randomSeed(analogRead(A0) ^ analogRead(A1) ^ analogRead(A2));
}

void loop() {
    switch(phase) {
        // 게임 준비
        case PHASE::READY:
        p_ready(PHASEMODE::LOOP);
        break;

        // 게임 시작, 타이머 작동
        case PHASE::STANDBY:
        p_standby(PHASEMODE::LOOP);
        break;

        // 게임 시작, 연타 횟수 기록
        case PHASE::MEASUREMENT:
        p_measurement(PHASEMODE::LOOP);
        break;

        // 게임 종료, 기록 전송
        case PHASE::RESULT:
        p_result(PHASEMODE::LOOP);
        break;
    }
}

void p_ready(uint8_t mode) {
    static uint8_t bar = 0;
    switch(mode) {
        case PHASEMODE::SETUP:
        lcd.clear();
        lcd.setCursor(2, 0);
        lcd.print("Press 2 Btns");
        lcd.setCursor(0, 1);
        lcd.print("    to Start    ");
        break;

        case PHASEMODE::LOOP:
        btn[0].update(); btn[1].update();
        if(!btn[0].status && !btn[1].status) {
            bar += 1;
            char str[16];
            for(int i = 0; i < sizeof(str); i++) {
                if(bar < i) str[i] = ' ';
                else str[i] = 0xff;
            }
            lcd.setCursor(0, 1);
            lcd.print(str);
            delay(100);
            if(bar == 16) {
                phase = PHASE::STANDBY;
                p_standby(PHASEMODE::SETUP);
            }
        }
        else {
            lcd.setCursor(0, 1);
            lcd.print("    to Start    ");
            bar = 0;
            delay(100);
        }
        break;
    }
}


void p_standby(uint8_t mode) {
    static int rTimer;
    switch(mode) {
        case PHASEMODE::SETUP:
        lcd.clear();
        lcd.setCursor(5, 0);
        lcd.print("READY?");
        rTimer = random(20, 50);
        break;

        case PHASEMODE::LOOP:
        delay(100);
        if(--rTimer == 0) {
            phase = PHASE::MEASUREMENT;
            p_measurement(PHASEMODE::SETUP);
        }
        break;
    }
}


void p_measurement(uint8_t mode) {
    static uint32_t startTime;
    static uint32_t lcdTaskTime;
    switch(mode) {
        case PHASEMODE::SETUP:
        lcd.clear();
        lcd.setCursor(1, 0);
        lcd.print("! PRESS !");
        lcd.setCursor(0, 1);
        lcd.print("TIME[          ]");
        startTime = millis();
        counter.set(0);
        tone(PIN_BUZ, 1000);
        break;

        case PHASEMODE::LOOP:
        uint32_t duration = millis() - startTime;
        // if(firstPressResponse > duration) firstPressResponse = duration;
        btn[0].update(); btn[1].update();
        if(duration > 300) noTone(PIN_BUZ);
        if(duration > lcdTaskTime) {
            lcdTaskTime += 300;
            char str[11];
            uint8_t bar = 10 - duration / 1000;
            for(int i = sizeof(str) - 1; i >= 0; i--) {
                if(bar > i) str[i] = '#';
                else str[i] = ' ';
            }
            str[10] = NULL;
            lcd.setCursor(11, 0);
            lcd.print(counter.get());
            lcd.setCursor(5, 1);
            lcd.print(str);
        }
        if(duration>=10000) {
            pressResult = counter.get();
            phase = PHASE::RESULT;
            p_result(PHASEMODE::SETUP);
        }
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