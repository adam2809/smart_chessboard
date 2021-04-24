#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/touch_pad.h"
#include "esp_log.h"
#include <driver/adc.h>
#include <string>
#include "esp_task_wdt.h"
#include <algorithm>
#include <vector>

#define TOUCH_PAD_NO_CHANGE   (-1)
#define TOUCH_THRESH_NO_USE   (0)
#define TOUCH_FILTER_MODE_EN  (1)
#define TOUCHPAD_FILTER_TOUCH_PERIOD (10)

gpio_config_t io_conf;

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

        void init(){
            touch_pad_config(channel,TOUCH_THRESH_NO_USE);
        }

        void update(){
            uint16_t touch_value;
            touch_pad_read_filtered(channel, &touch_value);
            val = touch_value;
            gpio_set_level(led_pin,val < thresh);
        }

        bool check_thresh(){
            return val < thresh;
        }

        void print_info() {
            printf("T%d:[%d] THESH:[%d] | ", channel, val,thresh);
        }

        touch_pad_t get_channel(){
            return channel;
        }
        int get_thresh(){
            return thresh;
        }

        int get_led_pin(){
            return led_pin;
        }
};

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
/*
  Read values sensed at all available touch pads.
 Print out values in a loop on a serial monitor.
 */
static void tp_example_read_task(void *pvParameter){
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
    ESP_ERROR_CHECK(touch_pad_init());
    touch_pad_set_voltage(TOUCH_HVOLT_2V7, TOUCH_LVOLT_0V5, TOUCH_HVOLT_ATTEN_1V);
    touch_pad_filter_start(TOUCHPAD_FILTER_TOUCH_PERIOD);
    touch_pads::init_all();

    all_led_pin_init();

    xTaskCreate(&tp_example_read_task, "touch_pad_read_task", 2048, NULL, 5, NULL);
}
