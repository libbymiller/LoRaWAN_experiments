import paho.mqtt.client as mqtt 
import time
import json
import traceback

def on_message(client, userdata, message):
    decoded_message=str(message.payload.decode("utf-8"))
    msg=json.loads(decoded_message)
    print("message received " ,decoded_message)
    print("message topic=",message.topic)

   # log file
   # received_at, device_id, dev_eui, dev_addr, gateway_id, eui, latitude, longitude, altitude

    try:
      print("message decoded_payload=",msg["uplink_message"]["decoded_payload"])
      print("message gateway_ids=",msg["uplink_message"]["rx_metadata"][0]["gateway_ids"])
      with open("gps_tester_log.txt", "a") as myfile:

         #received_at
         myfile.write(str(msg["received_at"]))
         myfile.write(", ")

         #device_id
         myfile.write(str(msg["end_device_ids"]["device_id"]))
         myfile.write(", ")

         #dev_eui
         myfile.write(str(msg["end_device_ids"]["dev_eui"]))
         myfile.write(", ")

         #dev_addr
         myfile.write(str(msg["end_device_ids"]["dev_addr"]))
         myfile.write(", ")

         #gateway_id
         myfile.write(str(msg["uplink_message"]["rx_metadata"][0]["gateway_ids"]["gateway_id"]))
         myfile.write(", ")

         #(gateway) eui
         myfile.write(str(msg["uplink_message"]["rx_metadata"][0]["gateway_ids"]["eui"]))
         myfile.write(", ")

         #gps data
         myfile.write(str(msg["uplink_message"]["decoded_payload"]["gps_1"]["latitude"]))
         myfile.write(", ")
         myfile.write(str(msg["uplink_message"]["decoded_payload"]["gps_1"]["longitude"]))
         myfile.write(", ")
         myfile.write(str(msg["uplink_message"]["decoded_payload"]["gps_1"]["altitude"]))
         myfile.write('\n')
    except Exception as e: 
      print(e)
      print("Exception")
      traceback.print_exc()

broker_address="127.0.0.1"

print("creating new instance")
client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION1, "P1") 

client.on_message=on_message #attach function to callback

print("connecting to broker")

client.connect(broker_address) #connect to broker

print("Subscribing to topic","/ttn/v3/foo-bar@ttn/devices/#")
client.subscribe("/ttn/v3/foo-bar@ttn/devices/#")

client.loop_forever()
