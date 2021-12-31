// NOTE FOR ESP32: Pins 34, 35, 36, 37, 38 and 39 do not have internal pull-up or pull-down resistors
// 26 = pause
// 27 = reset
// 14 = start

#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <time.h>

#define BTN_PAUSE 26 //11
#define BTN_RESET 27 //10
#define BTN_START 14 //8  

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C /// both are 0x3C don't listen to the lies
Adafruit_SSD1306 LED(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

boolean debug = true;

unsigned long btn_debounce_time = 250; // milliseconds for button debouncing
unsigned long btn_last_pressed_time = 0;

unsigned long debug_printer = 300;
unsigned long last_debug_print = 0;

unsigned long countdown_timeout = 1000;
unsigned long last_countdown_time = 0;

int minutes = 4;
int seconds = 0;

// track the state of buttons to check for transitions
// true = NOT PRESSED
boolean PAUSE_STATE_CURRENT  = true;
boolean RESET_STATE_CURRENT  = true;
boolean START_STATE_CURRENT  = true;
boolean PAUSE_STATE_PREVIOUS = true;
boolean RESET_STATE_PREVIOUS = true;
boolean START_STATE_PREVIOUS = true;

boolean paused = false;
boolean stopped = false;
boolean clock_running = false;
boolean time_exceeded = false;

void initClock()
{
  LED.clearDisplay();
  LED.setTextColor(SSD1306_WHITE);
  LED.setTextSize(4);
  LED.setCursor(0, 20);
  LED.print(minutes); LED.print(":"); 
  if(seconds < 10)
  {
    LED.print("0");
    LED.println(seconds);
  }
  else
  {
    LED.println(seconds);
  }
  
  LED.display();
}

void setup() 
{
  // all buttons are input pullup, so HIGH = UN-PRESSED
  pinMode(BTN_PAUSE, INPUT_PULLUP);
  pinMode(BTN_RESET, INPUT_PULLUP);
  pinMode(BTN_START, INPUT_PULLUP);
  
  Serial.begin(115200);

  if(debug) Serial.println("Allocating SSD1306...");
  if(!LED.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) 
  {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  } 

  delay(100);

  Serial.print("INITIAL PAUSE STATE: "); Serial.print(digitalRead(BTN_PAUSE));
  Serial.print(" || RESET STATE: ");     Serial.print(digitalRead(BTN_RESET));
  Serial.print(" || START STATE: ");     Serial.print(digitalRead(BTN_START));
  Serial.println();

  initClock();
}

void updateClock()
{
  if(time_exceeded == false)
  {
    if(minutes >= 0)
    {
      if(seconds >= 0)
      {
        seconds = seconds-1;

        if(seconds < 0)
        {
          seconds = 59;
          minutes = minutes - 1;

          if(minutes < 0)
          {
            time_exceeded = true;
            clock_running = false;
            minutes = 0;
            seconds = 0;
          }
        }
      }
    }
    else
    {
      time_exceeded = true;
      clock_running = false;
      minutes = 0;
      seconds = 0;
    }
  }
  
  LED.clearDisplay();
  LED.setTextColor(SSD1306_WHITE);
  LED.setTextSize(4);
  LED.setCursor(0, 20);
  LED.print(minutes); LED.print(":"); 
  if(seconds < 10)
  {
    LED.print("0");
    LED.println(seconds);
  }
  else
  {
    LED.println(seconds);
  }
  
  LED.display();
}


void loop() 
{
  PAUSE_STATE_CURRENT = digitalRead(BTN_PAUSE);
  RESET_STATE_CURRENT = digitalRead(BTN_RESET);
  START_STATE_CURRENT = digitalRead(BTN_START);
  
  // check if there has been a button state transition, and that the final
  // state of the button is FALSE (aka: pressed)
  if((PAUSE_STATE_CURRENT != PAUSE_STATE_PREVIOUS) && (millis() - btn_last_pressed_time > btn_debounce_time))
  {
    if(!clock_running)
    {
      clock_running = true;
    }
    else
    {
      clock_running = false;
    }

    btn_last_pressed_time = millis();
  }
  
  if((RESET_STATE_CURRENT != RESET_STATE_PREVIOUS) && (millis() - btn_last_pressed_time > btn_debounce_time))
  {
    minutes = 4;
    seconds = 0;
    time_exceeded = false;
    clock_running = false;
    initClock();
    btn_last_pressed_time = millis();
  }
  
  if((START_STATE_CURRENT != START_STATE_PREVIOUS) && (millis() - btn_last_pressed_time > btn_debounce_time))
  {
    minutes += 1;
    initClock();
    btn_last_pressed_time = millis();
  }

  if(millis() - last_debug_print > debug_printer && debug)
  {
    Serial.print("CURRENT PAUSE STATE: "); Serial.print(PAUSE_STATE_CURRENT);
    Serial.print(" || RESET STATE: ");     Serial.print(RESET_STATE_CURRENT);
    Serial.print(" || START STATE: ");     Serial.print(START_STATE_CURRENT);
    Serial.println();

    Serial.print("PREV    PAUSE STATE: "); Serial.print(PAUSE_STATE_PREVIOUS);
    Serial.print(" || RESET STATE: ");     Serial.print(RESET_STATE_PREVIOUS);
    Serial.print(" || START STATE: ");     Serial.print(START_STATE_PREVIOUS);
    Serial.println();

    Serial.print("** clock_running = "); Serial.print(clock_running);
    Serial.print(" || time_exceeded = "); Serial.println(time_exceeded);
    
    last_debug_print = millis();
  }

  if((millis() - last_countdown_time > countdown_timeout) && clock_running == true)
  {
    updateClock();
    last_countdown_time = millis();
  }

  PAUSE_STATE_PREVIOUS = PAUSE_STATE_CURRENT;
  RESET_STATE_PREVIOUS = RESET_STATE_CURRENT;
  START_STATE_PREVIOUS = START_STATE_CURRENT;
}