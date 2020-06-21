#pragma once
#include "Arduino.h"

enum EVENT {
	PRESS = 0x01,
	LONGPRESS = 0x02,
	RELEASE = 0x10
};

class ButtonController {
const uint8_t pin;
	void (*event_callback)(uint8_t, uint8_t) = NULL;
	uint8_t eventId = 0xff;
	bool lastRead = true;
	bool long_press_callback_called = false;
	uint32_t lastPressTime = 0;
	uint32_t lastReadTime = 0;
	uint16_t prevent_chattering_time = 20;
	uint16_t long_press_time = 2000;

public:
	ButtonController(const uint8_t pin) : pin(pin) {
	}

	void pullup(bool en) {
        pinMode(pin, en << 1);
    }

	// 버튼 이벤트 발생시 콜백함수 지정
	// id: 버튼에 아이디 값을 지정. 콜백함수 호출시 첫번째 인자로 들어감
	// callback 두 번째 uint8_t: 버튼 상태 EVENT 코드가 인자로 들어감
	void setEventCallback(void (*callback)(uint8_t, uint8_t), uint8_t id) {
		event_callback = callback;
		eventId = id;
	}

	void setLongPressTime(uint16_t t) {
        long_press_time = t;
    }

	void setPreventChatteringTime(uint16_t t) {
        prevent_chattering_time = t;
    }

	void update() {
		bool nowRead = digitalRead(pin);
		uint32_t nowTime = millis();
		if(nowTime - lastReadTime > prevent_chattering_time){
			if(!nowRead && lastRead) {
				lastReadTime = lastPressTime = nowTime;
				event_callback(eventId, EVENT::PRESS);
			}
			else if(!lastRead && nowRead) {
				lastReadTime = nowTime;
				long_press_callback_called = false;
				event_callback(eventId, EVENT::RELEASE);
			}
			else if(!lastRead && !nowRead && nowTime - lastPressTime > long_press_time && 	!long_press_callback_called) {
					event_callback(eventId, EVENT::LONGPRESS);
					long_press_callback_called = true;
			}
			lastRead = nowRead;
		}
	}

};