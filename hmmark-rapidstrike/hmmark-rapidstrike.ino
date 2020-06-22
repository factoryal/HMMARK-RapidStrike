#include "pin_definition.h"

#include <SoftwareSerial.h>
SoftwareSerial BT(PIN_BT_RX, PIN_BT_TX);

#include "ButtonController.h"
ButtonController btn[2] = { PIN_BTN_1, PIN_BTN_2 };

#include "LiquidCrystal_I2C.h"
LiquidCrystal_I2C lcd(0x27, 16, 2);

#include "CountController.h"
CountController counter;

bool pressFlag = false;

void keyEvent(uint8_t id, uint8_t event) {
	switch(event) {
		case EVENT::PRESS:
		digitalWrite(id + PIN_LED_1, HIGH);
		counter.increaseOne();
		pressFlag = true;
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
	RESULT,
	UPLOAD
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
	randomSeed(analogRead(A0) ^ analogRead(A1) ^ analogRead(A2) ^ analogRead(A3));
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

		// 게임 종료, 기록 확인
		case PHASE::RESULT:
		p_result(PHASEMODE::LOOP);
		break;

		// 기록 전송
		case PHASE::UPLOAD:
		p_upload(PHASEMODE::LOOP);
		break;
	}
}

void p_ready(uint8_t mode) {
	static uint8_t bar = 0;
	switch(mode) {
		case PHASEMODE::SETUP:
		lcd.clear();
		lcd.setCursor(0, 0);
		lcd.print("  Press 2 Btns  ");
		lcd.setCursor(0, 1);
		lcd.print("    to Start    ");
		break;

		case PHASEMODE::LOOP:
		btn[0].update(); btn[1].update();
		if(!btn[0].status && !btn[1].status) {
			bar += 1;
			char str[17] = { 0 };
			for(int i = 0; i < sizeof(str) - 1; i++) {
				if(bar < i) str[i] = '-';
				else str[i] = '#';
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
		digitalWrite(PIN_LED_1, HIGH);
		digitalWrite(PIN_LED_2, HIGH);
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
		lcdTaskTime = 0;
		counter.set(0);
		pressFlag = false;
		tone(PIN_BUZ, 1000);
		digitalWrite(PIN_LED_1, LOW);
		digitalWrite(PIN_LED_2, LOW);
		break;

		case PHASEMODE::LOOP:
		uint32_t duration = millis() - startTime;
		if(!pressFlag) firstPressResponse = duration;
		btn[0].update(); btn[1].update();
		if(duration > 300) noTone(PIN_BUZ);
		if(duration > lcdTaskTime) {
			lcdTaskTime += 300;
			char str[11] = { 0 };
			uint8_t bar = 10 - duration / 1000;
			for(int i = sizeof(str) - 2; i >= 0; i--) {
				if(bar > i) str[i] = '#';
				else str[i] = '-';
			}
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
	static char buf[50];
	static int bufidx = 0;
	static uint32_t waitStartTime = 0;
	static uint8_t bar = 0;
	switch(mode) {
		case PHASEMODE::SETUP:
		tone(PIN_BUZ, 1000);
		lcd.clear();
		lcd.setCursor(2, 0);
		lcd.print("!! FINISH !!");
		for(int i = 0; i < 10; i++) {
			digitalWrite(PIN_LED_1, HIGH);
			digitalWrite(PIN_LED_2, HIGH);
			delay(100);
			digitalWrite(PIN_LED_1, LOW);
			digitalWrite(PIN_LED_2, LOW);
			delay(100);
		}
		noTone(PIN_BUZ);

		lcd.clear();
		lcd.setCursor(0, 0);
		lcd.print("Result: ");
		lcd.setCursor(10, 0);
		lcd.print(pressResult);
		lcd.setCursor(0, 1);
		lcd.print("Response: ");
		lcd.setCursor(10, 1);
		lcd.print(firstPressResponse);
		lcd.print("ms");
		delay(5000);

		lcd.clear();
		lcd.setCursor(2, 0);
		lcd.print("Press 2 Btns");
		lcd.setCursor(0, 1);
		lcd.print(" to send Discord ");
		waitStartTime = millis();
		break;

		case PHASEMODE::LOOP:
		btn[0].update(); btn[1].update();
		if(!btn[0].status && !btn[1].status) {
			waitStartTime = millis();
			bar += 1;
			char str[17] = { 0 };
			for(int i = 0; i < sizeof(str) - 1; i++) {
				if(bar < i) str[i] = '-';
				else str[i] = '#';
			}
			lcd.setCursor(0, 1);
			lcd.print(str);
			delay(100);
			if(bar == 16) {
				phase = PHASE::UPLOAD;
				p_upload(PHASEMODE::SETUP);
			}
		}
		else {
			lcd.setCursor(0, 1);
			lcd.print("to Upload Record");
			bar = 0;
			delay(100);
		}
		if(millis() - waitStartTime > 5000) {
			phase = PHASE::READY;
			p_ready(PHASEMODE::SETUP);
		}
		break;
	}
}

void p_upload(uint8_t mode) {
	char rxbuf[50] = { 0 };
	char rxbufidx = 0;
	switch(mode) {
		case PHASEMODE::SETUP:
		lcd.clear();
		lcd.print("Uploading...");
		BT.print("R ");
		BT.print(pressResult);
		BT.print(' ');
		BT.print(firstPressResponse);
		BT.print('\n');
		while(1) {
			if(BT.available()) {
				tone(PIN_BUZ, 1000, 100);
				char c = BT.read();
				if(c != '\n') rxbuf[rxbufidx++] = c;
				else {
					if(!strcmp(rxbuf, "OK")) {
						lcd.clear();
						lcd.print("Upload");
						lcd.setCursor(0, 1);
						lcd.print("Completed!");
						delay(3000);
						break;
					}
					memset(rxbuf, 0, sizeof(rxbuf));
					rxbufidx = 0;
				}
			}
		}
		phase = PHASE::READY;
		p_ready(PHASEMODE::SETUP);
		return;

		case PHASEMODE::LOOP:
		return;
	}
}