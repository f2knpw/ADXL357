# ADXL357
3x ADXL357 high accuracy accelerometer acquistion board

This project is a high accuracy triple accelerometer board.

# Specification
My son wanted a high accuracy accelerometer board to characterize a vibrating pot.

Here is an accurate (but still cheap) measurement board. 

Specifications are :

- 3 x 3 axis accelerometers 20bits accuracy
- +-10g acceleration range
- 3 sensors fully synchronized
- high rate of acquisition (up to 4kHz. Target 2kHz)
- interfaced with PC via USB

Critical point of this specification is the requirement for synchronization of the 3 sensors. This forbids cheap accelerometers such as MPU6050... 

The selected chip was thus ADXL357. It exposes SYNC signal to trigger acquisition on several chips in parallel and can be intercaed with MPU with highspeed SPI bus.

The project is fully describded on my hackaday pages : https://hackaday.io/project/206547-esp32-s3-3-x-adxl357-acquisition-board
