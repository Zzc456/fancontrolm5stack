#include <IRremote.hpp>
#include <M5Unified.h>

#define IR_SEND_PIN    21      // GPIO pin for IR transmitter
#define IR_RECEIVE_PIN 22      // GPIO pin for IR receiver

// Variable to store the captured IR signal
IRData storedIRData;
bool hasStoredSignal = false;

void setup() {
    M5.begin();
    Serial.begin(115200);
    
    // --- Display Setup ---
    M5.Display.setTextColor(TFT_BLACK);
    M5.Display.setTextFont(&fonts::FreeMonoBoldOblique9pt7b);
    M5.Display.clear(TFT_WHITE);
    
    // --- IR Setup ---
    IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK); // Start Receiver
    IrSender.begin(DISABLE_LED_FEEDBACK);                  // Start Sender
    IrSender.setSendPin(IR_SEND_PIN);

    // --- Initial UI ---
    M5.Display.setCursor(10, 10);
    M5.Display.println("IR LEARNING REMOTE");
    M5.Display.println("------------------");
    M5.Display.println("Btn A: LEARN (Recv)");
    M5.Display.println("Btn B: SEND (Replay)");
    M5.Display.println("------------------");
    M5.Display.println("Status: Ready.");
}

void loop() {
    M5.update(); // Update button states

    // ---------------------------------------------------------
    // BUTTON A: RECEIVE & STORE (One-shot capture)
    // ---------------------------------------------------------
    if (M5.BtnA.wasPressed()) {
        M5.Display.fillRect(0, 110, 320, 130, TFT_WHITE); // Clear status area
        M5.Display.setCursor(0, 120);
        M5.Display.setTextColor(TFT_BLUE);
        M5.Display.println("Waiting for signal...");
        Serial.println("Entering Receive Mode...");

        // Resume receiver to clear buffers and prepare
        IrReceiver.resume(); 

        // Loop until we get a signal or user cancels (optional)
        bool signalReceived = false;
        while (!signalReceived) {
            M5.update(); // Keep checking buttons if you want a cancel feature
            
            if (IrReceiver.decode()) {
                // Check if it's a valid protocol (not just noise)
                if (IrReceiver.decodedIRData.protocol != UNKNOWN) {
                    
                    // 1. STORE THE DATA
                    storedIRData = IrReceiver.decodedIRData;
                    hasStoredSignal = true;
                    signalReceived = true;
                    // --- ADD THIS BLOCK TO SEND TO PC ---
                    Serial.print("SAVE_IR,");
                    Serial.print(getProtocolString(storedIRData.protocol));
                    Serial.print(",");
                    Serial.print(storedIRData.address, HEX);
                    Serial.print(",");
                    Serial.println(storedIRData.command, HEX);
                    // ------------------------------------
                    // 2. VISUAL FEEDBACK
                    M5.Display.fillRect(0, 110, 320, 130, TFT_WHITE);
                    M5.Display.setCursor(0, 120);
                    M5.Display.setTextColor(TFT_GREEN);
                    M5.Display.printf("CAPTURED!\nProt: %s\nAddr: 0x%04X\nCmd: 0x%02X", 
                        getProtocolString(storedIRData.protocol), 
                        storedIRData.address, 
                        storedIRData.command);
                    
                    Serial.println("Signal Captured and Stored.");
                } else {
                    // Start listening again if it was just noise/unknown
                    IrReceiver.resume(); 
                }
            }
            
            // Optional: Break loop if Button C is pressed to Cancel
            if (M5.BtnC.wasPressed()) {
                M5.Display.fillRect(0, 110, 320, 130, TFT_WHITE);
                M5.Display.setCursor(0, 120);
                M5.Display.setTextColor(TFT_RED);
                M5.Display.println("Cancelled.");
                break;
            }
        }
    }

    // ---------------------------------------------------------
    // BUTTON B: SEND STORED SIGNAL
    // ---------------------------------------------------------
    if (M5.BtnB.wasPressed()) {
        M5.Display.fillRect(0, 110, 320, 130, TFT_WHITE);
        M5.Display.setCursor(0, 120);

        if (hasStoredSignal) {
            M5.Display.setTextColor(TFT_MAGENTA);
            M5.Display.println("Sending Signal...");
            
            // The write function automatically handles protocol details 
            // based on the data we stored in storedIRData
            IrSender.write(&storedIRData);
            
            delay(100); // Small visual delay
            M5.Display.println("Sent!");
            Serial.println("Stored signal sent.");
            
            // IMPORTANT: Re-enable receiver after sending
            // (The sender library often disables the receiver interrupt)
            IrReceiver.restartAfterSend(); 
            
        } else {
            M5.Display.setTextColor(TFT_RED);
            M5.Display.println("No signal stored!\nPress A to learn first.");
        }
    }
}