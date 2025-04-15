//import libs
//#include <MultiStepper.h>
#include <AccelStepper.h>
#include <Servo.h>
#include <EEPROM.h>

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
#define rotPin 9
#define clampPin 11
#define tiltPin 10

const int RotationEEPROMAddress = 0;
const int TiltEEPROMAddress = 1;
const int ClampEEPROMAddress = 2;
const int ArmHeightEEPROMAddress = 3;
const int ArmPosEEPROMAddress = 4;

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
             stepperPickIntermediatePos,
             stepperPickPos2,
             rotArmClearancePos,
             rotArmPickPos2,
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

//eeprom, stored data
byte RotStoredPos, TiltStoredPos, ClampStoredPos, ArmHeightStoredPos, ArmPosStoredPos = 0;
bool ServoPositionKnown = false;

void setup() {
  Serial.begin(115200);
  // put your setup code here, to run once:

  //read last known servo positions at startup
  if (!ServoPositionKnown) {
    RotStoredPos = EEPROM.read(RotationEEPROMAddress);
    Serial.println(RotStoredPos);
    if (RotStoredPos == 255) {
      //first time boot, no position stored yet
      RotStoredPos = RotationOut;  //store reference pos
    }
    TiltStoredPos = EEPROM.read(TiltEEPROMAddress);
    Serial.println(TiltStoredPos);
    if (TiltStoredPos == 255) {
      TiltStoredPos = TiltVertical;
    }
    ClampStoredPos = EEPROM.read(ClampEEPROMAddress);
    Serial.println(ClampStoredPos);
    if (ClampStoredPos == 255) {
      ClampStoredPos = ClampOpen;
    }
    ArmHeightStoredPos = EEPROM.read(ArmHeightEEPROMAddress);
    Serial.println(ArmHeightStoredPos);
    if (ArmHeightStoredPos == 255) {
      ArmHeightStoredPos = DOWN;  //DOWN
    }
    ArmPosStoredPos = EEPROM.read(ArmPosEEPROMAddress);
    Serial.println(ArmPosStoredPos);
    if (ArmPosStoredPos == 255) {
      ArmPosStoredPos = BASE;
    }

    ServoPositionKnown = true;
  }


  Startup();

  //homingSequence();

  CaseStep = jukePiCommand;
  delay(500);
}

void loop() {
  // put your main code here, to run repeatedly:

  while (Serial.available()) {
    char character = Serial.read();
    message += character;
    delay(2);  //delay to allow characters to be filled into buffer while reading
  }

  if (message.length() > 0) {
    message.trim();

    //simple move commands
    /*
    s - Stepper
    r - rotation (arm in/out)
    t - tilt (arm horizontal/vertical)
    c - clamp (clamp arm in/out)
    u - up, tone arm up/down
    i - in, tonearm in/out
    */
    if (message[0] == 'x' && message.length() > 2) {
      if (message.substring(2).toInt() != 0) {
        int number = message.substring(2).toInt();
        if (message[1] == 's') {
          if (number > 0) {
            //move stepper to absolute position to value above 0
            stepper.moveTo(StepperPos(number));
          }

        } else if (message[1] == 'r') {
          //move rotation
          MoveRotationServo(number, 2);
          //Rotation.write(number);

        } else if (message[1] == 't') {
          //move tilt
          MoveTiltServo(number, 2);
          //Tilt.write(number);

        } else if (message[1] == 'c') {
          //move clamp
          MoveClampServo(number, 2);
          //Clamp.write(number);

        } else if (message[1] == 'u') {
          //move toneArm Up/down
          MoveArmHeightServo(number, 2);
          //toneArmHeight.write(number);

        } else if (message[1] == 'i') {
          //move toneArm in/out
          MoveArmPosServo(number, 2);
          //toneArmPos.write(number);
        }
      }
    }

    //other commands
    if (message.equalsIgnoreCase("demo")) {
      //CaseStep = stepperPlaceTopPos;  //go to top of player
      CaseStep = stepperPickBottomPos;
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

    } else if (message.equalsIgnoreCase("up")) {
      //toggle relay on or off
      MoveArmHeightServo(UP, 1);
      //toneArmHeight.write(UP);
      Serial.println("tonearm UP");
      commandActivated = millis();

    } else if (message.equalsIgnoreCase("down")) {
      //toggle relay on or off
      MoveArmHeightServo(DOWN, 1);
      //toneArmHeight.write(DOWN);
      Serial.println("tonearm DOWN");
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
      CaseStep = stopPlaying;
      //stopped = true;
      //StopPlay();
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
      stepper.moveTo(StepperPos(LPPositions[0]));
      if (stepper.distanceToGo() == 0) {
        CaseStep = rotArmPickPlacePos;
      }
      //statement
      break;


    //move stepper to intermediate position of pick pos
    case stepperPickIntermediatePos:
      if (transit) {
        Serial.println("stepperPickTopPos");
      }
      stepper.moveTo(StepperPos(LPPositions[9]));
      MoveRotationServo(RotationPos2, 1);
      if (stepper.distanceToGo() == 0) {
        CaseStep = stepperPickPos2;
      }
      //statement
      break;


    case stepperPickPos2:
      if (transit) {
        Serial.println("stepperPickPos2");
      }
      stepper.moveTo(StepperPos(LPPositions[10]));
      MoveRotationServo(RotationIn,1);
      if (stepper.distanceToGo() == 0) {
        CaseStep = clampIn;
      }
      break;



    //rotate arm to clearance position, free from box and ready for vertical tilt
    case rotArmClearancePos:
      if (transit) {
        Serial.println("rotArmClearancePos");
      }
      MoveRotationServo(RotationOut, 1);
      //Rotation.write(RotationTryPos);
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
      MoveTiltServo(TiltHorizontal, 1);
      //Tilt.write(TiltVertical);
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
      stepper.moveTo(StepperPos(LPPositions[8]));
      if (stepper.distanceToGo() == 0) {
        CaseStep = tiltArmHorizontal;
      }

      //statement
      break;


    //rotate arm to inwards position for pick or place. should always be the same rotation as all LP's are in line
    case rotArmPickPlacePos:
      if (transit) {
        Serial.println("rotArmPickPlacePos");
      }
      blockUpdateTime = true;
      if (isHoldingLP) {  //is at top pos, need to release LP
        MoveRotationServo(RotationOut, 1);
        //Rotation.write(RotationIn);
      } else {  //at pickup pos, need to clamp LP
        MoveRotationServo(RotationPos1, 1);
        //Rotation.write(RotationIn);
      }
      if (millis() - prevMillis > 5000) {
        blockUpdateTime = false;
        if (isHoldingLP) {
          CaseStep = stepperPlaceBottomPos;
        } else {
          CaseStep = stepperPickIntermediatePos;
        }
      }
      //statement
      break;



    case rotArmPickPos2:
      if (transit) {
        Serial.println("rotArmPickPos2");
      }

      blockUpdateTime = true;
      MoveRotationServo(RotationPos2, 1);
      if (millis() - prevMillis > 5000) {
        blockUpdateTime = false;
        CaseStep = stepperPickPos2;
      }
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
      MoveTiltServo(TiltVertical, 1);
      //Tilt.write(TiltVertical);
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
      MoveClampServo(ClampTryPos, 1);
      //Clamp.write(ClampTryPos);
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
    case stopPlaying:  //from either JukePi or player end sensor
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


  }  //end case

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
  //end Loop
}






void Startup() {

  pinMode(RPM33, OUTPUT);  //output to transistor to toggle 33 RPM button
  pinMode(RPM45, OUTPUT);  //output to transistor to toggle 45 RPM button
  pinMode(STOP, OUTPUT);   //output to transistor to toggle stop button
  pinMode(RLYOn, OUTPUT);  //output to Relay that switches on the RP (Record Player)
  pinMode(iEndStop, INPUT_PULLUP);

  //move servo's to default positions before attach()
  Rotation.write(RotStoredPos);
  Tilt.write(TiltStoredPos);
  Clamp.write(ClampStoredPos);
  toneArmHeight.write(ArmHeightStoredPos);
  toneArmPos.write(ArmPosStoredPos);

  //attach servo's
  Rotation.attach(rotPin);
  Tilt.attach(tiltPin);
  Clamp.attach(clampPin);

  toneArmHeight.attach(armHeight);
  toneArmPos.attach(armPos);

  homingSequence();
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
  MoveArmHeightServo(DOWN, 1);
  //toneArmHeight.write(DOWN);    //DOWN
  MoveArmPosServo(BASE, 1);
  //toneArmPos.write(BASE);     //base position
  MoveClampServo(ClampOpen, 1);
  //Clamp.write(ClampOpen);       //loosen the clamp
  MoveRotationServo(RotationOut, 1);
  //Rotation.write(RotationOut);  //~ middle
  //delay(250);                   //wait for it to be outwards at least a bit, NOT NEEDED WITH NEW FUNCTIONS
  MoveTiltServo(TiltVertical, 1);
  //Tilt.write(TiltVertical);     //vertical


  //Clamp.detach();
  //Rotation.detach();
  //Tilt.detach();

  //home stepper
  HomeStepper();

  stepper.setSpeed(-250);  //lower speed
  HomeStepper();
}
