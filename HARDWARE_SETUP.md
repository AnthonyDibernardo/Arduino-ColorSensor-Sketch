# Hardware Setup Guide

## Adafruit TCS34725 Color Sensor

### Pin Configuration

**I2C Connections (Standard across most Arduino boards):**

| Sensor Pin | Arduino Pin | Notes |
|-----------|-----------|-------|
| GND | GND | Ground |
| VIN | 3.3V* | Power (max 5.5V) |
| SDA | SDA** | I2C Data |
| SCL | SCL** | I2C Clock |
| LED | Optional | Can control sensor LED |

*: Use 3.3V if your Arduino runs at 3.3V (Teensy, Arduino Due). Use 5V for 5V boards, but the sensor has onboard regulation.
**: I2C pins vary by board:
- **Arduino Uno/Nano**: A4 (SDA), A5 (SCL)
- **Arduino Mega**: 20 (SDA), 21 (SCL)
- **Arduino Leonardo**: 2 (SDA), 3 (SCL)
- **Teensy 3.x**: 18 (SDA), 19 (SCL)
- **Teensy 4.0**: 18 (SDA), 19 (SCL)

## RGB LED Configuration

### Default Pin Mappings (from sketch)

```cpp
const int LEDS[][3] = {
    {21, 17, 16},   // LED 0
    {2, 3, 4},      // LED 1
    {41, 13, 40},   // LED 2
    {7, 8, 9}       // LED 3
};
```

Each LED uses 3 pins: [Red, Green, Blue]

### LED Wiring

**Common Cathode RGB LED (recommended):**

```
         +5V
          |
     [Resistor]
          |
    ______|______
   |Red |Green|Blue|
   | |_____| |_____| |_____| |
   |  |       |       |
   R  G       B
   |  |       |
   [Pin 21] [Pin 17] [Pin 16]  (for LED 0)
```

**Connection Steps:**

1. Place resistors (~220Ω for 5V, ~100Ω for 3.3V) in series with each LED leg
2. Connect RGB cathode (long leg) to GND through resistors
3. Connect RGB anodes (short legs) to Arduino pins via resistors
4. Update `LEDS` array to match your pin assignments

### Resistor Selection

- **5V power supply**: Use 220Ω resistors
- **3.3V power supply**: Use 100-150Ω resistors
- **Calculation**: R = (Supply Voltage - LED Forward Voltage) / Desired Current
  - Typical LED forward voltage: ~2V
  - Typical desired current: ~20mA

## Breadboard Layout Example

```
Adafruit TCS34725      Arduino (e.g., Teensy 4.0)
-----------------      -----------------------
   VIN ────────────────→ 3.3V
   GND ────────────────→ GND
   SDA ────────────────→ Pin 18
   SCL ────────────────→ Pin 19

RGB LED 0 (Common Cathode)
--------------------------
Red LED:
  Anode (+) → [220Ω Resistor] → Arduino Pin 21
  Cathode (-) → GND

Green LED:
  Anode (+) → [220Ω Resistor] → Arduino Pin 17
  Cathode (-) → GND

Blue LED:
  Anode (+) → [220Ω Resistor] → Arduino Pin 16
  Cathode (-) → GND
```

## Library Installation

### Arduino IDE

1. Open Arduino IDE
2. Go to **Sketch → Include Library → Manage Libraries**
3. Search for "Adafruit TCS34725"
4. Install the latest version by Adafruit
5. If prompted, install dependencies (Adafruit BusIO)

### PlatformIO

Add to `platformio.ini`:

```ini
lib_deps = 
    adafruit/Adafruit TCS34725@^1.4.3
```

## Calibration & Testing

### Step 1: Test LED Output

Uncomment in sketch to test:

```cpp
// In setup() after initialization:
testAllLeds();
```

This cycles through all LEDs testing each color.

### Step 2: Verify Sensor Communication

Enable debug mode:

```cpp
#define COLOR_SENSOR_DEBUG
```

Open Serial Monitor (115200 baud) to see color readings and distances.

### Step 3: Calibrate Colors

1. Enable COLOR_SENSOR_TUNING mode
2. Hold reference colors near the sensor
3. Note the RGB values in Serial output
4. Update the `COLORS` array with these values

Example output:
```
Color sensor read: (255, 12, 8)    // Red
Color sensor read: (18, 245, 15)   // Green
Color sensor read: (8, 22, 248)    // Blue
```

## Troubleshooting

### Sensor not detected

**Error:** "Color sensor failed to initialize"

**Solutions:**
- Verify I2C connections (SDA/SCL)
- Check power supply (3.3V or 5V)
- Enable I2C pull-up resistors (usually on sensor breakout)
- Use I2C scanner sketch to verify address (0x29)

### LEDs not lighting

**Problem:** LEDs stay off

**Solutions:**
- Verify LED polarity (common cathode vs anode)
- Check pin assignments in `LEDS` array
- Test with multimeter for continuity
- Verify resistor values and connections

### Inaccurate color detection

**Problem:** Wrong colors detected

**Solutions:**
- Adjust lighting conditions
- Enable COLOR_SENSOR_DEBUG to check Euclidean distances
- Update reference color values in `COLORS` array
- Ensure sensor has 614ms integration time for stable readings

### I2C conflicts

**Problem:** Multiple I2C devices interfere

**Solutions:**
- Use different I2C addresses if available
- Check for duplicate address conflicts
- Add pull-up resistors if needed (usually built-in)

## Power Considerations

- **Sensor**: Draws ~65mA at 3.3V
- **LED**: Draws ~20mA per color (typical)
- **Total typical draw**: ~125mA for sensor + 4 LEDs
- Ensure USB power or dedicated supply can handle this

Use a regulated power supply for stable operation.
