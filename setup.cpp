#include "main.h"

int interval_comm;

byte data_to_send[9];

OLED displayRobot;

CRGB leds[1];
CRGB leds2[1];

MG90 Mata(EYE);
MG90 Kipas(FAN);
MG90 Tangan_Kanan(RIGHT_HAND);
MG90 Tangan_Kiri(LEFT_HAND);

LedBender LED_main(0, leds);
// LedBender LED_main(1, leds);
// LedBender LED_main(1, leds);
LedBender LED_interface(0, leds2);

EasyButton mode_a(PIN_BUTTON1, 50);
EasyButton mode_b(PIN_BUTTON2, 50);
EasyButton trig(PIN_TRIG, 50);
EasyButton button(0, 50, true, true);

esp_info_t esp_info = {
    .index = 0,
    .sound = false,
    .xl_status = 0,
    .mx_status = 0,
    .mata_status = 0,
    .sync_play = 0,
    .index_sent = 0};

void initButton() {
    mode_a.begin();
    mode_a.onPressed(Button_Mode_A_Callback);
    mode_a.onSequence(2, 1000, Button_Mode_A_2_Sequence_Callback);
    // mode_a.onSequence(3, 1000, Button_Mode_A_3_Sequence_Callback);
    mode_a.onPressedFor(1000, Button_Mode_A_Hold_Callback);

    mode_b.begin();
    mode_b.onPressed(Button_Mode_B_Callback);
    mode_b.onSequence(2, 1000, Button_Mode_B_2_Sequence_Callback);
    // mode_b.onSequence(3, 1000, Button_Mode_B_3_Sequence_Callback);
    mode_b.onPressedFor(500, Button_Mode_B_Hold_Callback);

    trig.begin();
    trig.onPressed(Button_Trig_Callback);
    // trig.onSequence(2, 1000, Button_Trig_2_Sequence_Callback);
    // trig.onPressedFor(1000, Button_Trig_Hold_Callback);

    button.begin();
    button.onPressedFor(3000, []() {
        EEPROM.writeBool(ADDR_USE_BT_SOUND, !EEPROM.readBool(ADDR_USE_BT_SOUND));
        EEPROM.commit();
        DEBUG_PRINTF("Set ADDR_USE_BT_SOUND to: %d", EEPROM.readBool(ADDR_USE_BT_SOUND));
        ESP.restart();
    });
    button.onSequence(2, 1000, []() { EEPROM.writeBool(ADDR_DEBUG_MAIN, !EEPROM.readBool(ADDR_DEBUG_MAIN)); EEPROM.commit();
        DEBUG_PRINTF("\nSet ADDR_DEBUG_MAIN to: %d\n", EEPROM.readBool(ADDR_DEBUG_MAIN)); });
    button.onSequence(3, 1000, []() { EEPROM.writeBool(ADDR_ESP_NOW, !EEPROM.readBool(ADDR_ESP_NOW)); EEPROM.commit();
            DEBUG_PRINTF("\nSet ADDR_ESP_NOW to: %d\n", EEPROM.readBool(ADDR_ESP_NOW)); });
    button.onSequence(5, 1000, []() { byte data_send[9]={POST_MOTION_PLAY};int value =1;memcpy(data_send + 1, &value, sizeof(value));memcpy(data_send + 5, &value, sizeof(value));serial_send(data_send, sizeof(data_send)); });
}

// void initTaskBattery() {
//     xTaskCreate(
//         [](void* pvParameters) {
//             uint8_t data[2] = {GET_BATTERY_VOLTAGES};
//             vTaskDelay(15000 / portTICK_PERIOD_MS);
//             for (;;) {
//                 // esp_now_send(mac_addresses[ESP_MAC_INDEX == 0 ? 2 : 3], data, sizeof(data));
//                 serial_send(data, sizeof(data));
//                 vTaskDelay(5000 / portTICK_PERIOD_MS);
//             }
//         },
//         "Battery",
//         10000,
//         NULL,
//         0,
//         NULL);
// }