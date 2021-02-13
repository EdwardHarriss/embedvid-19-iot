import board
import math
import busio
import adafruit_vl53l0x
import adafruit_tmp006
import adafruit_mlx90393
import time
from gpiozero import Button
from gpiozero import Buzzer

buzzer = Buzzer(24)

button = Button(10)

button.wait_for_press()

i2c = busio.I2C(board.SCL, board.SDA)
lidarsensor = adafruit_vl53l0x.VL53L0X(i2c)
tempsensor = adafruit_tmp006.TMP006(i2c)
magnosensor = adafruit_mlx90393.MLX90393(i2c, gain=adafruit_mlx90393.GAIN_1X)
mx,my,mz = magnosensor.magnetic
oPos = [mx, my, mz]
x = 1
start = time.time()
initTime = start
while x == 1:
	print('Range: {}mm'.format(lidarsensor.range))
	end = time.time()
	if (end - start) > 5:
		temp = tempsensor.temperature
		start = time.time()
		print("Temperature: ", temp)
		if temp > 24:
			print("Too Warm")
		if temp < 16:
			print("Too Cold")	
	mx, my, mz = magnosensor.magnetic
	print("[{}]".format(time.monotonic()))
	print("X: {} uT".format(mx))
	print("Y: {} uT".format(my))
	print("Z: {} uT".format(mz))
	cPos = [mx,my,mz]
	mPos = [abs(cPos[0] - oPos[0]), abs(cPos[1] - oPos[1]), abs(cPos[2] - oPos[2])] 
	d = mPos[0]*mPos[0] + mPos[1]*mPos[1] + mPos[2]*mPos[2]
	d = math.sqrt(d)
	if(d > 10):
		buzzer.on()
		time.sleep(0.5)
	buzzer.off()
	button.wait_for_press(2)
	if button.is_pressed:
		time.sleep(1)
		if button.is_pressed == False:
			print("Button reset")
			mx,my,mz = magnosensor.magnetic	
			oPos = [mx, my, mz]
		else:
			x = x + 1
			buzzer.on()
			time.sleep(1)
			buzzer.off()
			print("Device was used for" , time.time() - initTime, "seconds")
	
	if magnosensor.last_status > adafruit_mlx90393.STATUS_OK:
		magnosensor.display_status()
