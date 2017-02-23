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
  //pinMode(BUTTON, INPUT_PULLUP);
  matrix.begin();
  matrix.show();
  randomSeed(analogRead(0));
  Serial.begin(19200);
}

void loop() {
  for(int i = 0; i < 8; i++){
    set_line(i);  
    delay(2000);
  }
  delay(2000);
}

void set_line(int start){
  uint32_t colour = get_random_colour();
  turn_all_off();
  delay(1000);
  int distance = 1+start*2; 
  int minimum = distance;
  int i = start;
  while (i < 64){
     matrix.setPixelColor(i, colour);
     if (distance == minimum){
        distance = 15-(start*2);
      }
      else{
        distance = 1+(start*2);  
      }
      i = i + distance; 
  }
  matrix.show();
}


void turn_all_off() {
  for (int i = 0; i < matrix.numPixels(); i++){
    matrix.setPixelColor(i, off); 
  } 
  matrix.show();  
}

uint32_t get_random_colour() {
  return matrix.Color(random(256), random(256), random(256));
}
