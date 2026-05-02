# Arduino Color Sensor Sketch

A standalone Arduino sketch implementing the Adafruit TCS34725 RGB color sensor logic from the Gold-Rush-Robotics CAN-FD_interface project (SerialAtomics branch).

## Overview

This sketch reads RGB color values from an Adafruit TCS34725 sensor and controls RGB LEDs based on color detection. It uses Euclidean distance in RGB space to match detected colors against reference colors (Red, Green, Blue).

## Features

- **5-Sample Averaging**: Takes 5 color readings with 10ms delays between each to reduce noise
- **Euclidean Distance Matching**: Compares detected colors to reference colors using RGB distance calculations
- **RGB LED Control**: Sets 4 RGB LEDs (12 pins total) to matched colors
- **Purple Detection**: Special handling for purple detection when blue is detected with high red values
- **Debug/Tuning Modes**: Compile-time options for detailed debugging output
- **Lux Reading**: Can measure ambient light level

## Hardware Requirements

- **Microcontroller**: Arduino, Teensy, or compatible board
- **Color Sensor**: Adafruit TCS34725 RGB Color Sensor
- **RGB LEDs**: 4x RGB LEDs with current-limiting resistors
- **I2C Connection**: Wire library support
- **Power**: 3.3V or 5V depending on sensor/board

## Installation

### 1. Install Required Libraries

In Arduino IDE:
- Sketch → Include Library → Manage Libraries
- Search for "Adafruit TCS34725"
- Install by Adafruit

### 2. Configure Pin Mappings

Edit the `LEDS` array in the sketch to match your hardware:

```cpp
const int LEDS[][3] = {
    {21, 17, 16},   // LED 0: Red, Green, Blue pins
    {2, 3, 4},      // LED 1: Red, Green, Blue pins
    // ... etc
};
```

### 3. Upload

- Select your board and port
- Upload the sketch

## Core Functions

### `readColor()`
Reads a single RGB color sample from the sensor and returns a `Color` struct.

### `readThenSetLED(size_t led)`
Main function that:
1. Takes 5 color readings
2. Averages them
3. Matches against reference colors
4. Sets the specified LED to the matched color

### `readLux()`
Returns ambient light level in lux.

### `setLedColor(size_t led, bool r, bool g, bool b)`
Sets individual LED colors.

## Reference Colors

The sketch includes these default reference colors:
- **Red**: (255, 0, 0) → Red LED output (HIGH, LOW, LOW)
- **Green**: (0, 255, 0) → Green LED output (LOW, HIGH, LOW)
- **Blue**: (0, 0, 255) → Blue LED output (LOW, LOW, HIGH)

Modify the `COLORS` array to add or adjust color matching.

## Debug Modes

### COLOR_SENSOR_DEBUG
Uncomment to print Euclidean distance from each reference color:
```cpp
#define COLOR_SENSOR_DEBUG
```

### COLOR_SENSOR_TUNING
Uncomment to print raw color readings without LED control:
```cpp
#define COLOR_SENSOR_TUNING
```

## Usage Example

```cpp
void setup() {
    // ... initialization code
}

void loop() {
    // Read color and set LED 0
    readThenSetLED(0);
    delay(500);
}
```

## Wiring Diagram

```
Adafruit TCS34725:
- VIN → 3.3V or 5V
- GND → GND
- SDA → Arduino SDA (I2C)
- SCL → Arduino SCL (I2C)

RGB LED Example (LED 0):
- Red pin (21) → Current limiting resistor → Red LED cathode → GND
- Green pin (17) → Current limiting resistor → Green LED cathode → GND
- Blue pin (16) → Current limiting resistor → Blue LED cathode → GND
- LED anodes → VCC (through resistors)
```

## Troubleshooting

**Color sensor fails to initialize:**
- Check I2C connections
- Verify sensor power supply
- Ensure Adafruit library is installed

**LEDs not lighting:**
- Check pin assignments match your hardware
- Verify LED polarity
- Test with `testAllLeds()` function

**Inaccurate color detection:**
- Enable COLOR_SENSOR_TUNING mode to view raw readings
- Adjust reference color values in `COLORS` array
- Check lighting conditions

## Original Source

Based on the ColorSensor implementation from:
- Repository: [Gold-Rush-Robotics/CAN-FD_interface](https://github.com/Gold-Rush-Robotics/CAN-FD_interface)
- Branch: SerialAtomics
- Files: `include/ColorSensor.h`, `include/LED.h`, `src/main-Arm.cpp`

## License

See original repository for license information.
