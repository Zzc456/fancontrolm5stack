#ifndef CONFIG_H
#define CONFIG_H

// --- RENAME THIS FILE TO config.h AND EDIT ---

const char* WIFI_SSID     = "YOUR_WIFI_NAME_HERE";
const char* WIFI_PASS     = "YOUR_WIFI_PASS_HERE";
const char* MQTT_SERVER   = "YOUR_MQTT_IP_HERE";
const char* MQTT_TOPIC    = "home/sensors/temp";

// --- IR CODES ---
const uint16_t IR_ADDR    = 0xDE80;
const uint8_t CMD_ON      = 0x00;
const uint8_t CMD_UP      = 0x08;
const uint8_t CMD_DOWN    = 0x10;

#endif