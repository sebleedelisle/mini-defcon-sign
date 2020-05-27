#include <Adafruit_DotStar.h>
#include <avr/power.h>
#include <EEPROM.h>

#include "Light.h"

#define NUMPIXELS 6 // Number of LEDs in strip

#define DATAPIN    0
#define CLOCKPIN   1
#define BUTTONPIN  2

#define MODE_MANUAL 0
#define MODE_PINGPONG 1
#define MODE_RANDOM 2
#define MODE_DISCO 3
#define MODE_BORIS 4

#define CHANGE_RISING 0
#define CHANGE_PINGPONG 1
#define CHANGE_RANDOM 2
#define CHANGE_NONE 3

Adafruit_DotStar strip(NUMPIXELS, DATAPIN, CLOCKPIN, DOTSTAR_BGR);

int pixelNum = 0;
int mode = 0;
const int numModes = 5;

uint32_t colours[] = {0xffee88, 0xffaa33, 0xffcc66, 0x66ff33, 0x669988, 0xffee88};
Light lights[6];

bool buttonPressed = false;
unsigned long lastButtonPressTime = 0;
unsigned long lastButtonReleaseTime = 0;
unsigned long lastModeChangeTime = 0;
unsigned long lastLightChangeTime = 0;

bool showCurrentMode = true;
bool rising = true;
bool autoChange = false;
unsigned long autoChangeFrequency = 5000;
unsigned long lastAutoChangeTime = 0;
int changeMode = CHANGE_RISING;
int flashSpeed = 1000;

void setup() {

#if defined(__AVR_ATtiny85__) && (F_CPU == 16000000L)
  clock_prescale_set(clock_div_1); // Enable 16 MHz on Trinket
#endif

  strip.begin(); // Initialize pins for output
  strip.show();  // Turn all LEDs off ASAP

  pinMode(BUTTONPIN, INPUT_PULLUP);

  for (int i = 0; i < 6; i++) {
    lights[i].setBase(colours[i]);
    lights[i].turnOff();
  }


  // if the button is pressed on power up then clear the EEPROM
  if (!digitalRead(BUTTONPIN)) {
    clearEEPROM();
  }

  // load the latest state from the EEPROM

  if (EEPROM.length() >= 2) {
    pixelNum = EEPROM.read(0);
    mode = EEPROM.read(1);

    // Error checking
    if ((pixelNum < 0) || (pixelNum > 4)) {
      pixelNum = 0;
      clearEEPROM();
    }
    if ((mode < 0) || (mode > 4)) {
      mode = 0;
      clearEEPROM();
    }
    startMode(mode);
  }


}

void loop() {


  // if we're currently in mode selection, flash the
  // light to represent the mode
  if (showCurrentMode) {
    if (millis() % 300 < 100) lights[mode].turnOn();
    else lights[mode].turnOff();
    lights[5].turnOff();
    if (millis() - lastModeChangeTime > 2000) {
      showCurrentMode = false;
      // reset lights
      clearLights();
      lights[pixelNum].turnOn();
    }

  } else {
    // if the lights auto change, and it's time to
    // change, then do it!
    if (autoChange) {
      if (millis() - lastLightChangeTime > autoChangeFrequency) {
        changeLight();
      }
    }

    // if we changed the light recently, then flash everything
    if ((millis() - lastLightChangeTime) < 5000) {
      long timeSinceChange = millis() - lastLightChangeTime;
      if (timeSinceChange % flashSpeed < (flashSpeed * 0.7f)) {
        lights[pixelNum].turnOn();
        lights[5].turnOn();
      } else {
        lights[pixelNum].turnOff();
        lights[5].turnOff();
      }
    } else {
      if (!lights[pixelNum].isOn()) lights[pixelNum].turnOn();
      if (!lights[5].isOn()) lights[5].turnOn();
    }

    if (mode == MODE_BORIS) {
      int m = getNoise();
      if (m < 100)  {
        lights[pixelNum].turnOff();
        if (pixelNum > 0) lights[pixelNum - 1].turnOn();
      } else {
        lights[pixelNum].turnOn();
        if (pixelNum > 0) lights[pixelNum - 1].turnOff();

      }
      if (m > 200) {
        lights[5].turnOff();
      } else {
        lights[5].turnOn();
      }

    }

  }

  for (int i = 0; i < 6; i++) {
    lights[i].update();
    strip.setPixelColor(i, lights[i].getColour());
  }

  strip.show();                     // Refresh strip

  // if button is pressed
  if (!digitalRead(BUTTONPIN)) {

    // ...and the button wasn't already pressed
    if ((!buttonPressed)&&((millis() - lastButtonPressTime) > 100)) {
      lastButtonPressTime = millis();

      if (showCurrentMode) {
        // if the mode indicator is currently flashing and
        // the button is pressed then change mode
        clearLights();
        mode++;
        if (mode >= numModes) mode = 0;
        startMode(mode);
      }
      // if the button has been held for a while then switch
      // to mode selection
    } else if ((!showCurrentMode) && ((millis() - lastButtonPressTime) > 2000)) {
      //MODE CHANGE
      clearLights();
      lastModeChangeTime = millis();
      showCurrentMode = true;
    }

    buttonPressed = true;

  } else { // else if the button is released
    // ... and it was previously pressed, then change the light
    if ((buttonPressed) && ((millis() - lastButtonReleaseTime) > 100)) {
      if (!showCurrentMode) {
        changeLight();
      }
      lastButtonReleaseTime = millis();
    }
    buttonPressed = false;
  }

  // wait 1ms to ensure the loop doesn't run too fast.
  // This helps with button debouncing and the animation
  // speed
  delay(1);


}

void clearLights() {
  for (int i = 0; i < 5; i++) lights[i].turnOff();
}

void changeLight() {
  clearLights();
  // MANUAL MODE
  if (changeMode == CHANGE_RISING) {

    if (pixelNum == 0) pixelNum = 4;
    else pixelNum--;

  lights[pixelNum].turnOn();

    // PING PONG MODE
  } else if (changeMode == CHANGE_PINGPONG) {

    if (rising) {
      if (pixelNum < 4) {
        pixelNum++;
      } else {
        rising = false;
        pixelNum--;
      }
    } else {
      if (pixelNum > 0) {
        pixelNum--;
      } else {
        rising = true;
        pixelNum++;
      }

    }
    lights[pixelNum].turnOn();
    // RANDOM
  } else if (changeMode == CHANGE_RANDOM) {

    if ((pixelNum == 0) || ((random(20) < 10) && (pixelNum < 4) )) {
      pixelNum++;
    } else {
      pixelNum--;
    }
    lights[pixelNum].turnOn();
  }
  lastLightChangeTime = millis();
  EEPROM.write(0, pixelNum);
}

void startMode(int newmode) {
  lastModeChangeTime = millis();

  switch (mode) {
    case MODE_MANUAL:
      autoChange = false;
      changeMode = CHANGE_RISING;
      break;

    case MODE_PINGPONG:
      changeMode = CHANGE_PINGPONG;
      autoChangeFrequency = 60000; // change every min
      autoChange = true;
      break;

    case MODE_RANDOM:
      changeMode = CHANGE_RANDOM;
      autoChangeFrequency = 60000;
      autoChange = true;
      break;

    case MODE_DISCO:
      changeMode = CHANGE_PINGPONG;
      autoChangeFrequency = flashSpeed;
      autoChange = true;
      break;

    case MODE_BORIS:
      autoChange = false;
      changeMode = CHANGE_RISING;
      pixelNum = 3;
      break;

  }

  EEPROM.write(1, mode);
}

void clearEEPROM()
{
  for (int i = 0 ; i < EEPROM.length() ; i++) {
    if (EEPROM.read(i) != 0)
    {
      EEPROM.write(i, 0);
    }
  }

}
