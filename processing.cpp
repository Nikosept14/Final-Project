#include "main.h"
// CRC32 crc;

bool is_get_index = false;

void processing_command(uint8_t *data, int len, int mac_index) {
    COMMAND cmd = (COMMAND)data[0];

    DEBUG_PRINTF("[PROCESS] CMD: %s, PARAMS: ", command_code_to_name(cmd));

    // ====================================================================================================
    // ===================================           MOTION          ======================================
    // ====================================================================================================

    if (cmd == POST_MOTION_STATE) {
        DEBUG_PRINTF("[POST] Motion Status Mode %d\n", data[1]);
        switch (data[1]) {
            case MX:
                DEBUG_PRINTF("[POST] MX Done\n");
                esp_info.mx_status = true;
                break;
            case XL:
                DEBUG_PRINTF("[POST] XL Done\n");
                esp_info.xl_status = true;
                break;
            default:
                break;
        }
    }

    if (cmd == POST_MOTION_MG) {
        DEBUG_PRINTF("[POST] Motion MG ");

        int angle = *(int *)(data + 2);
        int duration = *(int *)(data + 6);

        if (data[1] == MATA) {
            DEBUG_PRINTF("Mata\n");
            DEBUG_PRINTF("[POST] Eye Angle: %d, Duration: %d\n", angle, duration);
            Mata.PlayAngle(angle, duration);
        } else if (data[1] == TANGAN_KANAN) {
            DEBUG_PRINTF("Mata\n");
            DEBUG_PRINTF("[POST] Tangan Kanan Angle: %d, Duration: %d\n", angle, duration);
            Tangan_Kanan.PlayAngle(angle, duration);
        } else if (data[1] == TANGAN_KIRI) {
            DEBUG_PRINTF("Mata\n");
            DEBUG_PRINTF("[POST] Tangan Kiri Angle: %d, Duration: %d\n", angle, duration);
            Tangan_Kiri.PlayAngle(angle, duration);
        } else if (data[1] == KIPAS) {
            DEBUG_PRINTF("Mata\n");
            DEBUG_PRINTF("[POST] Kipas Angle: %d, Duration: %d\n", angle, duration);
            Kipas.PlayAngle(angle, duration);
        }
    }

    if (cmd == POST_MOTION_PLAY) {
        DEBUG_PRINTF("[POST] Motion Play\n");
        esp_info.mx_status = false;
        esp_info.xl_status = false;
        esp_info.index = *(int *)(data + 1);
        EEPROM.writeInt(ADDR_LAST_INDEX, esp_info.index);
        EEPROM.commit();
        // LED_main.turnOn(CRGB::Blue);

        byte data_send[9];
        data_send[0] = POST_MOTION_PLAY;
        memcpy(data_send + 1, &esp_info.index, sizeof(int));
        memcpy(data_send + 5, &esp_info.index, sizeof(int));
        // if (EEPROM.readBool(ADDR_ESP_NOW_ACTIVE))
        //     esp_now_send(mac_addresses[mac_index_target], data_send, sizeof(data_send));
        // else
        serial_send(data_send, sizeof(data_send));
    }

    // ====================================================================================================
    // ===================================           INDEX          ======================================
    // ====================================================================================================

    if (cmd == GET_INDEX) {
        DEBUG_PRINTF("[GET] Index\n");

        uint8_t data_sent[9];
        data_sent[0] = POST_COMUNICATION_CHAIN;
        data_sent[1] = EEPROM.readByte(ADDR_IDENTITY) == 0 ? 3 : 2;
        data_sent[2] = -1;
        data_sent[3] = RESPONSE_INDEX;
        mempcpy(data_sent + 4, &esp_info.index, sizeof(esp_info.index));
        serial_send(data_sent, sizeof(data_sent));
        esp_info.sync_play = true;
        // EEPROM.readByte(ADDR_IDENTITY) == 2 ? LED_main.setFlicker(CRGB::DeepPink, 250, 250); : LED_main.setFlicker(CRGB::Cyan, 250, 250);
    }

    if (cmd == RESPONSE_SERVO_JOINT) {
        DEBUG_PRINTF("[RESPONSE] Servo Joint: %d\n", data[1]);
        // esp_info.xl_joint = data[1];
        // esp_info.mx_joint = data[2];
    }

    if (cmd == POST_INDEX_CHANGED) {
        DEBUG_PRINTF("[POST] Index Changed %d\n", data[1]);
        if (is_get_index) {
            // mx_status = true;
            // xl_status = true;
            is_get_index = false;
        }
    }

    if (cmd == RESPONSE_INDEX) {
        DEBUG_PRINTF("[RESPONSE] Index %d\n", data[1]);

        byte data_send[9];
        data_send[0] = POST_MOTION_PLAY;
        int index_init = 1;
        memcpy(data_send + 1, &index_init, sizeof(index_init));
        memcpy(data_send + 5, &index_init, sizeof(index_init));
        // if (EEPROM.readBool(ADDR_ESP_NOW_ACTIVE))
        //     esp_now_send(mac_addresses[mac_index_target], data_send, sizeof(data_send));
        // else
        EEPROM.readByte(ADDR_IDENTITY) == 2 ? LED_main.setFlicker(CRGB::DeepPink, 250, 250) : LED_main.setFlicker(CRGB::Cyan, 250, 250);
        serial_send(data_send, sizeof(data_send));

        esp_info.index = data[1];
        EEPROM.writeInt(ADDR_LAST_INDEX, esp_info.index);
        EEPROM.commit();
        is_get_index = true;
    }

    if (cmd == RESPONSE_SPIFFS_MOTION) {
        if (data[1] == INIT) {
            DEBUG_PRINTF("[RESPONSE] SPIFFS Motion Init\n");
            LED_main.setFlicker(CRGB::Green, 250, 250);
            EEPROM.writeByte(ADDR_MOTION_MOVIE_LENGTH, data[2]);
            EEPROM.commit();
        }
    }

    if (cmd == RESPONSE_BATTERY_VOLTAGES) {
        int battery_voltage;
        mempcpy(&battery_voltage, data + 1, sizeof(battery_voltage));
        DEBUG_PRINTF("[RESPONSE] Battery Voltage: %f\n", battery_voltage);

        displayRobot.setBattery(battery_voltage);
    }

    if (cmd == RESPONSE_SYSTEM_CHECKUP) {
        DEBUG_PRINTF("[RESPONSE] System Checkup\n");
        DEBUG_PRINTF("[Main-Micro] Error Checking\n");
        for (int i = 0; i < len - 2; i++) {
            if (data[i + 2]) {
                error_feedback(data[i + 2], data[1]);
            }
        }
    }

    if (cmd == ESP_RESTART) {
        DEBUG_PRINTF("[RESPONSE] ESP RESTART\n");
        delay(1000);
        ESP.restart();
    }

    if (cmd == PING) {
        DEBUG_PRINTF("[RESPONSE] PING\n");
        byte mac_index = EEPROM.readByte(ADDR_IDENTITY);
        if (len < 2) {
            byte data_sent[2] = {
                RESPONSE_PING, mac_index};
            serial_send(data_sent, sizeof(data_sent));
        } else if (len < 3) {
            byte data_sent[4] = {POST_COMUNICATION_CHAIN, data[1], RESPONSE_PING, mac_index};
            serial_send(data_sent, sizeof(data_sent));
        } else if (len < 4) {
            byte data_sent[5] = {POST_COMUNICATION_CHAIN, data[1], data[2], RESPONSE_PING, mac_index};
            serial_send(data_sent, sizeof(data_sent));
        } else {
            byte data_sent[6] = {POST_COMUNICATION_CHAIN, data[1], data[2], data[3], RESPONSE_PING, mac_index};
            serial_send(data_sent, sizeof(data_sent));
        }
        for (int i = 0; i < 10; i++) {
            digitalWrite(2, !digitalRead(2));
        }
    }
    if (cmd == SET_IDENTITY) {
        EEPROM.writeByte(ADDR_IDENTITY, data[1]);
        EEPROM.writeByte(ADDR_ASSIGNED_PARTNER, data[2]);
        EEPROM.commit();
        DEBUG_PRINTF("[Main-Micro] Identity Set to %s[%d]\n", mac_names[EEPROM.readByte(ADDR_IDENTITY)], EEPROM.readByte(ADDR_IDENTITY));
        DEBUG_PRINTF("[Main-Micro] Partner Set to %s[%d]\n", mac_names[EEPROM.readByte(ADDR_ASSIGNED_PARTNER)], EEPROM.readByte(ADDR_ASSIGNED_PARTNER));
        // if (esp_mac_index == 0 || esp_mac_index == 1) {
        // } else {
        //     DEBUG_PRINTF("[Main-Micro] Identity Unknown neither of 0 or 1");
        //     DEBUG_PRINTF("[Main-Micro] Identity Set to %s[%d]\n", mac_names[EEPROM.readByte(ADDR_IDENTITY)], EEPROM.readByte(ADDR_IDENTITY));
        //     EEPROM.writeByte(ADDR_IDENTITY, 0);
        // }
        LED_main.turnOnFor(CRGB::Blue, 500);
        delay(500);
        // DEBUG_PRINTF("[Main-Micro] Initiating Reboot");
        // byte data_sent[1] = {ESP_RESTART};
        // serial_send(data_sent, sizeof(data_sent), -1);
        // delay(500);
        ESP.restart();
    }

    if (cmd == GET_BT_STATUS) {
        byte data_sent[3] = {RESPONSE_BT_STATUS, EEPROM.readByte(ADDR_USE_BT_SOUND), sound->getConnectionStatus()};
        serial_send(data_sent, sizeof(data_sent));
    }
}