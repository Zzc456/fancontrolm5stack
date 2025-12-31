#include <M5Unified.h>
#include <IRremote.hpp>
#include <WiFi.h>
#include <PubSubClient.h>

// IMPORT CONFIG
#include "config.h" 

#define IR_SEND_PIN 21

// --- GLOBAL VARIABLES ---
WiFiClient espClient;
PubSubClient client(espClient);

// Logic variables
int currentSpeed = 1; 

// --- DYNAMIC THRESHOLDS (Defaults) ---
// These can be overwritten by MQTT messages
int lev0 = 90;  // Slot 1 Range Start
int lev1 = 120; // Slot 2 Threshold
int lev2 = 150; // Slot 3 Threshold
int lev3 = 180; // Slot 4 Threshold
int lev4 = 210; // Slot 5 Threshold

// Averaging variables
const int numReadings = 10;
int readings[numReadings];  
int readIndex = 0;          
int total = 0;              
int average = 0;            

// --- FUNCTION DECLARATIONS ---
void setupWiFi();
void reconnect();
void mqttCallback(char* topic, byte* payload, unsigned int length);
void adjustFanSpeed(int target);
void updateDisplay(int raw, int avg, int slot);

void setup() {
    M5.begin();
    M5.Display.setTextColor(TFT_BLACK);
    M5.Display.setTextFont(&fonts::FreeMonoBoldOblique9pt7b);
    M5.Display.clear(TFT_WHITE);
    
    // 1. Initialize IR
    IrSender.begin(DISABLE_LED_FEEDBACK);
    IrSender.setSendPin(IR_SEND_PIN);

    // 2. Initialize Buffer
    for (int i = 0; i < numReadings; i++) readings[i] = 0;

    // 3. Connect to WiFi
    setupWiFi();

    // 4. Connect to MQTT
    client.setServer(MQTT_SERVER, 1883); 
    client.setCallback(mqttCallback);

    // 5. FAN INITIALIZATION
    M5.Display.println("Resetting Fan...");
    IrSender.sendNEC(IR_ADDR, CMD_ON, 0);
    delay(1000); 
    for(int i=0; i<5; i++) {
        IrSender.sendNEC(IR_ADDR, CMD_DOWN, 0);
        delay(300);
    }
    currentSpeed = 1; 
    
    M5.Display.clear(TFT_WHITE);
    M5.Display.setCursor(0, 0);
    M5.Display.println("Auto Fan Control");
}

void loop() {
    M5.update();
    if (!client.connected()) reconnect();
    client.loop();
}

// --- INTELLIGENT CALLBACK ---
void mqttCallback(char* topic, byte* payload, unsigned int length) {
    // Convert payload to String
    String message;
    for (int i = 0; i < length; i++) message += (char)payload[i];
    int val = message.toInt();
    String strTopic = String(topic);

    // CASE 1: CONFIGURATION UPDATE
    // Check if the topic starts with our config base path
    if (strTopic.startsWith(TOPIC_CONFIG_BASE)) {
        if (strTopic.endsWith("level0")) lev0 = val;
        else if (strTopic.endsWith("level1")) lev1 = val;
        else if (strTopic.endsWith("level2")) lev2 = val;
        else if (strTopic.endsWith("level3")) lev3 = val;
        else if (strTopic.endsWith("level4")) lev4 = val;
        
        Serial.printf("Config Updated: %s = %d\n", topic, val);
        updateDisplay(0, average, currentSpeed); // Refresh screen to show new config
        return; // Done, don't run fan logic
    }

    // CASE 2: DATA STREAM (Fan Control)
    if (strTopic.equals(TOPIC_DATA)) {
        // Run Averaging
        total = total - readings[readIndex];
        readings[readIndex] = val;
        total = total + readings[readIndex];
        readIndex = readIndex + 1;
        if (readIndex >= numReadings) readIndex = 0;
        average = total / numReadings;

        // Calculate Slot based on Dynamic Thresholds
        int targetSlot = 1;

        if (average < lev1) {
            targetSlot = 1; // Below Level 1 is always Slot 1
        } else if (average >= lev1 && average < lev2) {
            targetSlot = 2;
        } else if (average >= lev2 && average < lev3) {
            targetSlot = 3;
        } else if (average >= lev3 && average < lev4) {
            targetSlot = 4;
        } else if (average >= lev4) {
            targetSlot = 5;
        }

        // Adjust Fan
        if (targetSlot != currentSpeed) {
            adjustFanSpeed(targetSlot);
        }

        updateDisplay(val, average, targetSlot);
    }
}

void adjustFanSpeed(int target) {
    if (target > currentSpeed) {
        int steps = target - currentSpeed;
        for (int i = 0; i < steps; i++) {
            IrSender.sendNEC(IR_ADDR, CMD_UP, 0);
            delay(300);
        }
    } else {
        int steps = currentSpeed - target;
        for (int i = 0; i < steps; i++) {
            IrSender.sendNEC(IR_ADDR, CMD_DOWN, 0);
            delay(300);
        }
    }
    currentSpeed = target;
}

void setupWiFi() {
    delay(10);
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    while (WiFi.status() != WL_CONNECTED) delay(500);
}

void reconnect() {
    while (!client.connected()) {
        String clientId = "M5Stack-" + String(random(0xffff), HEX);
        if (client.connect(clientId.c_str())) {
            // Subscribe to Data Stream
            client.subscribe(TOPIC_DATA);
            
            // Subscribe to Config Topics using Wildcard (+)
            // This listens to fitness/control/powermeter/ANYTHING
            String configWildcard = String(TOPIC_CONFIG_BASE) + "+";
            client.subscribe(configWildcard.c_str());
            
            Serial.println("Connected & Subscribed");
        } else {
            delay(5000);
        }
    }
}

void updateDisplay(int raw, int avg, int slot) {
    M5.Display.startWrite(); // Lock display bus
    M5.Display.fillRect(0, 30, 320, 210, TFT_WHITE); 
    M5.Display.setCursor(0, 35);
    
    // Show Current Data
    M5.Display.setTextColor(TFT_BLACK);
    M5.Display.setTextSize(1);
    M5.Display.printf("Incoming: %d  Avg: %d\n", raw, avg);
    
    // Show Current Thresholds (Small text at bottom)
    M5.Display.setTextColor(TFT_BLUE);
    M5.Display.setCursor(0, 70);
    M5.Display.printf("CFG: L0:%d L1:%d\n     L2:%d L3:%d\n     L4:%d\n", 
                      lev0, lev1, lev2, lev3, lev4);

    // Show Big Slot Number
    M5.Display.setTextColor(TFT_MAGENTA);
    M5.Display.setTextSize(3); 
    M5.Display.setCursor(150, 110);
    M5.Display.printf("%d", slot);
    M5.Display.setTextSize(1);
    M5.Display.setCursor(140, 160);
    M5.Display.printf("SLOT");
    
    M5.Display.endWrite();
}