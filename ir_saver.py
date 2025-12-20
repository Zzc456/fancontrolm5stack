import serial
import time
import csv

# --- CONFIGURATION ---
# Windows: 'COM3', 'COM4', etc. (Check Device Manager)
# Mac/Linux: '/dev/tty.usbserial...', etc.
SERIAL_PORT = 'COM4'  
BAUD_RATE = 115200
OUTPUT_FILE = 'my_remotes.csv'

try:
    ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
    print(f"Listening on {SERIAL_PORT}...")
    print("Press Ctrl+C to stop.")

    # Open file in append mode
    with open(OUTPUT_FILE, mode='a', newline='') as file:
        writer = csv.writer(file)
        
        # Write header if file is empty
        if file.tell() == 0:
            writer.writerow(["Type", "Protocol", "Address", "Command"])

        while True:
            if ser.in_waiting > 0:
                try:
                    # Read line from M5Stack
                    line = ser.readline().decode('utf-8').strip()
                    
                    # Check if it's our save command
                    if line.startswith("SAVE_IR"):
                        parts = line.split(',')
                        # parts = ['SAVE_IR', 'NEC', '4', '55']
                        
                        if len(parts) == 4:
                            protocol = parts[1]
                            address = parts[2]
                            command = parts[3]
                            
                            # Save to file
                            writer.writerow(["IR_SIGNAL", protocol, address, command])
                            file.flush() # Ensure it saves immediately
                            
                            print(f"Saved: Protocol={protocol}, Addr={address}, Cmd={command}")
                        
                except Exception as e:
                    print(f"Error reading line: {e}")
                    
except serial.SerialException:
    print(f"Could not open {SERIAL_PORT}. Is the Arduino Monitor open? Close it first!")
except KeyboardInterrupt:
    print("\nStopping...")