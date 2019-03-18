#define USB_RAWHID

#include <FastLED.h>
#include <Metro.h>
#include <i2c_t3.h>
#include "MCP23017.h"
#include "SimpleKalmanFilter.h"

#define NUM_LEDS 6
#define DATA_PIN_0 0
#define DATA_PIN_1 1
#define COLOR_ORDER GRB
#define LED_TYPE WS2812

#define TIMER_MS 5000

// zero-indexed button LED IDs
#define OP1_LED 5 
#define OP2_LED 4
#define OP3_LED 3
#define OP4_LED 0
#define OP5_LED 1
#define OP6_LED 2

// zero-indexed MCP buttons
#define OP1_BTN 0
#define OP2_BTN 1
#define OP3_BTN 2
#define OP4_BTN 3
#define OP5_BTN 4
#define OP6_BTN 5
#define INCR_BTN 6
#define DECR_BTN 7
#define MODE1_BTN 8
#define MODE2_BTN 9
#define MODE3_BTN 10

// DO LEDs (PWM)
#define RED_LAMP 3
#define GREEN_LAMP 4

// DI buttons
#define BLK1_BTN        5
#define BLK2_BTN        6
#define WHT1_BTN        7
#define WHT2_BTN        8
#define SINGLESWITCH    9
#define DUALSWITCH_UP   10
#define DUALSWITCH_DOWN 11
#define KEYSWITCH       12

//Analog ins
#define SLIDER1         A0
#define SLIDER2         A1

#define MCP_ADDRESS 0x20

const byte DIs[] = {
  BLK1_BTN, BLK2_BTN, WHT1_BTN, WHT2_BTN, SINGLESWITCH, DUALSWITCH_UP, DUALSWITCH_DOWN, KEYSWITCH
};

const byte Analogs[] = {
  SLIDER1, SLIDER2
};

const byte orderedLeds[] = {
  OP6_LED, OP5_LED, OP4_LED, OP3_LED, OP2_LED, OP1_LED // prototype order
  //OP1_LED, OP2_LED, OP3_LED, OP4_LED, OP5_LED, OP6_LED // pcb V2 order
};

CRGB rawLeds[6];
CRGBSet leds(rawLeds, 6);

CRGBSet op1Led(leds(OP1_LED, OP1_LED));
CRGBSet op2Led(leds(OP2_LED, OP2_LED));
CRGBSet op3Led(leds(OP3_LED, OP3_LED));
CRGBSet op4Led(leds(OP4_LED, OP4_LED));
CRGBSet op5Led(leds(OP5_LED, OP5_LED));
CRGBSet op6Led(leds(OP6_LED, OP6_LED));

CRGBSet topRow(leds(OP1_LED, OP3_LED));
CRGBSet botRow(leds(OP4_LED, OP6_LED));

CRGB rawInsideLeds[NUM_LEDS];

MCP23017 mcp;
Metro debugMetro = Metro(500);
Metro stickMetro = Metro(5);
Metro panelMetro = Metro(20);

SimpleKalmanFilter a0Filter(2, 2, 0.01);
SimpleKalmanFilter a1Filter(2, 2, 0.01);

unsigned long last_change = 0;
unsigned long now = 0;
uint16_t allButtons;
int angle = 0;

void setup() {
  mcp.begin(MCP_ADDRESS);

  FastLED.addLeds<LED_TYPE,DATA_PIN_0,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);

  analogReadRes(13);

  pinMode(RED_LAMP, OUTPUT);
  pinMode(GREEN_LAMP, OUTPUT);

  for (int i = 0; i < sizeof(DIs); i++) {
    pinMode(DIs[i], INPUT_PULLUP);
  }

  Joystick.useManualSend(true);
}

void loop() {
  doLed();
  
  if (debugMetro.check() == 1) doDebug();
  
  if (stickMetro.check() == 1) doStick();
  
  if (panelMetro.check() == 1) doPanel();
}

// Panel stuff
int greenBrightness = 0;
int greenFadeAmount = 25;

int redBrightness = 0;
int redFadeAmount = 1;

void doPanel() {
	greenBrightness = greenBrightness + greenFadeAmount;
	redBrightness = redBrightness + redFadeAmount;

    if (greenBrightness <= 0 || greenBrightness >= 255) {
      greenFadeAmount = -greenFadeAmount;
    }
	if (redBrightness <= 0 || redBrightness >= 255) {
      redFadeAmount = -redFadeAmount;
    }
	
	// if keyswitch OFF
	if (!digitalRead(KEYSWITCH)) {
		analogWrite(RED_LAMP, 0);
		analogWrite(GREEN_LAMP, greenBrightness); 
	} else {
		analogWrite(RED_LAMP, redBrightness);
		analogWrite(GREEN_LAMP, 0); 
	}
}

// Joystick stuff

void doStick() {
  // Analogs
  Joystick.X(getAnalog(SLIDER1));
  Joystick.Y(getAnalog(SLIDER2));
  Joystick.Z(0);
  Joystick.Zrotate(0);
  Joystick.sliderLeft(0);
  Joystick.sliderRight(0);
  
//  POV Hat
//  angle = angle + 1;
//  if (angle >= 360) angle = 0;
//  Joystick.hat(angle);

  int btnNumber = 0;

  // MCP buttons  
  allButtons = mcp.readGPIOAB();
  for(int i = 0; i < 11; i++) {  
    btnNumber++;
    Joystick.button(i + 1, getMcpButton(i));
  }

  // DI buttons
  Joystick.button(12, getDIButton(0));
  Joystick.button(13, getDIButton(1));
  Joystick.button(14, getDIButton(2));
  Joystick.button(15, getDIButton(3));
  Joystick.button(16, getDIButton(4));
  Joystick.button(17, getDIButton(5));
  Joystick.button(18, getDIButton(6));
  Joystick.button(19, getDIButton(7));

  Joystick.send_now();
}

bool getMcpButton(int butNum) {
  return bitRead(allButtons, butNum);
}

bool getDIButton(int butNum) {
  return !digitalRead(DIs[butNum]);
}

bool getRawDIButton(int butNum) {
  return !digitalRead(butNum);
}

uint16_t getAnalog(int anaNum) {
  uint16_t raw = analogRead(anaNum);
  
  if (anaNum == A0) {
    raw = a0Filter.updateEstimate(raw);
    raw = map(raw, 8192, 0, 0, 1023);
  } else if (anaNum == A1) {
    raw = a1Filter.updateEstimate(raw);
    raw = map(raw, 8192, 0, 0, 1023);
  } else {
    raw = 0;
  }
  return raw;
}

void doDebug() {
  Serial.print("PORTA: ");
  printBits((byte)(allButtons & 0xFF));
  Serial.print(", PORTB: ");  
  printBits((byte)(allButtons >> 8));
  Serial.print(", PORTAB: ");
  printBits(allButtons);
  Serial.print(", A0: ");
  Serial.print(bitRead(allButtons, 0));
  Serial.print(", B0: ");
  Serial.print(bitRead(allButtons, 8));
  Serial.println();

  Serial.print("SLIDER1: ");
  Serial.print(getAnalog(SLIDER1));
  Serial.print("SLIDER2: ");
  Serial.print(getAnalog(SLIDER2));
  Serial.println();
}

// WS2812 LED Stuff

uint32_t color;
uint32_t butColors[6];
bool pixelColor = false;

uint8_t gHue = 0; // rotating "base color" used by many of the patterns

CRGB intakeColor = CRGB::Cyan;
CRGB cargoShipColor = CRGB::Magenta;
CRGB rocketHatchColor = CRGB::BurlyWood;
CRGB rocketCargoColor = CRGB::DarkGoldenrod;

int fadeBrightness = 0;
int fadeAmount = 10;

void doLed() {
  // put your main code here, to run repeatedly:
  now = millis();

  // reset button colors
  memset(butColors, 0, sizeof(butColors));
//  for (int i = 0; i < 6; i++) {
//    butColors[i] = 0x000000;
//  }
  
  // run main pattern here  
  op1Led.fill_rainbow(gHue + 20);
  op2Led.fill_rainbow(gHue + 40);
  op3Led.fill_rainbow(gHue + 60);
  botRow.fill_solid(CRGB(255, 0, 0));

  if (getMcpButton(MODE1_BTN)) { // cargo ship
    leds.fill_solid(cargoShipColor);
    topRow.fill_solid(CRGB::Green);
  } else if (getMcpButton(MODE2_BTN)) { // rocket hatch
    leds.fill_solid(rocketHatchColor);
    topRow.fill_solid(CRGB::Green);
  } else if (getMcpButton(MODE3_BTN)) { // rocket cargo
    leds.fill_solid(rocketCargoColor);
    topRow.fill_solid(CRGB::Green);
  } else {
    botRow.fill_solid(intakeColor);
  }

  if (getMcpButton(OP1_BTN)) {
    breathe(op1Led);
    breathe(op4Led);
  }
  if (getMcpButton(OP2_BTN)) {
    breathe(op2Led);
    breathe(op5Led);
  }
  if (getMcpButton(OP3_BTN)) {
    breathe(op3Led);
    breathe(op6Led);
  }
  
  if (getMcpButton(OP4_BTN)) {
    breathe(op4Led);
  }
  if (getMcpButton(OP5_BTN)) {
    breathe(op5Led);
  }
  if (getMcpButton(OP6_BTN)) {
    breathe(op6Led);
  }

  FastLED.show();  
  // insert a delay to keep the framerate modest
  FastLED.delay(1000/100); 

  EVERY_N_MILLISECONDS( 5 ) { gHue++; } // slowly cycle the "base color" through the rainbow
}

void breathe(CRGBSet ledSet) {
  double ms = 190.0;
  breathe(ledSet, ms);
}

void breathe(CRGBSet ledSet, double ms) {
  float breath = (exp(sin(millis()/ms*PI)) - 0.36787944)*108.0;
  ledSet[0].fadeToBlackBy(breath);
}

void printBits(uint16_t x) {
  for (byte i = 0; i < 16; i++) {
    Serial.print(bitRead(x, i));
  }
}

void printBits(byte x){
 for (byte i = 0; i < 8; i++) {
  Serial.print(bitRead(x, i));
 }
}
