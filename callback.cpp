#include "main.h"

int temp_esp_index = 0;
// int mac_index_target = EEPROM.readByte(ADDR_IDENTITY) == 0 ? 2 : 3;
int mac_index_target = EEPROM.readByte(ADDR_IDENTITY);
int mac_index_partner = EEPROM.readByte(ADDR_ASSIGNED_PARTNER);
byte data_button[9];

bool paused = false;
byte cmd_status = 0;

void soundStatus() {
    // Serial.println("[Sound] Sound Status : ");
    // Serial.println(sound->getOutputSoundA2DP());
    byte data_sound[9];
    if (sound->getOutputSoundA2DP() == 1 && esp_info.index == 1) {
        EEPROM.readByte(ADDR_IDENTITY) == 2 ? LED_main.turnOn(CRGB::DeepPink) : LED_main.turnOn(CRGB::Cyan);

        esp_info.mx_status = false;
        esp_info.xl_status = false;
        esp_info.index = 2;
        EEPROM.writeBool(PLAY_MOTION_FINISHED, false);
        EEPROM.writeInt(ADDR_LAST_INDEX, esp_info.index);
        EEPROM.commit();
        // data_sound[0] = POST_COMUNICATION_CHAIN;
        // data_sound[1] = EEPROM.readByte(ADDR_ASSIGNED_PARTNER);
        // data_sound[2] = -1;
        // data_sound[3] = POST_MOTION_PLAY;
        // memcpy(data_sound + 4, &esp_info.index, sizeof(esp_info.index));
        // // if (EEPROM.readBool(ADDR_ESP_NOW_ACTIVE))
        // //     esp_now_send(mac_addresses[mac_index_target], data_sound, sizeof(data_sound));
        // // else
        // serial_send(data_sound, sizeof(data_sound));

        // memset(data_sound, 0, sizeof(data_sound));
        data_sound[0] = POST_MOTION_PLAY;
        memcpy(data_sound + 1, &esp_info.index, sizeof(esp_info.index));
        memcpy(data_sound + 5, &esp_info.index, sizeof(esp_info.index));
        // if (EEPROM.readBool(ADDR_ESP_NOW_ACTIVE))
        //     esp_now_send(mac_addresses[mac_index_target], data_sound, sizeof(data_sound));
        // else
        serial_send(data_sound, sizeof(data_sound));
        DEBUG_PRINTF("[Sound] Active\nRobot Berjalan\n\n");
        esp_info.sound = true;
    } else if (sound->getOutputSoundA2DP() == 0 && esp_info.index > 1 && !paused) {
        // pause by sound
        byte data_soud_status[3];
        data_soud_status[0] = POST_MOTION_STATE;
        data_soud_status[1] = (STATE)PAUSE;
        data_soud_status[2] = (STATE)PAUSE;
        serial_send(data_soud_status, sizeof(data_soud_status));
        DEBUG_PRINTF("[Sound] Inactive\nRobot Berhenti\n\n");
        paused = true;
        esp_info.sound = false;
    } else if (sound->getOutputSoundA2DP() == 1 && esp_info.index > 1 && paused) {
        // resume by sound
        byte data_soud_status[3];
        data_soud_status[0] = POST_MOTION_STATE;
        data_soud_status[1] = (STATE)RESUME;
        data_soud_status[2] = (STATE)RESUME;
        serial_send(data_soud_status, sizeof(data_soud_status));
        DEBUG_PRINTF("[Sound] Active\nRobot Berjalan\n\n");
        paused = false;
    }
}

void on_serial_recv_cb(const uint8_t *data, int len, int mac_index) {
    DEBUG_PRINTF("[SERIAL] RECV Data: ");
    for (int i = 0; i < len; i++) DEBUG_PRINTF("%d ", data[i]);
    DEBUG_PRINTF("\n");

    processing_command((uint8_t *)data, len, mac_index);
}

void on_data_sent_cb(const uint8_t *mac, esp_now_send_status_t status) {
    uint8_t mac_index = getMACIndex(mac);
    ledStatus();

    // DEBUG_PRINTF("[ESP-NOW] SEND to %s, Status: %s\n", mac_names[mac_index], status == ESP_NOW_SEND_SUCCESS ? "Success" : "Fail");
}

void on_data_recv_cb(const uint8_t *mac, const uint8_t *data, int len) {
    uint8_t mac_index = getMACIndex(mac);
    ledStatus();

    DEBUG_PRINTF("[ESP-NOW] RECV from %s, Data: ", mac_names[mac_index]);
    if (len > 16) {
        for (int i = 0; i < 16; i++) DEBUG_PRINTF("%d ", data[i]);
        DEBUG_PRINTF("...(continue) \n");
    } else {
        for (int i = 0; i < len; i++) DEBUG_PRINTF("%d ", data[i]);
        DEBUG_PRINTF("\n");
    }

    processing_command((uint8_t *)data, len, mac_index);
}

// Init Motion
void Button_Mode_A_Callback() {
    DEBUG_PRINTF("\n[Button] A Pressed\n");
    LED_interface.turnOn(CRGB::Green);
    // #if SOUND_BT == 1
    if (sound != nullptr) {
        if (!sound->getConnectionStatus()) {
            sound->connect(0);
            LED_main.setFlicker(CRGB::Snow, 250, 250);
        } else {
            LED_main.turnOn(CRGB::Green);
        }
    }
    // #endif
    temp_esp_index = 1;
    data_button[0] = POST_MOTION_PLAY;
    memcpy(data_button + 1, &temp_esp_index, sizeof(temp_esp_index));
    memcpy(data_button + 5, &temp_esp_index, sizeof(temp_esp_index));
    if (!EEPROM.readBool(PLAY_MOTION_FINISHED)) {
        EEPROM.writeBool(PLAY_MOTION_FINISHED, true);
        EEPROM.commit();
    }
}

// Play Motion
void Button_Mode_A_2_Sequence_Callback() {
    DEBUG_PRINTF("\n[Button] A 2 Sequence Pressed\n");
    LED_interface.setFade(CRGB::Green, 250, 250);
    temp_esp_index = 2;
    data_button[0] = POST_MOTION_PLAY;
    memcpy(data_button + 1, &temp_esp_index, sizeof(temp_esp_index));
    memcpy(data_button + 5, &temp_esp_index, sizeof(temp_esp_index));
    if (!EEPROM.readBool(PLAY_MOTION_FINISHED)) {
        EEPROM.writeBool(PLAY_MOTION_FINISHED, true);
        EEPROM.commit();
    }
}

void Button_Mode_A_3_Sequence_Callback() {
    DEBUG_PRINTF("\n[Button] Mode A 3 Sequence\n");
    LED_interface.turnOff();
}

// MOTION Request to other robot
void Button_Mode_A_Hold_Callback() {
    DEBUG_PRINTF("\n[Button] Mode A Hold\n");
    LED_interface.setFade(CRGB::Yellow, 250, 250);
    // DEBUG_PRINTF("[Button] Mode A Hold\n");
    temp_esp_index = 1;

    data_button[0] = POST_COMUNICATION_CHAIN;
    data_button[1] = EEPROM.readByte(ADDR_ASSIGNED_PARTNER);
    data_button[2] = -1;
    data_button[3] = GET_INDEX;
    memcpy(data_button + 4, &temp_esp_index, sizeof(temp_esp_index));
    if (!EEPROM.readBool(PLAY_MOTION_FINISHED)) {
        EEPROM.writeBool(PLAY_MOTION_FINISHED, true);
        EEPROM.commit();
    }
}

// Connect to Bluetooth Transmitter
void Button_Mode_B_Callback() {
    DEBUG_PRINTF("\n[Button] B Pressed\n");
    LED_interface.turnOff();
    // #if (SOUND_BT == 1)
    if (sound != nullptr) {
        if (!sound->getConnectionStatus()) {
            sound->connect(1);
            LED_main.setFlicker(CRGB::Amethyst, 250, 250);
        } else {
            LED_main.turnOn(CRGB::Green);
        }
    }
}

// Motion_Play Sync
void Button_Mode_B_2_Sequence_Callback() {
    DEBUG_PRINTF("\n[Button] B 2 Sequence Pressed\n");
    LED_interface.setFade(CRGB::Blue, 250, 250);
    temp_esp_index = 2;
    data_button[0] = POST_COMUNICATION_CHAIN;
    data_button[1] = EEPROM.readByte(ADDR_ASSIGNED_PARTNER);
    data_button[2] = -1;
    data_button[3] = POST_MOTION_PLAY;
    memcpy(data_button + 4, &temp_esp_index, sizeof(temp_esp_index));
    if (!EEPROM.readBool(PLAY_MOTION_FINISHED)) {
        EEPROM.writeBool(PLAY_MOTION_FINISHED, true);
        EEPROM.commit();
    }
}

void Button_Mode_B_3_Sequence_Callback() {
    DEBUG_PRINTF("\n[Button] B 3 Sequence Pressed\n");
    LED_interface.turnOff();
}

// MOTION Recovery Upon Robot Reboot
void Button_Mode_B_Hold_Callback() {
    DEBUG_PRINTF("\n[Button] B Hold Pressed\n");
    temp_esp_index = EEPROM.readInt(ADDR_LAST_INDEX);
    data_button[0] = POST_COMUNICATION_CHAIN;
    data_button[1] = EEPROM.readByte(ADDR_ASSIGNED_PARTNER);
    data_button[2] = -1;
    data_button[3] = POST_MOTION_PLAY;
    memcpy(data_button + 4, &temp_esp_index, sizeof(temp_esp_index));
    LED_interface.setFlicker(CRGB::Blue, 250, 250);
    if (!EEPROM.readBool(PLAY_MOTION_FINISHED)) {
        EEPROM.writeBool(PLAY_MOTION_FINISHED, true);
        EEPROM.commit();
    }
}

void Button_Trig_Callback() {
    DEBUG_PRINTF("\n[Button] Trig Pressed\n");
    DEBUG_PRINTF("Command Sent\n");
    LED_interface.turnOff();

    // if (EEPROM.readBool(ADDR_ESP_NOW_ACTIVE)) {
    //     esp_now_send(mac_addresses[mac_index_target], data_button, sizeof(data_button));
    // } else {
    serial_send(data_button, sizeof(data_button));
    // }
    if (data_button[0] == POST_MOTION_PLAY) {
        if (temp_esp_index == 1) {
            esp_info.mx_status = false;
            esp_info.xl_status = false;
            esp_info.index = 1;

        } else if (temp_esp_index == 2) {
            esp_info.mx_status = false;
            esp_info.xl_status = false;
            esp_info.index = 2;
            EEPROM.writeBool(PLAY_MOTION_FINISHED, false);
            EEPROM.writeInt(ADDR_LAST_INDEX, esp_info.index);
            EEPROM.commit();
        }
    } else if (data_button[0] == POST_COMUNICATION_CHAIN) {
        if (data_button[3] != GET_INDEX) {
            // LED_main.turnOn(CRGB::DeepPink);
        } else {
            LED_main.setFlicker(CRGB::Yellow, 250, 250);
        }

        if (temp_esp_index == 1) {
            esp_info.mx_status = false;
            esp_info.xl_status = false;
            esp_info.index = 1;
            EEPROM.writeBool(PLAY_MOTION_FINISHED, false);
            EEPROM.commit();
        } else if (temp_esp_index == 2) {
            esp_info.mx_status = false;
            esp_info.xl_status = false;
            esp_info.index = 2;
            EEPROM.writeInt(ADDR_LAST_INDEX, esp_info.index);
            EEPROM.writeBool(PLAY_MOTION_FINISHED, false);
            EEPROM.commit();
        } else if (temp_esp_index == EEPROM.readInt(ADDR_LAST_INDEX)) {
            esp_info.mx_status = false;
            esp_info.xl_status = false;
            esp_info.index = EEPROM.readInt(ADDR_LAST_INDEX);
            EEPROM.writeBool(PLAY_MOTION_FINISHED, false);
            EEPROM.commit();
        }
        memset(data_button, 0, sizeof(data_button));
        data_button[0] = POST_MOTION_PLAY;
        memcpy(data_button + 1, &temp_esp_index, sizeof(temp_esp_index));
        memcpy(data_button + 5, &temp_esp_index, sizeof(temp_esp_index));
        // if (EEPROM.readBool(ADDR_ESP_NOW_ACTIVE)) {
        //     esp_now_send(mac_addresses[mac_index_target], data_button, sizeof(data_button));
        // } else {
        // }
        serial_send(data_button, sizeof(data_button));
    }
}

void Button_Trig_2_Sequence_Callback() {
    DEBUG_PRINTF("[Button] Trig 2 Sequence\n");
}

void Button_Trig_Hold_Callback() {
    DEBUG_PRINTF("[Button] Trig Hold 1s\n");
    // displayRobot.switchFocus();
}

// void serial_event() {
//     int serial_cmd = 0;
//     DEBUG_PRINTF("[DEBUG] Serial Command: %d\n", serial_cmd);

//     if (serial_cmd == 1) {
//         byte command = Serial.parseInt();
//         int data = Serial.parseInt();

//         uint8_t data_sent[9];
//         memset(data_sent, 0, sizeof(data_sent));
//         data_sent[0] = command;

//         memcpy(data_sent + 1, &data, sizeof(data));
//         memcpy(data_sent + 5, &data, sizeof(data));
//         esp_info.index = data;

//         DEBUG_PRINTF("[DEBUG] Send to %s, Command: %d, Data: %d\n", mac_names[mac_index_target], command, data);
//         serial_send(data_sent, sizeof(data_sent));
//     }

//     if (serial_cmd == 2) {
//         Serial2.write(0x01);
//         Serial1.write(0x01);
//     }
// }

// void OLED_read() {
//     if (displayRobot.getPath() == "/0/0/0") {
//         DEBUG_PRINTF("TOQUE ON/OFF");
//     } else if (displayRobot.getPath() == "/0/0/1") {
//         data_button[0] = GET_SERVO_JOINT;
//         serial_send(data_button, sizeof(data_button));
//         DEBUG_PRINTF("Get MX");
//     } else if (displayRobot.getPath() == "/0/0/2/" + String(displayRobot.path[3]) && !displayRobot.getMotion) {
//         data_button[0] = GET_SPIFFS_MOTION;
//         data_button[1] = displayRobot.path[3];
//         // esp_now_send(mac_addresses[mac_index], data_button, 2);
//     } else if (displayRobot.getPath() == "/0/0/3/" + String(displayRobot.path[3]) + "/" + String(displayRobot.path[4]) && displayRobot.getMode() != 99) {
//         File file;
//         JsonDocument doc;
//         int idmx;
//         int idxl;
//         if (displayRobot.path[3] == 0 || displayRobot.path[3] == 2) {
//             file = SPIFFS.open("/active_mx_bucket.json", FILE_READ);
//             deserializeJson(doc, file);
//             JsonArray motion_movie = doc["motion_movie"];
//             for (JsonVariant v : motion_movie) {
//                 if (v["name"] == displayRobot.postMotion[displayRobot.path[4]]) {
//                     idmx = v["id"];

//                     DEBUG_PRINTF(" " + String(idmx) + "\n");
//                 }
//             }
//             file.close();
//         }
//         if (displayRobot.path[3] == 1 || displayRobot.path[3] == 0) {
//             file = SPIFFS.open("/active_xl_bucket.json", FILE_READ);
//             deserializeJson(doc, file);
//             JsonArray motion_movie2 = doc["motion_movie"];
//             for (JsonVariant v : motion_movie2) {
//                 if (v["name"] == displayRobot.postMotion[displayRobot.path[4]]) {
//                     idxl = v["id"];

//                     DEBUG_PRINTF(" " + String(idxl) + "\n");
//                 }
//             }
//             file.close();
//         }

//         uint8_t data_sent[9] = {POST_MOTION_PLAY};
//         memcpy(data_sent + 1, &idxl, sizeof(idxl));
//         memcpy(data_sent + 5, &idmx, sizeof(idmx));
//         // esp_now_send(mac_addresses[mac_index], data_sent, sizeof(data_sent));
//     } else if (displayRobot.getPath() == "/0/0/4/" + String(displayRobot.path[3]) + "/" + String(displayRobot.path[4]) && displayRobot.getMode() == 99) {
//         data_button[0] = POST_MOTION_STATE;
//         data_button[1] =  // displayRobot.getKursor() % 2;
//             serial_send(data_button, sizeof(data_button));
//         DEBUG_PRINTF("POST_MOTION_STATE %d \n", data_button[1]);
//     }
// }