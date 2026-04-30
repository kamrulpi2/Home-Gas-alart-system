import network
import time
import dht
from machine import Pin, ADC, reset
import urequests as requests
import gc

# ====================== CONFIG ======================
WIFI_SSID = "bubt"
WIFI_PASSWORD = "bubt1234"

DATABASE_URL = "https://dht22iot-5b3fd-default-rtdb.firebaseio.com"

MQ5_PIN = 4
RELAY_PIN = 2
BUZZER_PIN = 7
DHT_PIN = 3

ALERT_LEVEL = 30

# ====================== WIFI CONNECTION ======================
print("=== ESP32-C3 Gas + DHT11 Firebase Monitor Starting ===")

def connect_wifi():
    sta_if = network.WLAN(network.STA_IF)
    sta_if.active(False)
    time.sleep(0.5)
    sta_if.active(True)
    time.sleep(1)

    print(f"Connecting to WiFi: {WIFI_SSID}")
    sta_if.connect(WIFI_SSID, WIFI_PASSWORD)

    timeout = 15
    start = time.time()

    while not sta_if.isconnected():
        if time.time() - start > timeout:
            print("\nWiFi Timeout!")
            return None
        print(".", end="")
        time.sleep(0.6)

    print("\n✅ WiFi Connected!")
    print("IP:", sta_if.ifconfig()[0])
    return sta_if

wlan = connect_wifi()
if wlan is None:
    print("WiFi failed. Rebooting...")
    time.sleep(3)
    reset()

# ====================== SENSORS & ACTUATORS ======================
dht_sensor = dht.DHT11(Pin(DHT_PIN))
mq5 = ADC(Pin(MQ5_PIN))
mq5.atten(ADC.ATTN_11DB)

relay = Pin(RELAY_PIN, Pin.OUT)
buzzer = Pin(BUZZER_PIN, Pin.OUT)
relay.value(0)
buzzer.value(0)

def map_gas(x):
    return max(0, min(100, (x - 300) * 100 // (4047 - 300)))

print("System Ready!\n")

# ====================== MAIN LOOP ======================
while True:
    try:
        raw = mq5.read()
        gas = map_gas(raw)

        dht_sensor.measure()
        temp = dht_sensor.temperature()
        hum = dht_sensor.humidity()

        alert = gas >= ALERT_LEVEL

        if alert:
            relay.value(1)
            buzzer.value(1)
            time.sleep(0.2)
            buzzer.value(0)
        else:
            relay.value(0)
            buzzer.value(0)

        print(f"Gas: {gas:3d}% | Temp: {temp:4.1f}°C | Hum: {hum:3d}% | Alert: {alert}")

        # Send to Firebase
        data = {
            "gas": gas,
            "temperature": temp,
            "humidity": hum,
            "alert": alert,
            "timestamp": time.time()
        }

        response = requests.patch(DATABASE_URL + "/sensor.json", json=data, timeout=5)
        if response.status_code == 200:
            print("✅ Data sent to Firebase")
        response.close()

    except Exception as e:
        print("Error:", e)
        if "Wifi" in str(e) or "connect" in str(e):
            print("Reconnecting WiFi...")
            wlan = connect_wifi()
            if wlan is None:
                reset()

    gc.collect()
    time.sleep(2)
