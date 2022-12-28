//import libs
#include <MultiStepper.h>
#include <AccelStepper.h>
#include <Servo.h>


//platenspeler:
#define RPM33 2
#define RPM45 3
#define STOP 4
#define RLYOn 12
#define iLDR A7

//stepper pins
#define iEndStop 8
#define stepPin 5
#define dirPin 6

//servo pins
#define rot 9
#define clamp 10
#define tilt 11

//create classes
AccelStepper stepper = AccelStepper(1, stepPin, dirPin);
Servo Clamp;
Servo Tilt;
Servo Rotation;


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
  pinMode(LED_BUILTIN, OUTPUT);

  Clamp.attach(clamp);
  Tilt.attach(tilt);
  Rotation.attach(rot);

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
      digitalWrite(RPM33, HIGH);
      Serial.println("start 33 RPM rotation");
      commandActivated = millis();
    } else if (message.equalsIgnoreCase("45")) {
      //45 RPM mode
      digitalWrite(RPM45, HIGH);
      Serial.println("start 45 RPM rotation");
      commandActivated = millis();
    } else if (message.equalsIgnoreCase("STOP")) {
      //stop rotation
      digitalWrite(STOP, HIGH);
      Serial.println("stop any rotation");
      commandActivated = millis();
    } else if (message.equalsIgnoreCase("togglepower")) {
      //toggle relay on or off
      powerStatus = !powerStatus;
      digitalWrite(RLYOn, powerStatus);
      analogReadActive = false;
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
    }

    digitalWrite(13, donePlaying);
    //Serial.println(analogRead(iLDR));
  }




  //leave this line last in loop
  prevMillis = millis();
}


int StepperPos(int pos) {
  return pos * stepsPermm;
}

void HomeStepper() {
  while (digitalRead(iEndStop)) {
    //keep running up
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
  Clamp.write(90);     //loosen the clamp
  Rotation.write(97);  //~ middle
  delay(250);          //wait for it to be outwards at least a bit
  Tilt.write(98);      //vertical


  //home stepper
  HomeStepper();

  stepper.setSpeed(-250);  //lower speed
  HomeStepper();
}




/*
 * Servo S-curve:
 * Sigmoid function
 * S(x)=A/(1+e^(-Bx+C))
 * A = bereik curve, 0->A
 * B = slope
 * C = shift links/rechts (positief naar rechts)
 * 
 * 
 */
