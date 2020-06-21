#pragma once
#include "Arduino.h"

class CountController {
private:
    int32_t cnt = 0;

public:

    void increaseOne() {
        cnt += 1;
    }

    void increase(int32_t n) {
        cnt += n;
    }

    void set(int32_t n) {
        cnt = n;
    }

    int32_t get() {
        return cnt;
    }

};