import board
import busio
import smbus2
import adafruit_mlx90393
import adafruit_vl53l0x
import time
from gpiozero import Button, Buzzer
import math

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
    Vobj_bin = int.from_bytes(read_result.buf[0]+read_result.buf[1],'big')
    Vobj = Vobj_bin*(0.00000015625)
    
    meas_tdie = smbus2.i2c_msg.write(0x40,[0x01])
    bus.i2c_rdwr(meas_tdie)
    time.sleep(0.1)
    read_result = smbus2.i2c_msg.read(0x40,2)
    bus.i2c_rdwr(read_result)
    Tdie = ((int.from_bytes(read_result.buf[0]+read_result.buf[1],'big') / 4)*0.03125)+273.14
 
    b0 = -0.0000294
    b1 = -0.00000057
    b2 = 0.00000000463
    Tref = 298.15
    c2 = 13.4
    S0 = 0.000000000000063
    a1 = 0.00175
    a2 = -0.00001678

    Vos = b0 + b1*(Tdie-Tref) + b2*math.pow((Tdie-Tref),2.0)
    print("Vos : ", Vos)
    FVobj = (Vobj-Vos) + c2*math.pow((Vobj-Vos),2.0)
    print("FVobj : ", FVobj)
    S = S0*(1 + a1*(Tdie-Tref) + a2*math.pow((Tdie-Tref),2.0))
    print("S : ", S)
    Tobj = math.sqrt(math.sqrt(math.pow(Tdie,4.0) + (FVobj/S))) - 273.15
    print(Tobj)


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

print("System ready")
user = User()

button.wait_for_press()
pressed()
time_scale = time.time()
number = 1
distance_ave = 0
angle_ave = 0
get_temp()

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
        mt = sensor_temp.temperature
        movement = movement_calc(angle_ave, distance_ave)
        if movement:
            buzzer.on()
            time.sleep(5.0)
            buzzer.off()
        angle_ave = 0
        distance_ave = 0
        number = 1
        time_scale = time.time()


