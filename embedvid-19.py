import board
import busio
import smbus2
import adafruit_vl53l0x
import time
import datetime
import json
import jwt
import ssl
from gpiozero import Button, Buzzer
import math
import paho.mqtt.client as mqtt

i2c = busio.I2C(board.SCL, board.SDA)
sensor_tof = adafruit_vl53l0x.VL53L0X(i2c)
button = Button(10)
buzzer = Buzzer(24)
bus = smbus2.SMBus(1)

class User:
    def set_values(self, x, y, z, d):
        self.x = x
        self.y = y
        self.z = z
        self.d = d

    def get_values(self):
        return [self.x, self.y, self.z, self.d]; 

def get_temp():
    meas_vobj = smbus2.i2c_msg.write(0x40,[0x00])
    bus.i2c_rdwr(meas_vobj)
    time.sleep(0.1)
    read_result = smbus2.i2c_msg.read(0x40,2)
    bus.i2c_rdwr(read_result)
    Vobj_bin = int.from_bytes(read_result.buf[0]+read_result.buf[1],'big', signed = True)
    Vobj = Vobj_bin*(0.00000015625)
    
    meas_tdie = smbus2.i2c_msg.write(0x40,[0x01])
    bus.i2c_rdwr(meas_tdie)
    time.sleep(0.1)
    read_result = smbus2.i2c_msg.read(0x40,2)
    bus.i2c_rdwr(read_result)
    Tdie = ((int.from_bytes(read_result.buf[0]+read_result.buf[1],'big', signed = False) >> 2)/32.0)+273.14

    S0 = 7e-14
    a1 = 1.75e-3
    a2 = -1.678e-5
    Tref = 298.15
    b0 = -2.94e-5
    b1 = -5.7e-7
    b2 = 4.63e-9
    c2 = 13.4

    S = S0 * (1 + a1*(Tdie-Tref) + a2*(Tdie-Tref)**2)
    Vos = b0 + b1*(Tdie-Tref) + b2*(Tdie-Tref)**2
    fVobj = (Vobj-Vos) + c2*(Vobj-Vos)**2
    Tobj = (Tdie**4 + (fVobj/S))**0.25
    return Tobj - 276.55

def getMagValues():
	config = [0x00, 0x5C, 0x00]
	bus.write_i2c_block_data(0x0C, 0x60, config)
	data = bus.read_byte(0x0C)
	config = [0x02, 0xB4, 0x08]
	bus.write_i2c_block_data(0x0C, 0x60, config)
	data = bus.read_byte(0x0C) 
	bus.write_byte(0x0C, 0x3E)
	time.sleep(0.1)
	data = bus.read_byte(0x0C)
	time.sleep(0.1)
	data= bus.read_i2c_block_data(0x0C, 0x4E, 7)
	#convert data  
	xMag = data[1] * 256 + data[2]
	yMag = data[3] * 256 + data[4]
	zMag = data[5] * 256 + data[6]
	if xMag > 32767 :
		xMag -= 65536
	if yMag > 32767 :
		yMag -= 65536
	if zMag > 32767 :
		zMag -= 65536
	bus.write_byte(0x0C, 0x80)
	time.sleep(0.1)
	data = bus.read_byte(0x0C)
	return xMag, yMag, zMag


def reset():
    global movement
    movement = False
    global time_scale
    time_scale = time.time()
    mx, my, mz = getMagValues()
    user.set_values(mx, my, mz, sensor_tof.range)
    send_data(movement, sensor_tof.range, float("{:.1f}".format(get_temp())))


def pressed():
    buzzer.on()
    time.sleep(0.2)
    buzzer.off()
    time.sleep(2.0)
    if button.is_pressed:
        buzzer.on()
        time.sleep(0.1)
        buzzer.off()
        time.sleep(0.1)
        buzzer.on()
        time.sleep(0.1)
        buzzer.off()
        send_data(True, 0,0)
        exit()
    else:
        reset()

def average_calculator(mx, my, mz, md, distance_diff, distance_change_ave, angle_ave, distance_ave, number): 
    angle_ave = ((angle_ave*(number-1)) +  math.sqrt(mx**2 + my**2 + mz**2))/number
    distance_change_ave = ((distance_change_ave*(number-1)) + distance_diff)/number  
    distance_ave = ((distance_ave*(number-1)) + md)/number
    number = number + 1
    return angle_ave, distance_change_ave, distance_ave, number


def movement_calc(angle_ave, distance_ave, distance_change_ave):
    if angle_ave > 17 or distance_change_ave > 200 or distance_ave == 0 or distance_ave > 2000:
        return True
    return False

def create_jwt(project_id, private_key_file, algorithm):
    token = {
            "iat":datetime.datetime.utcnow(),
            "exp":datetime.datetime.utcnow() + datetime.timedelta(minutes=20),
            "aud":project_id,}
    with open(private_key_file, "r") as f:
        private_key = f.read()

    return jwt.encode(token, private_key, algorithm=algorithm)

def on_connect(client, userdata, flags, rc):
    if rc==0:
        print("Connected OK")
    else:
        print("Bad connection Returned code=",rc)

def on_publish(client, userdata, result):
    print(result)

def send_data(movement, distance_ave, mt):
    client_id = "projects/bubbly-realm-305414/locations/europe-west1/registries/my-registry/devices/ed_pi"
    client = mqtt.Client(client_id)
    client.username_pw_set(username="unused", password=create_jwt("bubbly-realm-305414", "rsa_private.pem", "RS256"))
    client.tls_set(ca_certs = "roots.pem",tls_version=ssl.PROTOCOL_TLSv1_2)

    client.on_connect=on_connect
    client.on_publish=on_publish

    error = client.connect("mqtt.googleapis.com", 8883)
    if error != 0:
        return
    if movement == False:
        data_package = { "time": int(time.time()+0.5), "away_from_desk": movement, "average_distance": int(distance_ave+0.5), "temperature": mt }
    else:
        data_package = { "time": int(time.time()+0.5), "away_from_desk": movement, "average_distance": 0, "temperature": 0}
    json_string = json.dumps(data_package)
    client.loop_start()
    client.publish("/devices/ed_pi/events","Hello",qos=1)
    time.sleep(1.0)
    client.publish("/devices/ed_pi/events",json_string, qos=1)
    time.sleep(1.0)
    client.loop_stop()
    client.disconnect()
    print("Message Sent")

print("System ready")
user = User()

time_scale = time.time()
number = 1
distance_ave = 0
distance_change_ave = 0
angle_ave = 0
movement = False

button.wait_for_press()
pressed()

while True:
    
    while movement:   #need to press button again
        if time.time() - time_scale < 30:
            buzzer.on()
            time.sleep(3.0)
            buzzer.off()
            time.sleep(3.0)
        else:
            send_data(movement, distance_ave, mt)
            time_scale = time.time()
        if button.is_pressed:
            pressed()
            movement = False

    mx, my, mz = getMagValues()
    md = sensor_tof.range
    u = user.get_values()
    mx = abs(mx-u[0])
    my = abs(my-u[1])
    mz = abs(mz-u[2])
    delta_d = abs(md-u[3])
    button.when_pressed = pressed
    angle_ave, distance_change_ave, distance_ave, number = average_calculator(mx, my, mz, md, delta_d, distance_change_ave, angle_ave, distance_ave, number)
    if time.time() - time_scale > 29:
        mt = float("{:.1f}".format(get_temp()))
        movement = movement_calc(angle_ave, distance_ave, distance_change_ave)
        send_data(movement, distance_ave, mt)
        angle_ave = 0
        distance_ave = 0
        ditance_change_ave = 0
        number = 1
        time_scale = time.time()


