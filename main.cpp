#include "main.h"

int last_time;
unsigned long index_time = 0;
Sound* sound = nullptr;

// bisma faq

void on_startup_sequence() {
    DEBUG_PRINTF("[Main-Micro] Communication Start Up, Expecting a ping from Sub program\n");
    LED_main.setFlicker(CRGB::Red, 100, 100);
    int timeout = 2000;
    for (int i = timeout; i > 0; i--) {
        digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
        if (Serial2.available()) {
            // if (Serial2.read() != 0xFD && Serial2.read() != 0x00 && Serial2.read()) continue;
            if (Serial2.read() != 0xFD) continue;
            if (Serial2.read() != 0x00) continue;
            DEBUG_PRINTF("[POST] RECV Data From:%d ", Serial2.read());
            int len = Serial2.read();
            if (Serial2.read() == PING) {
                LED_main.turnOn(CRGB::Yellow);
                DEBUG_PRINTF("[Main-Micro] Serial Comunnication Estabilished Success\n");
                DEBUG_PRINTF("[Main-Micro] Sending Ping Feedback\n");
                byte ping_data[1] = {RESPONSE_PING};
                serial_send(ping_data, sizeof(ping_data));
                delay(100);
                for (int i = timeout; i > 0; i--) {
                    if (Serial2.available()) {
                        // if (Serial2.read() != 0xFD && Serial2.read() != 0x00 && Serial2.read()) continue;
                        if (Serial2.read() != 0xFD) continue;
                        if (Serial2.read() != 0x00) continue;
                        DEBUG_PRINTF("[POST] RECV Data From:%d ", Serial2.read());
                        int len = Serial2.read();
                        if (Serial2.read() == SET_IDENTITY) {
                            DEBUG_PRINTF("[Main-Micro] Getting Identity\n");
                            EEPROM.writeByte(ADDR_IDENTITY, Serial2.read());
                            EEPROM.writeByte(ADDR_ASSIGNED_PARTNER, Serial2.read());
                            LED_main.turnOn(CRGB::Green);
                            EEPROM.commit();
                            DEBUG_PRINTF("[Main-Micro] Identity Set to %s[%d]\n", mac_names[EEPROM.readByte(ADDR_IDENTITY)], EEPROM.readByte(ADDR_IDENTITY));
                            DEBUG_PRINTF("[Main-Micro] Partner Set to %s[%d]\n", mac_names[EEPROM.readByte(ADDR_ASSIGNED_PARTNER)], EEPROM.readByte(ADDR_ASSIGNED_PARTNER));
                            i = 0;
                        }
                        break;
                    }
                }
            }
            return;
        }
        if (i <= 1) {
            DEBUG_PRINTF("[Main-Micro] Startup Timeout no [SUB-MICRO] PING received\n");
            DEBUG_PRINTF("[Main-Micro] Identity Set to default value [2][3]\n");
            EEPROM.writeByte(ADDR_IDENTITY, 2);
            EEPROM.writeByte(ADDR_ASSIGNED_PARTNER, 3);
            EEPROM.commit();

            // LED_main.turnOn(CRGB::Red);
            LED_main.setFlicker(CRGB::Red, 250, 250);
            digitalWrite(SOUND, HIGH);
            delay(500);
            digitalWrite(SOUND, LOW);
            return;
        }
        delay(1);
    }
}

void error_feedback(uint8_t error_value, bool sound) {
    digitalWrite(SOUND, LOW);
    DEBUG_PRINTF("[Main-Micro] Error: %s\n", error_code_to_name((ERROR)error_value));
    if (error_value == ESP_NOW_ERR) {
        LED_main.setFlicker(CRGB::Red, 500, 100);
        for (int i = 0; i < 10; i++) {
            digitalWrite(SOUND, !digitalRead(SOUND));
            delay(100);
        }
    } else if (error_value == SERIAL_ERR) {
        LED_main.setFlicker(CRGB::Red, 100, 50);
        digitalWrite(SOUND, HIGH);
        delay(500);
        digitalWrite(SOUND, LOW);
    } else if (error_value == MOTION_ERR) {
        LED_main.setFlicker(CRGB::Blue, 100, 50);
        for (int i = 0; i < 4; i++) {
            digitalWrite(SOUND, !digitalRead(SOUND));
            delay(100);
        }
    } else if (error_value == SERVO_ERR) {
        LED_main.setFlicker(CRGB::Black, 100, 50);
        for (int i = 0; i < 6; i++) {
            digitalWrite(SOUND, !digitalRead(SOUND));
            delay(100);
        }
    } else if (error_value == LOW_BATTERY) {
        LED_main.setFlicker(CRGB::Black, 500, 100);
        for (int i = 0; i < 8; i++) {
            digitalWrite(SOUND, !digitalRead(SOUND));
            delay(100);
        }
    }
    digitalWrite(SOUND, LOW);
    LED_main.turnOn(CRGB::Red);
}

void additional_loop(void* pvParameters) {
    DEBUG_PRINTF("[Main-Micro] addtitional loop begin\n");
    while (true) {
        // Button Event
        mode_a.read();
        mode_b.read();
        trig.read();
        button.read();

        // LED Event
        LED_main.loop();
        LED_interface.loop();
        vTaskDelay(1 / portTICK_PERIOD_MS);
    }
}

void setup() {
    EEPROM.begin(EEPROM_SIZE);

    Serial.begin(115200);
    Serial2.begin(115200, SERIAL_8N1, UART_2_RX, UART_2_TX);
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(SOUND, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);

    // EEPROM.writeBool(ADDR_ESP_NOW_ACTIVE, false);
    // EEPROM.commit();
    /* Initialize ESP-NOW */
    // if (EEPROM.readBool(ADDR_ESP_NOW_ACTIVE)) {
    //     esp_err_t result = initESPNow(ESP_MAC_INDEX, LED_BUILTIN);
    //     DEBUG_PRINTF("[ESP-NOW] Init as %s, err: %s\n", mac_names[ESP_MAC_INDEX], esp_err_to_name(result));
    //     digitalWrite(LED_BUILTIN, HIGH);

    // } else {
    //     DEBUG_PRINTF("[ESP-NOW] Disabled\n");
    // }

    DEBUG_PRINTF("[MAIN-Micro] FW Version: %s\n Last Upload: %s, %s\n", FIRMWARE_VERSION, __DATE__, __TIME__);
    DEBUG_PRINTF("COMMIT ID : %s, ", COMMIT_ID);
    DEBUG_PRINTF("COMMIT DATE : %s, ", COMMIT_DATE);
    DEBUG_PRINTF("BRANCH : %s, ", COMMIT_BRANCH);
    DEBUG_PRINTF("UPLOADER : %s\n", COMMIT_UPLOADER);

    /* Additonal Features (Button & LED)*/
    initButton();
    FastLED.addLeds<WS2812B, LED_PCB, GRB>(leds, 1);
    FastLED.addLeds<WS2812B, LED_INTERFACE, GRB>(leds2, 1);

    /* Startup Sequence */
    /* Initialize Pin Servo Tambahan */
    Mata.init(PIN_MATA);
    Tangan_Kanan.init(PIN_TANGAN_KANAN);
    Tangan_Kiri.init(PIN_TANGAN_KIRI);
    Mata.write(90);

    /* Startup Sequence with Sub program*/
    on_startup_sequence();
    digitalWrite(LED_BUILTIN, LOW);

    DEBUG_PRINTF("[Main-Micro] Startup Sequence Done\n");
    DEBUG_PRINTF("[Main-Micro] Identity: %s[%d]\n", mac_names[EEPROM.readByte(ADDR_IDENTITY)], EEPROM.readByte(ADDR_IDENTITY));
    DEBUG_PRINTF("[Main-Micro] Partner: %s[%d]\n", mac_names[EEPROM.readByte(ADDR_ASSIGNED_PARTNER)], EEPROM.readByte(ADDR_ASSIGNED_PARTNER));
    xTaskCreate(additional_loop, "additional_loop", 8192, NULL, 1, NULL);
    if (EEPROM.readByte(ADDR_USE_BT_SOUND) && EEPROM.readByte(ADDR_IDENTITY) == 2) {
        DEBUG_PRINTF("[Main-Micro] Use BT Sound\n");
        sound = new Sound("Vi-Rose ITS", false);
    } else {
        DEBUG_PRINTF("[Main-Micro] No BT Sound\n");
    }

    /* Initialize Sound Sensor */
    if (sound != nullptr) {
        // #if (SOUND_BT)
        for (int i = 0; i < 6; i++) {
            digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
            delay(50);
        }
        sound->start(&LED_main);
        // #else
        // Kipas.init(PIN_KIPAS);
        // #endif
    }

    EEPROM.readByte(ADDR_IDENTITY) == 2 ? LED_main.turnOn(CRGB::DeepPink) : LED_main.turnOn(CRGB::Cyan);
    delay(100);

    // if replay previous motion
    if (!EEPROM.readBool(PLAY_MOTION_FINISHED)) {
        esp_info.index = EEPROM.readInt(ADDR_LAST_INDEX);
    }
    DEBUG_PRINTF("[MAIN-MIRCRO] Ready\n");
    // LED_main.turnOff();
}

void loop() {
    // Read Button
    // mode_a.read();
    // mode_b.read();
    // trig.read();
    // button.read();

    // Additional Servo
    Tangan_Kanan.loop();
    Tangan_Kiri.loop();
    Mata.loop();

    if (sound != nullptr)
        soundStatus();
    // #else
    //     // Kipas.loop();
    // #endif

    // Check serial communication
    if (Serial2.available()) serial_recv(on_serial_recv_cb);

    // Check if the motion is done and stop the motion
    if (esp_info.mx_status && esp_info.xl_status && esp_info.index == EEPROM.readByte(ADDR_MOTION_MOVIE_LENGTH)) {
        esp_info.mx_status = false;
        esp_info.xl_status = false;
        esp_info.index = 0;
        EEPROM.writeInt(ADDR_LAST_INDEX, esp_info.index);
        EEPROM.writeBool(PLAY_MOTION_FINISHED, true);
        EEPROM.commit();
    }

    // Check if motion is done and play the next motion
    if (esp_info.mx_status && esp_info.xl_status && esp_info.index < EEPROM.readByte(ADDR_MOTION_MOVIE_LENGTH) && esp_info.index > 1) {
        esp_info.mx_status = false;
        esp_info.xl_status = false;
        esp_info.index++;
        EEPROM.writeInt(ADDR_LAST_INDEX, esp_info.index);
        EEPROM.commit();
        byte data_index[9];

        DEBUG_PRINTF("[POST] LAST INDEX: %d\n", esp_info.index);

        if (esp_info.sync_play) {
            data_index[0] = POST_COMUNICATION_CHAIN;
            data_index[1] = EEPROM.readByte(ADDR_ASSIGNED_PARTNER);
            data_index[2] = -1;
            data_index[3] = POST_MOTION_PLAY;
            mempcpy(data_index + 4, &esp_info.index, sizeof(esp_info.index));
            // if (EEPROM.readBool(ADDR_ESP_NOW_ACTIVE))
            //     esp_now_send(mac_addresses[mac_index_target], data_index, sizeof(data_index));
            // else
            serial_send(data_index, sizeof(data_index));
            LED_main.turnOn(CRGB::DeepPink);
            esp_info.sync_play = false;
        }
        memset(data_index, 0, sizeof(data_index));
        data_index[0] = POST_MOTION_PLAY;
        memcpy(data_index + 1, &esp_info.index, sizeof(esp_info.index));
        memcpy(data_index + 5, &esp_info.index, sizeof(esp_info.index));
        // if (EEPROM.readBool(ADDR_ESP_NOW_ACTIVE))
        //     esp_now_send(mac_addresses[mac_index_target], data_index, sizeof(data_index));
        // else
        serial_send(data_index, sizeof(data_index));
        DEBUG_PRINTF("[POST] Play Motion %d\n", esp_info.index);
    }
}