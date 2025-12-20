# Smart Trainer Fan DIY
 
Smart trainer fans on the market are often expensive, typically costing around $300. This project aims to provide an affordable alternative by transforming a standard low-cost fan into a smart trainer fan using a few simple add-ons and easy-to-follow steps. The goal is to make indoor training more comfortable and accessible for everyone without breaking the bank.

# Features

1. Continuously monitor data from the Power Meter, Heart Rate Sensor, and Cadence Sensor, and publish these updates to the MQTT server.
2. The ESP32 subscribes to the MQTT server, retrieves the latest sensor data, and—based on pre-programmed logic—controls the fan by turning it on/off or adjusting its speed accordingly.

# Acknowledgments

This project leverages ANT/ANT+ Python library from https://github.com/Tigge/openant. Appreciate the open-source community for making these resources available


# Steps

1. Infrared codes collection and validation

Find or purchase a fan with an infrared remote control,  collect, decode, and record its infrared codes with ESP32 + infrared receiver. Typically, the codes for turning on, turning off, increasing, or reducing the fan speed. 
   
<img width="445" height="147" alt="Screenshot 2025-12-20 at 11 51 12 AM" src="https://github.com/user-attachments/assets/805d1dcc-1574-4e5a-8780-a266d02266a8" />

Replay the recorded infrared codes with ESP32 + infrared emitter to validate each code. 

<img width="412" height="180" alt="Screenshot 2025-12-20 at 11 53 45 AM" src="https://github.com/user-attachments/assets/c9d4621d-268f-4436-b158-6804dc8e5c9e" />


2. Get ANT+ USB stick

To monitor your heart rate, cadence, and power output during training, a device that can sniff and receive ANT+ signal is needed.   In this project, we got a cheap ANT+ USB stick ~$20 from Amazon ( any ANT+ USB stick on Amazon should be Ok ). 

<img width="100" height="132" alt="Screenshot 2025-12-20 at 12 08 41 PM" src="https://github.com/user-attachments/assets/8c388d9c-d6bd-47a9-9397-6c0feb7fee32" />

   
3. Plug in the ANT+ USB stick, install the openant library and MQTT server

Plug in ANT+ USB stick in a Linux-based laptop/mini PC/Raspberry Pi and ensure ANT+ USB stick is recognized successfully. (Ubuntu 16.04 LTS is the first to have seamless, stable integration with modern ANT+ libraries like OpenAnt. Older systems like Ubuntu 12.04 often required manual kernel module loading)
<img width="776" height="153" alt="Screenshot 2025-12-20 at 12 43 44 PM" src="https://github.com/user-attachments/assets/fd2dadaa-41ef-4246-a43f-be806c9ece03" />


Following the README in https://github.com/Tigge/openant to prepare and install python libiary, MQTT Server

4. Validate the sensor data pub and sub with the MQTT Server by using mqtt docker image

<img width="669" height="135" alt="Screenshot 2025-12-20 at 12 57 35 PM" src="https://github.com/user-attachments/assets/7886f1bc-e084-4a1a-ae9b-7b76f1f242fb" />

5. 




