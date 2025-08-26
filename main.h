#pragma once

#include <Arduino.h>
#include <EEPROM.h>
#include <EasyButton.h>
#include <MG90.h>
#include <SPIFFS.h>
#include <Ticker.h>
#include <VIROSE_Lib.h>
#include <display.h>
#include <led.h>

#include "utility/debug.h"
#include "virose_com.h"
/*   ┌——————————————————————————————————————————————————————————————————————————————————————————————┐
     │                                        Definitions                                           │
     └——————————————————————————————————————————————————————————————————————————————————————————————┘     */

/* Pin Kumunikasi */
#define UART_1_RX 33
#define UART_1_TX 27
#define I2C_SDA 22
#define I2C_SCL 21
#define PIN_TANGAN_KANAN 100
#define PIN_TANGAN_KIRI 101

/*  */
#define UART_2_RX 13
#define UART_2_TX 25
#define PIN_BUTTON1 18
#define PIN_BUTTON2 4
#define PIN_TRIG 16
#define PIN_MATA 32
#define PIN_KIPAS 23
#define LED_INTERFACE 17
#define LED_PCB 19
#define SOUND 33

// /* Pin Kumunikasi LAMA*/
// #define UART_2_RX 25
// #define UART_2_TX 13
// // // /*  */
// #define PIN_BUTTON1 18
// #define PIN_BUTTON2 17
// #define PIN_TRIG 16
// #define PIN_MATA 27
// #define PIN_KIPAS 33
// #define LED_PCB 19
// #define LED_INTERFACE 4
// #define SOUND 34

/* ADDRESS EEPROM */
#define EEPROM_SIZE 128

/* ESP ADDRESS */
#define ADDR_ESP_NOW_ACTIVE 0

/* lAST INDEX */
#define ADDR_LAST_INDEX 4

#define PLAY_MOTION_FINISHED 8

#define FIRMWARE_VERSION "5.7.3"

/*   ┌——————————————————————————————————————————————————————————————————————————————————————————————┐
     │                                    Enum and Structure                                        │
     └——————————————————————————————————————————————————————————————————————————————————————————————┘     */

typedef struct esp_info {
    IPAddress mac;   // IP Address dari ESP32
    int index;       // Index Motion
    bool sound;      // Status Sound
    bool xl_status;  //  status XL-320
    bool mx_status;  //  status MX-28
    bool mata_status;
    bool sync_play;
    int index_sent;
} esp_info_t;

/*   ┌——————————————————————————————————————————————————————————————————————————————————————————————┐
     │                                     External Variables                                       │
     └——————————————————————————————————————————————————————————————————————————————————————————————┘     */

extern esp_info_t esp_info;  // Robot Info Struct
extern EasyButton mode_a;    // Button untuk Mode A
extern EasyButton mode_b;    // Button untuk Mode B
extern EasyButton trig;      // Button untuk Trigger
extern EasyButton button;    // Button untuk Interface
extern LedBender LED_main;
extern LedBender LED_interface;
extern CRGB leds[1];
extern CRGB leds2[1];
extern MG90 Mata;
extern MG90 Kipas;
extern MG90 Tangan_Kanan;
extern MG90 Tangan_Kiri;
extern Sound* sound;
extern int interval_comm;
extern int mac_index_target;
extern int mac_index_partner;
extern uint8_t IDENTITY;

/*   ┌——————————————————————————————————————————————————————————————————————————————————————————————┐
     │                                      Functions Prototype                                     │
     └——————————————————————————————————————————————————————————————————————————————————————————————┘     */

/**
 * @brief Aktivasi dan cek apakah untuk sound sensor sudah aman
 */
void soundStatus();

/**
 * @brief Callback serial event
 */
void serial_event();

/**
 * @brief Fungsi untuk init button
 */
void initButton();

/**
 * @brief Fungsi untuk mengirimkan data ke subcontroller
 * @param data data yang akan dikirimkan
 * @param len panjang data
 * @param mac_index index dari MAC Address
 */
void processing_command(uint8_t* data, int len, int mac_index = -1);

/**
 * @brief Callback untuk Mode Button A ketika dipencet
 */
void Button_Mode_A_Callback();

/**
 * @brief Callback untuk Mode Button A ketika ditekan lama
 */
void Button_Mode_A_Hold_Callback();

/**
 * @brief Callback untuk Mode Button A ketika dipencet 2 kali
 */
void Button_Mode_A_2_Sequence_Callback();

/**
 * @brief Callback untuk Mode Button A ketika dipencet 3 kali
 */
void Button_Mode_A_3_Sequence_Callback();

/**
 * @brief Callback untuk Mode Button B ketika dipencet
 */
void Button_Mode_B_Callback();

/**
 * @brief Callback untuk Mode Button B ketika ditekan lama
 */
void Button_Mode_B_Hold_Callback();

/**
 * @brief Callback untuk Mode Button B ketika dipencet 2 kali
 */
void Button_Mode_B_2_Sequence_Callback();

/**
 * @brief Callback untuk Mode Button B ketika dipencet 3 kali
 */
void Button_Mode_B_3_Sequence_Callback();

/**
 * @brief Callback untuk Trig Button ketika dipencet
 */
void Button_Trig_Callback();

/**
 * @brief Callback untuk Trig Button ketika ditekan lama
 */
void Button_Trig_Hold_Callback();

/**
 * @brief Callback untuk Trig Button ketika ditekan 2 kali
 */
void Button_Trig_2_Sequence_Callback();

// void initTaskBattery();

void error_feedback(uint8_t error_value, bool sound);
