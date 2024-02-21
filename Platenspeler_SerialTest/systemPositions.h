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



enum toneHeight {
  DOWN = 0,
  UP = 65
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

enum TiltPos {
  TiltHorizontal = 22,
  TiltVertical = 85
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



int maxPos = 650;