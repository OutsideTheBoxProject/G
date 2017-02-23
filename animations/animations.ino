#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif
#define LED_PIN 6
// 0 -- nothing happens
// 1 -- house pattern
// 2 -- elevator
// 3 -- circle
#define PATTERN 2
// Delay in milliseconds
#define PAUSE 500
//stand still of last pattern in milliseconds
#define GREAT_PAUSE 500

Adafruit_NeoPixel matrix = Adafruit_NeoPixel(64, LED_PIN, NEO_GRB + NEO_KHZ800);
uint32_t off = matrix.Color(0 ,0, 0);


void setup() {
  // put your setup code here, to run once:
  matrix.begin();
  matrix.show();
  randomSeed(analogRead(0));
  Serial.begin(19200);
}

void loop() {
  switch(PATTERN){
    case 1: 
      play_house(); 
      break;
    case 2: 
      play_elevator();
      break;
    case 3: 
      play_circle();
      break;
    default: 
      delay(GREAT_PAUSE);
  }
  delay(GREAT_PAUSE);
}

void play_house(){
  // step 1
  uint32_t colour = get_random_colour();
  turn_all_off();
  set_line(0, colour);
  set_line(7, colour);
  set_square(13, 19, colour);
  set_square(27, 37, colour);
  delay(PAUSE);
  // step 2
  colour = get_random_colour();
  turn_all_off();
  set_line(0, colour);
  set_line(7, colour);
  set_horizontal_line(0, colour);
  set_square(51, 61, colour);
  set_square(53, 59, colour);
  delay(PAUSE);
  // step 3
  colour = get_random_colour();
  turn_all_off();
  set_line(0, colour);
  set_line(7, colour);
  set_horizontal_line(0, colour);
  set_square(10, 22, colour);
  set_square(34, 46, colour);
  delay(PAUSE); 
}

void play_elevator(){
  //step 1
  uint32_t colour = get_random_colour();
  uint32_t corncolour = get_random_colour();
  turn_all_off(); 
  set_corners(corncolour);
  delay(PAUSE);
  // step 2
  turn_all_off(); 
  set_corners(corncolour);
  set_lower_9(colour);
  delay(PAUSE);  
  // step 3
  colour = get_random_colour();
  turn_all_off(); 
  set_corners(corncolour);
  set_upper_9(colour);
  delay(PAUSE);
  // step 4
  turn_all_off(); 
  set_corners(corncolour);
  colour = get_random_colour(); 
  ride_elevator(colour);
  delay(PAUSE); 
}

void play_circle(){
  // step 1
  turn_all_off();
  big_circle(get_random_colour());  
  delay(PAUSE);
  //step 2 
  small_circle(get_random_colour());
  delay(PAUSE);
  // step 3 
  small_lines(get_random_colour());
  delay(PAUSE);
}

void big_circle(uint32_t colour){
  int circle[] = {3, 4, 14, 9, 31, 32, 24, 39, 49, 54, 59, 60};
  for(int i = 0; i < sizeof(circle); i++){
    matrix.setPixelColor(circle[i], colour);  
  }
  // wtf?
  matrix.setPixelColor(0, off);
  matrix.show();
}

void small_circle(uint32_t colour){
  int circle[] = {19, 20, 26, 29, 34, 37, 43, 44};
  for(int i = 0; i < sizeof(circle); i++){
    matrix.setPixelColor(circle[i], colour);  
  }
  matrix.show();
}

void small_lines(uint32_t colour){
  for(int i = 10; i <= 13; i++){
    matrix.setPixelColor(i, colour);  
  }
  for(int i = 50; i <= 53; i++){
    matrix.setPixelColor(i, colour);  
  }
  matrix.show();
}

void set_corners(uint32_t colour){
  int corners[] = {0, 1, 2, 15, 16, 31, 47, 63, 58, 57, 56, 55, 40, 39, 23, 7};
  for(int i = 0; i < sizeof(corners); i++){
    matrix.setPixelColor(corners[i], colour);  
  }
  matrix.show();
}

void set_lower_9(uint32_t colour){
  int second[] = {44, 43, 42, 51, 52, 53, 58, 59, 60};
  for(int i = 0; i < sizeof(second); i++){
    matrix.setPixelColor(second[i], colour);  
  }
  matrix.show();
}

void set_upper_9(uint32_t colour){
  int third[] = {19, 20, 21, 26, 27, 28, 35, 36, 37};  
  for(int i = 0; i < sizeof(third); i++){
    matrix.setPixelColor(third[i], colour);  
  }
  matrix.show(); 
}

void ride_elevator(uint32_t colour){
  matrix.setPixelColor(61, colour);
  matrix.setPixelColor(62, colour); 
  matrix.show(); 
  delay(PAUSE/5);
  matrix.setPixelColor(49, colour);
  matrix.setPixelColor(50, colour); 
  matrix.show(); 
  delay(PAUSE/5);
  matrix.setPixelColor(45, colour);
  matrix.setPixelColor(46, colour); 
  matrix.show(); 
  delay(PAUSE/5);
  matrix.setPixelColor(33, colour);
  matrix.setPixelColor(34, colour);  
  matrix.show();
  delay(PAUSE/5);
  matrix.setPixelColor(29, colour);
  matrix.setPixelColor(30, colour);  
  matrix.show();
  delay(PAUSE/5);
  matrix.setPixelColor(17, colour);
  matrix.setPixelColor(18, colour);  
  matrix.show();
  delay(PAUSE/5);
  matrix.setPixelColor(13, colour);
  matrix.setPixelColor(14, colour);  
  matrix.show();
  delay(PAUSE/5);
}

void set_square(int start, int finish, uint32_t colour){
  matrix.setPixelColor(start, colour);
  matrix.setPixelColor(start-1, colour);
  matrix.setPixelColor(finish-1, colour);
  matrix.setPixelColor(finish, colour);
  matrix.show();
}

void set_line(int start, uint32_t colour){
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

void set_horizontal_line(int start, uint32_t colour){
  int i = start; 
  while (i < (start+8)){
    matrix.setPixelColor(i, colour);
    i++;
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

