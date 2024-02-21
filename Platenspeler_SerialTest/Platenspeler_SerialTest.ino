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
#define clamp 11
#define tilt 10

int rpmPin = RPM33;

//create classes
AccelStepper stepper = AccelStepper(1, stepPin, dirPin);
Servo Clamp;
Servo Tilt;
Servo Rotation;
Servo toneArmHeight;
Servo toneArmPos;  //150: full left (home), ~107: startpos plate, ~0: endpos plate

enum steps { startup,
             homing,
             jukePiCommand,  //wait for command
             stepperPickTopPos,
             rotArmClearancePos,
             tiltArmHorizontal,
             stepperPickBottomPos,
             rotArmPickPlacePos,  //determine next steps based on ClampIn or Out
             clampIn,             //clamp LP
             tiltArmVertical,
             stepperPlaceTopPos,
             stepperPlaceBottomPos,
             clampOut,  //release LP
             //rotArmClearLPPos,       //position to clear the LP, can't go all the way out because of clamp arm
             stopCommand,  //from either JukePi or player end sensor
             startPlaying,
             stopPlaying
};

steps CaseStep = startup, prevStep = startup;
bool transit = false;

int subroutineSteps = 0;
unsigned long subroutineMillis = 0;
bool subroutineDone = false;

bool running33RPM, running45RPM, RPOn, powerStatus, stopped = false;
bool analogReadActive = false;  //to stop analogRead when just turning on the LP because the value peaks to 700 shortly

//stepper values
const int stepsPermm = 32;
const int maxVel = 200 * stepsPermm;  //200mm/s
bool directionValue = false;

unsigned long prevMillis = 0;
unsigned long commandActivated = 0;
String message = "";

bool blockUpdateTime = false;
bool isHoldingLP = false;

int ClampTryPos = 90;
int RotationTryPos = 90;
int TiltTryPos = 90;

void setup() {
  // put your setup code here, to run once:

  Startup();

  homingSequence();

  CaseStep = jukePiCommand;
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
    if (message.equalsIgnoreCase("demo")) {
      CaseStep = stepperPlaceTopPos;  //go to top of player
      isHoldingLP = true;
    }

    if (message.equalsIgnoreCase("33")) {
      //33 RPM mode
      rpmPin = RPM33;
      CaseStep = startPlaying;
      Serial.println("start 33 RPM rotation");
      commandActivated = millis();

    } else if (message.equalsIgnoreCase("45")) {
      //45 RPM mode
      rpmPin = RPM45;
      CaseStep = startPlaying;
      Serial.println("start 45 RPM rotation");
      commandActivated = millis();

    } else if (message.equalsIgnoreCase("STOP")) {
      //stop rotation
      CaseStep = stopPlaying;
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
      //other commands
      if (CaseStep == clampOut) {
        if (message.equalsIgnoreCase("continue")) {
          CaseStep = rotArmClearancePos;
        }
        //write number to servo
        ClampTryPos = message.toInt();
        Serial.println(ClampTryPos);

      } else if (CaseStep == rotArmClearancePos
                 || CaseStep == rotArmPickPlacePos) {
        if (message.equalsIgnoreCase("continue")) {
          CaseStep = jukePiCommand;
        }
        //write number to servo
        RotationTryPos = message.toInt();
        Serial.println(RotationTryPos);

      } else if (CaseStep == tiltArmVertical) {
        if (message.equalsIgnoreCase("continue")) {
          CaseStep = jukePiCommand;
        }
        //write number to servo
        TiltTryPos = message.toInt();
        Serial.println(TiltTryPos);

      } else {
        Serial.println("invalid message");
      }
    }
    Serial.print(message + "\t");
    long timeDiff = millis() - prevMillis;
    Serial.println(timeDiff);
    message = "";
  }

  if (millis() - commandActivated > 250) {  //wait for 250ms
    //turn everything off
    //analogReadActive = true;
    digitalWrite(RPM33, LOW);
    digitalWrite(RPM45, LOW);
    digitalWrite(STOP, LOW);
  }


  if (analogReadActive) {
    bool donePlaying = analogRead(iLDR) > 600;
    RPOn = analogRead(iLDR) > 100;
    if (donePlaying) {
      stopped = true;
      StopPlay();
      CaseStep = jukePiCommand;
    }
    Serial.println(analogRead(iLDR));
  }

  switch (CaseStep) {
    case startup:
      if (transit) {
        Serial.println("startup");
      }
      while (true) {
        Serial.println("Unexpected Startup");
        delay(1000);
      }
      break;

    //Homing stepper and servos
    case homing:
      if (transit) {
        Serial.println("homing");
      }
      homingSequence();
      CaseStep = jukePiCommand;
      break;

    //wait for command from Raspberry Pi
    case jukePiCommand:
      if (transit) {
        Serial.println("jukePiCommand");
      }
      //read serial and determine next step there
      break;

    //move stepper to top position of pick pos
    case stepperPickTopPos:
      if (transit) {
        Serial.println("stepperPickTopPos");
      }
      //statement
      break;

    //rotate arm to clearance position, free from box and ready for vertical tilt
    case rotArmClearancePos:
      if (transit) {
        Serial.println("rotArmClearancePos");
      }
      Rotation.write(RotationTryPos);
      blockUpdateTime = true;
      if (millis() - prevMillis > 5000) {
        blockUpdateTime = false;
        CaseStep = tiltArmVertical;
      }
      break;

    //tilt gripper arm horizontal, ready to pick up or place down
    case tiltArmHorizontal:
      if (transit) {
        Serial.println("tiltArmHorizontal");
      }
      Tilt.write(TiltVertical);
      blockUpdateTime = true;
      if (millis() - prevMillis > 1000) {
        blockUpdateTime = false;
        CaseStep = jukePiCommand;
      }
      break;

    //move stepper to bottom position of pick pos. this is the actual height of the LP
    case stepperPickBottomPos:
      if (transit) {
        Serial.println("stepperPickBottomPos");
      }
      //statement
      break;

    //rotate arm to inwards position for pick or place. should always be the same rotation as all LP's are in line
    case rotArmPickPlacePos:
      if (transit) {
        Serial.println("rotArmPickPlacePos");
      }
      if (isHoldingLP) {  //is at top pos, need to release LP
        blockUpdateTime = true;
        Rotation.write(RotationIn);
        if (millis() - prevMillis > 5000) {
          blockUpdateTime = false;
          CaseStep = stepperPlaceBottomPos;
        }
      } else {  //at pickup pos, need to clamp LP
      }
      //statement
      break;

    //move the little clamp inwards to grip the LP
    case clampIn:  //clamp LP
      if (transit) {
        Serial.println("clampIn");
      }
      //statement
      break;

    //tilt arm vertical to move stepper up or down
    case tiltArmVertical:
      if (transit) {
        Serial.println("tiltArmVertical");
      }
      Tilt.write(TiltVertical);
      //blockUpdateTime = true;
      //if (millis() - prevMillis > 1000) {
      //  blockUpdateTime = false;
      //  CaseStep = jukePiCommand;
      //}
      //statement
      break;

    //move stepper to the top of the place position
    case stepperPlaceTopPos:
      if (transit) {
        Serial.println("stepperPlaceTopPos");
      }
      stepper.moveTo(StepperPos(LPPositions[0]));
      if (stepper.distanceToGo() == 0) {
        CaseStep = rotArmPickPlacePos;
      }
      //statement
      break;

    //move stepper to the bottom of the place position. LP should now rest on player or storage
    case stepperPlaceBottomPos:
      if (transit) {
        Serial.println("stepperPlaceBottomPos");
      }
      stepper.moveTo(StepperPos(LPPositions[8]));
      if (stepper.distanceToGo() == 0) {
        CaseStep = clampOut;
      }
      //statement
      break;

    //release LP clamp
    case clampOut:  //release LP
      if (transit) {
        Serial.println("clampOut");
      }
      Clamp.write(ClampTryPos);
      break;

    //case rotArmClearLPPos:       //position to clear the LP, can't go all the way out because of clamp arm

    //interupt steps
    //start rotating at set RPM and move arm on top of LP
    case startPlaying:
      if (transit) {
        Serial.println("startPlaying");
        subroutineSteps = 0;
        subroutineDone = false;
        //only write pin high once
        digitalWrite(rpmPin, HIGH);
      }
      startPlay();
      if (subroutineDone) {
        subroutineDone = false;
        CaseStep = jukePiCommand;
      }
      //statement
      break;

    //stop the player from playing and move arm away from LP. ready for LP to be picked up afterwards
    case stopPlaying: //from either JukePi or player end sensor
      if (transit) {
        Serial.println("stopPlaying");
        subroutineSteps = 0;
        subroutineDone = false;
        digitalWrite(STOP, HIGH);
      }
      StopPlay();
      if (subroutineDone) {
        subroutineDone = false;
        CaseStep = jukePiCommand;
      }
      //statement
      break;


  } //end case

//set transit bit
  if (CaseStep != prevStep) {
    transit = true;
    prevStep = CaseStep;
  } else {
    transit = false;
  }



  //leave this part last in loop
  //protect limits
  if (stepper.targetPosition() < StepperPos(maxPos) && stepper.targetPosition() > 0) {
    stepper.run();
  }

  //update time
  if (!blockUpdateTime) {
    prevMillis = millis();
  }
}


void Startup() {
  Serial.begin(115200);

  pinMode(RPM33, OUTPUT);  //output to transistor to toggle 33 RPM button
  pinMode(RPM45, OUTPUT);  //output to transistor to toggle 45 RPM button
  pinMode(STOP, OUTPUT);   //output to transistor to toggle stop button
  pinMode(RLYOn, OUTPUT);  //output to Relay that switches on the RP (Record Player)
  pinMode(iEndStop, INPUT_PULLUP);

  Clamp.attach(clamp);
  Tilt.attach(tilt);
  Rotation.attach(rot);
}


int StepperPos(int pos) {
  return pos * stepsPermm;
}

void HomeStepper() {
  int Counter = 0;
  while (digitalRead(iEndStop) && Counter < 10000) {
    //keep running up
    Serial.println("stepper up");
    stepper.runSpeed();
    Counter += 1;
  }
  //if endsensor, stop and set 0 position
  stepper.stop();
  stepper.setCurrentPosition(0);

  stepper.moveTo(StepperPos(5));
  stepper.runToPosition();
}


void homingSequence() {

  toneArmHeight.write(DOWN);  //prevent fast switch
  toneArmHeight.attach(armHeight);
  toneArmPos.write(BASE);
  toneArmPos.attach(armPos);

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
  toneArmHeight.write(0);       //DOWN
  Clamp.write(90);              //loosen the clamp
  Rotation.write(RotationOut);  //~ middle
  delay(250);                   //wait for it to be outwards at least a bit
  Tilt.write(TiltVertical);     //vertical


  //Clamp.detach();
  //Rotation.detach();
  //Tilt.detach();

  //home stepper
  HomeStepper();

  stepper.setSpeed(-250);  //lower speed
  HomeStepper();
}


