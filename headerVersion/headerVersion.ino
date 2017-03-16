/*********
  First Testrun for G's lighttable a.k.a. Öxe
  Created 12-Nov-2016
  F.G.

**********/
#include <Adafruit_NeoPixel.h>
#include <Adafruit_GFX.h>
#include <Adafruit_NeoMatrix.h>
#include <Wire.h>
#include "Adafruit_MPR121.h"
#ifdef __AVR__
#include <avr/power.h>
#include <Wiegand.h>
#include "data.h"

#endif
#define NEO_PIN 6
#define MAX_PRIMITIVES 18
#define MAX_X 48 // no more than 48 X and Y coordinate pairs == 48 pixels max
#define MAX_Y 48



// Helper Class for keeping track of objects / drawing primitives
// There can be max. 12 primitives
// ==============================================================

//int maxObjects = 12;

class DrawingObject
{
    int id;
    // boolean doesExist;
    int nElements = 0;
    int x[MAX_X];  // entweder geht der speicher zu neige, oder ich verletzte irgendwo den speicherbereich
    int y[MAX_Y];
    int col[3] = {255, 255, 255};

    // Constructor - creates a Flasher
    // and initializes the member variables and state
  public:
    DrawingObject() {}

    DrawingObject(int id_, int n, int *x_, int *y_, int r_, int g_, int b_) {
      id = id_;
      col[0] = r_;
      col[1] = g_;
      col[2] = b_;
      nElements = n;
      //memcpy(&x, &x_, n);
      //memcpy(&y, &x_, n);
      for (int i = 0; i < n; i++) {
        x[i] = *(x_ + i);
        y[i] = *(y_ + i);
      }
      //doesExist = true;
    }

    int getN() {
      return nElements;
    }

    int *getX_() {
      return x;
    }

    int *getY_() {
      return y;
    }

    int *getColors() {
      return col;
    }

    void updateCoordinates(int *x_, int *y_, int r_, int g_, int b_) {
      col[0] = r_;
      col[1] = g_;
      col[2] = b_;
      for (int i = 0; i < nElements; i++) {
        x[i] = *(x_ + i);
        //Serial.println(x[i]);
        y[i] = *(y_ + i);
      }
    }


    void updateCoordinates(int n, int *x_, int *y_, int r_, int g_, int b_) {
      col[0] = r_;
      col[1] = g_;
      col[2] = b_;
      nElements = n;
      //Serial.println(n);
      for (int i = 0; i < nElements; i++) {
        x[i] = *(x_ + i);
        //Serial.println(x[i]);
        y[i] = *(y_ + i);
      }
    }

    void updateCoordinates(int *x_, int *y_) {
      //Serial.println(n);
      for (int i = 0; i < nElements; i++) {
        x[i] = *(x_ + i);
        //Serial.println(x[i]);
        y[i] = *(y_ + i);
      }
    }


    int setColor(int r, int g, int b) {
      col[0] = r; col[1] = g; col[2] = b;
    }
    /*
        boolean getStat() {
          return doesExist;
        }

        void setStatus(boolean b) {
          doesExist = b;
        }
    */
};
// ===================================================


WIEGAND wg;
long lastRFID = 0;

DrawingObject allObjects[MAX_PRIMITIVES];

int objectCounter = 0; // max. 8 objects supported currently

// Our brand new 16 x 16 Matrix.
Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(16, 16, NEO_PIN,
                            NEO_MATRIX_TOP     + NEO_MATRIX_RIGHT +
                            NEO_MATRIX_COLUMNS + NEO_MATRIX_ZIGZAG,
                            NEO_GRB            + NEO_KHZ800);

const uint16_t colors[] = {
  matrix.Color(255, 0, 0), matrix.Color(0, 255, 0), matrix.Color(0, 0, 255)
};


int matrixBrightness = 100;


long lastPaused = 0;
boolean lastCleared = false;

// 0 == draw circles   1 == draw lines   2 == draw rectangles 3 == draw house and tree
int animationSelection = 3;
int animationPart = 0; // if animationSelection == house and tree pattern etc - which part of the animation is it?

// 0 == PICKANDPLACE    1 == BLINKiNG   2 == SPEEDING   3 == PURE ENTERTAINMENT
int mode = 3;

// 0 == red   1 == green   2 == blue
int colorSelection = 0;
boolean funkyMode = false;
int R_ = 0;
int G_ = 0;
int B_ = 0;
int lastColor = 0;

// 0 == low  1 == bright   2 == extra bright
int brightnessSelection = 2;

// Handling buttons and poti, debouncing
boolean currentPinState = HIGH;
long lastReading7 = 0;
long lastReading8 = 0;
long lastReading9 = 0;
long lastReading10 = 0;
long lastReading11 = 0;

Adafruit_MPR121 cap = Adafruit_MPR121();
// Keeps track of the last pins touched
// so we know when buttons are 'released'
uint16_t lasttouched = 0;
uint16_t currtouched = 0;

long lastInput = 0;
long lastInputRFID = 0;


// handling pauses
long lastTimeStamp = 0;

// for animation movement mode
int direction_ = 1;

boolean isPaused = false;


int co = 0; // just a helper variable for pure entertainment mode

// S E T U P
// ---------
// ---------
void setup() {
  //while (!Serial);
  Serial.begin(9600);
  Serial.println("firing up sketch");

  if (!cap.begin(0x5A)) {
    Serial.println("MPR121 not found, check wiring?");
    while (1);
  }
  Serial.println("MPR121 found!");

  // for the buttons and potis ...
  pinMode(7, INPUT);  // animationSelection 7 pattern pin
  digitalWrite(7, HIGH);
  pinMode(8, INPUT);  // clearscreen pin
  digitalWrite(8, HIGH);
  pinMode(9, INPUT);
  digitalWrite(9, HIGH); // undo last step pin

  pinMode(A0, INPUT);
  pinMode(A1, INPUT);
  pinMode(A2, INPUT);
  pinMode(A3, INPUT);

  // feedback LEDs
  pinMode(13, OUTPUT);
  digitalWrite(13, HIGH);
  pinMode(12, OUTPUT);
  digitalWrite(12, HIGH);
  pinMode(11, OUTPUT);
  digitalWrite(11, HIGH);
  pinMode(10, OUTPUT);
  digitalWrite(10, HIGH);
  delay(1000);
  digitalWrite(13, LOW);
  digitalWrite(12, LOW);
  digitalWrite(11, LOW);
  digitalWrite(10, LOW);


  matrix.begin();
  matrix.setBrightness(matrixBrightness);
  matrix.fillScreen(0);


  // LEONARDO: RFID Reader stuff. Connect to Pin0 and Pin1. Will be mapped to interrupt 2,3. Neccessary because i2c bus occupies standard interrupts
  //wg.begin(0, 2, 1, 3);
  wg.begin(); //ADK

  lastTimeStamp = millis();
}


void loop() {
  handleFeedbackLEDs();
  handleFeedbackBuzz();


  if ( wg.available() &&  (abs(millis() - lastRFID) > 1000) ) {
    lastRFID = millis();
    Serial.println(wg.getCode());
    lastInput = millis();
    lastInputRFID = millis();
    insertObject(wg.getCode(), 0);
  }

  // taking care of capacitive touch sensors
  // ---------------------------------------
  // Get the currently touched pads
  currtouched = cap.touched();
  for (uint8_t i = 0; i < 12; i++) {
    // it if *is* touched and *wasnt* touched before, alert!
    if ((currtouched & _BV(i)) && !(lasttouched & _BV(i)) ) {
      //Serial.print(i); Serial.println(" touched");
    }
    // if it *was* touched and now *isnt*, alert!
    if (!(currtouched & _BV(i)) && (lasttouched & _BV(i)) ) {
      Serial.print(i); Serial.println(" triggered!");
      lastInput = millis();

      if (objectCounter > 0) {
        //lastPaused = 0;
        if (i == 3) {// Move LEFT
          manipulateObject(&allObjects[objectCounter - 1], 1, false);
          mode = 0;
        }
        if (i == 5) {// Move RIGHT
          manipulateObject(&allObjects[objectCounter - 1], 2, false);
          mode = 0;
        }
        if (i == 6) {// Move TOP
          manipulateObject(&allObjects[objectCounter - 1], 4, false);
          mode = 0;
        }
        if (i == 4) {// Move DOWN
          manipulateObject(&allObjects[objectCounter - 1], 3, false);
          mode = 0;
        }
        //if (i == 11) {// Move DOWN
        if (i == 0) {// Move DOWN
          mode = 1;
          setDirection(3);
        }
        if (i == 1) {// Move UP
          mode = 1;
          setDirection(4);
        }
        if (i == 2) {// Move LEFT
          mode = 1;
          setDirection(1);
        }
        if (i == 7) {// Move RIGHT
          mode = 1;
          setDirection(2);
        }

      }
    }
  }
  // reset our state
  lasttouched = currtouched;

  currentPinState = digitalRead(7);
  if ((currentPinState == LOW) && abs(millis() - lastReading7) > 500) {
    lastReading7 = millis();
    lastInput = millis();
    Serial.println("touched button 7: undo");
    lastPaused = 0;
    if (objectCounter > 0)
      objectCounter--;
  }
  currentPinState = digitalRead(8);
  if ((currentPinState == LOW) && abs(millis() - lastReading8) > 500) {
    lastReading8 = millis();
    lastInput = millis();
    Serial.println("touched button 8: change mode to pure entertainment");
    mode = 3;
  }
  currentPinState = digitalRead(9);
  if ((currentPinState == LOW) && abs(millis() - lastReading9) > 500) {
    lastReading9 = millis();
    lastInput = millis();
    Serial.println("touched button 9: clearscreen");
    lastPaused = 0;
    mode = 0; // clear screen and 'pause'
    objectCounter = 0;
    matrix.fillScreen(0);
    matrix.show();
  }

  // taking care of pauses/ the speed
  int threshold = map(analogRead(A3), 0, 1023, 0, 1000);
  if (threshold > 9500) {
    //Serial.println("paused, that is, back to PICKANDPLACE-MODE");
    //mode = 0;
  }

  // taking care of the color mixer
  R_ = map(analogRead(A1), 0, 1023, 0, 255);
  G_ = map(analogRead(A2), 0, 1023, 0, 255);
  B_ = map(analogRead(A0), 0, 1023, 0, 255);
  lastColor = R_ + G_ + B_;
  switch (mode) {

    case 0:
      renderCanvas(0); // PICKANDPLACE-MODE
      break;
    case 1:
      renderCanvasMovement(threshold);
      break;
    case 2:
      renderCanvasBlinking(threshold);
      break;
    case 3:
      renderPureEntertainment(threshold);
      break;
  }

  delay(50); // needed so that the RFID reader can be processed
} // end of loop
// ===============================================================================




// helper functions
// ----------------


void handleFeedbackLEDs() {
  if ( abs(millis() - lastInput) > 300) {
    digitalWrite(13, LOW);
    //digitalWrite(12, LOW);
    digitalWrite(11, LOW);
    digitalWrite(10, LOW);
  } else {
    digitalWrite(13, HIGH);
    //digitalWrite(12, HIGH);
    digitalWrite(11, HIGH);
    digitalWrite(10, HIGH);
  }
}


void handleFeedbackBuzz() {
  if ( abs(millis() - lastInputRFID) > 300) {
    digitalWrite(12, LOW);
  } else {
    digitalWrite(12, HIGH);
  }
}


// renders all objects that we have. Called once per frame
// cannot be set to pause or speed up, because it is for ***PICKANDPLACE-MODE*** only
void renderCanvas(int mode_) {
  matrix.fillScreen(0); // best time for clearscreen?

  // Update colors first
  R_ = map(analogRead(A1), 0, 1023, 0, 255);
  G_ = map(analogRead(A2), 0, 1023, 0, 255);
  B_ = map(analogRead(A0), 0, 1023, 0, 255);
  DrawingObject *tmpObj = NULL;

  tmpObj = &allObjects[objectCounter - 1];

  int tmp = R_ + G_ + B_;
  if (objectCounter > 0 && (abs(lastColor - tmp) >= 2)) { // this is a quick and dirty hack and could be done with more care, but I dont think it is worth the effort
    tmpObj->setColor(R_, G_, B_);
  }

  int n_ = 0;
  int *x_p = NULL;
  int *y_p = NULL;
  int *c_p = NULL;
  for (int i = 0; i < objectCounter; i++) {

    n_ = allObjects[i].getN();
    x_p = allObjects[i].getX_();
    y_p = allObjects[i].getY_();
    c_p = allObjects[i].getColors();

    for (int j = 0; j < n_; j++) {
      matrix.drawPixel(*(y_p + j), *(x_p + j), matrix.Color(*(c_p + 0), *(c_p + 1), *(c_p + 2)));
      //Serial.println(*(y_p + j));
    }
  }

  matrix.show();
}


// Let the screen be blinking in animation mode
// ANIMATION-MODE: similiar to renderCanvas(), however, blinking and movement supported and primary goal
void renderCanvasBlinking(long pause) {
  //Serial.println("render blinking");


  if (abs(millis() - lastPaused) > pause) {
    lastPaused = millis();
    if (pause == 1000) { // if poti is closed, freeze image
      lastCleared = false;
    }
    if (lastCleared == true) {
      lastCleared = false;
    } else {
      lastCleared = true;
    }
    if (lastCleared == false) {
      matrix.fillScreen(0);
    } else {

      int n_ = 0;
      int *x_p = NULL;
      int *y_p = NULL;
      int *c_p = NULL;
      // render all existing objects
      for (int i = 0; i < objectCounter; i++) {
        n_ = allObjects[i].getN();
        x_p = allObjects[i].getX_();
        y_p = allObjects[i].getY_();
        c_p = allObjects[i].getColors();
        for (int j = 0; j < n_; j++) {
          matrix.drawPixel(*(y_p + j), *(x_p + j), matrix.Color(*(c_p + 0), *(c_p + 1), *(c_p + 2)));
          //Serial.println(*(y_p + j));
        }
      }
    }
  }
  matrix.show();
}



// Generate primitives-moving-effect (whole canvas)
// ANIMATION-MODE: similiar to renderCanvas(), however, movement supported and primary goal
void renderCanvasMovement(long pause) {

  if (abs(millis() - lastPaused) > pause) {
    matrix.fillScreen(0);
    lastPaused = millis();

    int n_ = 0;
    int *x_p = NULL;
    int *y_p = NULL;
    int *c_p = NULL;

    // move and render all existing objects
    for (int i = 0; i < objectCounter; i++) {

      // move em first
      manipulateObject(&allObjects[i], direction_, true);

      n_ = allObjects[i].getN();
      x_p = allObjects[i].getX_();
      y_p = allObjects[i].getY_();
      c_p = allObjects[i].getColors();
      for (int j = 0; j < n_; j++) {
        matrix.drawPixel(*(y_p + j), *(x_p + j), matrix.Color(*(c_p + 0), *(c_p + 1), *(c_p + 2)));
        //Serial.println(*(y_p + j));
      }
    }

  }
  matrix.show();
}

void renderPureEntertainment(long pause) {
  Serial.println(abs(millis() - lastPaused));

  if (abs(millis() - lastPaused) > pause) {

    lastPaused = millis();
    Serial.println("now");


    /*    co++;
        //matrix.fillScreen(0);
        lastPaused = millis();
        int r = random(3);
        switch (r) {
          case 0:
            matrix.drawCircle(random(16), random(16), random(6), matrix.Color(R_, G_, B_));
            break;
          case 1:
            matrix.fillRect(random(16), random(16), random(2), random(2), matrix.Color(R_, G_, B_));
            break;
          case 2:
            matrix.drawLine(random(16), random(16), random(16), random(16), matrix.Color(R_, G_, B_));
            break;
        }
      }
      if (co>20) {
        matrix.fillScreen(0);
        co = 0;
       }
      matrix.show();    */



    unsigned long ids[] = {10975563, 9572595, 6169245, 6043609, 13552585, 239135, 13435006, 3442143, 13413369, 5647386, 10835330, 6128352, 4631776, 9473543, 781403, 11071908, 4582666, 11073112, 234888, 795045, 13387277, 11000617, 10198496, 11072865, 4767054};
    unsigned long r = ids[random(26)]; // quick and dirty hardcoding number of objects
    //Serial.println(r);
    insertObject(r, 3);
    int r2 = random(1, 4);
    for (int i = 0; i < random(20); i++) {
      manipulateObject(&allObjects[objectCounter - 1], r2, false);
    }



    renderCanvas(3);

  }
}


// if the user wants to go into the opposite direction, set to blinking mode first
int setDirection(int t) {
  switch (t) {
    case 1:
      if (direction_ == 2) {
        Serial.println("mode2");
        direction_ = 0;
        mode = 2; // activate blinking
      } else  {
        direction_ = 1; // LEFT
        Serial.println("go left");
      }
      break;
    case 2:
      if (direction_ == 1) {
        direction_ = 0;
        mode = 2; // activate blinking
      } else
        direction_ = 2; // RIGHT
      break;
    case 3:
      if (direction_ == 4) {
        direction_ = 0;
        mode = 2; // activate blinking
      } else
        direction_ = 3; // UP
      break;
    case 4:
      if (direction_ == 3) {
        direction_ = 0;
        mode = 2; // activate blinking
      } else
        direction_ = 4; // DOWN
      break;
  }
}


// Manipulate primitive/object. 4 directions are currently supported
// 1 == LEFT    2 == RIGHT    3 == UP   4 == DOWN
void manipulateObject(DrawingObject * o, int mode, boolean ignoreColor) {
  //Serial.println("manipulating");
  int xfactor = 0;
  int yfactor = 0;
  switch (mode) {
    case 1:
      xfactor = -1;
      yfactor = 0;
      break;
    case 2:
      xfactor = 1;
      yfactor = 0;
      break;
    case 3:
      xfactor = 0;
      yfactor = 1;
      break;
    case 4:
      xfactor = 0;
      yfactor = -1;
      break;
  }
  int n_ = o->getN();
  int *x_p = NULL;
  x_p = o->getX_();
  int *y_p = NULL;
  y_p = o->getY_();
  for (int i = 0; i < n_; i++) {
    int tmpX = *(x_p + i) + xfactor;
    if (tmpX > 15)
      tmpX = 0;
    if (tmpX < 0)
      tmpX = 15;
    *(x_p + i) = tmpX;
    int tmpY = *(y_p + i) + yfactor;
    if (tmpY > 15)
      tmpY = 0;
    *(y_p + i) = tmpY;
    if (tmpY < 0)
      tmpY = 15;
    *(y_p + i) = tmpY;
  }
  if (ignoreColor)
    o->updateCoordinates(x_p, y_p);
  else
    o->updateCoordinates(x_p, y_p, R_, G_, B_);
}


void insertObject(unsigned long code, int mode_) {
  int updaterX[22];
  int updaterY[22];
  int updaterN;

  mode = mode_; // 0 == PICKNPLACE-MODE  3 == Random Entertainment

  Serial.print("I am looking for: "); Serial.println(code);

  // line vertical/1/I
  if (code == 6043609 || code == 12937240) {
    memcpy(updaterX, lineX, sizeof(lineX));
    memcpy(updaterY, lineY, sizeof(lineY));
    updaterN = sizeof(lineY)/2;
    objectCounter ++;
  }

  // line horizontal
  if (code == 13552585) {
    memcpy(updaterX, lineY, sizeof(lineY));
    memcpy(updaterY, lineX, sizeof(lineX));
    updaterN = sizeof(lineX)/2;
    objectCounter ++;
  }

  //O
  if (code == 13435006 || code == 757266) {
    memcpy(updaterX, OX, sizeof(OX));
    memcpy(updaterY, OY, sizeof(OY));
    updaterN = sizeof(OX)/2;
    objectCounter ++;
  }

  //Giraffe
  if (code == 13413369) {
    memcpy(updaterX, giraffeX, sizeof(giraffeX));
    memcpy(updaterY, giraffeY, sizeof(giraffeY));
    updaterN = sizeof(giraffeX);
    objectCounter ++;
  }

  //Schweini
  if (code == 5647386 || code == 3442143) {
    memcpy(updaterX, schweinX, sizeof(schweinX));
    memcpy(updaterY, schweinY, sizeof(schweinY));
    updaterN = sizeof(schweinX);
    objectCounter ++;
  }

  //Katze
  if (code == 10835330) {
    memcpy(updaterX, katzeX, sizeof(katzeX));
    memcpy(updaterY, katzeY, sizeof(katzeY));
    updaterN = sizeof(katzeX);
    objectCounter ++;
  }

  //Menschlein
  if (code == 6128352) {
    memcpy(updaterX, menschX, sizeof(menschX));
    memcpy(updaterY, menschY, sizeof(menschY));
    updaterN = sizeof(menschX);
    objectCounter ++;
  }

  //+
  if (code == 10983777) {
    memcpy(updaterX, plusX, sizeof(plusX));
    memcpy(updaterY, plusY, sizeof(plusY));
    updaterN = sizeof(plusX)/2;
    objectCounter ++;
  }

  //-
  if (code == 461385) {
    memcpy(updaterX, minusX, sizeof(minusX));
    memcpy(updaterY, minusY, sizeof(minusY));
    updaterN = 3;
    objectCounter ++;
  }

  //dot
  if (code == 9474971 || code == 6169245) {
    memcpy(updaterX, dotX, sizeof(dotX));
    memcpy(updaterY, dotY, sizeof(dotY));
    updaterN = sizeof(dotX)/2;
    objectCounter ++;
  }

  ///
  if (code == 5645630) {
    memcpy(updaterX, minusX, sizeof(minusX));
    memcpy(updaterY, divY, sizeof(divY));
    updaterN = sizeof(minusX)/2;
    objectCounter ++;
  }

  //=
  if (code == 236768) {
    memcpy(updaterX, equalsX, sizeof(equalsX));
    memcpy(updaterY, equalsY, sizeof(equalsY));
    updaterN = sizeof(equalsX)/2;
    objectCounter ++;
  }

  //2
  if (code == 4631776) {
    memcpy(updaterX, ZWEIX, sizeof(ZWEIX));
    memcpy(updaterY, ZWEIY, sizeof(ZWEIY));
    updaterN = sizeof(ZWEIX)/2;
    objectCounter ++;
  }

  //3
  if (code == 9473543) {
    memcpy(updaterX, DREIX, sizeof(DREIX));
    memcpy(updaterY, DREIY, sizeof(DREIY));
    updaterN = sizeof(DREIX)/2;
    objectCounter ++;
  }

  //4
  if (code == 781403) {
    memcpy(updaterX, VIERX, sizeof(VIERX));
    memcpy(updaterY, VIERY, sizeof(VIERY));
    updaterN = sizeof(VIERX)/2;
    objectCounter ++;
  }

  //5/S
  if (code == 239135 || code == 13467599) {
    memcpy(updaterX, ZWEIX, sizeof(ZWEIX));
    memcpy(updaterY, FUENFY, sizeof(FUENFY));
    updaterN = sizeof(ZWEIX)/2;
    objectCounter ++;
  }

  //6
  if (code == 11071908) {
    memcpy(updaterX, SECHSX, sizeof(SECHSX));
    memcpy(updaterY, SECHSY, sizeof(SECHSY));
    updaterN = sizeof(SECHSX)/2;
    objectCounter ++;
  }

  //7
  if (code == 4582666) {
    memcpy(updaterX, SIEBENX, sizeof(SIEBENX));
    memcpy(updaterY, SIEBENY, sizeof(SIEBENY));
    updaterN = sizeof(SIEBENX)/2;
    objectCounter ++;
  }

  //8
  if (code == 11073112) {
    memcpy(updaterX, achtX, sizeof(achtX));
    memcpy(updaterY, achtY, sizeof(achtY));
    updaterN = sizeof(achtX)/2;
    Serial.println(updaterN);
    objectCounter ++;
  }

  //9
  if (code == 234888) {
    memcpy(updaterX, neunX, sizeof(neunX));
    memcpy(updaterY, neunY, sizeof(neunY));
    updaterN = sizeof(neunX)/2;
    objectCounter ++;
  }

  //A
  if (code == 13305467) {
    memcpy(updaterX, AX, sizeof(AX));
    memcpy(updaterY, AY, sizeof(AY));
    updaterN = sizeof(AX)/2;
    objectCounter ++;
  }

  //B
  if (code == 11063881) {
    memcpy(updaterX, BX, sizeof(BX));
    memcpy(updaterY, BY, sizeof(BY));
    updaterN = sizeof(BX)/2;
    objectCounter ++;
  }

  //C
  if (code == 6107621) {
    memcpy(updaterX, CX, sizeof(CX));
    memcpy(updaterY, CY, sizeof(CY));
    updaterN = sizeof(CX)/2;
    objectCounter ++;
  }
  
  //D
  if (code == 13545983) {
    memcpy(updaterX, DX, sizeof(DX));
    memcpy(updaterY, OY, sizeof(OY));
    updaterN = sizeof(DX)/2;
    objectCounter ++;
  }
  
  //E
  if (code == 11072865) {
    memcpy(updaterX, EX, sizeof(EX));
    memcpy(updaterY, FUENFY, sizeof(FUENFY));
    updaterN = sizeof(EX)/2;
    objectCounter ++;
  }

  //F
  if (code == 6039841) {
    memcpy(updaterX, FX, sizeof(FX));
    memcpy(updaterY, FY, sizeof(FY));
    updaterN = sizeof(FX)/2;
    objectCounter ++;
  }

  //G
  if (code == 795045) {
    memcpy(updaterX, GX, sizeof(GX));
    memcpy(updaterY, SECHSY, sizeof(SECHSY));
    updaterN = sizeof(GX)/2;
    objectCounter ++;
  }

  //H
  if (code == 8980862) {
    memcpy(updaterX, AX, sizeof(AX));
    memcpy(updaterY, HY, sizeof(HY));
    updaterN = sizeof(AX)/2;
    objectCounter ++;
  }

  //J
  if (code == 4734280) {
    memcpy(updaterX, JX, sizeof(JX));
    memcpy(updaterY, JY, sizeof(JY));
    updaterN = sizeof(JX)/2;
    objectCounter ++;
  }

  //K
  if (code == 10198496) {
    memcpy(updaterX, KX, sizeof(KX));
    memcpy(updaterY, KY, sizeof(KY));
    updaterN = sizeof(KX)/2;
    objectCounter ++;
  }

  //L
  if (code == 6117314) {
    memcpy(updaterX, LX, sizeof(LX));
    memcpy(updaterY, LY, sizeof(LY));
    updaterN = sizeof(LX)/2;
    objectCounter ++;
  }

  //M
  if (code == 4767054) {
    memcpy(updaterX, MX, sizeof(MX));
    memcpy(updaterY, MY, sizeof(MY));
    updaterN = sizeof(MX)/2;
    objectCounter ++;
  }

  //N
  if (code == 10953856) {
    memcpy(updaterX, MX, sizeof(MX));
    memcpy(updaterY, NY, sizeof(NY));
    updaterN = sizeof(MX)/2;
    objectCounter ++;
  }

  //P
  if (code == 6076062) {
    memcpy(updaterX, FX, sizeof(FX));
    memcpy(updaterY, PY, sizeof(PY));
    updaterN = sizeof(FX)/2;
    objectCounter ++;
  }

  //Q
  if (code == 10801194) {
    memcpy(updaterX, QX, sizeof(QX));
    memcpy(updaterY, QY, sizeof(QY));
    updaterN = sizeof(QX)/2;
    objectCounter ++;
  }

  //R
  if (code == 11000617) {
    memcpy(updaterX, EX, sizeof(EX));
    memcpy(updaterY, RY, sizeof(RY));
    updaterN = sizeof(EX)/2;
    objectCounter ++;
  }

  //T
  if (code == 10812977) {
    memcpy(updaterX, TX, sizeof(TX));
    memcpy(updaterY, SIEBENY, sizeof(SIEBENY));
    updaterN = sizeof(TX)/2;
    objectCounter ++;
  }

  //U
  if (code == 11248826) {
    memcpy(updaterX, UX, sizeof(UX));
    memcpy(updaterY, UY, sizeof(UY));
    updaterN = sizeof(UX)/2;
    objectCounter ++;
  }

  //V
  if (code == 164353) {
    memcpy(updaterX, VX, sizeof(VX));
    memcpy(updaterY, UY, sizeof(UY));
    updaterN = sizeof(VX)/2;
    objectCounter ++;
  }  

  //W
  if (code == 4631194) {
    memcpy(updaterX, MX, sizeof(MX));
    memcpy(updaterY, WY, sizeof(WY));
    updaterN = sizeof(MX)/2;
    objectCounter ++;
  } 

  //X
  if (code == 4634097) {
    memcpy(updaterX, VX, sizeof(VX));
    memcpy(updaterY, XY, sizeof(XY));
    updaterN = sizeof(VX)/2;
    objectCounter ++;
  }  

  //Y
  if (code == 13426082) {
    memcpy(updaterX, YX, sizeof(YX));
    memcpy(updaterY, YY, sizeof(YY));
    updaterN = sizeof(YX)/2;
    objectCounter ++;
  }

  //Z
  if (code == 4603097) {
    memcpy(updaterX, ZX, sizeof(ZX));
    memcpy(updaterY, ZY, sizeof(ZY));
    updaterN = sizeof(ZX)/2;
    objectCounter ++;
  } 

  //Ä
  if (code == 10959492) {
    memcpy(updaterX, AEX, sizeof(AEX));
    memcpy(updaterY, AEY, sizeof(AEY));
    updaterN = sizeof(AEX)/2;
    objectCounter ++;
  } 

  //Ö
  if (code == 13387277) {
    memcpy(updaterX, AEX, sizeof(AEX));
    memcpy(updaterY, OEY, sizeof(OEY));
    updaterN = sizeof(AEX)/2;
    objectCounter ++;
  }

  //Ü
  if (code == 4942433) {
    memcpy(updaterX, UX, sizeof(UX));
    memcpy(updaterY, UEY, sizeof(UEY));
    updaterN = sizeof(UX)/2;
    objectCounter ++;
  }

  if (objectCounter > MAX_PRIMITIVES)
    objectCounter = 1;

  if (objectCounter > 0) {

    allObjects[objectCounter - 1].updateCoordinates(updaterN, updaterX, updaterY, R_, G_, B_);
    //
    //lastPaused = 0;
  }

  //Serial.print("Number of objects: "); Serial.println(objectCounter);

}


