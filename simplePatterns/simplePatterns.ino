#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif
#define LED_PIN 6
#define BUTTON 4


Adafruit_NeoPixel matrix = Adafruit_NeoPixel(64, LED_PIN, NEO_GRB + NEO_KHZ800);
int buttonState = 0;
int count = 0;
uint32_t off = matrix.Color(0 ,0, 0);
int maximum = 32;

void setup() {
  pinMode(BUTTON, INPUT_PULLUP);
  matrix.begin();
  matrix.show();
  randomSeed(analogRead(0));
}

void loop() {
  buttonState = digitalRead(BUTTON); 

  if(buttonState == LOW){
    count = count + 1; 
    if (count == maximum){
      // turn the matrix off
      count = 0;
      turn_all_off();
    }
    else{
      // turn on the number of lights acc. to count in a random colour
      uint32_t colour = get_random_colour();
      turn_all_off();
      for(int i = 0; i < count; i++){
        matrix.setPixelColor(random(64), colour);
      }
      matrix.show();
    }
  }
  delay(200);
}

void turn_all_off() {
  for (int i; i < matrix.numPixels(); i++){
    matrix.setPixelColor(i, off); 
  } 
  matrix.show();  
}

uint32_t get_random_colour() {
  return matrix.Color(random(256), random(256), random(256));
}
