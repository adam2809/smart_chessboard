#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/touch_pad.h"
#include "esp_log.h"
#include <driver/adc.h>

#define TOUCH_PAD_NO_CHANGE   (-1)
#define TOUCH_THRESH_NO_USE   (0)
#define TOUCH_FILTER_MODE_EN  (1)
#define TOUCHPAD_FILTER_TOUCH_PERIOD (10)

gpio_config_t io_conf;

constexpr uint8_t touch_pads_count = 4;
constexpr touch_pad_t touch_pads[touch_pads_count] = {TOUCH_PAD_NUM4,TOUCH_PAD_NUM5,TOUCH_PAD_NUM8,TOUCH_PAD_NUM9};
constexpr int thresh_vals[touch_pads_count] = {600,640,680,695};
constexpr gpio_num_t led_pins[touch_pads_count] = {GPIO_NUM_4,GPIO_NUM_18,GPIO_NUM_19,GPIO_NUM_23};


class touch_pad{
    public:
        touch_pad(const touch_pad_t ch,const int th,const gpio_num_t lp) : 
            channel(ch),
            thresh(th),
            led_pin(lp){
            touch_pad_config(ch,TOUCH_THRESH_NO_USE);
        }
    private:
        const touch_pad_t channel;
        const int thresh;
        const gpio_num_t led_pin;
};

namespace touch_pads{
    constexpr uint8_t count = 4;
    touch_pad tp0{ TOUCH_PAD_NUM4,600,GPIO_NUM_4 };
    touch_pad tp1{ TOUCH_PAD_NUM5,640,GPIO_NUM_18 };
    touch_pad tp2{ TOUCH_PAD_NUM8,680,GPIO_NUM_19 };
    touch_pad tp3{ TOUCH_PAD_NUM9,695,GPIO_NUM_23 };
    touch_pad all[count] { tp0,tp1,tp2,tp3 };
}
/*
  Read values sensed at all available touch pads.
 Print out values in a loop on a serial monitor.
 */
static void tp_example_read_task(void *pvParameter){
    uint16_t touch_filter_value;
    while (1) {
        bool touched[touch_pads_count] = { false };
        for (int i = 0; i < touch_pads_count; i++) {
            touch_pad_read_filtered(touch_pads[i], &touch_filter_value);
            printf("T%d:[%d] THESH:[%d] | ", touch_pads[i], touch_filter_value,thresh_vals[i]);
            if(touch_filter_value < thresh_vals[i]){
                touched[i] = true;
            }
        }
        printf(" Touched:");
        for (int i = 0; i < touch_pads_count; i++) {
            gpio_set_level(led_pins[i],touched[i]);
            if(touched[i]){
                printf("%d,",i);
            }
        }
        printf("\n");
        vTaskDelay(200 / portTICK_PERIOD_MS);
    }
}

static void all_touch_pad_init(void)
{
    for (int i = 0;i < touch_pads_count;i++) {
        touch_pad_config(touch_pads[i],TOUCH_THRESH_NO_USE);
    }
}

static void all_led_pin_init(void){
    //disable interrupt
    io_conf.intr_type = GPIO_INTR_DISABLE;
    //set as output mode
    io_conf.mode = GPIO_MODE_OUTPUT;

    uint64_t pin_select_mask = 0;
    for(int i=0;i<touch_pads_count;i++){
        pin_select_mask |= (1ULL<<led_pins[i]);
    }

    //bit mask of the pins that you want to set,e.g.GPIO18/19
    io_conf.pin_bit_mask = pin_select_mask;
    
    //disable pull-down mode
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    //disable pull-up mode
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    //configure GPIO with the given settings
    gpio_config(&io_conf);
}

extern "C" 
void app_main(void)
{
    // Initialize touch pad peripheral.
    // The default fsm mode is software trigger mode.
    ESP_ERROR_CHECK(touch_pad_init());
    // Set reference voltage for charging/discharging
    // In this case, the high reference valtage will be 2.7V - 1V = 1.7V
    // The low reference voltage will be 0.5
    // The larger the range, the larger the pulse count value.
    touch_pad_set_voltage(TOUCH_HVOLT_2V7, TOUCH_LVOLT_0V5, TOUCH_HVOLT_ATTEN_1V);
    all_touch_pad_init();
    touch_pad_filter_start(TOUCHPAD_FILTER_TOUCH_PERIOD);

    all_led_pin_init();

    // Start task to read values sensed by pads
    xTaskCreate(&tp_example_read_task, "touch_pad_read_task", 2048, NULL, 5, NULL);
}
