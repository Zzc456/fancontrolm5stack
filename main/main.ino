#include <M5Unified.h>
#include <IRremote.hpp>
#include <WiFi.h>
#include <PubSubClient.h>

#include "config.h" 

#define IR_SEND_PIN 21

// --- GLOBAL VARIABLES ---
WiFiClient espClient;
PubSubClient client(espClient);

// Logic variables
int currentSpeed = 1;
int thresholds[6]; // Stores our dynamic limits (0 to 5)

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

    // 2. Load Defaults from config.h
    for(int i=0; i<6; i++) {
        thresholds[i] = DEFAULT_LEVELS[i];
    }

    // 3. Initialize Averaging Buffer
    for (int i = 0; i < numReadings; i++) readings[i] = 0;

    // 4. Setup Network
    setupWiFi();
    client.setServer(MQTT_SERVER, 1883); 
    client.setCallback(mqttCallback);

    // 5. Fan Reset Sequence
    M5.Display.setCursor(10, 10);
    M5.Display.println("Resetting Fan...");
    IrSender.sendNEC(IR_ADDR, CMD_ON, 0);
    delay(1000); 
    for(int i=0; i<5; i++) {
        IrSender.sendNEC(IR_ADDR, CMD_DOWN, 0);
        delay(300);
    }
    currentSpeed = 1; 
    
    M5.Display.clear(TFT_WHITE);
    updateDisplay(0, 0, 1);
}

void loop() {
    M5.update();
    if (!client.connected()) reconnect();
    client.loop();
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
    // 1. Convert Payload to Int
    String message;
    for (int i = 0; i < length; i++) message += (char)payload[i];
    int value = message.toInt();
    String strTopic = String(topic);

    // --- CASE A: IT IS A CONFIGURATION UPDATE ---
    // Check if topic starts with "fitness/control/powermeter/"
    if (strTopic.startsWith(TOPIC_LEVELS)) {
        // Find which level it is (last character)
        // e.g. "fitness/control/powermeter/level3" -> char '3'
        char levelChar = strTopic.charAt(strTopic.length() - 1);
        int levelIndex = levelChar - '0'; // Convert char '3' to int 3

        if (levelIndex >= 0 && levelIndex <= 5) {
            thresholds[levelIndex] = value;
            Serial.printf("Updated Level %d threshold to %d\n", levelIndex, value);
        }
        return; // Don't run fan logic for config updates
    }

    // --- CASE B: IT IS DATA (Update Fan) ---
    if (strTopic.equals(TOPIC_DATA)) {
        // 1. Update Average
        total = total - readings[readIndex];
        readings[readIndex] = value;
        total = total + readings[readIndex];
        readIndex = readIndex + 1;
        if (readIndex >= numReadings) readIndex = 0;
        average = total / numReadings;

        // 2. Determine Target Slot (Dynamic Logic)
        // We check from highest (5) down to lowest (1)
        int targetSlot = 1; // Default minimum

        for (int i = 5; i >= 1; i--) {
            if (average >= thresholds[i]) {
                targetSlot = i;
                break; // Found our slot, stop checking
            }
        }

        // 3. Adjust Fan
        if (targetSlot != currentSpeed) {
            adjustFanSpeed(targetSlot);
        }

        updateDisplay(value, average, targetSlot);
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
    M5.Display.println();
    M5.Display.print("Connecting to ");
    // Use variable from config.h
    M5.Display.println(WIFI_SSID); 
    
    // Use variables from config.h
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        M5.Display.print(".");
    }
    M5.Display.println("WiFi connected");
}

void reconnect() {
    while (!client.connected()) {
        M5.Display.print("Conn MQTT...");
        String clientId = "M5Stack-" + String(random(0xffff), HEX);
        
        if (client.connect(clientId.c_str())) {
            M5.Display.println("OK");
            
            // 1. Subscribe to Data Stream
            client.subscribe(TOPIC_DATA);
            
            // 2. Subscribe to ALL Config Levels (level0 - level5)
            // We construct the string "fitness/control/powermeter/levelX"
            for(int i=0; i<=5; i++) {
                String configTopic = String(TOPIC_LEVELS) + "level" + String(i);
                client.subscribe(configTopic.c_str());
                Serial.print("Subscribed: "); Serial.println(configTopic);
            }
            
        } else {
            M5.Display.print("Fail rc=");
            M5.Display.print(client.state());
            delay(2000);
        }
    }
}

void updateDisplay(int raw, int avg, int slot) {
    // Only redraw the bottom half to avoid flicker
    M5.Display.fillRect(0, 40, 320, 200, TFT_WHITE); 
    
    M5.Display.setCursor(0, 45);
    M5.Display.printf("Input: %d  (Avg: %d)\n", raw, avg);
    
    // Show current Thresholds for debugging
    M5.Display.setTextSize(1);
    M5.Display.setCursor(0, 70);
    M5.Display.printf("L1:%d L2:%d L3:%d \nL4:%d L5:%d", 
        thresholds[1], thresholds[2], thresholds[3], thresholds[4], thresholds[5]);

    M5.Display.setTextSize(3); 
    M5.Display.setCursor(60, 130);
    M5.Display.setTextColor(TFT_MAGENTA);
    M5.Display.printf("SLOT %d", slot);
    M5.Display.setTextColor(TFT_BLACK);
    M5.Display.setTextSize(1); 
}