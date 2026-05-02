#include <Arduino.h>
#include <Adafruit_TCS34725.h>
#include <LiquidCrystal.h>
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
// LCD 1602 DISPLAY CONFIGURATION
// ============================================================================

// LCD pin connections (parallel mode)
// Modify these pins to match your wiring
#define RS_PIN 12      // Register Select
#define EN_PIN 13      // Enable
#define D4_PIN 5       // Data line 4
#define D5_PIN 6       // Data line 5
#define D6_PIN 7       // Data line 6
#define D7_PIN 8       // Data line 7

// Initialize the LCD library with the pin numbers
LiquidCrystal lcd(RS_PIN, EN_PIN, D4_PIN, D5_PIN, D6_PIN, D7_PIN);

// ============================================================================
// BUTTON CONFIGURATION
// ============================================================================

#define RED_BUTTON_PIN 2      // Red button trigger pin
#define GREEN_BUTTON_PIN 3    // Green button trigger pin

// ============================================================================
// PWM CONFIGURATION
// ============================================================================

#define PWM_PIN_5 5           // PWM output pin 5
#define PWM_PIN_6 6           // PWM output pin 6
#define PWM_PIN_9 9           // PWM output pin 9
#define PWM_VALUE 127         // Half of max (255 / 2 = 127.5)

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
// Calibrated colors from your testing
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
// INTERRUPT SERVICE ROUTINES
// ============================================================================

volatile unsigned long lastRedButtonTime = 0;
volatile unsigned long lastGreenButtonTime = 0;

/**
 * Interrupt handler for Red button (Pin 2)
 * Debounced to 50ms to prevent false triggers
 */
void redButtonISR() {
    unsigned long currentTime = millis();
    if (currentTime - lastRedButtonTime > 50) {
        lastRedButtonTime = currentTime;
        forceRedRead();
    }
}

/**
 * Interrupt handler for Green button (Pin 3)
 * Debounced to 50ms to prevent false triggers
 */
void greenButtonISR() {
    unsigned long currentTime = millis();
    if (currentTime - lastGreenButtonTime > 50) {
        lastGreenButtonTime = currentTime;
        forceGreenRead();
    }
}

/**
 * Force display of Red color
 */
void forceRedRead() {
    Serial.println("=== RED BUTTON PRESSED ===");
    displayOnLCD("Color:", "Red");
}

/**
 * Force display of Green color
 */
void forceGreenRead() {
    Serial.println("=== GREEN BUTTON PRESSED ===");
    displayOnLCD("Color:", "Green");
}

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
 * Display text on LCD
 * Line 1 = top row, Line 2 = bottom row
 */
void displayOnLCD(const char* line1, const char* line2) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(line1);
    if (line2 != NULL) {
        lcd.setCursor(0, 1);
        lcd.print(line2);
    }
}

/**
 * Initialize PWM outputs on pins 5, 6, and 9 to half max value (127)
 */
void initializePWM() {
    pinMode(PWM_PIN_5, OUTPUT);
    pinMode(PWM_PIN_6, OUTPUT);
    pinMode(PWM_PIN_9, OUTPUT);
    
    analogWrite(PWM_PIN_5, PWM_VALUE);
    analogWrite(PWM_PIN_6, PWM_VALUE);
    analogWrite(PWM_PIN_9, PWM_VALUE);
    
    Serial.print("PWM initialized on pins 5, 6, 9 with value: ");
    Serial.println(PWM_VALUE);
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
 * Read color from sensor and display on LCD and Serial
 * 
 * Process:
 * 1. Takes 5 sequential color readings with 10ms delays
 * 2. Averages the readings to reduce noise
 * 3. Compares against reference colors using Euclidean distance
 * 4. Displays the detected color on LCD
 * 5. Prints debug information to Serial
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
    const char* closestColorName = "Unknown";
    
    for (ReferenceColor color : COLORS) {
        float distance = calculateEuclidianDistance(readFinal, color.match);
        if (distance < lowestDistance) {
            lowestDistance = distance;
            closestColorName = color.name;
        }

        #ifdef COLOR_SENSOR_DEBUG
        Serial.print("Distance from ");
        Serial.print(color.name);
        Serial.print(": ");
        Serial.println(distance);
        #endif
    }

    // Print detected color to Serial
    Serial.print("Detected Color: ");
    Serial.println(closestColorName);
    Serial.println("---");

    // Display detected color on LCD
    // Line 1: "Color:"
    // Line 2: The color name (e.g., "Red", "Green", etc.)
    char lcdLine1[16] = "Color:";
    char lcdLine2[16];
    
    // Copy color name to line 2, truncate to fit 16 characters
    strncpy(lcdLine2, closestColorName, 15);
    lcdLine2[15] = '\0';
    
    displayOnLCD(lcdLine1, lcdLine2);
}

// ============================================================================
// ARDUINO SETUP AND LOOP
// ============================================================================

void setup() {
    // Initialize Serial communication for debugging
    Serial.begin(9600);
    delay(100);
    
    // Initialize LCD display (16 columns, 2 rows)
    lcd.begin(16, 2);
    
    // Display startup message
    displayOnLCD("Color Sensor", "Initializing...");
    delay(2000);
    
    pinMode(COLORLEDPIN, OUTPUT);
    
    // Initialize button pins with pull-up resistors and attach interrupts
    pinMode(RED_BUTTON_PIN, INPUT_PULLUP);
    pinMode(GREEN_BUTTON_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(RED_BUTTON_PIN), redButtonISR, FALLING);
    attachInterrupt(digitalPinToInterrupt(GREEN_BUTTON_PIN), greenButtonISR, FALLING);
    
    Serial.println("Starting Color Sensor Sketch...");
    Serial.println("LCD Display: 16x2 initialized");
    Serial.println("Buttons: Red (Pin 2), Green (Pin 3)");

    // Initialize the color sensor
    if (!colorSensor.begin()) {
        Serial.println("ERROR: Color sensor failed to initialize!");
        displayOnLCD("ERROR:", "Sensor Failed!");
        while (1) {
            delay(1000);
        }
    }
    
    Serial.println("Color sensor initialized successfully");
    Serial.println("Ready to detect colors!");
    
    // Initialize PWM outputs
    initializePWM();
    
    // Display ready message
    displayOnLCD("Ready to", "Detect Colors!");
    delay(2000);
}

void loop() {
    read();
    delay(500);
}
