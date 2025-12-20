
import paho.mqtt.client as mqtt
from openant.easy.node import Node

from openant.devices.common import DeviceType
from openant.devices.scanner import Scanner
from openant.devices.utilities import auto_create_device
from openant.devices import ANTPLUS_NETWORK_KEY
from openant.devices.power_meter import PowerMeter, PowerData

#replace it with your own MQTT server address
broker_address = "10.0.0.78"
port = 1883
topic = "fitness/powermeter"

def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print("Connected to MQTT Broker!")
    else:
        print(f"Failed to connect, return code {rc}")


def ant_scan(file_path=None, device_id=0, device_type=0, auto_create=True):
    # list of auto created devices
    devices = []

    # ANT USB node
    node = Node()
    node.set_network_key(0x00, ANTPLUS_NETWORK_KEY)

    scanner = Scanner(node, device_id=device_id, device_type=device_type)

    def on_update(device_tuple, common):
        device_id = device_tuple[0]
        print(f"Device #{device_id} common data update: {common}")

    def on_device_data(device, page_name, data):
        print(f"Device {device} broadcast {page_name} data: {data}")
        if isinstance(data, PowerData):
            print(f"Power Data {data.instantaneous_power}")
            print("update to mqtt fitness/powermeter.")
            client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION1) # Use VERSION1 for the callbacks below
            client.on_connect = on_connect
            client.connect(broker_address, port, 60)
            client.loop_start() # Start the non-blocking network loop
            msg = f"{data.instantaneous_power}"
            # Publish a message with a specific topic and payload
            result = client.publish(topic, msg)
            status = result[0]
            if status == 0:
                print(f"Sent `{msg}` to topic `{topic}`")
            else:
                print(f"Failed to send message to topic {topic}")
            client.loop_stop()
            client.disconnect()
            print("mqtt update finished.")

    def on_found(device_tuple):
        device_id, device_type, device_trans = device_tuple
        print(
            f"Found new device #{device_id} {DeviceType(device_type)}; device_type: {device_type}, transmission_type: {device_trans}"
        )

        if auto_create and len(devices) < 16:
            try:
                if DeviceType(device_type) == DeviceType.PowerMeter:
                    dev = auto_create_device(node, device_id, device_type, device_trans)
                    # closure callback of on_device_data with device
                    dev.on_device_data = lambda _, page_name, data, dev=dev: on_device_data(
                        dev, page_name, data
                    )
                    devices.append(dev)
                    print(f"Create new device #{device_id} {DeviceType(device_type)}")
            except Exception as e:
                print(f"Could not auto create device: {e}")

    scanner.on_found = on_found
    scanner.on_update = on_update

    try:
        print(
            f"Starting scanner for #{device_id}, type {device_type}, press Ctrl-C to finish"
        )
        node.start()
    except KeyboardInterrupt:
        print("Closing ANT+ node...")
    finally:
        scanner.close_channel()
        if file_path:
            print(f"Saving/updating found devices to {file_path}")
            scanner.save(file_path)

        for dev in devices:
            dev.close_channel()

        node.stop()


if __name__ == "__main__":
    ant_scan()
