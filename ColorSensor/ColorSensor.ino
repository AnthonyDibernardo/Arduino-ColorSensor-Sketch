#include <Arduino.h>
#include <Adafruit_TCS34725.h>
#include <math.h>

// ============================================================================
// CONFIGURATION
// ============================================================================

// Uncomment to enable detailed debug output for color matching distances
#define COLOR_SENSOR_DEBUG

// Uncomment to enable tuning mode (prints all color readings, no LED control)
#define COLOR_SENSOR_TUNING

// Sensor integration time and gain settings
#define SENSOR_INTEGRATION_TIME TCS34725_INTEGRATIONTIME_614MS
#define SENSOR_GAIN TCS34725_GAIN_1X
#define COLORLEDPIN 11

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
};

// Reference colors for matching
// Stores the color name, RGB target values, and LED output values
const ReferenceColor COLORS[] = {
    {
        .name = "Red",
        .match = {106, 60, 50},
    },
    {
        .name = "Green",
        .match = {60, 110, 60},
    },
    {
        .name = "Blue",
        .match = {100, 80, 60},
    },
    {
        .name = "Yellow",
        .match = {126, 80, 40},
    },
    {
        .name = "Orange",
        .match = {140, 70, 50},
    }

};

// ============================================================================
// LED CONFIGURATION
// ============================================================================

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
    return {(uint8_t) round(r), (uint8_t) round(g), (uint8_t) round(b)};
}
/**
 * Read ambient light level (lux) from the sensor
 */
uint16_t readLux() {
    float r, g, b;
    colorSensor.getRGB(&r, &g, &b);
    return colorSensor.calculateLux(r, g, b);
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
void read() {
    digitalWrite(COLORLEDPIN, HIGH);
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
    digitalWrite(COLORLEDPIN, LOW);

    // Average all 5 samples
    uint8_t red = (read1.r + read2.r + read3.r + read4.r + read5.r) / 5;
    uint8_t green = (read1.g + read2.g + read3.g + read4.g + read5.g) / 5;
    uint8_t blue = (read1.b + read2.b + read3.b + read4.b + read5.b) / 5;
    Color readFinal = {red, green, blue};

    Serial.print("Color sensor read: ");
    debugColor(readFinal);

    // Find closest matching color using Euclidean distance
    float lowestDistance = INFINITY;
    for (ReferenceColor color : COLORS) {
        float distance = calculateEuclidianDistance(readFinal, color.match);
        if (distance < lowestDistance) {
            lowestDistance = distance;
            const char* closestColor = color.name;
            Serial.println(color.name);
        }

        Serial.print("Distance from ");
        Serial.print(color.name);
        Serial.print(": ");
        Serial.println(distance);
    }

}

// ============================================================================
// ARDUINO SETUP AND LOOP
// ============================================================================

void setup() {
    // Initialize Serial communication for debugging
    Serial.begin(9600);
    delay(100);
    pinMode(COLORLEDPIN, OUTPUT);
    
    Serial.println("Starting Color Sensor Sketch...");

    // Initialize the color sensor
    if (!colorSensor.begin()) {
        Serial.println("ERROR: Color sensor failed to initialize!");
        while (1) {
            delay(1000);
        }
    }
    
    Serial.println("Color sensor initialized successfully");
    Serial.println("Ready to detect colors!");
}

void loop() {
	  read();
    delay(500);
}
