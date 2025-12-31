#ifndef CONFIG_H
#define CONFIG_H

// --- SENSITIVE DATA ---
const char* WIFI_SSID     = "YOUR_WIFI_NAME";
const char* WIFI_PASS     = "YOUR_WIFI_PASSWORD";
const char* MQTT_SERVER   = "192.168.1.100";

// --- TOPICS ---
// The stream of numbers (Wattage/Speed)
const char* TOPIC_DATA    = "home/sensors/temp"; 

// The base path for settings. 
// We will listen to "fitness/control/powermeter/+"
const char* TOPIC_CONFIG_BASE = "fitness/control/powermeter/";

// --- IR CODES ---
const uint16_t IR_ADDR    = 0xDE80;
const uint8_t CMD_ON      = 0x00;
const uint8_t CMD_UP      = 0x08;
const uint8_t CMD_DOWN    = 0x10;

#endif