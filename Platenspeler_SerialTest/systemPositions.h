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



enum toneHeight{
  DOWN = 0,
  UP = 65
} toneArmHeightEnum;

enum tonePos{
  BASE = 180,
  START = 100,
  END = 0
}toneArmPosEnum;

