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
const int StepperEEPROMAddress = 5;

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

             //steps 1xx: Pick A-side sequence from player
             Step100_PickA_StepperPickBottomPos,
             Step110_PickA_TiltArmHorizontal,
             Step120_PickA_RotArmPickPos1,
             Step130_PickA_StepperPickIntermediatePos,
             Step135_PickA_RotArmPickPos2,
             Step140_PickA_StepperPickPos2,
             Step150_PickA_RotArmPickIn,
             Step160_PickA_CloseClamp,
             Step170_PickA_StepperPickTopPos,
             Step180_PickA_TiltPickSlightlyVertical,
             Step190_PickA_RotArmPickOutwards,
             Step195_PickA_TiltPickVertical,


             //steps 2xx: Pick B-side sequence from player
             Step200_PickB_StepperPickTopPos,  //stepper to top to allow tilt horizontal
             Step210_PickB_TiltArmHorizontal,  //tilt arm horizontal for B-side
             Step220_PickB_RotArmPickOut,      //rotate arm in to 20 (maybe not necessary?)
             Step230_PickB_StepperBottomPos,
             Step240_PickB_RotArmPickPos1,            //rotate arm in to 35, slightly lift LP with inner arm
             Step250_PickB_StepperIntermediatePos,    //lift arm so outer arm is level with LP
             Step260_PickB_RotArmPickPos2,            //reach LP with outer arm
             Step270_PickB_StepperPickPos2,           //move stepper up but stay on center pin
             Step280_PickB_RotArmPickIn,              //Rotate arm to 50 to fully pick LP
             Step290_PickB_CloseClamp,                //does it even need a comment?..
             Step291_PickB_StepperPickTopPos,         //stepper up to 40 to allow tilt again
             Step292_PickB_TiltPickSlightlyVertical,  //tilt slightly to allow better rotation movement without potentially losing LP
             Step293_PickB_RotArmPickOutwards,        //Rotate arm out to 130
             Step295_PickB_TiltPickVertical,


             //steps 3xx: Place A-side sequence on player
             Step300_PlaceA_stepperPlaceTopPos,     //stepper to height where we can place LP (120-ish should be fine?)
             Step310_PlaceA_TiltPlaceHorizontal,    //tilt horizontal A-side to place A side on top
             Step320_PlaceA_RotArmPlaceIn,          //rotate arm in to place position (should be the same for all picks and places)
             Step330_PlaceA_StepperPlaceCenterPin,  //stepper to height where LP is just within center pin
             Step340_PlaceA_RotArmShakeOnPin,       //Special move to shake rotation around 50-52-ish to account for inaccuracy and make sure LP is on Pin
             Step350_PlaceA_StepperPlaceBottomPos,  //move stepper down a bit (maybe not necessary?)
             Step360_PlaceA_ClampOpen,
             Step370_PlaceA_RotArmPlaceOutwards,  //move arm outwards to 130
             Step380_PlaceA_TiltPlaceVertical,    //tilt vertically again when free to do so


             //steps 4xx: Place B-side sequence on player
             Step400_PlaceB_StepperPlaceTopPos,     //move stepper to 40 to allow tilt for B-side
             step410_PlaceB_TiltPlaceHorizontal,    //tilt horizontal for B-side (around 7deg)
             Step420_PlaceB_RotArmPlaceIn,          //rotate arm in to place position (should be the same for all picks and places)
             Step430_PlaceB_StepperPlaceBottomPos,  //stepper to height where LP is just within center pin
             Step440_PlaceB_RotArmShakeOnPin,       //Special move to shake rotation around 50-52-ish to account for inaccuracy and make sure LP is on Pin
             Step450_PlaceB_ClampOpen,
             Step460_PlaceB_RotArmPlaceRelease,   //rotate arm out to 20 to release LP onto pin
             Step470_PlaceB_StepperPlaceTopPos,   //move stepper up to allow tilt back
             Step480_PlaceB_RotArmPlaceOutwards,  //rotate out to 130
             Step490_PlaceB_TiltPlaceVertical,    //tilt vertically again when free to do so


             //steps 5xx: Pick sequence from rack
             Step500_PickRack_StepperPickRackPos,     //stepper to position
             Step510_PickRack_TiltHorizontal,         //tilt vertcially A side (B side not possible on storage)
             step520_PickRack_RotateInPos1,           //rotate in, touch with inner side of arm
             Step530_PickRack_StepperPickPos2,        //stepper up 5mm, line up outer arm with lp
             Step540_PickRack_RotateIn,               //rotate in completely for storage
             Step550_PickRack_CloseClamp,             //Close clamp
             Step560_PickRack_StepperPickTop,         //stepper up 15/17mm (max 18mm with current spacing). to clear pin
             Step570_PickRack_RotateOut,              //rotate out to front
             Step580_PickRack_StepperToClearancePos,  //optionally, move stepper down before moving tilt vertical
             Step585_PickRack_TiltVertical,           //tilt vertical, ONLY POSSIBLE WHEN HEIGHT > 420!


             //steps 6xx: Place sequence on rack
             Step600_PlaceRack_StepperPlaceTopPos,  //stepper to top place position
             Step610_PlaceRack_TiltHorizontally,    //tilt horizontally A side (B side not possible for storage)
             Step620_PlaceRack_RotateIn,            //rotate in completely so hole of lp lines up with pin
             Step630_PlaceRack_StepperBottomPos,    //stepper down to place bottom pos
             Step635_PlaceRack_RotArmShakeOnPin,
             Step640_PlaceRack_OpenClamp,              //release LP from clamp
             Step650_PlaceRack_RotateOut,              //rotate out to front
             Step660_PlaceRack_StepperToClearancePos,  //optionally, move stepper down before moving tilt vertical
             Step665_PlaceRack_TiltVertically,         //tilt vertical, ONLY POSSIBLE WHEN HEIGHT > 420!


             //rotArmClearLPPos,       //position to clear the LP, can't go all the way out because of clamp arm
             stopCommand,  //from either JukePi or player end sensor
             startPlaying,
             stopPlaying
};

steps CaseStep = startup, prevStep = startup;
bool transit, next, finish, StepperBelowPlayer = false;

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


//eeprom, stored data
byte RotStoredPos, TiltStoredPos, ClampStoredPos, ArmHeightStoredPos, ArmPosStoredPos = 0;
byte RackPosition = 0;  //which position in the rack we want to pick/place
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
      RotStoredPos = RotationOutFront;  //store reference pos
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
    if (message.equalsIgnoreCase("demoPickA")) {
      CaseStep = Step100_PickA_StepperPickBottomPos;
    } else if (message.equalsIgnoreCase("demoPickB")) {
      CaseStep = Step200_PickB_StepperPickTopPos;
    } else if (message.equalsIgnoreCase("demoPlaceA")) {
      CaseStep = Step300_PlaceA_stepperPlaceTopPos;
    } else if (message.equalsIgnoreCase("demoPlaceB")) {
      CaseStep = Step400_PlaceB_StepperPlaceTopPos;
    } else if (message.equalsIgnoreCase("demoPickRack")) {
      CaseStep = Step500_PickRack_StepperPickRackPos;
    } else if (message.equalsIgnoreCase("demoPlaceRack")) {
      CaseStep = Step600_PlaceRack_StepperPlaceTopPos;
    }

    if (message.equalsIgnoreCase("next")) {
      next = true;
    } else if (message.equalsIgnoreCase("finish")) {
      next = true;
      finish = true;
    }

    if (CaseStep == Step500_PickRack_StepperPickRackPos         //step500, wait for rack pick position
        || CaseStep == Step600_PlaceRack_StepperPlaceTopPos) {  //step600, wait for rack place position
      if (message.toInt() > 0 && message.toInt() < 10) {
        RackPosition = message.toInt();  //set Rack position, reset after 5xx and 6xx sequence
      }
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
      Serial.println("invalid message");
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
      finish = false;
      //read serial and determine next step there
      break;


//Pick steps from player
#pragma region pickA from player


    //move stepper to bottom position of pick pos. this is the actual height of the LP
    case Step100_PickA_StepperPickBottomPos:
      if (transit) {
        Serial.println("stepperPickBottomPos");
      }
      stepper.moveTo(StepperPos(LPPositions[1]));
      if (stepper.distanceToGo() == 0) {
        if (next) {
          CaseStep = Step110_PickA_TiltArmHorizontal;
        }
      }
      break;


    //tilt gripper arm horizontal, ready to pick up or place down
    case Step110_PickA_TiltArmHorizontal:
      if (transit) {
        Serial.println("tiltArmHorizontal");
      }
      MoveTiltServo(TiltHorizontalA, 2);
      if (next) {
        CaseStep = Step120_PickA_RotArmPickPos1;
      }
      break;



    //rotate arm to inwards position for pick. only inner side will allign vertically because of flex in arm
    case Step120_PickA_RotArmPickPos1:
      if (transit) {
        Serial.println("rotArmPickPlacePos");
      }
      MoveRotationServo(RotationPos1A, 2);
      if (next) {
        CaseStep = Step130_PickA_StepperPickIntermediatePos;
      }
      break;



    //move stepper to intermediate position of pick pos, alligns outer side of arm with LP
    case Step130_PickA_StepperPickIntermediatePos:
      if (transit) {
        Serial.println("stepperPickIntermediatePos");
      }
      stepper.moveTo(StepperPos(LPPositions[2]));
      if (stepper.distanceToGo() == 0) {
        if (next) {
          CaseStep = Step135_PickA_RotArmPickPos2;
        }
      }
      break;



    //Rotate arm further in to also allow outer side of arm to grip LP
    case Step135_PickA_RotArmPickPos2:
      if (transit) {
        Serial.println("rotArmPickPos2");
      }
      MoveRotationServo(RotationPos2A, 1);
      if (next) {
        CaseStep = Step140_PickA_StepperPickPos2;
      }
      break;



    //Lift LP slightly, but still on center Pin
    case Step140_PickA_StepperPickPos2:
      if (transit) {
        Serial.println("stepperPickPos2");
      }
      stepper.moveTo(StepperPos(LPPositions[3]));
      if (stepper.distanceToGo() == 0) {
        if (next) {
          CaseStep = Step150_PickA_RotArmPickIn;
        }
      }
      break;



    //completely move rotation IN
    case Step150_PickA_RotArmPickIn:
      if (transit) {
        Serial.println("rotArmPickPos2");
      }
      MoveRotationServo(RotationIn, 1);
      if (next) {
        CaseStep = Step160_PickA_CloseClamp;
      }
      break;



    //Close the clamp to hold LP in place
    case Step160_PickA_CloseClamp:
      if (transit) {
        Serial.println("clampIn");
      }
      MoveClampServo(ClampClose, 1);
      isHoldingLP = true;
      if (next) {
        CaseStep = Step170_PickA_StepperPickTopPos;
      }
      break;



    //move stepper to top position of pick pos
    case Step170_PickA_StepperPickTopPos:
      if (transit) {
        Serial.println("stepperPickTopPos");
      }
      stepper.moveTo(StepperPos(LPPositions[0]));
      if (stepper.distanceToGo() == 0) {
        if (next) {
          CaseStep = Step180_PickA_TiltPickSlightlyVertical;
        }
      }
      break;



    //move tilt slightly vertical to hold LP better before rotating outwards
    case Step180_PickA_TiltPickSlightlyVertical:
      if (transit) {
        Serial.println("stepperPickTopPos");
      }
      MoveTiltServo(TiltSlightlyVerticalA, 1);
      if (next) {
        CaseStep = Step190_PickA_RotArmPickOutwards;
      }
      break;



    //move rotation OUT
    case Step190_PickA_RotArmPickOutwards:
      if (transit) {
        Serial.println("rotArmPickOutwards");
      }
      MoveRotationServo(RotationOutFront, 1);
      if (next) {
        CaseStep = Step195_PickA_TiltPickVertical;
      }
      break;



    //put tilt vertical to allow stepper to move
    case Step195_PickA_TiltPickVertical:
      if (transit) {
        Serial.println("tiltPickVertical");
      }
      MoveTiltServo(TiltVertical, 1);
      CaseStep = jukePiCommand;

      break;


#pragma endregion end of PickA sequence from player


#pragma region PickB from player

    //stepper up to where we can tilt horizontally to B side, needs more space because bend moves down
    case Step200_PickB_StepperPickTopPos:
      if (transit) {
        Serial.println("stepperPickTopPos");
      }
      stepper.moveTo(StepperPos(LPPositions[4]));
      if (stepper.distanceToGo() == 0) {
        if (next) {
          CaseStep = Step210_PickB_TiltArmHorizontal;
        }
      }
      break;

    //tilt arm to B side
    case Step210_PickB_TiltArmHorizontal:
      if (transit) {
        Serial.println("tiltPickhorizontal");
      }
      MoveTiltServo(TiltHorizontalB, 2);
      if (next) {
        CaseStep = Step220_PickB_RotArmPickOut;
      }
      break;


    //rotate arm to outside pos for B side
    case Step220_PickB_RotArmPickOut:
      if (transit) {
        Serial.println("rotArmPickPos1");
      }
      MoveRotationServo(RotationOutB, 2);
      if (next) {
        CaseStep = Step230_PickB_StepperBottomPos;
      }
      break;


    //move stepper down so inner arm is level with LP
    case Step230_PickB_StepperBottomPos:
      if (transit) {
        Serial.println("stepperPickBottomPos");
      }
      stepper.moveTo(StepperPos(LPPositions[1]));
      Serial.println(stepper.distanceToGo());
      if (stepper.distanceToGo() == 0) {
        if (next) {
          CaseStep = Step240_PickB_RotArmPickPos1;
        }
      }
      break;


    //rotate arm inwards so LP gets lifted slightly by inner arm
    case Step240_PickB_RotArmPickPos1:
      if (transit) {
        Serial.println("rotArmPickPos1");
      }
      MoveRotationServo(RotationPos1B, 1);
      if (next) {
        CaseStep = Step250_PickB_StepperIntermediatePos;
      }
      break;


    //rotate a bit inwards to lift LP a bit and level outside of arm with LP
    case Step250_PickB_StepperIntermediatePos:
      if (transit) {
        Serial.println("stepperPickIntermediatePos");
      }
      stepper.moveTo(StepperPos(LPPositions[2]));
      if (stepper.distanceToGo() == 0) {
        if (next) {
          CaseStep = Step260_PickB_RotArmPickPos2;
        }
      }
      break;


    //rotate arm inwards to outer arm reaches LP
    case Step260_PickB_RotArmPickPos2:
      if (transit) {
        Serial.println("rotArmPickPos2");
      }
      MoveRotationServo(RotationPos2B, 1);
      if (next) {
        CaseStep = Step270_PickB_StepperPickPos2;
      }
      break;


    //move stepper up to lift LP, but stay on center pin
    case Step270_PickB_StepperPickPos2:
      if (transit) {
        Serial.println("stepperPickPos2");
      }
      stepper.moveTo(StepperPos(LPPositions[3]));
      if (stepper.distanceToGo() == 0) {
        if (next) {
          CaseStep = Step280_PickB_RotArmPickIn;
        }
      }
      break;


    //completely move rotation IN
    case Step280_PickB_RotArmPickIn:
      if (transit) {
        Serial.println("rotArmPickPos2");
      }
      MoveRotationServo(RotationIn, 1);
      if (next) {
        CaseStep = Step290_PickB_CloseClamp;
      }
      break;


    //close clamp to hold LP
    case Step290_PickB_CloseClamp:
      if (transit) {
        Serial.println("clampIn");
      }
      MoveClampServo(ClampClose, 2);
      isHoldingLP = true;
      if (next) {
        CaseStep = Step291_PickB_StepperPickTopPos;
      }
      break;


    //move stepper up completely
    case Step291_PickB_StepperPickTopPos:
      if (transit) {
        Serial.println("stepperPickTopPos");
      }
      stepper.moveTo(StepperPos(LPPositions[4]));
      if (stepper.distanceToGo() == 0) {
        if (next) {
          CaseStep = Step292_PickB_TiltPickSlightlyVertical;
        }
      }
      break;


    //slightly tilt before rotating to make sure we're holding the LP better
    case Step292_PickB_TiltPickSlightlyVertical:
      if (transit) {
        Serial.println("stepperPickTopPos");
      }
      MoveTiltServo(TiltSlightlyVerticalB, 1);
      if (next) {
        CaseStep = Step293_PickB_RotArmPickOutwards;
      }
      break;


    //rotate fully forward
    case Step293_PickB_RotArmPickOutwards:
      if (transit) {
        Serial.println("rotArmPickOutwards");
      }
      MoveRotationServo(RotationOutFront, 1);
      if (next) {
        CaseStep = Step295_PickB_TiltPickVertical;
      }
      break;


    case Step295_PickB_TiltPickVertical:
      if (transit) {
        Serial.println("tiltPickVertical");
      }
      MoveTiltServo(TiltVertical, 2);
      CaseStep = jukePiCommand;
      break;


#pragma endregion end of PickB sequence from player


#pragma region Place sequence A side


    //move stepper to the top of the place position
    case Step300_PlaceA_stepperPlaceTopPos:
      if (transit) {
        Serial.println("stepperPlaceTopPos");
      }
      stepper.moveTo(StepperPos(LPPositions[0]));
      if (stepper.distanceToGo() == 0) {
        if (next) {
          CaseStep = Step310_PlaceA_TiltPlaceHorizontal;
        }
      }
      break;


    case Step310_PlaceA_TiltPlaceHorizontal:
      if (transit) {
        Serial.println("tiltPlaceAHorizontal");
      }
      MoveTiltServo(TiltHorizontalA, 1);
      if (next) {
        CaseStep = Step320_PlaceA_RotArmPlaceIn;
      }
      break;


    case Step320_PlaceA_RotArmPlaceIn:
      if (transit) {
        Serial.println("rotArmPlaceIn");
      }
      MoveRotationServo(RotationIn, 1);
      if (next) {
        CaseStep = Step330_PlaceA_StepperPlaceCenterPin;
      }
      break;


    case Step330_PlaceA_StepperPlaceCenterPin:
      if (transit) {
        Serial.println("StepperPlaceCenterPin");
      }
      stepper.moveTo(StepperPos(LPPositions[2]));
      if (stepper.distanceToGo() == 0) {
        if (next) {
          CaseStep = Step340_PlaceA_RotArmShakeOnPin;
        }
      }
      break;


    case Step340_PlaceA_RotArmShakeOnPin:
      if (transit) {
        Serial.println("RotShakeOnPin");
      }
      ShakeRotation(RotationIn);
      if (next) {
        //CaseStep = Step350_PlaceA_StepperPlaceBottomPos;
        CaseStep = Step360_PlaceA_ClampOpen;
      }
      break;


    //move stepper to the bottom of the place position. LP should now rest on player or storage
    case Step350_PlaceA_StepperPlaceBottomPos:
      if (transit) {
        Serial.println("StepperPlaceBottomPos");
      }
      stepper.moveTo(StepperPos(LPPositions[1]));
      if (stepper.distanceToGo() == 0) {
        if (next) {
          CaseStep = Step360_PlaceA_ClampOpen;
        }
      }
      //statement
      break;


    case Step360_PlaceA_ClampOpen:
      if (transit) {
        Serial.println("ClampOpen");
      }
      MoveClampServo(ClampOpen, 2);
      isHoldingLP = false;
      if (next) {
        CaseStep = Step370_PlaceA_RotArmPlaceOutwards;
      }
      break;


    case Step370_PlaceA_RotArmPlaceOutwards:
      if (transit) {
        Serial.println("RotArmPlaceOutwards");
      }
      MoveRotationServo(RotationOutFront, 1);
      if (next) {
        CaseStep = Step380_PlaceA_TiltPlaceVertical;
      }
      break;


    case Step380_PlaceA_TiltPlaceVertical:
      if (transit) {
        Serial.println("TiltPlaceVertical");
      }
      MoveTiltServo(TiltVertical, 2);
      CaseStep = jukePiCommand;
      break;


#pragma endregion end region place A side on player


#pragma region Place sequence B side

    case Step400_PlaceB_StepperPlaceTopPos:
      if (transit) {
        Serial.println("stepperPlaceTopPos");
      }
      stepper.moveTo(StepperPos(LPPositions[4]));
      if (stepper.distanceToGo() == 0) {
        if (next) {
          CaseStep = step410_PlaceB_TiltPlaceHorizontal;
        }
      }
      break;


    case step410_PlaceB_TiltPlaceHorizontal:
      if (transit) {
        Serial.println("TiltPlaceHorizontalB");
      }
      MoveTiltServo(TiltHorizontalB, 1);
      if (next) {
        CaseStep = Step420_PlaceB_RotArmPlaceIn;
      }
      break;


    case Step420_PlaceB_RotArmPlaceIn:
      if (transit) {
        Serial.println("RotArmPlaceBIn");
      }
      MoveRotationServo(RotationIn, 1);
      if (next) {
        CaseStep = Step430_PlaceB_StepperPlaceBottomPos;
      }
      break;


    case Step430_PlaceB_StepperPlaceBottomPos:
      if (transit) {
        Serial.println("StepperPlaceBottomPos");
      }
      stepper.moveTo(StepperPos(LPPositions[2]));
      if (stepper.distanceToGo() == 0) {
        if (next) {
          CaseStep = Step440_PlaceB_RotArmShakeOnPin;
        }
      }
      break;


    case Step440_PlaceB_RotArmShakeOnPin:
      if (transit) {
        Serial.println("RotArmShakeOnPin");
      }
      ShakeRotation(RotationIn);
      if (next) {
        CaseStep = Step450_PlaceB_ClampOpen;
      }
      break;


    case Step450_PlaceB_ClampOpen:
      if (transit) {
        Serial.println("ClampOpen");
      }
      MoveClampServo(ClampOpen, 2);
      isHoldingLP = false;
      if (next) {
        CaseStep = Step460_PlaceB_RotArmPlaceRelease;
      }
      break;


    case Step460_PlaceB_RotArmPlaceRelease:
      if (transit) {
        Serial.println("RotArmPlaceRelease");
      }
      MoveRotationServo(RotationOutB, 1);
      if (next) {
        CaseStep = Step470_PlaceB_StepperPlaceTopPos;
      }
      break;


    case Step470_PlaceB_StepperPlaceTopPos:
      if (transit) {
        Serial.println("StepperPlaceBTopPos");
      }
      stepper.moveTo(StepperPos(LPPositions[4]));
      if (stepper.distanceToGo() == 0) {
        if (next) {
          CaseStep = Step480_PlaceB_RotArmPlaceOutwards;
        }
      }
      break;


    case Step480_PlaceB_RotArmPlaceOutwards:
      if (transit) {
        Serial.println("RotArmPlaceBFront");
      }
      MoveRotationServo(RotationOutFront, 2);
      if (next) {
        CaseStep = Step490_PlaceB_TiltPlaceVertical;
      }
      break;


    case Step490_PlaceB_TiltPlaceVertical:
      if (transit) {
        Serial.println("TiltPlaceBVerticalFront");
      }
      MoveTiltServo(TiltVertical, 2);
      CaseStep = jukePiCommand;
      break;



#pragma endregion end region place B side on player


#pragma region pick sequence from rack


    case Step500_PickRack_StepperPickRackPos:
      if (transit) {
        Serial.println("StepperPickRackPos");
      }
      if (RackPosition == 0) {
        Serial.println("choose a storage position");
      }
      if (RackPosition != 0) {
        stepper.moveTo(StepperPos(StoragePositions[RackPosition]));
        if (stepper.distanceToGo() == 0) {
          CaseStep = Step510_PickRack_TiltHorizontal;
        }
      }
      break;


    case Step510_PickRack_TiltHorizontal:
      if (transit) {
        Serial.println("TiltHorizontal");
      }
      MoveTiltServo(TiltHorizontalA, 2);  //'A' side, open arm side towards rack
      CaseStep = Step540_PickRack_RotateIn;
      //CaseStep = step520_PickRack_RotateInPos1;
      break;

      /*
    case step520_PickRack_RotateInPos1:
      if (transit) {
        Serial.println("RotateInPos1");
      }
      MoveRotationServo(RotationIn, 2);  //pos 1 where inner arm touches LP is same angle as middle of player
      if (next) {
        CaseStep = Step530_PickRack_StepperPickPos2;
      }
      break;


    case Step530_PickRack_StepperPickPos2:
      if (transit) {
        Serial.println("StepperPickPos2");
      }
      stepper.moveTo(StepperPos(StoragePositions[RackPosition] - 5));  //move set 5mm up to line up outer arm with LP
      if (stepper.distanceToGo() == 0) {
        if (next) {
          CaseStep = Step540_PickRack_RotateIn;
        }
      }
      break;
*/

    case Step540_PickRack_RotateIn:
      if (transit) {
        Serial.println("RotateIn");
      }
      MoveRotationServo(RotationPos1B, 2);  //rotation Pos1 B-side is same as angle of pin in rack
      CaseStep = Step550_PickRack_CloseClamp;
      break;


    case Step550_PickRack_CloseClamp:
      if (transit) {
        Serial.println("CloseClamp");
      }
      MoveClampServo(ClampClose, 2);
      CaseStep = Step560_PickRack_StepperPickTop;
      break;


    case Step560_PickRack_StepperPickTop:
      if (transit) {
        Serial.println("StepperPickTop");
      }
      stepper.moveTo(StepperPos(StoragePositions[RackPosition] - StoragePositions[0]));  //move 15mm up to clear pin
      if (stepper.distanceToGo() == 0) {
        CaseStep = Step570_PickRack_RotateOut;
      }
      break;


    case Step570_PickRack_RotateOut:
      if (transit) {
        Serial.println("RotateOut");
      }
      MoveRotationServo(RotationOutFront, 2);
      CaseStep = Step580_PickRack_StepperToClearancePos;
      break;


    case Step580_PickRack_StepperToClearancePos:
      if (transit) {
        Serial.println("StepperClearancePos");
        Serial.println(Steppermm(stepper.currentPosition()));
      }
      if (Steppermm(stepper.currentPosition()) <= 420) {
        stepper.moveTo(StepperPos(425));  //move 5mm below clearance pos to freely tilt (possibly won't reach 425 as it will stop at 420)
        if (stepper.distanceToGo() == 0) {
          CaseStep = Step585_PickRack_TiltVertical;
        }
      } else {
        CaseStep = Step585_PickRack_TiltVertical;
      }
      break;


    case Step585_PickRack_TiltVertical:
      if (transit) {
        Serial.println("TiltVertical");
      }
      MoveTiltServo(TiltVertical, 2);
      RackPosition = 0;  //reset rack position to be used for other pick/place sequence
      CaseStep = jukePiCommand;
      break;


#pragma endregion end region pick sequence from rack



#pragma region place sequence on rack

    case Step600_PlaceRack_StepperPlaceTopPos:
      if (transit) {
        Serial.println("StepperPlaceTopPos");
      }
      if (RackPosition == 0) {
        Serial.println("choose a storage position");
      }
      if (RackPosition != 0) {
        if ((StoragePositions[RackPosition] - StoragePositions[0]) > 420) {                  //CAN ONLY DO THIS ON 420 OR PHYSICALLY LOWER WHEN TILT IS VERTICAL!
          stepper.moveTo(StepperPos(StoragePositions[RackPosition] - StoragePositions[0]));  //stepper to set position of rack, but with offset to top
          if (stepper.distanceToGo() == 0) {
            CaseStep = Step610_PlaceRack_TiltHorizontally;
          }
        }else{
          //move to 420 or lower?
        }
      }
      break;


    case Step610_PlaceRack_TiltHorizontally:
      if (transit) {
        Serial.println("tiltHorizontal");
      }
      MoveTiltServo(TiltHorizontalA, 2);
      CaseStep = Step620_PlaceRack_RotateIn;
      break;


    case Step620_PlaceRack_RotateIn:
      if (transit) {
        Serial.println("RotateIn");
      }
      MoveRotationServo(RotationPos1B, 2);
      CaseStep = Step630_PlaceRack_StepperBottomPos;
      break;


    case Step630_PlaceRack_StepperBottomPos:
      if (transit) {
        Serial.println("StepperBottomPos");
      }
      stepper.moveTo(StepperPos(StoragePositions[RackPosition]));  //stepper to set position of rack
      if (stepper.distanceToGo() == 0) {
        CaseStep = Step635_PlaceRack_RotArmShakeOnPin;
      }
      break;

    case Step635_PlaceRack_RotArmShakeOnPin:
      if (transit) {
        Serial.println("RotArmShakeOnPin");
      }
      ShakeRotation(RotationPos1B);
      CaseStep = Step640_PlaceRack_OpenClamp;
      break;


    case Step640_PlaceRack_OpenClamp:
      if (transit) {
        Serial.println("OpenClamp");
      }
      MoveClampServo(ClampOpen, 2);
      CaseStep = Step650_PlaceRack_RotateOut;
      break;


    case Step650_PlaceRack_RotateOut:
      if (transit) {
        Serial.println("RotateOut");
      }
      MoveRotationServo(RotationOutFront, 2);
      CaseStep = Step660_PlaceRack_StepperToClearancePos;
      break;


    case Step660_PlaceRack_StepperToClearancePos:
      if (transit) {
        Serial.println("StepperClearancePos");
        Serial.println(stepper.currentPosition());
      }
      if (Steppermm(stepper.currentPosition()) <= 420) {
        stepper.moveTo(StepperPos(425));  //move 5mm below clearance pos to freely tilt
        if (stepper.distanceToGo() == 0) {
          CaseStep = Step665_PlaceRack_TiltVertically;
        }
      } else {
        CaseStep = Step665_PlaceRack_TiltVertically;
      }
      break;


    case Step665_PlaceRack_TiltVertically:
      if (transit) {
        Serial.println("TiltVertically");
      }
      MoveTiltServo(TiltVertical, 2);
      RackPosition = 0;  //reset rack position for next pick/place sequence
      CaseStep = jukePiCommand;
      break;


#pragma endregion end region place sequence on rack




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
    if (!finish) {
      next = false;
    }
    prevStep = CaseStep;
  } else {
    transit = false;
  }



  //store stepper position in EEPROM when switches from above player to below player
  if (stepper.currentPosition() < 325 && !StepperBelowPlayer) {
    EEPROM.update(StepperEEPROMAddress, 1);
    StepperBelowPlayer = true;
  } else if (stepper.currentPosition() >= 325 && StepperBelowPlayer) {
    EEPROM.update(StepperEEPROMAddress, 0);
    StepperBelowPlayer = false;
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

int Steppermm(int pos) {
  return pos / stepsPermm;
}

void HomeStepper() {
  //int Counter = 0;
  prevMillis = millis();
  while (digitalRead(iEndStop) && millis() - prevMillis < 20000) {  //timeout of 20s
    //keep running up
    Serial.println("stepper up");
    stepper.runSpeed();
    //Counter += 1;
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
  bool stepperHomed = false;
  if (RotStoredPos < 125 && EEPROM.read(StepperEEPROMAddress) == 0) {
    //if rotation is inwards and arm is not below player, home stepper first before we can move tilt
    HomeStepper();
    stepper.setSpeed(-250);  //lower speed
    HomeStepper();
    stepperHomed = true;
  }

  MoveArmHeightServo(DOWN, 1);
  //toneArmHeight.write(DOWN);    //DOWN
  MoveArmPosServo(BASE, 1);
  //toneArmPos.write(BASE);     //base position
  MoveClampServo(ClampOpen, 1);
  //Clamp.write(ClampOpen);       //loosen the clamp
  MoveRotationServo(RotationOutFront, 1);
  //Rotation.write(RotationOut);  //~ middle
  //delay(250);                   //wait for it to be outwards at least a bit, NOT NEEDED WITH NEW FUNCTIONS
  MoveTiltServo(TiltVertical, 1);
  //Tilt.write(TiltVertical);     //vertical


  //Clamp.detach();
  //Rotation.detach();
  //Tilt.detach();
  if (!stepperHomed) {
    //home stepper
    HomeStepper();

    stepper.setSpeed(-250);  //lower speed
    HomeStepper();
  }
}
