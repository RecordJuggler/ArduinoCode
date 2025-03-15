//toneArmPos; //150: full left (home), ~107: startpos plate, ~0: endpos plate
//toneArmHeight; //0: down, 60: up

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


//Clamp Min Max:
//in: 40, clamp closed
//Out: 140, clamp open


enum toneHeight {
  DOWN = 0,
  UP = 30
} toneArmHeightEnum;

enum tonePos {
  BASE = 170,
  START = 105,
  END = 0
} toneArmPosEnum;


enum RotationPos {
  //determine correct angles for Rotation
  RotationIn = 35,
  RotationOut = 120
} RotationArmPosEnum;


enum ClampPos {
  ClampClose = 40,
  ClampOpen = 140
} ClampPosEnum;
/*tilt:
5: horizontal, opening outwards
135: horizontal, opening inwards (towards record player), 130 puts LP slightly higher
70: vertical, opening up
75: probably save to move stepper up/down
*/
enum TiltPos {
  TiltHorizontal = 135,
  TiltVertical = 75
} TiltArmPosEnum;


int LPPositions[] = {
  120,  //[0] player  top pos
  400,  //[1] storage pos 1
  440,  //[2] storage pos 2
  480,  //[3] storage pos 3
  520,  //[4] storage pos 4
  560,  //[5] storage pos 5
  600,  //[6] storage pos 6
  640,  //[7] storage pos 7
  130   //[8] player bottom pos
};


/*
pick sequence positions:
~163, height to start reach in to pick from player
rotate in to ~70
stepper up to 155 to lift LP slightly on arm base side
rotate in to 57
stepper up to 150
rotate in to ~50
clamp 40, dicht
move stepper up (to 120 should be fine)
stepper op 120, kan tilt naar 125 om lp better vast te houden
rotate 120
tilt 75



*/


int maxPos = 650;