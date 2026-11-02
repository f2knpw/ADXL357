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

Serial port output is described here after :
# ADXL357 Multi-Sensor Acquisition System (ESP32-S3)

High-frequency, synchronized 3-axis accelerometer data acquisition system running at 2000 Hz using an ESP32-S3 microcontroller and three ADXL357 sensors. Data is streamed in real time over a high-speed serial connection to a custom VB.NET client application.

---

## 1. Serial Port Communication Parameters

| Parameter | Specification | Note |
| :--- | :--- | :--- |
| **Baud Rate** | `2,000,000` bps (2 Mbps) | Ensures sufficient bandwidth for 84 kB/s payload |
| **Data Bits** | 8 | Standard UART frame |
| **Stop Bits** | 1 | Standard UART frame |
| **Parity** | None | `8N1` configuration |
| **Flow Control** | None | Direct binary stream |
| **Data Format** | Raw Binary | Little-Endian byte order |
| **Sampling Frequency** | 2000 Hz ($\Delta t = 500\,\mu\text{s}$) | Hardware timer-aligned |

---

## 2. Binary Packet Structure (42 Bytes)

Each frame sent from the ESP32-S3 consists of exactly **42 bytes** packaged in Little-Endian byte order:

| Offset (Bytes) | Field Name | Data Type | Size (Bytes) | Description / Value |
| :---: | :--- | :---: | :---: | :--- |
| `00 – 01` | **Sync Header** | `uint16_t` | 2 | `0xAA55` (LSB first: `0x55`, then `0xAA`) |
| `02 – 05` | **Timestamp** | `uint32_t` | 4 | Time elapsed since boot ($\mu\text{s}$) |
| `06 – 09` | **Sensor 1 - X** | `float` (IEEE 754) | 4 | Acceleration in $g$ |
| `10 – 13` | **Sensor 1 - Y** | `float` (IEEE 754) | 4 | Acceleration in $g$ |
| `14 – 17` | **Sensor 1 - Z** | `float` (IEEE 754) | 4 | Acceleration in $g$ |
| `18 – 21` | **Sensor 2 - X** | `float` (IEEE 754) | 4 | Acceleration in $g$ |
| `22 – 25` | **Sensor 2 - Y** | `float` (IEEE 754) | 4 | Acceleration in $g$ |
| `26 – 29` | **Sensor 2 - Z** | `float` (IEEE 754) | 4 | Acceleration in $g$ |
| `30 – 33` | **Sensor 3 - X** | `float` (IEEE 754) | 4 | Acceleration in $g$ |
| `34 – 37` | **Sensor 3 - Y** | `float` (IEEE 754) | 4 | Acceleration in $g$ |
| `38 – 41` | **Sensor 3 - Z** | `float` (IEEE 754) | 4 | Acceleration in $g$ |

---

## 3. Byte Layout Diagram

```
 +-----------------+-------------------+-------------------+-------------------+
 |  Header (2 B)   |  Timestamp (4 B)  |   Sensor 1 (12 B) |   Sensor 2 (12 B) |
 |  0x55  |  0xAA  |  [32-bit uint]    |   [X]  [Y]  [Z]   |   [X]  [Y]  [Z]   |
 +-----------------+-------------------+-------------------+-------------------+
  0        1        2                 5 6                17 18               29

 +-------------------+
 |   Sensor 3 (12 B) |
 |   [X]  [Y]  [Z]   |
 +-------------------+
 30               41
```

---

## 4. Receiver Parsing Algorithm (VB.NET)

To achieve reliable frame alignment and prevent buffer drift:

1. **Header Alignment:** Search the incoming stream byte-by-byte for `0x55` followed immediately by `0xAA`.
2. **Payload Read:** Once synchronized, read the remaining **40 bytes** into a memory buffer in a single block operation.
3. **Deserialization:**
   - **Timestamp:** Extracted at offset `2` using 32-bit unsigned integer conversion.
   - **Sensor Channels:** Iteratively converted to 32-bit single-precision float values from offset `6 + (i * 4)` for $i \in [0, 8]$.
4. **CSV Export:** Values formatted using the `Invariant` culture to preserve dot separators for floating-point values.
