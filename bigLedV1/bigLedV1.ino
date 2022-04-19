// Slave arduino. Controlled by controlUnit.ino
// Needed because the LCD display uses a lot of pins and has an interrupt handler for mode button changes

#include <FastLED.h>

// Which pins are mapped to what
#define LED_PIN 6 // Data bus for LEDS
#define NUM_LEDS 300 // How many leds

// RGBW Pins
#define RED_PIN A0
#define GREEN_PIN A1
#define BLUE_PIN A2
#define BRIGHT_PIN A3

#define UPDATES_PER_SECOND 250

// We read the mode from a 2nd arduino which controls the mode & LCD display
#define MODE_PIN0 2 // low bit 1 orange
#define MODE_PIN1 3 // 2 yellow
#define MODE_PIN2 4 // High bit 4 green

// Turn Vcc for LEDs on/off.
#define RELAY_PIN 5  // todo

// Initalise the leds
CRGB leds[NUM_LEDS];

// Helper variables
/*int red;
int green;
int blue;
int brightness;*/
#define refreshDelay 100

bool gReverseDirection = false;
CRGBPalette16 gPal;

CRGBPalette16 currentPalette;
TBlendType    currentBlending;

extern CRGBPalette16 myRedWhiteBluePalette;
extern const TProgmemPalette16 myRedWhiteBluePalette_p PROGMEM;

#define COOLING  55
#define SPARKING 120

#define debug true
void setup() {
  delay(500); // Boot recovery delay
  if(debug){Serial.begin(9600);}
  
  // Initalise r/w pins
  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);

  // pots
  pinMode(RED_PIN, INPUT);
  pinMode(GREEN_PIN, INPUT);
  pinMode(BLUE_PIN, INPUT);
  pinMode(BRIGHT_PIN, INPUT);

  // 3 bit parallel comms
  pinMode(MODE_PIN0, INPUT);
  pinMode(MODE_PIN1, INPUT);
  pinMode(MODE_PIN2, INPUT);

//todo
  pinMode(RELAY_PIN, OUTPUT);
}

void loop() {

  int currentMode = getMode(); // Find out which mode we're in
  delay(50);
  if(currentMode==0)
  {
    modeOff();
  }
  else if(currentMode==1)
  {
    modeAll();
  }
  else if(currentMode==2)
  {
    modeSweep();
  }
  else if(currentMode==3)
  {
    modePalette();
  }
  else if(currentMode==4)
  {
    modeFire();
  }
  else{Serial.println("unknown mode");}
  
}

// Find out which mode the controller says
// 3 PINS -> 8 MODES SUPPORTED
int getMode()
{
  unsigned int retVal=0;

  if(debug){
    Serial.println("entering getMode");
    Serial.print("Bits: ");
    Serial.print("   0: ");Serial.print(digitalRead(MODE_PIN0), DEC);
    
    Serial.print("   1: ");Serial.print(digitalRead(MODE_PIN1), DEC);
    Serial.print("  2: ");Serial.print(digitalRead(MODE_PIN2), DEC);
  }
  // cast the pins to an int and then extract the value from a String[]

  retVal = digitalRead(MODE_PIN0) +  (digitalRead(MODE_PIN1)*2) + (digitalRead(MODE_PIN2)*4);
  
  if(debug){Serial.print("  mode: ");Serial.println(retVal,DEC);}
  return retVal;
}



//////////
// Drive the leds according to the mode
//////////

// Turn the LEDs off
void modeOff()
{
  if(debug){Serial.println("entering mode off");}
  for (int i = 0; i <= NUM_LEDS; i++) {
    leds[i] = CRGB (0,0,0);
    
  }
  FastLED.show();
}

// Turn on all of the LEDs according to their brightness and rgb values
void modeAll()
{
  if(debug){Serial.println("entering mode rgbw()");}
 // readRgbw();
  int brightness = getBrightness();
  int red = getRed();
  int blue=getBlue();
  int green=getGreen();
  
  if(debug){Serial.print("red ");Serial.print(red,DEC);Serial.print(" green ");Serial.print(green,DEC);Serial.print(" blue ");Serial.print(blue,DEC);Serial.print(" bright ");Serial.println(brightness,DEC);}
  
  for (int i = 0; i <= NUM_LEDS; i++) {
    leds[i] = CRGB (red,green,blue);
  }
  FastLED.show();
  delay(refreshDelay);
  
}

void modeSweep()
{
  if(debug){Serial.println("entering mode sweep");}
   // Step 1.  Cool down every cell a little
    int red=random(0,255);
    int green=random(0,255);
    int blue=random(0,255);
    for (int i = 0; i <= NUM_LEDS; i++) {
      leds[i] = CRGB (red,green,blue);
      FastLED.show();
  }
}


// From fastLed examples
void modeFire()
{
  if(debug){Serial.println("entering mode fire");}
  FastLED.setBrightness( 128 );
  // Array of temperature readings at each simulation cell
  static uint8_t heat[NUM_LEDS];

  // Step 1.  Cool down every cell a little
    for( int i = 0; i < NUM_LEDS; i++) {
      heat[i] = qsub8( heat[i],  random8(0, ((COOLING * 10) / NUM_LEDS) + 2));
    }
  
    // Step 2.  Heat from each cell drifts 'up' and diffuses a little
    for( int k= NUM_LEDS - 1; k >= 2; k--) {
      heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2] ) / 3;
    }
    
    // Step 3.  Randomly ignite new 'sparks' of heat near the bottom
    if( random8() < SPARKING ) {
      int y = random8(7);
      heat[y] = qadd8( heat[y], random8(160,255) );
    }

    // Step 4.  Map from heat cells to LED colors
    for( int j = 0; j < NUM_LEDS; j++) {
      // Scale the heat value from 0-255 down to 0-240
      // for best results with color palettes.
      uint8_t colorindex = scale8( heat[j], 240);
      CRGB color = ColorFromPalette( gPal, colorindex);
      int pixelnumber;
      if( gReverseDirection ) {
        pixelnumber = (NUM_LEDS-1) - j;
      } else {
        pixelnumber = j;
      }
      leds[pixelnumber] = color;
    }
 
}


// Taken from fastLed examples/ColourPalette
void modePalette()
{
  if(debug){Serial.println("entering mode palette");}
   ChangePalettePeriodically();
    
    static uint8_t startIndex = 0;
    startIndex = startIndex + 1; 
    
    FillLEDsFromPaletteColors( startIndex);
    
    FastLED.show();
    FastLED.delay(1000 / UPDATES_PER_SECOND);
}


void FillLEDsFromPaletteColors( uint8_t colorIndex)
{
    uint8_t brightness = 255;
    
    for( int i = 0; i < NUM_LEDS; ++i) {
        leds[i] = ColorFromPalette( currentPalette, colorIndex, brightness, currentBlending);
        colorIndex += 3;
    }
}


// There are several different palettes of colors demonstrated here.
//
// FastLED provides several 'preset' palettes: RainbowColors_p, RainbowStripeColors_p,
// OceanColors_p, CloudColors_p, LavaColors_p, ForestColors_p, and PartyColors_p.
//
// Additionally, you can manually define your own color palettes, or you can write
// code that creates color palettes on the fly.  All are shown here.

void ChangePalettePeriodically()
{
    uint8_t secondHand = (millis() / 1000) % 60;
    static uint8_t lastSecond = 99;
    
    if( lastSecond != secondHand) {
        lastSecond = secondHand;
        if( secondHand ==  0)  { currentPalette = RainbowColors_p;         currentBlending = LINEARBLEND; }
        if( secondHand == 10)  { currentPalette = RainbowStripeColors_p;   currentBlending = NOBLEND;  }
        if( secondHand == 15)  { currentPalette = RainbowStripeColors_p;   currentBlending = LINEARBLEND; }
        if( secondHand == 20)  { SetupPurpleAndGreenPalette();             currentBlending = LINEARBLEND; }
        if( secondHand == 25)  { SetupTotallyRandomPalette();              currentBlending = LINEARBLEND; }
        if( secondHand == 30)  { SetupBlackAndWhiteStripedPalette();       currentBlending = NOBLEND; }
        if( secondHand == 35)  { SetupBlackAndWhiteStripedPalette();       currentBlending = LINEARBLEND; }
        if( secondHand == 40)  { currentPalette = CloudColors_p;           currentBlending = LINEARBLEND; }
        if( secondHand == 45)  { currentPalette = PartyColors_p;           currentBlending = LINEARBLEND; }
        if( secondHand == 50)  { currentPalette = myRedWhiteBluePalette_p; currentBlending = NOBLEND;  }
        if( secondHand == 55)  { currentPalette = myRedWhiteBluePalette_p; currentBlending = LINEARBLEND; }
    }
}

// This function fills the palette with totally random colors.
void SetupTotallyRandomPalette()
{
    for( int i = 0; i < 16; ++i) {
        currentPalette[i] = CHSV( random8(), 255, random8());
    }
}

// This function sets up a palette of black and white stripes,
// using code.  Since the palette is effectively an array of
// sixteen CRGB colors, the various fill_* functions can be used
// to set them up.
void SetupBlackAndWhiteStripedPalette()
{
    // 'black out' all 16 palette entries...
    fill_solid( currentPalette, 16, CRGB::Black);
    // and set every fourth one to white.
    currentPalette[0] = CRGB::White;
    currentPalette[4] = CRGB::White;
    currentPalette[8] = CRGB::White;
    currentPalette[12] = CRGB::White;
    
}

// This function sets up a palette of purple and green stripes.
void SetupPurpleAndGreenPalette()
{
    CRGB purple = CHSV( HUE_PURPLE, 255, 255);
    CRGB green  = CHSV( HUE_GREEN, 255, 255);
    CRGB black  = CRGB::Black;
    
    currentPalette = CRGBPalette16(
                                   green,  green,  black,  black,
                                   purple, purple, black,  black,
                                   green,  green,  black,  black,
                                   purple, purple, black,  black );
}


// This example shows how to set up a static color palette
// which is stored in PROGMEM (flash), which is almost always more
// plentiful than RAM.  A static PROGMEM palette like this
// takes up 64 bytes of flash.
const TProgmemPalette16 myRedWhiteBluePalette_p PROGMEM =
{
    CRGB::Red,
    CRGB::Gray, // 'white' is too bright compared to red and blue
    CRGB::Blue,
    CRGB::Black,
    
    CRGB::Red,
    CRGB::Gray,
    CRGB::Blue,
    CRGB::Black,
    
    CRGB::Red,
    CRGB::Red,
    CRGB::Gray,
    CRGB::Gray,
    CRGB::Blue,
    CRGB::Blue,
    CRGB::Black,
    CRGB::Black
};


// There are several different palettes of colors demonstrated here.
//
// FastLED provides several 'preset' palettes: RainbowColors_p, RainbowStripeColors_p,
// OceanColors_p, CloudColors_p, LavaColors_p, ForestColors_p, and PartyColors_p.
//
// Additionally, you can manually define your own color palettes, or you can write
// code that creates color palettes on the fly.  All are shown here.


//////////
// Helper functions
//////////


// Divide all of these by 4 to covert analogue 0-1023 -> 0-254
int getBrightness()
{ 
  //return (analogRead(BRIGHT_PIN)/4);
  return 255;
}

int getRed()
{
  return analogRead(RED_PIN)/4;
}
int getGreen()
{
  return analogRead(GREEN_PIN)/4;
}
int getBlue()
{
  return analogRead(BLUE_PIN)/4;
}
