#ifndef CONFIG_H
#define CONFIG_H

// --- USER SECRETS ---
const char* WIFI_SSID     = "YOUR_WIFI_SSID";
const char* WIFI_PASS     = "YOUR_WIFI_PASS";
const char* MQTT_SERVER   = "YOUR_MQTT_IP";

// --- MQTT TOPICS ---
const char* TOPIC_DATA    = "home/sensors/temp";
const char* TOPIC_LEVELS  = "fitness/control/powermeter/"; 

// --- DEFAULTS ---
const int DEFAULT_LEVELS[6] = { 0, 90, 120, 150, 180, 210 };

// --- IR CODES ---
const uint16_t IR_ADDR    = 0xDE80;
const uint8_t CMD_ON      = 0x00;
const uint8_t CMD_UP      = 0x08;
const uint8_t CMD_DOWN    = 0x10;

#endif