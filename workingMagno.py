import smbus2
import time

bus = smbus2.SMBus(1)

while True:
	config = [0x00, 0x5C, 0x00]
	bus.write_i2c_block_data(0x0C, 0x60, config)
	data = bus.read_byte(0x0C)
	config = [0x02, 0xB4, 0x08]
	bus.write_i2c_block_data(0x0C, 0x60, config)
	data = bus.read_byte(0x0C) 
	bus.write_byte(0x0C, 0x3E)
	data = bus.read_byte(0x0C)
	time.sleep(1)
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
	#output data
	print("Magnometer x = %d" %xMag)
	print("Magnometer y = %d" %yMag)
	print("Magnometer z = %d" %zMag)
	print("\n")
	time.sleep(1.0)
	bus.write_byte(0x0C, 0x80)
	time.sleep(1.0)
	data = bus.read_byte(0x0C)
	time.sleep(1.0)
