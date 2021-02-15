import time
import busio
import board
import smbus2

bus = smbus2.SMBus(1)
i2c = busio.I2C(board.SCL, board.SDA)

while True:
	m = smbus2.i2c_msg.write(0x29, [0x30, 0x0E])
	bus.i2c_rdwr(m)
	time.sleep(1.0)
	print("read/write")
	readm = smbus2.i2c_msg.read(0x29, 0x0E) 
	bus.i2c_rdwr(readm)
	print(readm)
	print(readm.buf[1], "x")
	print(readm.buf[2], "y")
	print(readm.buf[3], "z")
	xi = (readm.buf[0] << 8) | readm.buf[1]
	yi = (readm.buf[2] << 8) | readm.buf[3]
	zi = (readm.buf[4] << 8) | readm.buf[5]
	xi = xi - 0x8000	
	yi = yi - 0x8000
	zi = zi - 0x8000
	print("X = ", xi, "Y = ", yi, "Z = ", zi)
	time.sleep(1.0)	

