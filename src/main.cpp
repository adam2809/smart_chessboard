#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "touch_pad.hpp"
#include "driver/gpio.h"

#include <algorithm>
#include <vector>

gpio_config_t io_conf;
namespace touch_pads{
    constexpr uint8_t count = 4;
    touch_pad tp0{ TOUCH_PAD_NUM4,300,GPIO_NUM_4 };
    touch_pad tp1{ TOUCH_PAD_NUM5,300,GPIO_NUM_18 };
    touch_pad tp2{ TOUCH_PAD_NUM8,300,GPIO_NUM_19 };
    touch_pad tp3{ TOUCH_PAD_NUM9,300,GPIO_NUM_23 };
    std::vector<touch_pad> all = { tp0,tp1,tp2,tp3 };

    void update_all(){
        std::for_each(
            touch_pads::all.begin(),
            touch_pads::all.end(),
            [](touch_pad& tp){
                tp.update();
            }
        );
    }

    void init_all(){
        std::for_each(
            touch_pads::all.begin(),
            touch_pads::all.end(),
            [](touch_pad& tp){
                tp.init();
            }
        );
    }

    void print_all(){
        std::for_each(
            touch_pads::all.begin(),
            touch_pads::all.end(),
            [](touch_pad& tp){
                tp.print_info();
            }
        );
    }
}

static void tp_task(void *pvParameter){
    while (1) {
        touch_pads::update_all();
        touch_pads::print_all();
        
        printf("Touched:");
        int i=0;
        std::for_each(
            touch_pads::all.begin(),
            touch_pads::all.end(),
            [&i](touch_pad& tp){
                if(tp.check_thresh()){
                    printf("%d,",i);
                }
                i++;
            }
        );
        printf("\n");

        vTaskDelay(200 / portTICK_PERIOD_MS);
    }
}

static void all_led_pin_init(void){
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;

    uint64_t pin_select_mask = 0;
    for(int i=0;i<touch_pads::count;i++){
        pin_select_mask |= (1ULL<<touch_pads::all[i].get_led_pin());
    }
    io_conf.pin_bit_mask = pin_select_mask;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    gpio_config(&io_conf);
}

extern "C" 
void app_main(void){
    init_sensor();
    touch_pads::init_all();

    all_led_pin_init();

    xTaskCreate(&tp_task, "touch_pad_task", 2048, NULL, 5, NULL);
}
