import time
import busio
import board
import smbus2

bus = smbus2.SMBus(1)

i2c = busio.I2C(board.SCL, board.SDA)

while True:
	m = smbus2.i2c_msg.write(0x0c, [0x30])
	bus.i2c_rdwr(m)
	time.sleep(1.0)
	#rm = smbus2.i2c_msg.write(0x0c, [0x40])
	#bus.i2c_rdwr(rm)
	time.sleep(1.0)
	print("read/write")
	readm = smbus2.i2c_msg.read(0x0c, 6) 
	bus.i2c_rdwr(readm)
	x = int.from_bytes(readm.buf[0]+readm.buf[1],'big')
	y = int.from_bytes(readm.buf[2]+readm.buf[3],'big')
	z = int.from_bytes(readm.buf[4]+readm.buf[5],'big')
	print("x  = ", x, "y = ", y, "z = ",z) 
	#xi = (readm.buf[0] << 8) | readm.buf[1]
	#yi = (readm.buf[2] << 8) | readm.buf[3]
	#zi = (readm.buf[4] << 8) | readm.buf[5]
	#xi = xi - 0x8000	
	#yi = yi - 0x8000
	#zi = zi - 0x8000
	#print("X = ", xi, "Y = ", yi, "Z = ", zi)
	time.sleep(1.0)	


