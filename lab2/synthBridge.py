import serial
import paho.mqtt.client as mqtt
import re

SER_PORT = "/dev/tty.usbmodem14203"
GROUP_NAME = "Embedvid-19"
MY_NAME = "Matt"
BROKER_URL = "test.mosquitto.org"
BROKER_PORT = 1883

MESSAGE_RE = b"[PR][0-8][0-9A-B]"
PUB_TOPIC = "IC.embedded/" + GROUP_NAME + "/synth/" + MY_NAME
SUB_TOPIC = "IC.embedded/" + GROUP_NAME + "/synth" + '/+'

#Callback for incoming MQTT messages
def on_message(client, userdata, msg):
	pub_name = msg.topic.split('/')[-1]
	match = re.match(MESSAGE_RE,msg.payload)
	if match:
		if pub_name != MY_NAME:
			ser.write(msg.payload+b'\n')
			print("In from {}: {}".format(pub_name,str(match.group(0),'ascii')))
	else:
		print("Syntax Error in message from {}: {}".format(pub_name,msg.payload))

#Initialise serial
try:
	ser = serial.Serial(SER_PORT, 115200, timeout=1)
except:
	print("Could not open serial port")
	
	quit()

#Connect to broker
client = mqtt.Client()
try:
	client.connect(BROKER_URL, BROKER_PORT, 60)
except:
	print("Couldn't connect to broker")
	ser.close()
	quit()
	
client.subscribe(SUB_TOPIC)
client.on_message = on_message
client.loop_start()

#Wait for messages on serial port
try:
	while(True):
		line = ser.readline()
		if line:
			match = re.match(MESSAGE_RE,line)
			if match:
				client.publish(PUB_TOPIC,match.group(0))
				print("Out: "+str(match.group(0),'ascii'))
			else:
				print("Syntax Error in message from synth: "+str(line,'ascii'),end='')
except KeyboardInterrupt:
	ser.close()
	client.disconnect()
except:
	ser.close()
	client.disconnect()
	raise
	
		
