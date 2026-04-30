# ESP32-C3 Gas Leak Detector with DHT11 & Firebase

IoT project using ESP32-C3, MQ-5 Gas Sensor, DHT11, Relay, and Buzzer. Data is sent to Firebase Realtime Database in real-time.

## Features
- Real-time Gas, Temperature & Humidity monitoring
- Automatic Relay + Buzzer alert when gas exceeds 30%
- Live Web Dashboard
- Data stored in Firebase

## Hardware Connections

| Component       | ESP32-C3 Pin |
|-----------------|--------------|
| MQ-5 Gas Sensor | GPIO 4       |
| DHT11           | GPIO 3       |
| Relay           | GPIO 2       |
| Buzzer          | GPIO 7       |

## Web Dashboard
Open `webapp/index.html` in browser (it connects to Firebase automatically).

## How to Run
1. Flash MicroPython firmware on ESP32-C3
2. Upload `boot.py` and `main.py`
3. Press RST button
4. Open Serial Monitor (115200 baud)

## Web App
The web dashboard is inside the `webapp` folder.

Made with ❤️ using MicroPython + Firebase
<img width="1402" height="783" alt="image" src="https://github.com/user-attachments/assets/8b726bfa-0279-4104-845f-f61b55dbdc1a" />

