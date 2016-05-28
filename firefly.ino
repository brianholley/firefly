#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

#define PIN 6

// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)
const uint32_t NumPixels = 64;
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NumPixels, PIN, NEO_GRB + NEO_KHZ800);

// IMPORTANT: To reduce NeoPixel burnout risk, add 1000 uF capacitor across
// pixel power leads, add 300 - 500 Ohm resistor on first pixel's data input
// and minimize distance between Arduino and first pixel.  Avoid connecting
// on a live circuit...if you must, connect GND first.

// c9ff2f
// b4e52a
// a0cc25

uint32_t nightColor = strip.Color(0, 0, 8);
uint32_t fireflyColor = strip.Color(0x50, 0x66, 0x12);

typedef struct Pixel
{
  uint32_t color;
};
uint32_t backgroundTimer = 0;
uint32_t BackgroundDelay = 100;
uint8_t BackgroundSame = 0;
uint8_t BackgroundConverge = 10;
uint8_t BackgroundDiverge = 100;

typedef struct Firefly
{
  uint8_t pixel;
  uint32_t currentColor;
  uint32_t targetColor;
  uint32_t tick;
  uint32_t duration;
};

Pixel pixels[NumPixels];
Firefly firefly;

void setup() {
  strip.begin();
  strip.setBrightness(128);

  fill(nightColor);
  strip.show();

  resetFirefly(firefly);
  for (uint8_t i=0; i < NumPixels; i++) {
    pixels[i].color = nightColor;
  }
}

void loop() {

  backgroundTimer++;
  if (backgroundTimer >= BackgroundDelay) {
    jitterBackground();
    backgroundTimer = 0;
  }
  //setRandomPixel(strip.Color(0,0,4), strip.Color(0,0,10), 10);

  bool valid = updateFirefly(firefly);
  strip.setPixelColor(firefly.pixel, firefly.currentColor);
  strip.show();

  if (!valid) {
    resetFirefly(firefly);
  }
  delay(1);
}

void jitterBackground() {
  uint8_t bNight = B(nightColor);
  for (uint8_t i=0; i < NumPixels; i++) {
    uint8_t r = random(100);
    if (r < BackgroundSame) {
      // No change
    }
    else if (r < BackgroundConverge) {
      uint8_t bCurrent = B(pixels[i].color);
      if (bCurrent < bNight)
        bCurrent++;
      else if (bCurrent > bNight)
        bCurrent--;
      pixels[i].color = strip.Color(0, 0, bCurrent);
    }
    else if (r < BackgroundDiverge) {
      uint8_t bCurrent = B(pixels[i].color);
      if (bCurrent < bNight && bCurrent > 0)
        bCurrent--;
      else if (bCurrent > bNight && bCurrent < 255)
        bCurrent++;
      pixels[i].color = strip.Color(0, 0, bCurrent);
    }
    strip.setPixelColor(i, pixels[i].color);
  }
}

bool updateFirefly(Firefly& ff) {

  ff.tick++;
  float d = ff.tick / (float)(ff.duration / 2);

  if (ff.targetColor == fireflyColor) {
    uint8_t r = fade(R(nightColor), R(fireflyColor), d);
    uint8_t g = fade(G(nightColor), G(fireflyColor), d);
    uint8_t b = fade(B(nightColor), B(fireflyColor), d);
    ff.currentColor = strip.Color(r, g, b);
    
    if (ff.tick == ff.duration / 2) {
      ff.targetColor = nightColor;
      ff.tick = 0;
    }
  }
  else {
    uint8_t r = fade(R(fireflyColor), R(nightColor), d);
    uint8_t g = fade(G(fireflyColor), G(nightColor), d);
    uint8_t b = fade(B(fireflyColor), B(nightColor), d);
    ff.currentColor = strip.Color(r, g, b);
    
    if (ff.tick == ff.duration / 2) {
      return false;
    }
  }
  return true;
}

void resetFirefly(Firefly& ff) {
  
  ff.pixel = random(strip.numPixels());
  ff.currentColor = nightColor;
  ff.targetColor = fireflyColor;
  ff.tick = 0;
  ff.duration = (random(500, 1200) / 2) * 2;
}

void setRandomPixel(uint32_t colBase, uint32_t colRand, uint8_t threshold) {
  uint8_t pixel = random(strip.numPixels());
  if (random(100) < threshold) {
    strip.setPixelColor(pixel, colRand);
  }
  else {
    strip.setPixelColor(pixel, colBase);
  }
  strip.show();
}

void fill(uint32_t c) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
  }
  strip.show();
}

uint8_t fade(uint8_t c1, uint8_t c2, float d) {
  return (uint8_t)(((int32_t)c2 - c1) * d + c1);
}

// Per Adafruit Neopixel docs:
// Packed format is always RGB, regardless of LED strand color order.
uint8_t R(uint32_t col) { return (col >> 16) & 0xff; }
uint8_t G(uint32_t col) { return (col >> 8) & 0xff; }
uint8_t B(uint32_t col) { return col & 0xff; }

