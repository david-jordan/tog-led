// LCD and debounced mode state button reader
// Drives mode on slave arduino running bigLed.ino
// Needed because the LCD display uses a lot of pins and has an interrupt handler for mode button changes

/* PINS TO CONNECT
 *  0 TX
 *  1 RX
 *  n/a LED power MOSFET (NOT on a pin: directly drive mosfet)
 *  2 Mode change button, with tie down resistor 10k: need pin 2 as it's an IRQ pin
 *  3 Free IRQ pin
 *  4 LCD enable brown
 *  5 LCD RS white
 *  6 LCD data 4 pin orange
 *  7 LCD data 5 pin yellow
 *  8 LCD data 6 pin green
 *  9 LCD data 7 pin  violet
 *  
 *  LCD r/w to gnd black  
 *  LCD VSS to gnd black
 *  LCD V0 to gnd black
 *  LCD K to gnd
 *  LCD VCC to +5v red
 *  LCD A to 3.3v grey
 *  
 *  10 driver data 0 orange
 *  11 driver data 1 yellow
 *  12 driver data 2 green
 */

// include the library code:
#include <LiquidCrystal.h>
int currentMode;

#define MODE_CHANGE_PIN 2
#define DUCK_PIN 3

#define LCD_D4 4
#define LCD_D5 5
#define LCD_D6 6
#define LCD_D7 7
#define LCD_RS 8
#define LCD_EN 9

#define LCD_COLS 16
#define LCD_ROWS 2

#define MODE_PIN0 10 // Low bit 1 orange
#define MODE_PIN1 11 // 2 yellow
#define MODE_PIN2 12 // High bit 4 green

// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to
// const int d4 = 5, d5 = 6, d6 = 7, d7 = 8, rs = 4, en = 3; // Move to define to save a few bytes
LiquidCrystal lcd(LCD_RS, LCD_EN, LCD_D4, LCD_D5, LCD_D6, LCD_D7);

#define DUCK_MODE_NUMBER 5

//String modeString[8];
const String modeString[8] ={"0 Off", "1 All", "2 Sweep", "3 Patleete", "4 Fire", "5 Duck", "6 nothing yet", "7 nothing yet"};
// Debouncing helper vars
unsigned long debounceDelay = 200;    // the debounce time; increase if the mode changes more than 1 place
static unsigned long last_interrupt_time = 0;

#define DEBUG true
void setup() {
  delay(100); // Post boot delay
  if(DEBUG){Serial.begin(9600);}
  
  lcd.begin(LCD_COLS, LCD_ROWS); // set up the LCD's number of columns and rows:
  lcd.noAutoscroll();
  lcd.clear();
  lcd.display(); // Turn on the display

  currentMode = 0;

  pinMode(MODE_PIN0, OUTPUT);
  pinMode(MODE_PIN1, OUTPUT);
  pinMode(MODE_PIN2, OUTPUT);
  
  //pinMode(ON_OFF_PIN, INPUT);
  // pinMode(RELAY_PIN, OUTPUT, INPUT_PULLUP);


  attachInterrupt(digitalPinToInterrupt(MODE_CHANGE_PIN), changeModeInterrupt, RISING);
  //attachInterrupt(digitalPinToInterrupt(DUCK_PIN), duckMode, RISING);
}

void loop() {
  //digitalWrite(ON_OFF_PIN, digitalRead(RELAY_PIN)); // Turn the mosfet on or off. No need to read from arduino
  setModeOutput();
  lcd.clear();
  lcd.print("Mode: "+modeString[currentMode]);

  if(DEBUG){Serial.print("Mode: "+modeString[currentMode]+" ");Serial.println(currentMode,DEC);}
  delay(500);
}


// Communicate the current mode state to the slave arduino
void setModeOutput()
{
  Serial.print("Bit MODE_PIN0: ");
  Serial.print(bitRead(currentMode,0));
  Serial.print("    Bit MODE_PIN1: ");
  Serial.print(bitRead(currentMode,1));
  Serial.print("   Bit MODE_PIN2: ");
  Serial.print(bitRead(currentMode,2));
    digitalWrite(MODE_PIN0, bitRead(currentMode,0));
    digitalWrite(MODE_PIN1, bitRead(currentMode,1));
    digitalWrite(MODE_PIN2, bitRead(currentMode,2));
}



void changeModeInterrupt()
{
  noInterrupts();
  if(DEBUG){Serial.println("Interrupt");}
  unsigned long interrupt_time = millis();
  // If interrupts come faster than 200ms, assume it's a bounce and ignore
  if (interrupt_time - last_interrupt_time > debounceDelay) 
  {
    currentMode++;
    if(currentMode>=8){currentMode=0;}
    if(DEBUG){Serial.print("New mode=");Serial.println(currentMode,DEC);}
  }
  last_interrupt_time = interrupt_time;
  interrupts();
}

void duckMode()
{
  if(DEBUG){Serial.println("Duck mode");}
  noInterrupts();
  if(DEBUG){Serial.println("Interrupt");}
  unsigned long interrupt_time = millis();
  // If interrupts come faster than 200ms, assume it's a bounce and ignore
  if (interrupt_time - last_interrupt_time > debounceDelay) 
  {
    currentMode = DUCK_MODE_NUMBER;
    if(DEBUG){Serial.print("New mode=");Serial.println(currentMode,DEC);}
  }
  last_interrupt_time = interrupt_time;
  interrupts();
}
