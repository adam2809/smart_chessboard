#ifndef TOUCH_PAD_H
#define TOUCH_PAD_H

#include "driver/touch_pad.h"
#include <algorithm>
#include <vector>
#include "driver/gpio.h"

#define TOUCH_THRESH_NO_USE   (0)
#define TOUCHPAD_FILTER_TOUCH_PERIOD (10)


class touch_pad{
    private:
        const touch_pad_t channel;
        const int thresh;
        const gpio_num_t led_pin;
        uint16_t val;

    public:
        touch_pad(const touch_pad_t ch,const int th,const gpio_num_t lp) : 
            channel(ch),
            thresh(th),
            led_pin(lp),
            val(0){}
        void init();
        void update();
        bool check_thresh();
        void print_info();
        touch_pad_t get_channel();
        int get_thresh();
        int get_led_pin();
};

void init_sensor();
#endif
