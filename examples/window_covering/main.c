#include <stdio.h>
#include <espressif/esp_wifi.h>
#include <espressif/esp_sta.h>
#include <espressif/esp_system.h>
#include <esp/uart.h>
#include <esp8266.h>
#include <etstimer.h>
#include <esplibs/libmain.h>
#include <FreeRTOS.h>
#include <task.h>
#include <homekit/homekit.h>
#include <homekit/characteristics.h>
#include "wifi.h"
#include <unistd.h>
#include <dht/dht.h>

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

#define POSITION_OPEN 100
#define POSITION_CLOSED 0
#define POSITION_STATE_CLOSING 0
#define POSITION_STATE_OPENING 1
#define POSITION_STATE_STOPPED 2

TaskHandle_t updateStateTask;
homekit_characteristic_t current_position;
homekit_characteristic_t target_position;
homekit_characteristic_t position_state;
homekit_accessory_t *accessories[];

const int write_gpio = 5;
const int read_gpio = 4;

bool val_o = 0;
bool val_c = 0;
bool val_r = 0;
static void wifi_init() {
    struct sdk_station_config wifi_config = {
        .ssid = WIFI_SSID,
        .password = WIFI_PASSWORD,
    };

    sdk_wifi_set_opmode(STATION_MODE);
    sdk_wifi_station_set_config(&wifi_config);
    sdk_wifi_station_connect();
}

void update_state() {
    while (true) {
        // uint8_t position = current_position.value.int_value;
        // int8_t direction = position_state.value.int_value == POSITION_STATE_OPENING ? 1 : -1;
        val_o=0;
        val_c=0;
        //int16_t newPosition = position + direction;
        bool val_before = val_r;
        val_r = gpio_read(read_gpio);
        if(val_r!=val_before){
          if(position_state.value.int_value == POSITION_STATE_OPENING) val_o=1;
          else if(position_state.value.int_value == POSITION_STATE_CLOSING) val_c=1;
        }

        // if(val_o) current_position.value.int_value = POSITION_OPEN;
        // if(val_c) current_position.value.int_value = POSITION_CLOSED;
        // homekit_characteristic_notify(&current_position, current_position.value);
        // homekit_characteristic_notify(&position_state, position_state.value);
        // vTaskSuspend(updateStateTask);

        printf("val_o %u\n", val_o);
        printf("val_c %u\n", val_c);

        //current_position.value.int_value = newPosition;


        //if (newPosition == target_position.value.int_value) {
        if (val_o) {
            //printf("reached destination open");
            current_position.value.int_value = POSITION_OPEN;
            position_state.value.int_value = POSITION_STATE_STOPPED;
            homekit_characteristic_notify(&current_position, current_position.value);
            homekit_characteristic_notify(&position_state, position_state.value);
            vTaskSuspend(updateStateTask);
            vTaskDelay(pdMS_TO_TICKS(500));
        }

        if (val_c) {
            //printf("reached destination closed");
            current_position.value.int_value = POSITION_CLOSED;
            position_state.value.int_value = POSITION_STATE_STOPPED;
            homekit_characteristic_notify(&current_position, current_position.value);
            homekit_characteristic_notify(&position_state, position_state.value);
            vTaskSuspend(updateStateTask);
            vTaskDelay(pdMS_TO_TICKS(500));
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void update_state_init() {
    xTaskCreate(update_state, "UpdateState", 256, NULL, tskIDLE_PRIORITY, &updateStateTask);
    current_position.value.int_value = POSITION_CLOSED;
    position_state.value.int_value = POSITION_STATE_STOPPED;
    homekit_characteristic_notify(&current_position, current_position.value);
    homekit_characteristic_notify(&position_state, position_state.value);
    vTaskSuspend(updateStateTask);
}

void window_covering_identify(homekit_value_t _value) {
    //printf("Curtain identify\n");
}

void on_update_target_position(homekit_characteristic_t *ch, homekit_value_t value, void *context);

homekit_characteristic_t current_position = {
    HOMEKIT_DECLARE_CHARACTERISTIC_CURRENT_POSITION(POSITION_CLOSED)
};

homekit_characteristic_t target_position = {
    HOMEKIT_DECLARE_CHARACTERISTIC_TARGET_POSITION(POSITION_CLOSED, .callback=HOMEKIT_CHARACTERISTIC_CALLBACK(on_update_target_position))
};

homekit_characteristic_t position_state = {
    HOMEKIT_DECLARE_CHARACTERISTIC_POSITION_STATE(POSITION_STATE_STOPPED)
};

homekit_accessory_t *accessories[] = {
    HOMEKIT_ACCESSORY(.id=1, .category=homekit_accessory_category_window_covering, .services=(homekit_service_t*[]) {
        HOMEKIT_SERVICE(ACCESSORY_INFORMATION, .characteristics=(homekit_characteristic_t*[]) {
            HOMEKIT_CHARACTERISTIC(NAME, "Crabzebo"),
            HOMEKIT_CHARACTERISTIC(MANUFACTURER, "PP"),
            HOMEKIT_CHARACTERISTIC(SERIAL_NUMBER, "001"),
            HOMEKIT_CHARACTERISTIC(MODEL, "Crabzebo V1"),
            HOMEKIT_CHARACTERISTIC(FIRMWARE_REVISION, "0.1"),
            HOMEKIT_CHARACTERISTIC(IDENTIFY, window_covering_identify),
            NULL
        }),
        HOMEKIT_SERVICE(WINDOW_COVERING, .primary=true, .characteristics=(homekit_characteristic_t*[]) {
            HOMEKIT_CHARACTERISTIC(NAME, "Window blind"),
            &current_position,
            &target_position,
            &position_state,
            NULL
        }),
        NULL
    }),
    NULL
};

void on_update_target_position(homekit_characteristic_t *ch, homekit_value_t value, void *context) {
    //printf("Update target position to: %u\n", target_position.value.int_value);

    //turn gpio 0 on
    if (target_position.value.int_value <= 50){

      gpio_write(write_gpio, 0);
      vTaskDelay(500 / portTICK_PERIOD_MS);
      current_position.value.int_value = 70;
      homekit_characteristic_notify(&current_position, current_position.value);
      position_state.value.int_value = POSITION_STATE_CLOSING;
      homekit_characteristic_notify(&position_state, position_state.value);
      vTaskResume(updateStateTask);
      //gpio_write(1, 0);
      //gpio_write(2,0);
      //gpio_disable(1);
      //gpio_enable(1, GPIO_INPUT);
    }

    if (target_position.value.int_value >50){

      gpio_write(write_gpio, 1);
      vTaskDelay(500 / portTICK_PERIOD_MS);
      current_position.value.int_value = 30;
      homekit_characteristic_notify(&current_position, current_position.value);
      position_state.value.int_value = POSITION_STATE_OPENING;
      homekit_characteristic_notify(&position_state, position_state.value);
      vTaskResume(updateStateTask);
      //gpio_enable(1, GPIO_INPUT);
    }

    // if (target_position.value.int_value == current_position.value.int_value) {
    //     printf("Current position equal to target. Stopping.\n");
    //     position_state.value.int_value = POSITION_STATE_STOPPED;
    //     homekit_characteristic_notify(&position_state, position_state.value);
    //     vTaskSuspend(updateStateTask);
    // } else {
    //     position_state.value.int_value = target_position.value.int_value > current_position.value.int_value
    //         ? POSITION_STATE_OPENING
    //         : POSITION_STATE_CLOSING;
    //
    //     homekit_characteristic_notify(&position_state, position_state.value);
    //     vTaskResume(updateStateTask);
    // }
}

homekit_server_config_t config = {
    .accessories = accessories,
    .password = "111-11-111"
};

void gpio_init() {
    gpio_enable(read_gpio, GPIO_INPUT);
    gpio_enable(write_gpio, GPIO_OUTPUT);
    gpio_write(write_gpio, 1);
}

void user_init(void) {
    uart_set_baud(0, 115200);
    gpio_init();
    wifi_init();
    homekit_server_init(&config);
    update_state_init();
    printf("init complete\n");
    //vTaskDelay(1000 / portTICK_PERIOD_MS);


}
