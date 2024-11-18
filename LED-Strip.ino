#include <FastLED.h>

#define LED_PIN 8
#define LED_COUNT 300

#define TOP_R 60
#define DESKLAMP_R 130
#define DESKLAMP_L 150
#define DESKLAMP2_R 75
#define DESKLAMP2_L 100
#define TOP_L 173
#define BOTTOM_L 228

#define Y_PIN A1
#define X_PIN A2
#define BUTTON 9
#define PIN_DIF 430

// STATIC
CRGB leds[LED_COUNT];

// VARIABLES
float r;
float g;
float b;

float r2;
float g2;
float b2;

float brightnessFactor;

int chunkSize;
int chunkOffCount;

int shouldAnimateTop = 0;
int animationChunkSize = 1;
int animationDuration = 0;

// CONTROL
int didChange = 1;
int isDesklampOn = 0;

#define COLOR_WHITE 0
#define COLOR_COLD 1
#define COLOR_WARM 2
#define COLOR_RED 3
int color = COLOR_WARM;

int shouldIgnoreButton = 0;
int shouldIgnorePin = 0;

int isPinLow(int x) {
  return (x < 500 - PIN_DIF) ? 1 : 0;
}
int isPinHigh(int x) {
  return (x > 500 + PIN_DIF) ? 1 : 0;
}

// SETUP
void setup() {
  Serial.begin(9600);

  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, LED_COUNT);

  pinMode(BUTTON, INPUT_PULLUP);
}

// UTIL
void setLEDSection(int first, int last) {
  int ledNumber;
  int ledNumberInChunk = 0;
  float iterationCount = 0;

  int isOn = 0;

  float rSpan = r - r2;
  float gSpan = g - g2;
  float bSpan = b - b2;

  int stepCount = last - first;
  float rDifferencePerStep = rSpan / stepCount;
  float gDifferencePerStep = gSpan / stepCount;
  float bDifferencePerStep = bSpan / stepCount;

  for (ledNumber = first; ledNumber < last + 1; ledNumber++) {
    if (chunkOffCount == 0) {
      isOn = 1;
    } else if (ledNumberInChunk == chunkOffCount) {
      isOn = 1;
    } else if (ledNumberInChunk == chunkSize) {
      isOn = 0;
      ledNumberInChunk = 0;
    }
    ledNumberInChunk++;

    float currentR = r - iterationCount * rDifferencePerStep;
    float currentG = g - iterationCount * gDifferencePerStep;
    float currentB = b - iterationCount * bDifferencePerStep;
    iterationCount++;

    if (isOn == 1) {
      leds[ledNumber] = CRGB(currentR * brightnessFactor, currentG * brightnessFactor, currentB * brightnessFactor);
    } else {
      leds[ledNumber] = CRGB(0, 0, 0);
    }

    if (animationDuration == 0) {
      continue;
    }

    if (ledNumber % animationChunkSize == 0) {
      delay(animationDuration);
      FastLED.show();
    }
  }
  FastLED.show();
}

void allOff() {
  r = 0;
  g = 0;
  b = 0;
  resetGradient();

  setLEDSection(0, LED_COUNT);
}

void setTopAnimation() {
  if (shouldAnimateTop == 0) return;
  animationDuration = 2;
  animationChunkSize = 3;
}

void setColor(int newColor) {
  if (color == newColor || shouldIgnorePin == 1) {
    return;
  }

  color = newColor;
  didChange = 1;
  shouldIgnorePin = 1;
}

// BRIGHTNESS
void setBrightness(float factor) {
  brightnessFactor = factor;
}

void resetBrightness() {
  setBrightness(1);
}

// CHUNKS
void setChunks(int size, int off) {
  chunkSize = size;
  chunkOffCount = off;
}

void resetChunks() {
  setChunks(1, 0);
}

// GRADIENT
void resetGradient() {
  r2 = r;
  g2 = g;
  b2 = b;
}

void flipGradient() {
  int r2Old = r2;
  int g2Old = g2;
  int b2Old = b2;

  r2 = r;
  g2 = g;
  b2 = b;

  r = r2Old;
  g = g2Old;
  b = b2Old;
}

// COLORS
void setWhite() {
  r = 255;
  g = 255;
  b = 255;
  resetGradient();
}

void setCold() {
  r = 150;
  g = 250;
  b = 255;
  resetGradient();
}

void setWarm() {
  r = 255;
  g = 50;
  b = 0;
  resetGradient();
}

void setRed() {
  r = 255;
  g = 0;
  b = 0;
  resetGradient();
}

void setDarkRed() {
  r = 1;
  g = 0;
  b = 0;
  resetGradient();
}

// RANGES
void applyAll() {
  setLEDSection(0, LED_COUNT);
}

void applyDesklamp() {
  resetChunks();
  resetBrightness();
  setTopAnimation();

  setLEDSection(DESKLAMP2_R, DESKLAMP2_L);
  setLEDSection(DESKLAMP_R, DESKLAMP_L);
}

void applyTop() {
  setLEDSection(TOP_R, TOP_L);
}

void applyBottom() {
  setLEDSection(BOTTOM_L, LED_COUNT);
}

void applyLeft() {
  setLEDSection(TOP_L, BOTTOM_L);
}

void applyRight() {
  setLEDSection(0, TOP_R);
}

// PRESETS
void standard(int withDesklamp) {
  resetGradient();
  float mainBrightness = withDesklamp == 1 ? 0.1 : 0.3;

  setBrightness(mainBrightness);
  setChunks(2, 1);
  applyAll();

  if (withDesklamp == 1) {
    applyDesklamp();
  }
}

void setSunset() {
  resetBrightness();
  setChunks(2, 1);

  int rTop = 250;
  int gTop = 25;
  int bTop = 0;

  int rBottom = 20;
  int gBottom = 0;
  int bBottom = 10;

  r = rTop;
  g = gTop;
  b = bTop;
  resetGradient();
  applyTop();

  r2 = rBottom;
  g2 = gBottom;
  b2 = bBottom;
  applyLeft();

  flipGradient();
  applyRight();

  r = rBottom;
  g = gBottom;
  b = bBottom;
  resetGradient();
  applyBottom();
}

void whiteDesklamp() {
  setWhite();
  applyDesklamp();
}

void lowRed() {
  setDarkRed();
  resetBrightness();
  setChunks(20, 19);
  applyTop();
  applyRight();
  applyBottom();
  resetChunks();
}

void redWithDesklamp() {
  setDarkRed();
  setChunks(3, 2);
  applyRight();
  resetChunks();

  setRed();
  setBrightness(0.1);
  applyBottom();

  applyDesklamp();
}

// LOOP
void loop() {
  if (digitalRead(BUTTON) == 0) {
    if (shouldIgnoreButton == 0) {
      isDesklampOn = isDesklampOn == 0 ? 1 : 0;
      shouldIgnoreButton = 1;
      shouldAnimateTop = 1;
      didChange = 1;
    }
  } else {
    shouldIgnoreButton = 0;
  }

  int x = analogRead(X_PIN);
  int y = analogRead(Y_PIN);

  if (isPinLow(x) == 1) {
    setColor(COLOR_COLD);
  } else if (isPinHigh(x) == 1) {
    setColor(COLOR_WARM);
  } else if (isPinLow(y) == 1) {
    setColor(COLOR_RED);
  } else if (isPinHigh(y) == 1) {
    setColor(COLOR_WHITE);
  } else {
    shouldIgnorePin = 0;
  }

  if (didChange == 0) return;
  allOff();

  switch (color) {
    case COLOR_WHITE:
      whiteDesklamp();
      break;
    case COLOR_COLD:
      setCold();
      standard(isDesklampOn);
      break;
    case COLOR_WARM:
      setSunset();
      break;
    case COLOR_RED:
      if (isDesklampOn == 1) {
        redWithDesklamp();
      } else {
        lowRed();
      }
      break;
  }

  shouldAnimateTop = 0;
  animationDuration = 0;
  didChange = 0;
}
