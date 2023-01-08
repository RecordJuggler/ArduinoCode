//import libs
#include <MultiStepper.h>
#include <AccelStepper.h>
#include <Servo.h>
#include "systemPositions.h"


//platenspeler:
#define RPM33 2
#define RPM45 3
#define STOP 4
#define RLYOn 12
#define iLDR A7
#define armHeight 13  //platenspeler arm hoogte-knop-servo. bereik 0-70 graden
#define armPos 7      //platenspeler arm positie-servo

//stepper pins
#define iEndStop 8
#define stepPin 6
#define dirPin 5

//servo pins
#define rot 9
#define clamp 10
#define tilt 11



//create classes
AccelStepper stepper = AccelStepper(1, stepPin, dirPin);
Servo Clamp;
Servo Tilt;
Servo Rotation;
Servo toneArmHeight;
Servo toneArmPos;  //150: full left (home), ~107: startpos plate, ~0: endpos plate



bool running33RPM, running45RPM, LPOn, powerStatus, stopped = false;
bool analogReadActive = true;  //to stop analogRead when just turning on the LP because the value peaks to 700 shortly

//stepper values
const int stepsPermm = 32;
const int maxVel = 200 * stepsPermm;  //200mm/s
bool directionValue = false;

unsigned long prevMillis = 0;
unsigned long commandActivated = 0;
String message = "";


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(RPM33, OUTPUT);
  pinMode(RPM45, OUTPUT);
  pinMode(STOP, OUTPUT);
  pinMode(RLYOn, OUTPUT);
  pinMode(iEndStop, INPUT_PULLUP);

  Clamp.attach(clamp);
  Tilt.attach(tilt);
  Rotation.attach(rot);

  toneArmHeight.write(DOWN);  //prevent fast switch
  toneArmHeight.attach(armHeight);
  toneArmPos.write(180);
  toneArmPos.attach(armPos);

  stepper.setMaxSpeed(maxVel);  //200mm/s
  stepper.setAcceleration(maxVel);

  stepper.setSpeed(-1000);  //homing is up

  delay(100);

  homingSequence();

  delay(500);
}

void loop() {
  // put your main code here, to run repeatedly:

  while (Serial.available()) {
    char character = Serial.read();
    message += character;
    delay(1);  //delay to allow characters to be filled into buffer while reading
  }

  if (message.length() > 0) {
    message.trim();

    if (message.equalsIgnoreCase("33")) {
      //33 RPM mode
      //startPlay();
      digitalWrite(RPM33, HIGH);
      Serial.println("start 33 RPM rotation");
      commandActivated = millis();
    } else if (message.equalsIgnoreCase("45")) {
      //45 RPM mode
      startPlay();
      digitalWrite(RPM45, HIGH);
      Serial.println("start 45 RPM rotation");
      commandActivated = millis();
    } else if (message.equalsIgnoreCase("STOP")) {
      //stop rotation
      digitalWrite(STOP, HIGH);
      StopPlaying();
      Serial.println("stop any rotation");
      commandActivated = millis();
    } else if (message.equalsIgnoreCase("togglepower")) {
      //toggle relay on or off
      powerStatus = !powerStatus;
      digitalWrite(RLYOn, powerStatus);
      analogReadActive = false;  //disable analogRead
      Serial.println("toggled relay");
      commandActivated = millis();
    } else {
      Serial.println("invalid message");
    }
    Serial.print(message + "\t");
    long timeDiff = millis() - prevMillis;
    Serial.println(timeDiff);
    message = "";
  }
  if (millis() - commandActivated > 250) {  //wait for 500ms
    //turn everything off
    analogReadActive = true;
    digitalWrite(RPM33, LOW);
    digitalWrite(RPM45, LOW);
    digitalWrite(STOP, LOW);
  }


  if (analogReadActive) {
    bool donePlaying = analogRead(iLDR) > 600;
    LPOn = analogRead(iLDR) > 100;
    if (donePlaying) {
      stopped = true;
      StopPlaying();
    }


    //Serial.println(analogRead(iLDR));
  }

  //toneArmHeight.write(0);
  //delay(2000);
  //toneArmPos.write(90);
  //toneArmHeight.write(60);
  //delay(1000);


  //leave this line last in loop
  prevMillis = millis();
}


int StepperPos(int pos) {
  return pos * stepsPermm;
}

void HomeStepper() {
  while (digitalRead(iEndStop)) {
    //keep running up
    Serial.println("stepper up");
    stepper.runSpeed();
  }
  //if endsensor, stop en set 0 position
  stepper.stop();
  stepper.setCurrentPosition(0);

  stepper.moveTo(StepperPos(5));
  stepper.runToPosition();
}


void homingSequence() {
  bool slowHome = false;
  stepper.setMaxSpeed(maxVel);  //200mm/s
  stepper.setAcceleration(maxVel);
  stepper.setSpeed(-1000);  //homing up

  delay(100);

  //homing
  /*
     when clamp is limited to 90deg out (or only slightly more) it cannot be in the way for homing,
     so then the rotation can be done first, then tilt and then clamp.
     that way when there is an LP in the clamp it won't fall out.

     when clamp is not limited, it can be in the way for rotation and tilt, so it needs to be homed first.
  */
  toneArmHeight.write(0);  //DOWN
  Clamp.write(90);         //loosen the clamp
  Rotation.write(97);      //~ middle
  delay(250);              //wait for it to be outwards at least a bit
  Tilt.write(98);          //vertical


  Clamp.detach();
  Rotation.detach();
  Tilt.detach();

  //home stepper
  HomeStepper();

  stepper.setSpeed(-250);  //lower speed
  HomeStepper();
}



void startPlay() {
  //move arm down, to allow positioning to go to base without interference
  toneArmHeightEnum = DOWN;
  toneArmHeight.write(toneArmHeightEnum);
  delay(1000);

  //move pos to base
  toneArmPosEnum = BASE;
  toneArmPos.write(toneArmPosEnum);

  delay(100);  //wait for it to be at base

  //arm back up
  toneArmHeightEnum = UP;
  toneArmHeight.write(toneArmHeightEnum);
  delay(2000);

  //slowly move arm to START pos
  for (int i = BASE; i >= START; i--) {
    toneArmPos.write(i);
    delay(100);
  }

  //arm back down
  toneArmHeightEnum = DOWN;
  toneArmHeight.write(toneArmHeightEnum);
}

void StopPlaying() {
  toneArmPos.write(END);
  delay(100);

  toneArmHeightEnum = UP;
  toneArmHeight.write(toneArmHeightEnum);
  delay(2000);

  for (int i = END; i <= START; i++) {
    toneArmPos.write(i);
    delay(50);
  }
  //to holder pos
  for (int i = START; i <= 120; i++) {
    toneArmPos.write(i);
    delay(150);
  }

  toneArmHeightEnum = DOWN;
  toneArmHeight.write(toneArmHeightEnum);
  delay(1000);

  //all the way to (second) base
  for (int i = 120; i <= BASE; i++) {
    toneArmPos.write(i);
    delay(100);
  }
}
