from flask import Flask, render_template, request, jsonify
import subprocess
import psutil
import paho.mqtt.client as mqtt
import paho.mqtt.publish as publish
import paho.mqtt.subscribe as subscribe



app = Flask(__name__)

# CONFIGURATION: Set your server IP here
SERVER_IP = "10.0.0.78" 
POWER_TOPIC = "fitness/powermeter"

# where is ant_monitor.py
SCRIPT_PATH = "/home/roland/venv/ant_monitor.py" 


# Global variable to store the latest reading
latest_power_value = "Waiting..."

# MQTT Subscriber Setup
def on_connect(client, userdata, flags, rc):
    client.subscribe(POWER_TOPIC)

def on_message(client, userdata, msg):
    global latest_power_value
    latest_power_value = msg.payload.decode()


# Start background MQTT subscriber
mqtt_client = mqtt.Client()
mqtt_client.on_connect = on_connect
mqtt_client.on_message = on_message
mqtt_client.connect(SERVER_IP, 1883, 60)
mqtt_client.loop_start()


@app.route("/")
def index():
    return render_template("index.html")

@app.route("/get_power")
def get_power():
    # Opens 'powermeter.txt' located in the same directory as your app
    with app.open_resource('powermeter.txt', mode='r') as f:
        latest_power_value = f.read()
    return jsonify(value=latest_power_value)

@app.route("/control", methods=["POST"])
def control():
    action = request.form.get("action")
    # 1. ANT+ Monitoring Logic
    if action == "start_monitoring":
        result = subprocess.Popen(["/home/roland/start_antplus"],stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL,start_new_session=True)
        #return jsonify(status="Monitoring Started")
        
    elif action == "stop_monitoring":
        result = subprocess.Popen(["pkill","-f",SCRIPT_PATH], stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL)  
        #return jsonify(status="Monitoring Stopped")          

    # 2. Change Threshold Logic
    elif action == "change_threshold":
        thresholds = {
            "level0": request.form.get("level0"),
            "level1": request.form.get("level1"),
            "level2": request.form.get("level2"),
            "level3": request.form.get("level3"),
            "level4": request.form.get("level4"),
        }
        sent_updates = []
        for level, value in thresholds.items():
            if value: # Only run if value is provided
                topic = f"fitness/control/powermeter/{level}"
                try:
                    # Publishes a single message to the MQTT broker
                    publish.single(topic, payload=value, hostname=SERVER_IP,retain=True)
                    sent_updates.append(f"{level} -> {value}")
                except Exception as e:
                    sent_updates.append(f"Error updating {level}: {str(e)}")
            message = "Updates sent: " + " | ".join(sent_updates)

    #return jsonify(status="Command executed")
    return render_template("index.html", message=message)

@app.route("/get_script_status")
def get_script_status():
    is_running = False
    # Iterate through all running processes
    for proc in psutil.process_iter(['cmdline']):
        try:
            # Check if the process command line contains your script path
            if proc.info['cmdline'] and any(SCRIPT_PATH in arg for arg in proc.info['cmdline']):
                is_running = True
                break
        except (psutil.NoSuchProcess, psutil.AccessDenied):
            continue
    
    status_text = "Running" if is_running else "Stopped"
    status_color = "green" if is_running else "red"
    return jsonify(status=status_text, color=status_color)

if __name__ == "__main__":
    app.run(debug=True, host="0.0.0.0", port=5000)

