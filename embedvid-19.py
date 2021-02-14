import board
import busio
import smbus2
import adafruit_mlx90393
import adafruit_vl53l0x
import time
from gpiozero import Button, Buzzer
import math
import paho.mqtt.client as mqtt

i2c = busio.I2C(board.SCL, board.SDA)
sensor_mag = adafruit_mlx90393.MLX90393(i2c, gain=adafruit_mlx90393.GAIN_1X)
sensor_tof = adafruit_vl53l0x.VL53L0X(i2c)
button = Button(10)
buzzer = Buzzer(24)


class User:
    def set_values(self, x, y, z, d):
        self.x = x
        self.y = y
        self.z = z
        self.d = d

    def get_values(self):
        return [self.x, self.y, self.z, self.d]; 

def get_temp():
    bus = smbus2.SMBus(1)
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
    return Tobj - 273.15


def reset():
    mx, my, mz = sensor_mag.magnetic
    user.set_values(mx, my, mz, sensor_tof.range)


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
        quit()
    reset()

def average_calculator(mx, my, mz, md, angle_ave, distance_ave, number):
    angle_ave = (angle_ave/number) +  (math.sqrt(mx**2 + my**2 + mz**2)/number)
    distance_ave = (distance_ave/number) + (md/number)
    number = number + 1
    return angle_ave, distance_ave, number


def movement_calc(angle_ave, distance_ave):
    if angle_ave > 10 or distance_ave > 60:
        return True
    return False

def send_data(movement, distance_ave, mt):
    client = mqqtt.Client()
    error_code = client.connect("test.mosquitto.org",port=1883)
    if error_code != 0:
        return
    if movement == False:
        data_package = { "Time": time.time(), "At Desk": movement, "Average Distance": distance_ave, "Temperature": mt }
    else:
        data_package = { "Time": time.time(), "At Desk": movement, "Average Distance": 0, "Temperature": 0}
    client.publish("IC.embedded/Embedvid-19/data",data_package)

print("System ready")
user = User()

button.wait_for_press()
pressed()
time_scale = time.time()
number = 1
distance_ave = 0
angle_ave = 0

while True:
    mx, my, mz = sensor_mag.magnetic
    md = sensor_tof.range
    u = user.get_values()
    mx = abs(mx-u[0])
    my = abs(my-u[1])
    mz = abs(mz-u[2])
    md = abs(md-u[3])
    button.when_pressed = pressed
    angle_ave, distance_ave, number = average_calculator(mx, my, mz, md, angle_ave, distance_ave, number)
    if time.time() - time_scale > 29:
        mt = get_temp()
        movement = movement_calc(angle_ave, distance_ave)
        if movement:
            buzzer.on()
            time.sleep(5.0)
            buzzer.off()
        angle_ave = 0
        distance_ave = 0
        number = 1
        time_scale = time.time()
        send_data(movement, distance_ave, mt)


