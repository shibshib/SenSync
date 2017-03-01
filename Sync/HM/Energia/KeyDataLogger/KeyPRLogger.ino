/*  PS2Keyboard library example
  
  PS2Keyboard now requries both pins specified for begin()

  keyboard.begin(data_pin, irq_pin);
  
  Valid irq pins:
     Arduino Uno:  2, 3
     Arduino Due:  All pins, except 13 (LED)
     Arduino Mega: 2, 3, 18, 19, 20, 21
     Teensy 2.0:   All pins, except 13 (LED)
     Teensy 2.0:   5, 6, 7, 8
     Teensy 1.0:   0, 1, 2, 3, 4, 6, 7, 16
     Teensy++ 2.0: 0, 1, 2, 3, 18, 19, 36, 37
     Teensy++ 1.0: 0, 1, 2, 3, 18, 19, 36, 37
     Sanguino:     2, 10, 11
  
  for more information you can read the original wiki in arduino.cc
  at http://www.arduino.cc/playground/Main/PS2Keyboard
  or http://www.pjrc.com/teensy/td_libs_PS2Keyboard.html
  
  Like the Original library and example this is under LGPL license.
  
  Modified by Cuninganreset@gmail.com on 2010-03-22
  Modified by Paul Stoffregen <paul@pjrc.com> June 2010
*/

#include <PS2Keyboard.h>

const int DataPin = P1_5;
const int IRQpin =  P1_4;
const int delayTime = 10; // Delay so we can hit close to 100 Hz output rate

bool pressed = false;
int counter = 0;

PS2Keyboard keyboard;

byte readkey()
{
  byte _start = 0;
  byte buf = 0;
  byte _parity = 0;
  byte _stop = 0;
  
  waitClockLow();
    
  _start = digitalRead(DataPin);

  if (_start == 0)
  {
    waitClockLow();
    for (byte c=0; c<8; c++)
    {
      buf = buf | (digitalRead(DataPin) << c);
      waitClockLow();
    }
  
    _parity = digitalRead(DataPin);
    waitClockLow();
  
    _stop = digitalRead(DataPin);
  }
  return buf;
}

void waitClockLow()
{
  if (digitalRead(IRQpin) == LOW)           // If clock is already low, wait for it to change
    while (digitalRead(IRQpin) == LOW);
  while (digitalRead(IRQpin) == HIGH);
}

void setup() {
  delay(500);

// ONLY uncomment this if keyboard.begin is commented. These have conflicting calls and cannot be used in tandem
/*  pinMode(IRQpin, INPUT);
  pinMode(DataPin, INPUT);
  digitalWrite(IRQpin, HIGH);
  digitalWrite(DataPin, HIGH);
  */
  Serial.begin(256000);
  keyboard.begin(DataPin, IRQpin);
  Serial.println("Keyboard Test:");
}

void loop() {
delay(delayTime);    

if (keyboard.available()) {
    
    // read the next key
    char c = keyboard.read();
    Serial.println(2);         
   } else {
    Serial.println(1);
   }

 // Key press/release code
    /*
    byte b = readkey(); 
    char c;

    if (b == 0xF0)
    {
      // key released
        Serial.println("R");
        b = readkey(); 
    }
    else
    {
      Serial.println("P");
      
    } */
}
