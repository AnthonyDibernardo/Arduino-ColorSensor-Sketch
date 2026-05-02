#include <Arduino.h>
#include <Adafruit_TCS34725.h>

// ============================================================================
// CONFIGURATION
// ============================================================================

// Uncomment to enable detailed debug output for color matching distances
// #define COLOR_SENSOR_DEBUG

// Uncomment to enable tuning mode (prints all color readings, no LED control)
// #define COLOR_SENSOR_TUNING

// Sensor integration time and gain settings
#define SENSOR_INTEGRATION_TIME TCS34725_INTEGRATIONTIME_614MS
#define SENSOR_GAIN TCS34725_GAIN_1X

// ============================================================================
// COLOR SENSOR OBJECT
// ============================================================================

Adafruit_TCS34725 colorSensor(SENSOR_INTEGRATION_TIME, SENSOR_GAIN);

// ============================================================================
// COLOR DEFINITIONS
// ============================================================================

struct Color {
  uint8_t r;
  uint8_t g;
  uint8_t b;
};

struct ReferenceColor {
  const char* name;
  Color match;
  Color led;
};

// Reference colors for matching
// Stores the color name, RGB target values, and LED output values
const ReferenceColor COLORS[] = {
    {
        .name = "Red",
        .match = {255, 0, 0},
        .led = {HIGH, LOW, LOW}
    },
    {
        .name = "Green",
        .match = {0, 255, 0},
        .led = {LOW, HIGH, LOW}
    },
    {
        .name = "Blue",
        .match = {0, 0, 255},
        .led = {LOW, LOW, HIGH}
    }
};

// ============================================================================
// LED CONFIGURATION
// ============================================================================

// LED pin definitions. Each LED has 3 pins for R, G, and B respectively.
// Modify these pins to match your hardware setup
const int LEDS[][3] = {
    {21, 17, 16},   // LED 0: Red pin 21, Green pin 17, Blue pin 16
    {2, 3, 4},      // LED 1: Red pin 2, Green pin 3, Blue pin 4
    {41, 13, 40},   // LED 2: Red pin 41, Green pin 13, Blue pin 40
    {7, 8, 9}       // LED 3: Red pin 7, Green pin 8, Blue pin 9
};

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

/**
 * Calculate Euclidean distance between two colors in RGB space
 * Used to determine which reference color is closest to the detected color
 */
float calculateEuclidianDistance(Color current, Color match) {
    float diffR = current.r - match.r;
    float diffG = current.g - match.g;
    float diffB = current.b - match.b;
    
    long sumOfSquares = (long)diffR * diffR + (long)diffG * diffG + (long)diffB * diffB;
    
    return sqrt(sumOfSquares);
}

/**
 * Print a color value to Serial in (R, G, B) format
 */
void debugColor(Color color) {
    Serial.print("(");
    Serial.print(color.r); Serial.print(", ");
    Serial.print(color.g); Serial.print(", ");
    Serial.print(color.b);
    Serial.println(")");
}

/**
 * Read a single RGB color sample from the sensor
 * Returns a Color struct with 8-bit RGB values
 */
Color readColor() {
    float r, g, b;
    colorSensor.getRGB(&r, &g, &b);
    return {(uint8_t) std::round(r), (uint8_t) std::round(g), (uint8_t) std::round(b)};
}

/**
 * Read ambient light level (lux) from the sensor
 */
uint16_t readLux() {
    float r, g, b;
    colorSensor.getRGB(&r, &g, &b);
    return colorSensor.calculateLux(r, g, b);
}

/**
 * Set the color of a specific LED using digital pins
 */
void setLedColor(size_t led, bool r, bool g, bool b) {
    if (led >= std::size(LEDS) || led < 0) {
        Serial.println("ERROR: Invalid LED index");
        return;
    }

    digitalWrite(LEDS[led][0], r);
    digitalWrite(LEDS[led][1], g);
    digitalWrite(LEDS[led][2], b);  
}

/**
 * Test all LEDs by cycling through colors
 */
void testAllLeds() {
    for(int i = 0; i < std::size(LEDS); i++) {
        setLedColor(i, 1, 0, 0);
        delay(1000);
        setLedColor(i, 0, 1, 0);
        delay(1000);
        setLedColor(i, 0, 0, 1);
        delay(1000);
        setLedColor(i, 1, 0, 1);
        delay(1000);

        setLedColor(i, 0, 0, 0);
    }
}

// ============================================================================
// COLOR SENSOR MAIN FUNCTION
// ============================================================================

/**
 * Read color from sensor and set LED to matched color
 * 
 * Process:
 * 1. Takes 5 sequential color readings with 10ms delays
 * 2. Averages the readings to reduce noise
 * 3. Compares against reference colors using Euclidean distance
 * 4. Sets the specified LED to the closest matching color
 * 5. Includes special handling for purple detection
 */
void readThenSetLED(size_t led) {
    // Take 5 samples with delays between each
    Color read1 = readColor();
    delay(10);
    Color read2 = readColor();
    delay(10);
    Color read3 = readColor();
    delay(10);
    Color read4 = readColor();
    delay(10);
    Color read5 = readColor();

    // Average all 5 samples
    uint8_t red = (read1.r + read2.r + read3.r + read4.r + read5.r) / 5;
    uint8_t green = (read1.g + read2.g + read3.g + read4.g + read5.g) / 5;
    uint8_t blue = (read1.b + read2.b + read3.b + read4.b + read5.b) / 5;
    Color readFinal = {red, green, blue};

    #ifdef COLOR_SENSOR_TUNING
    Serial.print("Color sensor read: ");
    debugColor(readFinal);
    #endif

    // Find closest matching color using Euclidean distance
    float lowestDistance = INFINITY;
    Color closestColor = {LOW, LOW, LOW};
    for (ReferenceColor color : COLORS) {
        float distance = calculateEuclidianDistance(readFinal, color.match);
        if (distance < lowestDistance) {
            lowestDistance = distance;
            closestColor = color.led;
        }

        #ifdef COLOR_SENSOR_DEBUG
        Serial.print("Distance from ");
        Serial.print(color.name);
        Serial.print(": ");
        Serial.println(distance);
        #endif
    }
    
    // Special handling for purple: if blue is detected with high red value
    if (closestColor.b == HIGH && readFinal.r > 40) {
        closestColor.r = HIGH;
    }

    // Set the LED to the matched color
    setLedColor(led, closestColor.r, closestColor.g, closestColor.b);
}

// ============================================================================
// ARDUINO SETUP AND LOOP
// ============================================================================

void setup() {
    // Initialize Serial communication for debugging
    Serial.begin(115200);
    delay(100);
    
    Serial.println("Starting Color Sensor Sketch...");

    // Initialize all LED pins as outputs
    for (size_t i = 0; i < std::size(LEDS); i++) {
        for (size_t j = 0; j < 3; j++) {
            pinMode(LEDS[i][j], OUTPUT);
            digitalWrite(LEDS[i][j], LOW);
        }
    }

    // Initialize the color sensor
    if (!colorSensor.begin()) {
        Serial.println("ERROR: Color sensor failed to initialize!");
        while (1) {
            delay(1000);
        }
    }
    
    Serial.println("Color sensor initialized successfully");
    Serial.println("Ready to detect colors!");

    #ifdef COLOR_SENSOR_DEBUG
    Serial.println("DEBUG mode enabled - detailed color distances will be printed");
    #endif

    #ifdef COLOR_SENSOR_TUNING
    Serial.println("TUNING mode enabled - printing raw color readings only");
    #endif
}

void loop() {
    #ifdef COLOR_SENSOR_TUNING
    // Tuning mode: just print raw color readings
    readThenSetLED(0);
    delay(1000);
    #else
    // Normal mode: detect colors and set LEDs
    readThenSetLED(0);
    delay(500);
    #endif
}
