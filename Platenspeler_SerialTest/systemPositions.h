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
  UP = 40
} toneArmHeightEnum;

enum tonePos {
  BASE = 170,
  START = 105,
  END = 0
} toneArmPosEnum;


enum RotationPos {
  //determine correct angles for Rotation
  RotationIn = 50,    //rotation where center of LP is on center of stack/player
  RotationOut = 120,  //rotation far enough out to freely tilt
  RotationPos1 = 65,  //rotation where inside is just around LP
  RotationPos2 = 55  //rotation where tip is just around LP
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
  120,  //[0] player top pos
  400,  //[1] storage pos 1
  440,  //[2] storage pos 2
  480,  //[3] storage pos 3
  520,  //[4] storage pos 4
  560,  //[5] storage pos 5
  600,  //[6] storage pos 6
  640,  //[7] storage pos 7
  170,   //[8] player bottom pos
  165,   //[9] player lift pos1
  155,   //[10] player lift pos 2

};


/*
pick sequence positions:
xs (stepper) ~170, height to start reach in to pick from player
xt tilt 135
xr rotate in to ~65
xs stepper up to 165 to lift LP slightly on arm base side
xr rotate in to 56
xs stepper up to 155
xr rotate in to ~50
xc clamp 40, dicht
xs move stepper up (to 120 should be fine)
xt tilt naar 125 om lp better vast te houden
xr rotate 120
xt tilt 75



*/

//stepper max pos before bottoming out
int maxPos = 650;