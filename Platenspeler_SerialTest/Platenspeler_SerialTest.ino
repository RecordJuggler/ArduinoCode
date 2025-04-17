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

             //steps 1xx: Pick sequence from player
             Step100_stepperPickBottomPos,
             Step110_tiltArmHorizontal,
             Step120_rotArmPickPos1,
             Step130_stepperPickIntermediatePos,
             Step135_rotArmPickPos2,
             Step140_stepperPickPos2,
             Step150_rotArmPickIn,
             Step160_CloseClamp,
             Step170_stepperPickTopPos,
             Step180_tiltPickSlightlyVertical,
             Step190_rotArmPickOutwards,
             Step195_tiltPickVertical,

             //steps 2xx: Place sequence on player or rack
             Step200_stepperPlaceTopPos,
             Step250_stepperPlaceBottomPos,

             //steps 3xx: Pick sequence from rack
             Step300_stepperPick,

             rotArmClearancePos,
             tiltArmHorizontal,
             clampIn,  //clamp LP
             tiltArmVertical,

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
#pragma region individual movement commands
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
#pragma endregion


//other commands
#pragma region more complicated test commands
    if (message.equalsIgnoreCase("demo")) {
      //CaseStep = stepperPlaceTopPos;  //go to top of player
      CaseStep = Step100_stepperPickBottomPos;
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
                 || CaseStep == Step120_rotArmPickPos1) {
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
#pragma endregion


    Serial.print(message + "\t");
    long timeDiff = millis() - prevMillis;
    Serial.println(timeDiff);
    message = "";
  }

  //reset outputs after a small delay
  if (millis() - commandActivated > 250) {  //wait for 250ms
    //turn everything off
    //analogReadActive = true;
    digitalWrite(RPM33, LOW);
    digitalWrite(RPM45, LOW);
    digitalWrite(STOP, LOW);
  }

  //enable analog read to detect when LP is done playing
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

//steps sequence
#pragma region Step Sequence

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


//Pick steps from player
#pragma region pick from player


    //move stepper to bottom position of pick pos. this is the actual height of the LP
    case Step100_stepperPickBottomPos:
      if (transit) {
        Serial.println("stepperPickBottomPos");
      }
      stepper.moveTo(StepperPos(LPPositions[8]));
      if (stepper.distanceToGo() == 0) {
        CaseStep = Step110_tiltArmHorizontal;
      }
      break;


    //tilt gripper arm horizontal, ready to pick up or place down
    case Step110_tiltArmHorizontal:
      if (transit) {
        Serial.println("tiltArmHorizontal");
      }
      MoveTiltServo(TiltHorizontal, 1);
      CaseStep = Step120_rotArmPickPos1;
      break;



    //rotate arm to inwards position for pick. only inner side will allign vertically because of flex in arm
    case Step120_rotArmPickPos1:
      if (transit) {
        Serial.println("rotArmPickPlacePos");
      }
      MoveRotationServo(RotationPos1, 1);
      CaseStep = Step130_stepperPickIntermediatePos;
      break;



    //move stepper to intermediate position of pick pos, alligns outer side of arm with LP
    case Step130_stepperPickIntermediatePos:
      if (transit) {
        Serial.println("stepperPickIntermediatePos");
      }
      stepper.moveTo(StepperPos(LPPositions[9]));
      if (stepper.distanceToGo() == 0) {
        CaseStep = Step135_rotArmPickPos2;
      }
      break;



    //Rotate arm further in to also allow outer side of arm to grip LP
    case Step135_rotArmPickPos2:
      if (transit) {
        Serial.println("rotArmPickPos2");
      }
      MoveRotationServo(RotationPos2, 1);
      CaseStep = Step140_stepperPickPos2;
      break;



    //Lift LP slightly, but still on center Pin
    case Step140_stepperPickPos2:
      if (transit) {
        Serial.println("stepperPickPos2");
      }
      stepper.moveTo(StepperPos(LPPositions[10]));
      if (stepper.distanceToGo() == 0) {
        CaseStep = Step150_rotArmPickIn;
      }
      break;



    //completely move rotation IN
    case Step150_rotArmPickIn:
      if (transit) {
        Serial.println("rotArmPickPos2");
      }
      MoveRotationServo(RotationIn, 1);
      CaseStep = Step160_CloseClamp;
      break;



    //Close the clamp to hold LP in place
    case Step160_CloseClamp:
      if (transit) {
        Serial.println("clampIn");
      }
      MoveClampServo(ClampClose, 1);
      CaseStep = Step170_stepperPickTopPos;
      break;



    //move stepper to top position of pick pos
    case Step170_stepperPickTopPos:
      if (transit) {
        Serial.println("stepperPickTopPos");
      }
      stepper.moveTo(StepperPos(LPPositions[0]));
      if (stepper.distanceToGo() == 0) {
        CaseStep = Step180_tiltPickSlightlyVertical;
      }
      break;



    //move tilt slightly vertical to hold LP better before rotating outwards
    case Step180_tiltPickSlightlyVertical:
      if (transit) {
        Serial.println("stepperPickTopPos");
      }
      MoveTiltServo(TiltSlightlyVertical, 1);
      CaseStep = Step190_rotArmPickOutwards;
      break;



    //move rotation OUT
    case Step190_rotArmPickOutwards:
      if (transit) {
        Serial.println("rotArmPickOutwards");
      }
      MoveRotationServo(RotationOut, 1);
      CaseStep = Step195_tiltPickVertical;
      break;



    //put tilt vertical to allow stepper to move
    case Step195_tiltPickVertical:
      if (transit) {
        Serial.println("tiltPickVertical");
      }
      MoveTiltServo(TiltVertical, 1);
      CaseStep = jukePiCommand;
      break;




#pragma endregion end of Pick sequence from player


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
    case Step200_stepperPlaceTopPos:
      if (transit) {
        Serial.println("stepperPlaceTopPos");
      }
      stepper.moveTo(StepperPos(LPPositions[0]));
      if (stepper.distanceToGo() == 0) {
        CaseStep = Step120_rotArmPickPos1;
      }
      //statement
      break;

    //move stepper to the bottom of the place position. LP should now rest on player or storage
    case Step250_stepperPlaceBottomPos:
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
  }
#pragma endregion
  //end case

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
