

#include <Adafruit_DotStar.h>
#include <avr/power.h> 

#define NUMPIXELS 6 // Number of LEDs in strip

#define DATAPIN    0
#define CLOCKPIN   1
#define BUTTONPIN  2
Adafruit_DotStar strip(NUMPIXELS, DATAPIN, CLOCKPIN, DOTSTAR_BGR);

int pixelNum = 0; 
uint32_t colours[] = {0xffdd44, 0xff0000, 0xffff00, 0x00ff00, 0x0000ff, 0xffdd44};

void setup() {

#if defined(__AVR_ATtiny85__) && (F_CPU == 16000000L)
  clock_prescale_set(clock_div_1); // Enable 16 MHz on Trinket
#endif
  pinMode(BUTTONPIN, INPUT_PULLUP); 

  strip.begin(); // Initialize pins for output
  strip.show();  // Turn all LEDs off ASAP
}

void loop() {

  strip.clear(); 
  strip.setPixelColor(5, colours[5]); 
  strip.setPixelColor(pixelNum, colours[pixelNum]); 
  strip.show();                     // Refresh strip

  // wait for button to be released
  while(!digitalRead(BUTTONPIN)) delay(100);

  // wait for button to be pressed
  while(digitalRead(BUTTONPIN)) delay(100);

  pixelNum++; 
  if(pixelNum>=5) pixelNum = 0; 
  
}
