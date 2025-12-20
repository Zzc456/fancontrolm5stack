# Smart Trainer Fan DIY
 
Smart trainer fans on the market are often expensive, typically costing around $300. This project aims to provide an affordable alternative by transforming a standard low-cost fan into a smart trainer fan using a few simple add-ons and easy-to-follow steps. The goal is to make indoor training more comfortable and accessible for everyone without breaking the bank.

# Designs



# Steps

1. Infrared codes collection and validation

Find or purchase a fan with an infrared remote control,  collect, decode, and record its infrared codes with ESP32 + infrared receiver. Typically, the codes for turning on, turning off, increasing, or reducing the fan speed. 
   
<img width="445" height="147" alt="Screenshot 2025-12-20 at 11 51 12 AM" src="https://github.com/user-attachments/assets/805d1dcc-1574-4e5a-8780-a266d02266a8" />

Replay the recorded infrared codes with ESP32 + infrared emitter to validate each code. 

<img width="412" height="180" alt="Screenshot 2025-12-20 at 11 53 45 AM" src="https://github.com/user-attachments/assets/c9d4621d-268f-4436-b158-6804dc8e5c9e" />


2. Get ANT+ USB stick

To monitor your heart rate, cadence, and power output during training, a device that can sniff and receive ANT+ signal is needed.   In this project, we got a cheap ANT+ USB stick ~$20 from Amazon ( any ANT+ USB stick on Amazon should be Ok ). 

<img width="100" height="132" alt="Screenshot 2025-12-20 at 12 08 41 PM" src="https://github.com/user-attachments/assets/8c388d9c-d6bd-47a9-9397-6c0feb7fee32" />

   
