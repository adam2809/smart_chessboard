#include "touch_pad.hpp"

void touch_pad::init(){
    touch_pad_config(channel,TOUCH_THRESH_NO_USE);
}
void touch_pad::update(){
    uint16_t touch_value;
    touch_pad_read_filtered(channel, &touch_value);
    val = touch_value;
    gpio_set_level(led_pin,val < thresh);
}
bool touch_pad::check_thresh(){
    return val < thresh;
}
void touch_pad::print_info(){
    printf("T%d:[%d] THESH:[%d] | ", channel, val,thresh);
}
touch_pad_t touch_pad::get_channel(){
    return channel;
}
int touch_pad::get_thresh(){
    return thresh;
}
int touch_pad::get_led_pin(){
    return led_pin;
}

void init_sensor(){
    ESP_ERROR_CHECK(touch_pad_init());
    touch_pad_set_voltage(TOUCH_HVOLT_2V7, TOUCH_LVOLT_0V5, TOUCH_HVOLT_ATTEN_1V);
    touch_pad_filter_start(TOUCHPAD_FILTER_TOUCH_PERIOD);
}