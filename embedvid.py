import board
import busio
import adafruit_tmp006
import adafruit_mlx90393
import adafruit_vl53l0x
import time


i2c = busio.I2C(board.SCL, board.SDA)
sensor_temp = adafruit_tmp006.TMP006(i2c)
sensor_mag = adafruit_mlx90393.MLX90393(i2c, gain=adafruit_mlx90393.GAIN_1X)
sensor_tof = adafruit_vl53l0x.VL53L0X(i2c)

def find_average()



while True:
    
