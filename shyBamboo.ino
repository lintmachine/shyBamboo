/**
 * Shy Bamboo
 * 
 * Josh Lind, 11/2016
 * 
 * RGB LED Pixel stands and echo distance sensor.
 * 
 * @todo Apply color dim in stalk gaps rather than single dark color.
 */

// @TODO
// Reduce NeoPixel burnout risk, add 1000 uF capacitor across
// pixel power leads, add 300 - 500 Ohm resistor on first pixel's data input
// and minimize distance between Arduino and first pixel.  Avoid connecting
// on a live circuit...if you must, connect GND first.

#include <Adafruit_NeoPixel.h>

#define PIN1 2 // LED Pixel strand 1
#define PIN2 3 // LED Pixel strand 2
#define PIN3 4 // LED Pixel strand 3
#define PIN4 5 // LED Pixel strand 4
#define PIN5 6 // LED Pixel strand 5
#define trigPin1 8 // Sensor: Trigger Pin 1
#define echoPin1 9 // Sensor: Echo Pin 1
#define trigPin2 10 // Sensor: Trigger Pin 2
#define echoPin2 11 // Sensor: Echo Pin 3
//#define PIN_LED 13 // Debug LED

const uint8_t moveThreshhold = 16, // Debounce the range fluxtuation
  maximumRange = 120,              // Maximum range needed
  minimumRange = 10,               // Minimum range needed
  NUM_LEDS = 90,                   // Strand length
  OFFSET_1 = 215,                  // Strand offset to first pixel 1
  OFFSET_2 = 120,                  // Strand offset to first pixel 2
  OFFSET_3 = 0,                    // Strand offset to first pixel 3
  OFFSET_4 = 150,                  // Strand offset to first pixel 4
  OFFSET_5 = 150,                  // Strand offset to first pixel 5
  bambooSizeInit = 20,             // Length of first bamboo segment
  bambooSizeSegmentDiff = -2,      // Each subsequent segment is decreased in size by this amount
  shyCycles = 40,
  delayed = 50;

uint32_t bambooColor, bambooDark;
boolean debug = true;
uint8_t shyCounter;
uint8_t prevDistance = 0;

#define LIGHT_STRIP_COUNT 2
Adafruit_NeoPixel lightStrips[LIGHT_STRIP_COUNT] = {
  Adafruit_NeoPixel(NUM_LEDS + OFFSET_1, PIN1, NEO_GRB + NEO_KHZ800),
  Adafruit_NeoPixel(NUM_LEDS + OFFSET_2, PIN2, NEO_GRB + NEO_KHZ800),
  // Adafruit_NeoPixel(NUM_LEDS + OFFSET_3, PIN3, NEO_GRB + NEO_KHZ800),
  // Adafruit_NeoPixel(NUM_LEDS + OFFSET_4, PIN4, NEO_GRB + NEO_KHZ800),
  // Adafruit_NeoPixel(NUM_LEDS + OFFSET_5, PIN5, NEO_GRB + NEO_KHZ800)
};

Adafruit_NeoPixel colorStrip = lightStrips[0];

//Adafruit_NeoPixel strip1 = Adafruit_NeoPixel(NUM_LEDS + OFFSET_1, PIN1, NEO_GRB + NEO_KHZ800),
//  strip2 = Adafruit_NeoPixel(NUM_LEDS + OFFSET_2, PIN2, NEO_GRB + NEO_KHZ800);
  //strip3 = Adafruit_NeoPixel(NUM_LEDS + OFFSET_3, PIN3, NEO_GRB + NEO_KHZ800);
  //strip4 = Adafruit_NeoPixel(NUM_LEDS + OFFSET_4, PIN4, NEO_GRB + NEO_KHZ800);
//Adafruit_NeoPixel strip5 = Adafruit_NeoPixel(NUM_LEDS + OFFSET_5, PIN5, NEO_GRB + NEO_KHZ800);


void setup() {

  for (int i = 0; i < LIGHT_STRIP_COUNT; ++i) {
    lightStrips[i].begin();
  }

  for (int i = 0; i < LIGHT_STRIP_COUNT; ++i) {
    lightStrips[i].show();
  }

  // Echo sensor.
  pinMode(trigPin1, OUTPUT);
  pinMode(echoPin1, INPUT);

  if (debug) {
    Serial.begin (9600);
    //pinMode(PIN_LED, OUTPUT);
    //digitalWrite(PIN_LED, LOW);
  }

  // Initial plant look.
  bambooColor = lightStrips[0].Color(0, 80, 0);
  bambooDark = lightStrips[0].Color(0, 0, 0);
  resetBamboo();
}


void loop() {
  uint32_t randomColor = Wheel(random(0, 255));
  uint16_t randomPixel = random(0, NUM_LEDS);

  // @todo Avoid same random pixel on all stocks.

  // Carry on sparkling.
  if (!checkMovement()) {
    // Shy period.
    if (shyCounter < shyCycles) {
      shyCounter++;
    }
    else {
      // Set random pixel to random color.
      if (colorStrip.getPixelColor(randomPixel + OFFSET_1) != bambooDark) {
        lightStrips[0].setPixelColor(randomPixel + OFFSET_1, randomColor);
        lightStrips[1].setPixelColor(random(0, NUM_LEDS) + OFFSET_2, Wheel(random(0, 255)));
        //strip3.setPixelColor(random(0, NUM_LEDS) + OFFSET_3, Wheel(random(0, 255)));
        //strip4.setPixelColor(random(0, NUM_LEDS) + OFFSET_4, Wheel(random(0, 255)));
        //strip5.setPixelColor(random(0, NUM_LEDS) + OFFSET_5, Wheel(random(0, 255)));

      }

      for (int i = 0; i < LIGHT_STRIP_COUNT; ++i) {
        lightStrips[i].show();
      }
    }
  }
  else {
    resetBamboo();
    shyCounter = 0;
  }
  delay(delayed);
}


/**
 * Reset LEDs to look like bamboo.
 */
void resetBamboo() {
  uint8_t i,
    bambooSize = bambooSizeInit;

  for (i = 0; i < NUM_LEDS; ++i) {
    setAllStrands(i, bambooColor); // Bamboo color to strand 2
  }

  for (i = 0; i < NUM_LEDS; i += bambooSize) {
    setAllStrands(i, bambooDark); // Gap in bamboo color
    bambooSize += bambooSizeSegmentDiff;
  }

  for (int i = 0; i < LIGHT_STRIP_COUNT; ++i) {
    lightStrips[i].show();
  }

//  if (debug) {
//    digitalWrite(PIN_LED, LOW);
//  }
}


/**
 * DRY, multiple bamboo shoots.
 */
void setAllStrands(uint16_t pixel, uint32_t color) {
  for (int i = 0; i < LIGHT_STRIP_COUNT; ++i) {
    lightStrips[i].setPixelColor(pixel + OFFSET_1, color);
  }
}


/**
 * Detect movement with distance sensor.
 * 
 * Accept a range of valid distance to trigger reaction.
 * 
 * Out of range is considered movement (the desired effect is only display with an audience).
 * 
 * @todo Rolling average rather than prevDistance.
 */
boolean checkMovement() {
  long duration;
  uint8_t distance;
  
  // Trigger-echo cycle to find distance of nearest object.
  digitalWrite(trigPin1, LOW); 
  delayMicroseconds(2); 
  digitalWrite(trigPin1, HIGH);
  delayMicroseconds(10); 
  digitalWrite(trigPin1, LOW);
  duration = pulseIn(echoPin1, HIGH);

  // Calculate the distance (in cm), based on speed of sound.
  distance = (duration/2) / 29.1;

  // Outside of boundaries.
  if (distance >= maximumRange || distance <= minimumRange) {
    // Send a negative number to indicate out of range.
    if (debug) {
      Serial.print("OUT OF RANGE  ");
      Serial.println(distance);
    }
    // Save to ignore subsequent bad sensor readings.
    prevDistance = 0;
    return true;
  }
  else {
    if (debug) {
      Serial.println(distance);
    }
    // Check for movement.
    if (distance > 0 && prevDistance > 0 && (abs(prevDistance - distance) > moveThreshhold)) {
      // Considered movement.
      Serial.print("STARTLED!  D:");
      Serial.print(distance);
      Serial.print(" - P:");
      Serial.println(prevDistance);
      // Track past.
      prevDistance = distance;
      return true;
    }
    prevDistance = distance;

    // Considered noise by threshhold.
    return false;
  }
}


/**
 * Input a value 0 to 255 to get a color value.
 * The colours are a transition r - g - b - back to r.
 */
uint32_t Wheel(byte WheelPos) {
  if (WheelPos < 85) {
    return colorStrip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  }
  else if (WheelPos < 170) {
    WheelPos -= 85;
    return colorStrip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  else {
    WheelPos -= 170;
    return colorStrip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
}

