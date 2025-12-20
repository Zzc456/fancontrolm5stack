#include <M5Unified.h>
#include <IRremote.hpp>
#include <WiFi.h>
#include <PubSubClient.h>

#include "config.h"

// --- GLOBAL VARIABLES ---
WiFiClient espClient;
PubSubClient client(espClient);

// Logic variables
int currentSpeed = 1; // We track the fan's actual speed (1-5) here

// Averaging variables
const int numReadings = 10;
int readings[numReadings];  // The readings from the analog input
int readIndex = 0;          // The index of the current reading
int total = 0;              // The running total
int average = 0;            // The average

void setup() {
    M5.begin();
    M5.Display.setTextColor(TFT_BLACK);
    M5.Display.setTextFont(&fonts::FreeMonoBoldOblique9pt7b);
    M5.Display.clear(TFT_WHITE);
    
    // 1. Initialize IR
    IrSender.begin(DISABLE_LED_FEEDBACK);
    IrSender.setSendPin(IR_SEND_PIN);

    // 2. Initialize Averaging Buffer
    for (int i = 0; i < numReadings; i++) {
        readings[i] = 0;
    }

    // 3. Connect to WiFi
    setupWiFi();

    // 4. Connect to MQTT
    client.setServer(mqtt_server, 1883); // Default MQTT port
    client.setCallback(mqttCallback);

    // 5. FAN INITIALIZATION SEQUENCE
    M5.Display.setCursor(10, 10);
    M5.Display.println("Resetting Fan...");
    
    // A) Turn On
    IrSender.sendNEC(IR_ADDR, CMD_ON, 0);
    delay(1000); 
    
    // B) Reset to Lowest (Send DOWN 5 times)
    for(int i=0; i<5; i++) {
        IrSender.sendNEC(IR_ADDR, CMD_DOWN, 0);
        delay(300);
    }
    currentSpeed = 1; // We are now confident fan is at Speed 1
    
    M5.Display.clear(TFT_WHITE);
    M5.Display.setCursor(0, 0);
    M5.Display.println("Auto Fan Control");
    M5.Display.println("----------------");
}

void loop() {
    M5.update();
    
    // Ensure MQTT is connected
    if (!client.connected()) {
        reconnect();
    }
    client.loop(); // Check for new messages
}

// --- MQTT CALLBACK (Where the magic happens) ---
void mqttCallback(char* topic, byte* payload, unsigned int length) {
    // 1. Convert payload to String then Int
    String message;
    for (int i = 0; i < length; i++) {
        message += (char)payload[i];
    }
    int newValue = message.toInt();

    // 2. Calculate Moving Average
    // Subtract the last reading
    total = total - readings[readIndex];
    // Read new value
    readings[readIndex] = newValue;
    // Add the reading to the total
    total = total + readings[readIndex];
    // Advance to the next position in the array
    readIndex = readIndex + 1;
    // If we're at the end of the array...
    if (readIndex >= numReadings) {
        readIndex = 0; // ...wrap around to the beginning
    }
    // Calculate the average
    average = total / numReadings;

    // 3. Determine Target Slot based on Average
    int targetSlot = 1;

    if (average < 90) {
        targetSlot = 1; // Safety: below 90 stays at 1
    } else if (average >= 90 && average < 120) {
        targetSlot = 1;
    } else if (average >= 120 && average < 150) {
        targetSlot = 2;
    } else if (average >= 150 && average < 180) {
        targetSlot = 3;
    } else if (average >= 180 && average < 210) {
        targetSlot = 4;
    } else if (average >= 210) {
        targetSlot = 5;
    }

    // 4. Adjust Fan if needed
    if (targetSlot != currentSpeed) {
        adjustFanSpeed(targetSlot);
    }

    // 5. Update Display
    updateDisplay(newValue, average, targetSlot);
}

void adjustFanSpeed(int target) {
    // Determine direction
    if (target > currentSpeed) {
        // Need to go UP
        int steps = target - currentSpeed;
        for (int i = 0; i < steps; i++) {
            IrSender.sendNEC(IR_ADDR, CMD_UP, 0);
            delay(300); // Wait for fan to register
        }
    } else {
        // Need to go DOWN
        int steps = currentSpeed - target;
        for (int i = 0; i < steps; i++) {
            IrSender.sendNEC(IR_ADDR, CMD_DOWN, 0);
            delay(300);
        }
    }
    // Update our tracking variable
    currentSpeed = target;
}

// --- NETWORK HELPER FUNCTIONS ---
void setupWiFi() {
    delay(10);
    M5.Display.println();
    M5.Display.print("Connecting to ");
    M5.Display.println(ssid);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        M5.Display.print(".");
    }
    M5.Display.println("WiFi connected");
}

void reconnect() {
    while (!client.connected()) {
        M5.Display.print("Attempting MQTT connection...");
        String clientId = "M5StackClient-";
        clientId += String(random(0xffff), HEX);
        
        if (client.connect(clientId.c_str())) {
            M5.Display.println("connected");
            client.subscribe(mqtt_topic);
        } else {
            M5.Display.print("failed, rc=");
            M5.Display.print(client.state());
            M5.Display.println(" try again in 5s");
            delay(5000);
        }
    }
}

void updateDisplay(int raw, int avg, int slot) {
    // Only update the dynamic part of screen
    M5.Display.fillRect(0, 40, 320, 200, TFT_WHITE); 
    M5.Display.setCursor(0, 50);
    
    M5.Display.printf("Raw In: %d\n", raw);
    M5.Display.printf("Avg 10: %d\n", avg);
    
    M5.Display.setTextSize(2); // Make speed large
    M5.Display.setCursor(0, 100);
    M5.Display.printf("FAN SLOT: %d", slot);
    M5.Display.setTextSize(1); // Reset size
}